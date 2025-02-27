//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/EndpointFactory.h>
#include <Ice/Instance.h>
#include <Ice/EndpointFactoryManager.h>
#include <Ice/ProtocolInstance.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::EndpointFactory::EndpointFactory()
{
}

IceInternal::EndpointFactory::~EndpointFactory()
{
}

void
IceInternal::EndpointFactory::initialize()
{
    // Nothing to do, can be overridden by specialization to finish initialization.
}

IceInternal::EndpointFactoryPlugin::EndpointFactoryPlugin(const CommunicatorPtr& communicator,
                                                          const EndpointFactoryPtr& factory)
{
    assert(communicator);
    getInstance(communicator)->endpointFactoryManager()->add(factory);
}

void
IceInternal::EndpointFactoryPlugin::initialize()
{
}

void
IceInternal::EndpointFactoryPlugin::destroy()
{
}

IceInternal::EndpointFactoryWithUnderlying::EndpointFactoryWithUnderlying(const ProtocolInstancePtr& instance,
                                                                          int16_t type) :
    _instance(instance), _type(type)
{
}

void
IceInternal::EndpointFactoryWithUnderlying::initialize()
{
    //
    // Get the endpoint factory for the underlying type and clone it with
    // our protocol instance.
    //
    EndpointFactoryPtr factory = _instance->getEndpointFactory(_type);
    if(factory)
    {
        _underlying = factory->clone(_instance);
        _underlying->initialize();
    }
}

int16_t
IceInternal::EndpointFactoryWithUnderlying::type() const
{
    return _instance->type();
}

string
IceInternal::EndpointFactoryWithUnderlying::protocol() const
{
    return _instance->protocol();
}

EndpointIPtr
IceInternal::EndpointFactoryWithUnderlying::create(vector<string>& args, bool oaEndpoint) const
{
    if(!_underlying)
    {
        return nullptr; // Can't create an endpoint without underlying factory.
    }
    return createWithUnderlying(_underlying->create(args, oaEndpoint), args, oaEndpoint);
}

EndpointIPtr
IceInternal::EndpointFactoryWithUnderlying::read(InputStream* s) const
{
    if(!_underlying)
    {
        return nullptr; // Can't create an endpoint without underlying factory.
    }
    return readWithUnderlying(_underlying->read(s), s);
}

void
IceInternal::EndpointFactoryWithUnderlying::destroy()
{
    if(_underlying)
    {
        _underlying->destroy();
    }
    _instance = nullptr;
}

EndpointFactoryPtr
IceInternal::EndpointFactoryWithUnderlying::clone(const ProtocolInstancePtr& instance) const
{
    return cloneWithUnderlying(instance, _type);
}

IceInternal::UnderlyingEndpointFactory::UnderlyingEndpointFactory(const ProtocolInstancePtr& instance,
                                                                  int16_t type,
                                                                  int16_t underlying) :
    _instance(instance), _type(type), _underlying(underlying)
{
}

void
IceInternal::UnderlyingEndpointFactory::initialize()
{
    //
    // Get the endpoint factory of the given endpoint type. If it's a factory that
    // delegates to an underlying endpoint, clone it and instruct it to delegate to
    // our underlying factory.
    //
    EndpointFactoryPtr factory = _instance->getEndpointFactory(_type);
    if(factory)
    {
        EndpointFactoryWithUnderlying* f = dynamic_cast<EndpointFactoryWithUnderlying*>(factory.get());
        if(f)
        {
            _factory = f->cloneWithUnderlying(_instance, _underlying);
            _factory->initialize();
        }
    }
}

int16_t
IceInternal::UnderlyingEndpointFactory::type() const
{
    return _instance->type();
}

string
IceInternal::UnderlyingEndpointFactory::protocol() const
{
    return _instance->protocol();
}

EndpointIPtr
IceInternal::UnderlyingEndpointFactory::create(vector<string>& args, bool oaEndpoint) const
{
    if(!_factory)
    {
        return nullptr;
    }
    return _factory->create(args, oaEndpoint);
}

EndpointIPtr
IceInternal::UnderlyingEndpointFactory::read(InputStream* s) const
{
    if(!_factory)
    {
        return nullptr;
    }
    return _factory->read(s);
}

void
IceInternal::UnderlyingEndpointFactory::destroy()
{
    if(_factory)
    {
        _factory->destroy();
    }
    _instance = nullptr;
}

EndpointFactoryPtr
IceInternal::UnderlyingEndpointFactory::clone(const ProtocolInstancePtr& instance) const
{
    return make_shared<UnderlyingEndpointFactory>(instance, _type, _underlying);
}
