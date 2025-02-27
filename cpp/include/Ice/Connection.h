//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __Ice_Connection_h__
#define __Ice_Connection_h__

#include <IceUtil/PushDisableWarnings.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <optional>
#include <Ice/ObjectAdapterF.h>
#include <Ice/Identity.h>
#include <Ice/Endpoint.h>
#include <IceUtil/UndefSysMacros.h>

#ifndef ICE_API
#   if defined(ICE_STATIC_LIBS)
#       define ICE_API /**/
#   elif defined(ICE_API_EXPORTS)
#       define ICE_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace Ice
{

class ConnectionInfo;
class Connection;
class IPConnectionInfo;
class TCPConnectionInfo;
class UDPConnectionInfo;
class WSConnectionInfo;

}

namespace Ice
{

/**
 * The batch compression option when flushing queued batch requests.
 */
enum class CompressBatch : unsigned char
{
    /**
     * Compress the batch requests.
     */
    Yes,
    /**
     * Don't compress the batch requests.
     */
    No,
    /**
     * Compress the batch requests if at least one request was made on a compressed proxy.
     */
    BasedOnProxy
};

/**
 * Specifies the close semantics for Active Connection Management.
 */
enum class ACMClose : unsigned char
{
    /**
     * Disables automatic connection closure.
     */
    CloseOff,
    /**
     * Gracefully closes a connection that has been idle for the configured timeout period.
     */
    CloseOnIdle,
    /**
     * Forcefully closes a connection that has been idle for the configured timeout period, but only if the connection
     * has pending invocations.
     */
    CloseOnInvocation,
    /**
     * Combines the behaviors of CloseOnIdle and CloseOnInvocation.
     */
    CloseOnInvocationAndIdle,
    /**
     * Forcefully closes a connection that has been idle for the configured timeout period, regardless of whether the
     * connection has pending invocations or dispatch.
     */
    CloseOnIdleForceful
};

/**
 * Specifies the heartbeat semantics for Active Connection Management.
 */
enum class ACMHeartbeat : unsigned char
{
    /**
     * Disables heartbeats.
     */
    HeartbeatOff,
    /**
     * Send a heartbeat at regular intervals if the connection is idle and only if there are pending dispatch.
     */
    HeartbeatOnDispatch,
    /**
     * Send a heartbeat at regular intervals when the connection is idle.
     */
    HeartbeatOnIdle,
    /**
     * Send a heartbeat at regular intervals until the connection is closed.
     */
    HeartbeatAlways
};

/**
 * A collection of Active Connection Management configuration settings.
 * \headerfile Ice/Ice.h
 */
struct ACM
{
    /**
     * A timeout value in seconds.
     */
    int timeout;
    /**
     * The close semantics.
     */
    ::Ice::ACMClose close;
    /**
     * The heartbeat semantics.
     */
    ::Ice::ACMHeartbeat heartbeat;

    /**
     * Obtains a tuple containing all of the struct's data members.
     * @return The data members in a tuple.
     */
    std::tuple<const int&, const ::Ice::ACMClose&, const ::Ice::ACMHeartbeat&> ice_tuple() const
    {
        return std::tie(timeout, close, heartbeat);
    }
};

/**
 * Determines the behavior when manually closing a connection.
 */
enum class ConnectionClose : unsigned char
{
    /**
     * Close the connection immediately without sending a close connection protocol message to the peer and waiting
     * for the peer to acknowledge it.
     */
    Forcefully,
    /**
     * Close the connection by notifying the peer but do not wait for pending outgoing invocations to complete. On the
     * server side, the connection will not be closed until all incoming invocations have completed.
     */
    Gracefully,
    /**
     * Wait for all pending invocations to complete before closing the connection.
     */
    GracefullyWithWait
};

/**
 * A collection of HTTP headers.
 */
using HeaderDict = ::std::map<::std::string, ::std::string>;

using Ice::operator<;
using Ice::operator<=;
using Ice::operator>;
using Ice::operator>=;
using Ice::operator==;
using Ice::operator!=;

}

namespace Ice
{

/**
 * Base class providing access to the connection details.
 * \headerfile Ice/Ice.h
 */
class ICE_CLASS(ICE_API) ConnectionInfo
{
public:

