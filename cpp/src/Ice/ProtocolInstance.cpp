//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/ProtocolInstance.h>
#include <Ice/Instance.h>
#include <Ice/Initialize.h>
#include <Ice/IPEndpointI.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/TraceLevels.h>
#include <Ice/EndpointFactoryManager.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::ProtocolInstance::~ProtocolInstance()
{
    // Out of line to avoid weak vtable
}

IceInternal::ProtocolInstance::ProtocolInstance(const CommunicatorPtr& com, int16_t type, const string& protocol,
                                                 bool secure) :
    _instance(getInstance(com)),
    _traceLevel(_instance->traceLevels()->network),
    _traceCategory(_instance->traceLevels()->networkCat),
    _properties(_instance->initializationData().properties),
    _protocol(protocol),
    _type(type),
    _secure(secure)
{
}

IceInternal::ProtocolInstance::ProtocolInstance(const InstancePtr& instance, int16_t type, const string& protocol,
                                                bool secure) :
    _instance(instance),
    _traceLevel(_instance->traceLevels()->network),
    _traceCategory(_instance->traceLevels()->networkCat),
    _properties(_instance->initializationData().properties),
    _protocol(protocol),
    _type(type),
    _secure(secure)
{
}

const LoggerPtr&
IceInternal::ProtocolInstance::logger() const
{
    return _instance->initializationData().logger;
}

EndpointFactoryPtr
IceInternal::ProtocolInstance::getEndpointFactory(int16_t type) const
{
    return _instance->endpointFactoryManager()->get(type);
}

BufSizeWarnInfo
IceInternal::ProtocolInstance::getBufSizeWarn(int16_t type)
{
    return _instance->getBufSizeWarn(type);
}

void
IceInternal::ProtocolInstance::setSndBufSizeWarn(int16_t type, int size)
{
    _instance->setSndBufSizeWarn(type, size);
}

void
IceInternal::ProtocolInstance::setRcvBufSizeWarn(int16_t type, int size)
{
    _instance->setRcvBufSizeWarn(type, size);
}

bool
IceInternal::ProtocolInstance::preferIPv6() const
{
    return _instance->preferIPv6();
}

ProtocolSupport
IceInternal::ProtocolInstance::protocolSupport() const
{
    return _instance->protocolSupport();
}

const string&
IceInternal::ProtocolInstance::defaultHost() const
{
    return _instance->defaultsAndOverrides()->defaultHost;
}

const Address&
IceInternal::ProtocolInstance::defaultSourceAddress() const
{
    return _instance->defaultsAndOverrides()->defaultSourceAddress;
}

const EncodingVersion&
IceInternal::ProtocolInstance::defaultEncoding() const
{
    return _instance->defaultsAndOverrides()->defaultEncoding;
}

int
IceInternal::ProtocolInstance::defaultTimeout() const
{
    return _instance->defaultsAndOverrides()->defaultTimeout;
}

NetworkProxyPtr
IceInternal::ProtocolInstance::networkProxy() const
{
    return _instance->networkProxy();
}

size_t
IceInternal::ProtocolInstance::messageSizeMax() const
{
    return _instance->messageSizeMax();
}

void
IceInternal::ProtocolInstance::resolve(
    const string& host,
    int port,
    EndpointSelectionType type,
    const IPEndpointIPtr& endpoint,
    std::function<void(std::vector<ConnectorPtr>)> response,
    std::function<void(exception_ptr)> exception) const
{
    _instance->endpointHostResolver()->resolve(host, port, type, endpoint, response, exception);
}
