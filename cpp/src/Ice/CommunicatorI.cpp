//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/CommunicatorI.h>
#include <Ice/Instance.h>
#include <Ice/Properties.h>
#include <Ice/ConnectionFactory.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/TraceLevels.h>
#include <Ice/ThreadPool.h>
#include <Ice/Router.h>
#include "Ice/OutgoingAsync.h"
#include <Ice/UUID.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

CommunicatorFlushBatchAsync::~CommunicatorFlushBatchAsync()
{
    // Out of line to avoid weak vtable
}

CommunicatorFlushBatchAsync::CommunicatorFlushBatchAsync(const InstancePtr& instance) :
    OutgoingAsyncBase(instance)
{
    //
    // _useCount is initialized to 1 to prevent premature callbacks.
    // The caller must invoke ready() after all flush requests have
    // been initiated.
    //
    _useCount = 1;
}

void
CommunicatorFlushBatchAsync::flushConnection(const ConnectionIPtr& con, Ice::CompressBatch compressBatch)
{
    class FlushBatch : public OutgoingAsyncBase
    {
    public:

        FlushBatch(const CommunicatorFlushBatchAsyncPtr& outAsync,
                   const InstancePtr& instance,
                   InvocationObserver& observer) :
            OutgoingAsyncBase(instance), _outAsync(outAsync), _parentObserver(observer)
        {
        }

        virtual bool
        sent()
        {
            _childObserver.detach();
            _outAsync->check(false);
            return false;
        }

        virtual bool
        exception(std::exception_ptr ex)
        {
            _childObserver.failed(getExceptionId(ex));
            _childObserver.detach();
            _outAsync->check(false);
            return false;
        }

        virtual InvocationObserver&
        getObserver()
        {
            return _parentObserver;
        }

        virtual bool handleSent(bool, bool)
        {
            return false;
        }

        virtual bool handleException(std::exception_ptr)
        {
            return false;
        }

        virtual bool handleResponse(bool)
        {
            return false;
        }

        virtual void handleInvokeSent(bool, OutgoingAsyncBase*) const
        {
            assert(false);
        }

        virtual void handleInvokeException(std::exception_ptr, OutgoingAsyncBase*) const
        {
            assert(false);
        }

        virtual void handleInvokeResponse(bool, OutgoingAsyncBase*) const
        {
            assert(false);
        }

    private:

        const CommunicatorFlushBatchAsyncPtr _outAsync;
        InvocationObserver& _parentObserver;
    };

    {
        Lock sync(_m);
        ++_useCount;
    }

    try
    {
        OutgoingAsyncBasePtr flushBatch = make_shared<FlushBatch>(shared_from_this(), _instance, _observer);
        bool compress;
        int batchRequestNum = con->getBatchRequestQueue()->swap(flushBatch->getOs(), compress);
        if(batchRequestNum == 0)
        {
            flushBatch->sent();
        }
        else
        {
            if(compressBatch == CompressBatch::Yes)
            {
                compress = true;
            }
            else if(compressBatch == CompressBatch::No)
            {
                compress = false;
            }
            con->sendAsyncRequest(flushBatch, compress, false, batchRequestNum);
        }
    }
    catch(const LocalException&)
    {
        check(false);
        throw;
    }
}

void
CommunicatorFlushBatchAsync::invoke(const string& operation, CompressBatch compressBatch)
{
    _observer.attach(_instance.get(), operation);
    _instance->outgoingConnectionFactory()->flushAsyncBatchRequests(shared_from_this(), compressBatch);
    _instance->objectAdapterFactory()->flushAsyncBatchRequests(shared_from_this(), compressBatch);
    check(true);
}

void
CommunicatorFlushBatchAsync::check(bool userThread)
{
    {
        Lock sync(_m);
        assert(_useCount > 0);
        if(--_useCount > 0)
        {
            return;
        }
    }

    if(sentImpl(true))
    {
        if(userThread)
        {
            _sentSynchronously = true;
            invokeSent();
        }
        else
        {
            invokeSentAsync();
        }
    }
}