    ICE_MEMBER(ICE_API) virtual ~ConnectionInfo();

    ConnectionInfo() = default;

    ConnectionInfo(const ConnectionInfo&) = default;
    ConnectionInfo(ConnectionInfo&&) = default;
    ConnectionInfo& operator=(const ConnectionInfo&) = default;
    ConnectionInfo& operator=(ConnectionInfo&&) = default;

    /**
     * One-shot constructor to initialize all data members.
     * @param underlying The information of the underyling transport or null if there's no underlying transport.
     * @param incoming Whether or not the connection is an incoming or outgoing connection.
     * @param adapterName The name of the adapter associated with the connection.
     * @param connectionId The connection id.
     */
    ConnectionInfo(const ::std::shared_ptr<::Ice::ConnectionInfo>& underlying, bool incoming, const ::std::string& adapterName, const ::std::string& connectionId) :
        underlying(underlying),
        incoming(incoming),
        adapterName(adapterName),
        connectionId(connectionId)
    {
    }

    /**
     * The information of the underyling transport or null if there's no underlying transport.
     */
    ::std::shared_ptr<::Ice::ConnectionInfo> underlying;
    /**
     * Whether or not the connection is an incoming or outgoing connection.
     */
    bool incoming;
    /**
     * The name of the adapter associated with the connection.
     */
    ::std::string adapterName;
    /**
     * The connection id.
     */
    ::std::string connectionId;
};

/**
 * This method is called by the connection when the connection is closed. If the callback needs more information
 * about the closure, it can call {@link Connection#throwException}.
 * @param con The connection that closed.
 */
using CloseCallback = ::std::function<void(const ::std::shared_ptr<Connection>& con)>;

/**
 * This method is called by the connection when a heartbeat is received from the peer.
 * @param con The connection on which a heartbeat was received.
 */
using HeartbeatCallback = ::std::function<void(const ::std::shared_ptr<Connection>& con)>;

/**
 * The user-level interface to a connection.
 * \headerfile Ice/Ice.h
 */
class ICE_CLASS(ICE_API) Connection
{
public:

    ICE_MEMBER(ICE_API) virtual ~Connection();

    /**
     * Manually close the connection using the specified closure mode.
     * @param mode Determines how the connection will be closed.
     * @see ConnectionClose
     */
    virtual void close(ConnectionClose mode) noexcept = 0;

    /**
     * Create a special proxy that always uses this connection. This can be used for callbacks from a server to a
     * client if the server cannot directly establish a connection to the client, for example because of firewalls. In
     * this case, the server would create a proxy using an already established connection from the client.
     * @param id The identity for which a proxy is to be created.
     * @return A proxy that matches the given identity and uses this connection.
     * @see #setAdapter
     */
    virtual ObjectPrx createProxy(const Identity& id) const = 0;

    /**
     * Explicitly set an object adapter that dispatches requests that are received over this connection. A client can
     * invoke an operation on a server using a proxy, and then set an object adapter for the outgoing connection that
     * is used by the proxy in order to receive callbacks. This is useful if the server cannot establish a connection
     * back to the client, for example because of firewalls.
     * @param adapter The object adapter that should be used by this connection to dispatch requests. The object
     * adapter must be activated. When the object adapter is deactivated, it is automatically removed from the
     * connection. Attempts to use a deactivated object adapter raise {@link ObjectAdapterDeactivatedException}
     * @see #createProxy
     * @see #getAdapter
     */
    virtual void setAdapter(const ::std::shared_ptr<ObjectAdapter>& adapter) = 0;

    /**
     * Get the object adapter that dispatches requests for this connection.
     * @return The object adapter that dispatches requests for the connection, or null if no adapter is set.
     * @see #setAdapter
     */
    virtual ::std::shared_ptr<::Ice::ObjectAdapter> getAdapter() const noexcept = 0;

    /**
     * Get the endpoint from which the connection was created.
     * @return The endpoint from which the connection was created.
     */
    virtual ::std::shared_ptr<::Ice::Endpoint> getEndpoint() const noexcept = 0;

