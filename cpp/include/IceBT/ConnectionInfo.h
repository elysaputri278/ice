//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __IceBT_ConnectionInfo_h__
#define __IceBT_ConnectionInfo_h__

#include <IceUtil/PushDisableWarnings.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <optional>
#include <Ice/Connection.h>
#include <IceUtil/UndefSysMacros.h>

#ifndef ICEBT_API
#   if defined(ICE_STATIC_LIBS)
#       define ICEBT_API /**/
#   elif defined(ICEBT_API_EXPORTS)
#       define ICEBT_API ICE_DECLSPEC_EXPORT
#   else
#       define ICEBT_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace IceBT
{

class ConnectionInfo;

}

namespace IceBT
{

/**
 * Provides access to the details of a Bluetooth connection.
 * \headerfile IceBT/IceBT.h
 */
class ICE_CLASS(ICEBT_API) ConnectionInfo : public ::Ice::ConnectionInfo
{
public:

    ICE_MEMBER(ICEBT_API) virtual ~ConnectionInfo();

    ConnectionInfo() :
        localAddress(""),
        localChannel(-1),
        remoteAddress(""),
        remoteChannel(-1),
        uuid(""),
        rcvSize(0),
        sndSize(0)
    {
    }

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
     * @param localAddress The local Bluetooth address.
     * @param localChannel The local RFCOMM channel.
     * @param remoteAddress The remote Bluetooth address.
     * @param remoteChannel The remote RFCOMM channel.
     * @param uuid The UUID of the service being offered (in a server) or targeted (in a client).
     * @param rcvSize The connection buffer receive size.
     * @param sndSize The connection buffer send size.
     */
    ConnectionInfo(const ::std::shared_ptr<::Ice::ConnectionInfo>& underlying, bool incoming, const ::std::string& adapterName, const ::std::string& connectionId, const ::std::string& localAddress, int localChannel, const ::std::string& remoteAddress, int remoteChannel, const ::std::string& uuid, int rcvSize, int sndSize) :
        ::Ice::ConnectionInfo(underlying, incoming, adapterName, connectionId),
        localAddress(localAddress),
        localChannel(localChannel),
        remoteAddress(remoteAddress),
        remoteChannel(remoteChannel),
        uuid(uuid),
        rcvSize(rcvSize),
        sndSize(sndSize)
    {
    }

    /**
     * The local Bluetooth address.
     */
    ::std::string localAddress;
    /**
     * The local RFCOMM channel.
     */
    int localChannel = -1;
    /**
     * The remote Bluetooth address.
     */
    ::std::string remoteAddress;
    /**
     * The remote RFCOMM channel.
     */
    int remoteChannel = -1;
    /**
     * The UUID of the service being offered (in a server) or targeted (in a client).
     */
    ::std::string uuid;
    /**
     * The connection buffer receive size.
     */
    int rcvSize = 0;
    /**
     * The connection buffer send size.
     */
    int sndSize = 0;
};

}

/// \cond INTERNAL
namespace IceBT
{

using ConnectionInfoPtr = ::std::shared_ptr<ConnectionInfo>;

}
/// \endcond

#include <IceUtil/PopDisableWarnings.h>
#endif