void
Ice::CommunicatorI::destroy() noexcept
{
    if(_instance)
    {
        _instance->destroy();
    }
}

void
Ice::CommunicatorI::shutdown() noexcept
{
    try
    {
        _instance->objectAdapterFactory()->shutdown();
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
        // Ignore
    }
}

void
Ice::CommunicatorI::waitForShutdown() noexcept
{
    try
    {
        _instance->objectAdapterFactory()->waitForShutdown();
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
        // Ignore
    }
}

bool
Ice::CommunicatorI::isShutdown() const noexcept
{
    try
    {
        return _instance->objectAdapterFactory()->isShutdown();
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
        return true;
    }
}

std::optional<ObjectPrx>
Ice::CommunicatorI::stringToProxy(const string& s) const
{
    return _instance->proxyFactory()->stringToProxy(s);
}

string
Ice::CommunicatorI::proxyToString(const std::optional<ObjectPrx>& proxy) const
{
    return _instance->proxyFactory()->proxyToString(proxy);
}

std::optional<ObjectPrx>
Ice::CommunicatorI::propertyToProxy(const string& p) const
{
    return _instance->proxyFactory()->propertyToProxy(p);
}

PropertyDict
Ice::CommunicatorI::proxyToProperty(const std::optional<ObjectPrx>& proxy, const string& property) const
{
    return _instance->proxyFactory()->proxyToProperty(proxy, property);
}

string
Ice::CommunicatorI::identityToString(const Identity& ident) const
{
    return Ice::identityToString(ident, _instance->toStringMode());
}

ObjectAdapterPtr
Ice::CommunicatorI::createObjectAdapter(const string& name)
{
    return _instance->objectAdapterFactory()->createObjectAdapter(name, nullopt);
}

ObjectAdapterPtr
Ice::CommunicatorI::createObjectAdapterWithEndpoints(const string& name, const string& endpoints)
{
    string oaName = name;
    if(oaName.empty())
    {
        oaName = Ice::generateUUID();
    }

    getProperties()->setProperty(oaName + ".Endpoints", endpoints);
    return _instance->objectAdapterFactory()->createObjectAdapter(oaName, nullopt);
}

ObjectAdapterPtr
Ice::CommunicatorI::createObjectAdapterWithRouter(const string& name, const RouterPrx& router)
{
    string oaName = name;
    if(oaName.empty())
    {
        oaName = Ice::generateUUID();
    }

    PropertyDict properties = proxyToProperty(router, oaName + ".Router");
    for(PropertyDict::const_iterator p = properties.begin(); p != properties.end(); ++p)
    {
        getProperties()->setProperty(p->first, p->second);
    }

    return _instance->objectAdapterFactory()->createObjectAdapter(oaName, router);
}

PropertiesPtr
Ice::CommunicatorI::getProperties() const noexcept
{
    return _instance->initializationData().properties;
}

LoggerPtr
Ice::CommunicatorI::getLogger() const noexcept
{
    return _instance->initializationData().logger;
}

Ice::Instrumentation::CommunicatorObserverPtr
Ice::CommunicatorI::getObserver() const noexcept
{
    return _instance->initializationData().observer;
}

std::optional<RouterPrx>
Ice::CommunicatorI::getDefaultRouter() const
{
    return _instance->referenceFactory()->getDefaultRouter();
}

void
Ice::CommunicatorI::setDefaultRouter(const std::optional<RouterPrx>& router)
{
    _instance->setDefaultRouter(router);
}

optional<LocatorPrx>
Ice::CommunicatorI::getDefaultLocator() const
{
    return _instance->referenceFactory()->getDefaultLocator();
}

void
Ice::CommunicatorI::setDefaultLocator(const optional<LocatorPrx>& locator)
{
    _instance->setDefaultLocator(locator);
}

