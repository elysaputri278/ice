//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestHelper.h>

using namespace std;

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.AcceptClassCycles", "1");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    communicator->getProperties()->setProperty("Ice.Warn.Dispatch", "0");
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint() + " -t 2000");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPtr object = std::make_shared<TestI>();
    adapter->add(object, Ice::stringToIdentity("Test"));
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