    /**
     * Flush any pending batch requests for this connection. This means all batch requests invoked on fixed proxies
     * associated with the connection.
     * @param compress Specifies whether or not the queued batch requests should be compressed before being sent over
     * the wire.
     */
    ICE_MEMBER(ICE_API) void flushBatchRequests(CompressBatch compress);

    /**
     * Flush any pending batch requests for this connection. This means all batch requests invoked on fixed proxies
     * associated with the connection.
     * @param compress Specifies whether or not the queued batch requests should be compressed before being sent over
     * the wire.
     * @param exception The exception callback.
     * @param sent The sent callback.
     * @return A function that can be called to cancel the invocation locally.
     */
    virtual ::std::function<void()>
    flushBatchRequestsAsync(CompressBatch compress,
                            ::std::function<void(::std::exception_ptr)> exception,
                            ::std::function<void(bool)> sent = nullptr) = 0;

    /**
     * Flush any pending batch requests for this connection. This means all batch requests invoked on fixed proxies
     * associated with the connection.
     * @param compress Specifies whether or not the queued batch requests should be compressed before being sent over
     * the wire.
     * @return The future object for the invocation.
     */
    ICE_MEMBER(ICE_API) std::future<void> flushBatchRequestsAsync(CompressBatch compress);

    /**
     * Set a close callback on the connection. The callback is called by the connection when it's closed. The callback
     * is called from the Ice thread pool associated with the connection. If the callback needs more information about
     * the closure, it can call {@link Connection#throwException}.
     * @param callback The close callback object.
     */
    virtual void setCloseCallback(CloseCallback callback) = 0;

    /**
     * Set a heartbeat callback on the connection. The callback is called by the connection when a heartbeat is
     * received. The callback is called from the Ice thread pool associated with the connection.
     * @param callback The heartbeat callback object.
     */
    virtual void setHeartbeatCallback(HeartbeatCallback callback) = 0;

    /**
     * Send a heartbeat message.
     */
    ICE_MEMBER(ICE_API) void heartbeat();

    /**
     * Send a heartbeat message.
     * @param exception The exception callback.
     * @param sent The sent callback.
     * @return A function that can be called to cancel the invocation locally.
     */
    virtual ::std::function<void()>
    heartbeatAsync(::std::function<void(::std::exception_ptr)> exception,
                   ::std::function<void(bool)> sent = nullptr) = 0;

    /**
     * Send a heartbeat message.
     * @return The future object for the invocation.
     */
    ICE_MEMBER(ICE_API) std::future<void> heartbeatAsync();

    /**
     * Set the active connection management parameters.
     * @param timeout The timeout value in seconds, must be &gt;= 0.
     * @param close The close condition
     * @param heartbeat The heartbeat condition
     */
    virtual void setACM(const std::optional<int>& timeout, const std::optional<ACMClose>& close, const std::optional<ACMHeartbeat>& heartbeat) = 0;

    /**
     * Get the ACM parameters.
     * @return The ACM parameters.
     */
    virtual ::Ice::ACM getACM() noexcept = 0;

    /**
     * Return the connection type. This corresponds to the endpoint type, i.e., "tcp", "udp", etc.
     * @return The type of the connection.
     */
    virtual ::std::string type() const noexcept = 0;

    /**
     * Get the timeout for the connection.
     * @return The connection's timeout.
     */
    virtual int timeout() const noexcept = 0;

    /**
     * Return a description of the connection as human readable text, suitable for logging or error messages.
     * @return The description of the connection as human readable text.
     */
    virtual ::std::string toString() const noexcept = 0;

    /**
     * Returns the connection information.
     * @return The connection information.
     */
    virtual ::std::shared_ptr<::Ice::ConnectionInfo> getInfo() const = 0;

    /**
     * Set the connection buffer receive/send size.
     * @param rcvSize The connection receive buffer size.
     * @param sndSize The connection send buffer size.
     */
    virtual void setBufferSize(int rcvSize, int sndSize) = 0;

