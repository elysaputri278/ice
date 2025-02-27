//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h>

using namespace std;

namespace
{

//
// A no-op Logger, used when testing the Logger Admin
//

class NullLogger : public Ice::Logger, public std::enable_shared_from_this<NullLogger>
{
public:

    virtual void print(const string&)
    {
    }

    virtual void trace(const string&, const string&)
    {
    }

    virtual void warning(const string&)
    {
    }

    virtual void error(const string&)
    {
    }

    virtual string getPrefix()
    {
        return "NullLogger";
    }

    virtual Ice::LoggerPtr cloneWithPrefix(const string&)
    {
        return shared_from_this();
    }
};

}

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
    initData.properties->setProperty("Ice.Warn.Connections", "0");
    initData.logger = std::make_shared<NullLogger>();
    Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::Identity id = Ice::stringToIdentity("communicator");
    adapter->add(std::make_shared<RemoteCommunicatorI>(), id);
    adapter->activate();

    serverReady();

    // Disable ready print for further adapters.
    communicator->getProperties()->setProperty("Ice.PrintAdapterReady", "0");

    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
