//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __IceSSL_EndpointInfo_h__
#define __IceSSL_EndpointInfo_h__

#include <IceUtil/PushDisableWarnings.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <optional>
#include <Ice/Endpoint.h>
#include <IceUtil/UndefSysMacros.h>

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

class EndpointInfo;
/**
 * Provides access to an SSL endpoint information.
 * \headerfile IceSSL/IceSSL.h
 */
class ICE_CLASS(ICESSL_API) EndpointInfo : public ::Ice::EndpointInfo
{
public:

    ICE_MEMBER(ICESSL_API) virtual ~EndpointInfo();

    EndpointInfo() = default;

    EndpointInfo(const EndpointInfo&) = default;
    EndpointInfo(EndpointInfo&&) = default;
    EndpointInfo& operator=(const EndpointInfo&) = default;
    EndpointInfo& operator=(EndpointInfo&&) = default;

    /**
     * One-shot constructor to initialize all data members.
     * @param underlying The information of the underlying endpoint or null if there's no underlying endpoint.
     * @param timeout The timeout for the endpoint in milliseconds.
     * @param compress Specifies whether or not compression should be used if available when using this endpoint.
     */
    EndpointInfo(const ::std::shared_ptr<::Ice::EndpointInfo>& underlying, int timeout, bool compress) :
        ::Ice::EndpointInfo(underlying, timeout, compress)
    {
    }
};

}

/// \cond INTERNAL
namespace IceSSL
{

using EndpointInfoPtr = ::std::shared_ptr<EndpointInfo>;

}
/// \endcond

#include <IceUtil/PopDisableWarnings.h>
#endif
