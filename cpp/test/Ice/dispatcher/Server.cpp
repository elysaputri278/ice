//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestHelper.h>
#include <Dispatcher.h>

using namespace std;

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    //
    // Limit the recv buffer size, this test relies on the socket
    // send() blocking after sending a given amount of data.
    //
    initData.properties->setProperty("Ice.TCP.RcvSize", "50000");

    auto dispatcher = Dispatcher::create();
    initData.dispatcher = [=](function<void()> call, const shared_ptr<Ice::Connection>& conn)
        {
            dispatcher->dispatch(make_shared<DispatcherCall>(call), conn);
        };
    // The communicator must be destroyed before the dispatcher is terminated.
    {
        Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);

        communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
        communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1, "tcp"));
        communicator->getProperties()->setProperty("ControllerAdapter.ThreadPool.Size", "1");

        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
        Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("ControllerAdapter");

        TestIntfControllerIPtr testController = make_shared<TestIntfControllerI>(adapter);

        adapter->add(make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
        adapter->activate();

        adapter2->add(testController, Ice::stringToIdentity("testController"));
        adapter2->activate();

        serverReady();

        communicator->waitForShutdown();
    }
    dispatcher->terminate();
}

DEFINE_TEST(Server)