    /**
     * Throw an exception indicating the reason for connection closure. For example,
     * {@link CloseConnectionException} is raised if the connection was closed gracefully, whereas
     * {@link ConnectionManuallyClosedException} is raised if the connection was manually closed by
     * the application. This operation does nothing if the connection is not yet closed.
     */
    virtual void throwException() const = 0;
};

/**
 * Provides access to the connection details of an IP connection
 * \headerfile Ice/Ice.h
 */
class ICE_CLASS(ICE_API) IPConnectionInfo : public ::Ice::ConnectionInfo
{
public:

    ICE_MEMBER(ICE_API) virtual ~IPConnectionInfo();

    IPConnectionInfo() :
        localAddress(""),
        localPort(-1),
        remoteAddress(""),
        remotePort(-1)
    {
    }

    IPConnectionInfo(const IPConnectionInfo&) = default;
    IPConnectionInfo(IPConnectionInfo&&) = default;
    IPConnectionInfo& operator=(const IPConnectionInfo&) = default;
    IPConnectionInfo& operator=(IPConnectionInfo&&) = default;

    /**
     * One-shot constructor to initialize all data members.
     * @param underlying The information of the underlying transport or null if there's no underlying transport.
     * @param incoming Whether or not the connection is an incoming or outgoing connection.
     * @param adapterName The name of the adapter associated with the connection.
     * @param connectionId The connection id.
     * @param localAddress The local address.
     * @param localPort The local port.
     * @param remoteAddress The remote address.
     * @param remotePort The remote port.
     */
    IPConnectionInfo(const ::std::shared_ptr<::Ice::ConnectionInfo>& underlying, bool incoming, const ::std::string& adapterName, const ::std::string& connectionId, const ::std::string& localAddress, int localPort, const ::std::string& remoteAddress, int remotePort) :
        ConnectionInfo(underlying, incoming, adapterName, connectionId),
        localAddress(localAddress),
        localPort(localPort),
        remoteAddress(remoteAddress),
        remotePort(remotePort)
    {
    }

    /**
     * The local address.
     */
    ::std::string localAddress;
    /**
     * The local port.
     */
    int localPort = -1;
    /**
     * The remote address.
     */
    ::std::string remoteAddress;
    /**
     * The remote port.
     */
    int remotePort = -1;
};

/**
 * Provides access to the connection details of a TCP connection
 * \headerfile Ice/Ice.h
 */
class ICE_CLASS(ICE_API) TCPConnectionInfo : public ::Ice::IPConnectionInfo
{
public:

    ICE_MEMBER(ICE_API) virtual ~TCPConnectionInfo();

    TCPConnectionInfo() :
        rcvSize(0),
        sndSize(0)
    {
    }

    TCPConnectionInfo(const TCPConnectionInfo&) = default;
    TCPConnectionInfo(TCPConnectionInfo&&) = default;
    TCPConnectionInfo& operator=(const TCPConnectionInfo&) = default;
    TCPConnectionInfo& operator=(TCPConnectionInfo&&) = default;

    /**
     * One-shot constructor to initialize all data members.
     * @param underlying The information of the underlying transport or null if there's no underlying transport.
     * @param incoming Whether or not the connection is an incoming or outgoing connection.
     * @param adapterName The name of the adapter associated with the connection.
     * @param connectionId The connection id.
     * @param localAddress The local address.
     * @param localPort The local port.
     * @param remoteAddress The remote address.
     * @param remotePort The remote port.
     * @param rcvSize The connection buffer receive size.
     * @param sndSize The connection buffer send size.
     */
    TCPConnectionInfo(const ::std::shared_ptr<::Ice::ConnectionInfo>& underlying, bool incoming, const ::std::string& adapterName, const ::std::string& connectionId, const ::std::string& localAddress, int localPort, const ::std::string& remoteAddress, int remotePort, int rcvSize, int sndSize) :
        IPConnectionInfo(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort),
        rcvSize(rcvSize),
        sndSize(sndSize)
    {
    }

    /**
     * The connection buffer receive size.
     */
    int rcvSize = 0;
    /**
     * The connection buffer send size.
     */
    int sndSize = 0;
};

/**
 * Provides access to the connection details of a UDP connection
 * \headerfile Ice/Ice.h
 */
class ICE_CLASS(ICE_API) UDPConnectionInfo : public ::Ice::IPConnectionInfo
{
public:

