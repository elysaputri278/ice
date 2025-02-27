//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>
#include <StringConverterI.h>

using namespace std;

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    setProcessStringConverter(make_shared<Test::StringConverterI>());
    setProcessWstringConverter(make_shared<Test::WstringConverterI>());
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    Test::TestIntfPrxPtr allTests(Test::TestHelper*);
    Test::TestIntfPrxPtr test = allTests(this);
    test->shutdown();
}

DEFINE_TEST(Client)
