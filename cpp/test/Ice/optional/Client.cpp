//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h>

using namespace std;
using namespace Test;

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.AcceptClassCycles", "1");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    InitialPrxPtr allTests(Test::TestHelper*, bool);
    InitialPrxPtr initial = allTests(this, false);
    initial->shutdown();
}

DEFINE_TEST(Client)
