//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_PLUGIN_I_H
#define ICESSL_PLUGIN_I_H

#include <IceSSL/Plugin.h>
#include <IceSSL/SSLEngineF.h>
#include <IceSSL/ConnectionInfo.h>
#include <Ice/CommunicatorF.h>

namespace IceSSL
{

class ExtendedConnectionInfo : public ConnectionInfo
{
public:

    TrustError errorCode;
    std::string host;
};
using ExtendedConnectionInfoPtr = std::shared_ptr<ExtendedConnectionInfo>;

// TODO: This class provides new certificate virtual methods that canot be added directly to the certificate class
// without breaking binary compatibility. The class can be removed once the relevant methods can be marked as virtual in
// the certificate class in the next major release (3.8.x).
class ICESSL_API CertificateExtendedInfo
{
public:

    virtual unsigned int getKeyUsage() const = 0;
    virtual unsigned int getExtendedKeyUsage() const = 0;
};

class ICESSL_API PluginI : public virtual IceSSL::Plugin
{
public:

    PluginI(const Ice::CommunicatorPtr&, const IceSSL::SSLEnginePtr&);
    //
    // From Ice::Plugin.
    //
    virtual void initialize();
    virtual void destroy();

    //
    // From IceSSL::Plugin.
    //
    virtual void setCertificateVerifier(std::function<bool(const std::shared_ptr<ConnectionInfo>&)>);
    virtual void setPasswordPrompt(std::function<std::string()>);

    virtual CertificatePtr load(const std::string&) const = 0;
    virtual CertificatePtr decode(const std::string&) const = 0;
protected:

    SSLEnginePtr _engine;
};

}

#endif
