//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>
#include <TestHelper.h>
#include <Ice/NativePropertiesAdmin.h>

class RemoteCommunicatorI : public virtual Test::RemoteCommunicator
{
public:

    RemoteCommunicatorI(const Ice::CommunicatorPtr&);

    virtual Ice::ObjectPrxPtr getAdmin(const Ice::Current&);
    virtual Ice::PropertyDict getChanges(const Ice::Current&);

    virtual void addUpdateCallback(const Ice::Current&);
    virtual void removeUpdateCallback(const Ice::Current&);

    virtual void print(std::string, const Ice::Current&);
    virtual void trace(std::string, std::string, const Ice::Current&);
    virtual void warning(std::string, const Ice::Current&);
    virtual void error(std::string, const Ice::Current&);

    virtual void shutdown(const Ice::Current&);
    virtual void waitForShutdown(const Ice::Current&);
    virtual void destroy(const Ice::Current&);

    virtual void updated(const Ice::PropertyDict&);

private:

    Ice::CommunicatorPtr _communicator;
    Ice::PropertyDict _changes;

    std::function<void()> _removeCallback;
    std::mutex _mutex;
};
using RemoteCommunicatorIPtr = std::shared_ptr<RemoteCommunicatorI>;

class RemoteCommunicatorFactoryI : public Test::RemoteCommunicatorFactory
{
public:

    virtual Test::RemoteCommunicatorPrxPtr createCommunicator(Ice::PropertyDict, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
};

class TestFacetI : public Test::TestFacet
{
public:

    virtual void op(const Ice::Current&)
    {
    }
};

#endif