Ice::ImplicitContextPtr
Ice::CommunicatorI::getImplicitContext() const noexcept
{
    return _instance->getImplicitContext();
}

PluginManagerPtr
Ice::CommunicatorI::getPluginManager() const
{
    return _instance->pluginManager();
}

ValueFactoryManagerPtr
Ice::CommunicatorI::getValueFactoryManager() const noexcept
{
    return _instance->initializationData().valueFactoryManager;
}

#ifdef ICE_SWIFT

dispatch_queue_t
Ice::CommunicatorI::getClientDispatchQueue() const
{
    return _instance->clientThreadPool()->getDispatchQueue();
}

dispatch_queue_t
Ice::CommunicatorI::getServerDispatchQueue() const
{
    return _instance->serverThreadPool()->getDispatchQueue();
}

#endif

void
Ice::CommunicatorI::postToClientThreadPool(function<void()> call)
{
    _instance->clientThreadPool()->dispatch(call);
}

namespace
{

const ::std::string flushBatchRequests_name = "flushBatchRequests";

}

::std::function<void()>
Ice::CommunicatorI::flushBatchRequestsAsync(CompressBatch compress,
                                            function<void(exception_ptr)> ex,
                                            function<void(bool)> sent)
{
    class CommunicatorFlushBatchLambda : public CommunicatorFlushBatchAsync, public LambdaInvoke
    {
    public:

        CommunicatorFlushBatchLambda(const InstancePtr& instance,
                                     std::function<void(std::exception_ptr)> ex,
                                     std::function<void(bool)> sent) :
            CommunicatorFlushBatchAsync(instance), LambdaInvoke(std::move(ex), std::move(sent))
        {
        }
    };
    auto outAsync = make_shared<CommunicatorFlushBatchLambda>(_instance, ex, sent);
    outAsync->invoke(flushBatchRequests_name, compress);
    return [outAsync]() { outAsync->cancel(); };
}

ObjectPrx
Ice::CommunicatorI::createAdmin(const ObjectAdapterPtr& adminAdapter, const Identity& adminId)
{
    return _instance->createAdmin(adminAdapter, adminId);
}

std::optional<ObjectPrx>
Ice::CommunicatorI::getAdmin() const
{
    return _instance->getAdmin();
}

void
Ice::CommunicatorI::addAdminFacet(const shared_ptr<Object>& servant, const string& facet)
{
    _instance->addAdminFacet(servant, facet);
}

shared_ptr<Object>
Ice::CommunicatorI::removeAdminFacet(const string& facet)
{
    return _instance->removeAdminFacet(facet);
}

shared_ptr<Object>
Ice::CommunicatorI::findAdminFacet(const string& facet)
{
    return _instance->findAdminFacet(facet);
}

Ice::FacetMap
Ice::CommunicatorI::findAllAdminFacets()
{
    return _instance->findAllAdminFacets();
}

CommunicatorIPtr
Ice::CommunicatorI::create(const InitializationData& initData)
{
    Ice::CommunicatorIPtr communicator = make_shared<CommunicatorI>();
    try
    {
        const_cast<InstancePtr&>(communicator->_instance) = Instance::create(communicator, initData);

        //
        // Keep a reference to the dynamic library list to ensure
        // the libraries are not unloaded until this Communicator's
        // destructor is invoked.
        //
        const_cast<DynamicLibraryListPtr&>(communicator->_dynamicLibraryList) = communicator->_instance->dynamicLibraryList();
    }
    catch(...)
    {
        communicator->destroy();
        throw;
    }
    return communicator;
}

Ice::CommunicatorI::~CommunicatorI()
{
    if(_instance && !_instance->destroyed())
    {
        Warning out(_instance->initializationData().logger);
        out << "Ice::Communicator::destroy() has not been called";
    }
}

void
Ice::CommunicatorI::finishSetup(int& argc, const char* argv[])
{
    try
    {
        _instance->finishSetup(argc, argv, shared_from_this());
    }
    catch(...)
    {
        destroy();
        throw;
    }
}
