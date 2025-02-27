//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __IceSSL_ConnectionInfo_h__
#define __IceSSL_ConnectionInfo_h__

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
#include <IceSSL/Plugin.h>

#ifndef ICESSL_API
#   if defined(ICE_STATIC_LIBS)
#       define ICESSL_API /**/
#   elif defined(ICESSL_API_EXPORTS)
#       define ICESSL_API ICE_DECLSPEC_EXPORT
#   else
#       define ICESSL_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace IceSSL
{

class ConnectionInfo;

}

namespace IceSSL
{

/**
 * Provides access to the connection details of an SSL connection
 * \headerfile IceSSL/IceSSL.h
 */
class ICE_CLASS(ICESSL_API) ConnectionInfo : public ::Ice::ConnectionInfo
{
public:

    ICE_MEMBER(ICESSL_API) virtual ~ConnectionInfo();

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
     * @param cipher The negotiated cipher suite.
     * @param certs The certificate chain.
     * @param verified The certificate chain verification status.
     */
    ConnectionInfo(const ::std::shared_ptr<::Ice::ConnectionInfo>& underlying, bool incoming, const ::std::string& adapterName, const ::std::string& connectionId, const ::std::string& cipher, const std::vector<CertificatePtr>& certs, bool verified) :
        ::Ice::ConnectionInfo(underlying, incoming, adapterName, connectionId),
        cipher(cipher),
        certs(certs),
        verified(verified)
    {
    }

    /**
     * The negotiated cipher suite.
     */
    ::std::string cipher;
    /**
     * The certificate chain.
     */
    std::vector<CertificatePtr> certs;
    /**
     * The certificate chain verification status.
     */
    bool verified;
};

}

/// \cond INTERNAL
namespace IceSSL
{

using ConnectionInfoPtr = ::std::shared_ptr<ConnectionInfo>;

}
/// \endcond

#include <IceUtil/PopDisableWarnings.h>
#endif
