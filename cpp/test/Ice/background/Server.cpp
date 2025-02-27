//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h>
#include <Configuration.h>
#include <PluginI.h>

#include <Ice/Locator.h>
#include <Ice/Router.h>

#ifdef _MSC_VER
#   pragma comment(lib, ICE_LIBNAME("testtransport"))
#endif

using namespace std;

extern "C"
{

Ice::Plugin* createTestTransport(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);

};

class LocatorI : public Ice::Locator
{
public:

    virtual void
    findAdapterByIdAsync(string,
                         function<void(const Ice::ObjectPrxPtr&)> response,
                         function<void(exception_ptr)>,
                         const Ice::Current& current) const
    {
        _controller->checkCallPause(current);
        Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();
        response(current.adapter->createDirectProxy(Ice::stringToIdentity("dummy")));
    }

    virtual void
    findObjectByIdAsync(Ice::Identity id,
                        function<void(const Ice::ObjectPrxPtr&)> response,
                        function<void(exception_ptr)>,
                        const Ice::Current& current) const
    {
        _controller->checkCallPause(current);
        Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();
        response(current.adapter->createDirectProxy(id));
    }

    virtual Ice::LocatorRegistryPrxPtr
    getRegistry(const Ice::Current&) const
    {
        return nullopt;
    }

    LocatorI(const BackgroundControllerIPtr& controller) : _controller(controller)
    {
    }

private:

    BackgroundControllerIPtr _controller;
};

class RouterI : public Ice::Router
{
public:

    virtual Ice::ObjectPrxPtr
    getClientProxy(optional<bool>& hasRoutingTable, const Ice::Current& current) const
    {
        hasRoutingTable = true;
        _controller->checkCallPause(current);
        return nullopt;
    }

    virtual Ice::ObjectPrxPtr
    getServerProxy(const Ice::Current& current) const
    {
        _controller->checkCallPause(current);
        return nullopt;
    }

    virtual Ice::ObjectProxySeq
    addProxies(Ice::ObjectProxySeq, const Ice::Current&)
    {
        return Ice::ObjectProxySeq();
    }

    RouterI(const BackgroundControllerIPtr& controller)
    {
        _controller = controller;
    }

private:

    BackgroundControllerIPtr _controller;
};

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
#ifdef ICE_STATIC_LIBS
    Ice::registerPluginFactory("Test", createTestTransport, false);
#endif
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);

    //
    // This test kills connections, so we don't want warnings.
    //
    properties->setProperty("Ice.Warn.Connections", "0");

    properties->setProperty("Ice.MessageSizeMax", "50000");

    //
    // This test relies on filling the TCP send/recv buffer, so
    // we rely on a fixed value for these buffers.
    //
    properties->setProperty("Ice.TCP.RcvSize", "50000");

    //
    // Setup the test transport plug-in.
    //
    properties->setProperty("Ice.Plugin.Test", "TestTransport:createTestTransport");
    string defaultProtocol = properties->getPropertyWithDefault("Ice.Default.Protocol", "tcp");
    properties->setProperty("Ice.Default.Protocol", "test-" + defaultProtocol);

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
    communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1, "tcp"));
    communicator->getProperties()->setProperty("ControllerAdapter.ThreadPool.Size", "1");

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("ControllerAdapter");

    shared_ptr<PluginI> plugin = dynamic_pointer_cast<PluginI>(communicator->getPluginManager()->getPlugin("Test"));
    assert(plugin);
    ConfigurationPtr configuration = plugin->getConfiguration();
    BackgroundControllerIPtr backgroundController = make_shared<BackgroundControllerI>(adapter, configuration);

    adapter->add(make_shared<BackgroundI>(backgroundController), Ice::stringToIdentity("background"));
    adapter->add(make_shared<LocatorI>(backgroundController), Ice::stringToIdentity("locator"));
    adapter->add(make_shared<RouterI>(backgroundController), Ice::stringToIdentity("router"));
    adapter->activate();

    adapter2->add(backgroundController, Ice::stringToIdentity("backgroundController"));
    adapter2->activate();

    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