    ICE_MEMBER(ICE_API) virtual ~UDPConnectionInfo();

    UDPConnectionInfo() :
        mcastPort(-1),
        rcvSize(0),
        sndSize(0)
    {
    }

    UDPConnectionInfo(const UDPConnectionInfo&) = default;
    UDPConnectionInfo(UDPConnectionInfo&&) = default;
    UDPConnectionInfo& operator=(const UDPConnectionInfo&) = default;
    UDPConnectionInfo& operator=(UDPConnectionInfo&&) = default;

    /**
     * One-shot constructor to initialize all data members.
     * @param underlying The information of the underyling transport or null if there's no underlying transport.
     * @param incoming Whether or not the connection is an incoming or outgoing connection.
     * @param adapterName The name of the adapter associated with the connection.
     * @param connectionId The connection id.
     * @param localAddress The local address.
     * @param localPort The local port.
     * @param remoteAddress The remote address.
     * @param remotePort The remote port.
     * @param mcastAddress The multicast address.
     * @param mcastPort The multicast port.
     * @param rcvSize The connection buffer receive size.
     * @param sndSize The connection buffer send size.
     */
    UDPConnectionInfo(const ::std::shared_ptr<::Ice::ConnectionInfo>& underlying, bool incoming, const ::std::string& adapterName, const ::std::string& connectionId, const ::std::string& localAddress, int localPort, const ::std::string& remoteAddress, int remotePort, const ::std::string& mcastAddress, int mcastPort, int rcvSize, int sndSize) :
        IPConnectionInfo(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort),
        mcastAddress(mcastAddress),
        mcastPort(mcastPort),
        rcvSize(rcvSize),
        sndSize(sndSize)
    {
    }

    /**
     * The multicast address.
     */
    ::std::string mcastAddress;
    /**
     * The multicast port.
     */
    int mcastPort = -1;
    /**
     * The connection buffer receive size.
     */
    int rcvSize = 0;
    /**
     * The connection buffer send size.
     */
    int sndSize = 0;
};

/**
 * Provides access to the connection details of a WebSocket connection
 * \headerfile Ice/Ice.h
 */
class ICE_CLASS(ICE_API) WSConnectionInfo : public ::Ice::ConnectionInfo
{
public:

    ICE_MEMBER(ICE_API) virtual ~WSConnectionInfo();

    WSConnectionInfo() = default;

    WSConnectionInfo(const WSConnectionInfo&) = default;
    WSConnectionInfo(WSConnectionInfo&&) = default;
    WSConnectionInfo& operator=(const WSConnectionInfo&) = default;
    WSConnectionInfo& operator=(WSConnectionInfo&&) = default;

    /**
     * One-shot constructor to initialize all data members.
     * @param underlying The information of the underyling transport or null if there's no underlying transport.
     * @param incoming Whether or not the connection is an incoming or outgoing connection.
     * @param adapterName The name of the adapter associated with the connection.
     * @param connectionId The connection id.
     * @param headers The headers from the HTTP upgrade request.
     */
    WSConnectionInfo(const ::std::shared_ptr<::Ice::ConnectionInfo>& underlying, bool incoming, const ::std::string& adapterName, const ::std::string& connectionId, const ::Ice::HeaderDict& headers) :
        ConnectionInfo(underlying, incoming, adapterName, connectionId),
        headers(headers)
    {
    }

    /**
     * The headers from the HTTP upgrade request.
     */
    ::Ice::HeaderDict headers;
};

}

/// \cond INTERNAL
namespace Ice
{

using ConnectionInfoPtr = ::std::shared_ptr<ConnectionInfo>;

using ConnectionPtr = ::std::shared_ptr<Connection>;

using IPConnectionInfoPtr = ::std::shared_ptr<IPConnectionInfo>;

using TCPConnectionInfoPtr = ::std::shared_ptr<TCPConnectionInfo>;

using UDPConnectionInfoPtr = ::std::shared_ptr<UDPConnectionInfo>;

using WSConnectionInfoPtr = ::std::shared_ptr<WSConnectionInfo>;

}
/// \endcond

#include <IceUtil/PopDisableWarnings.h>
#endif
