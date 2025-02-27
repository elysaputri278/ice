//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>
#include <TestHelper.h>

class RemoteCommunicatorI : public Test::RemoteCommunicator
{
public:

    RemoteCommunicatorI();

    virtual Test::RemoteObjectAdapterPrxPtr createObjectAdapter(std::string, std::string,
                                                                              const Ice::Current&);
    virtual void deactivateObjectAdapter(Test::RemoteObjectAdapterPrxPtr, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    int _nextPort;
};

class RemoteObjectAdapterI : public Test::RemoteObjectAdapter
{
public:

    RemoteObjectAdapterI(const Ice::ObjectAdapterPtr&);

    virtual Test::TestIntfPrxPtr getTestIntf(const Ice::Current&);
    virtual void deactivate(const Ice::Current&);

private:

    const Ice::ObjectAdapterPtr _adapter;
    const Test::TestIntfPrxPtr _testIntf;
};

class TestI : public Test::TestIntf
{
public:

    virtual std::string getAdapterName(const Ice::Current&);
};

#endif
