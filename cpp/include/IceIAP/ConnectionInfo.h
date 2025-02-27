//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __IceIAP_ConnectionInfo_h__
#define __IceIAP_ConnectionInfo_h__

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

#ifndef ICEIAP_API
#   if defined(ICE_STATIC_LIBS)
#       define ICEIAP_API /**/
#   elif defined(ICEIAP_API_EXPORTS)
#       define ICEIAP_API ICE_DECLSPEC_EXPORT
#   else
#       define ICEIAP_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace IceIAP
{

class ConnectionInfo;

}

namespace IceIAP
{

/**
 * Provides access to the connection details of an IAP connection
 * \headerfile IceIAP/IceIAP.h
 */
class ICE_CLASS(ICEIAP_API) ConnectionInfo : public ::Ice::ConnectionInfo
{
public:

    ICE_MEMBER(ICEIAP_API) virtual ~ConnectionInfo();

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
     * @param name The accessory name.
     * @param manufacturer The accessory manufacturer.
     * @param modelNumber The accessory model number.
     * @param firmwareRevision The accessory firmare revision.
     * @param hardwareRevision The accessory hardware revision.
     * @param protocol The protocol used by the accessory.
     */
    ConnectionInfo(const ::std::shared_ptr<::Ice::ConnectionInfo>& underlying, bool incoming, const ::std::string& adapterName, const ::std::string& connectionId, const ::std::string& name, const ::std::string& manufacturer, const ::std::string& modelNumber, const ::std::string& firmwareRevision, const ::std::string& hardwareRevision, const ::std::string& protocol) :
        ::Ice::ConnectionInfo(underlying, incoming, adapterName, connectionId),
        name(name),
        manufacturer(manufacturer),
        modelNumber(modelNumber),
        firmwareRevision(firmwareRevision),
        hardwareRevision(hardwareRevision),
        protocol(protocol)
    {
    }

    /**
     * The accessory name.
     */
    ::std::string name;
    /**
     * The accessory manufacturer.
     */
    ::std::string manufacturer;
    /**
     * The accessory model number.
     */
    ::std::string modelNumber;
    /**
     * The accessory firmare revision.
     */
    ::std::string firmwareRevision;
    /**
     * The accessory hardware revision.
     */
    ::std::string hardwareRevision;
    /**
     * The protocol used by the accessory.
     */
    ::std::string protocol;
};

}

/// \cond INTERNAL
namespace IceIAP
{

using ConnectionInfoPtr = ::std::shared_ptr<ConnectionInfo>;

}
/// \endcond

#include <IceUtil/PopDisableWarnings.h>
#endif
