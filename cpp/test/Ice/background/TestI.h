//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>
#include <Configuration.h>

#include <set>

class BackgroundControllerI;
using BackgroundControllerIPtr = std::shared_ptr<BackgroundControllerI>;

class BackgroundI : public virtual Test::Background
{
public:

    virtual void op(const Ice::Current&);
    virtual void opWithPayload(Ice::ByteSeq, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

    BackgroundI(const BackgroundControllerIPtr&);

private:

    BackgroundControllerIPtr _controller;
};

class BackgroundControllerI : public Test::BackgroundController
{
public:

    virtual void pauseCall(std::string, const Ice::Current&);
    virtual void resumeCall(std::string, const Ice::Current&);
    virtual void checkCallPause(const Ice::Current&);

    virtual void holdAdapter(const Ice::Current&);
    virtual void resumeAdapter(const Ice::Current&);

    virtual void initializeSocketOperation(int, const Ice::Current&);
    virtual void initializeException(bool, const Ice::Current&);

    virtual void readReady(bool, const Ice::Current&);
    virtual void readException(bool, const Ice::Current&);

    virtual void writeReady(bool, const Ice::Current&);
    virtual void writeException(bool, const Ice::Current&);

    virtual void buffered(bool, const Ice::Current&);

    BackgroundControllerI(const Ice::ObjectAdapterPtr&, const ConfigurationPtr&);

private:

    Ice::ObjectAdapterPtr _adapter;
    std::set<std::string> _pausedCalls;
    ConfigurationPtr _configuration;
    std::mutex _mutex;
    std::condition_variable _condition;
};

#endif
