//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <TestI.h>
#include <Ice/Ice.h>

using namespace std;
using namespace Ice;

void
BackgroundI::op(const Ice::Current& current)
{
    _controller->checkCallPause(current);
}

void
BackgroundI::opWithPayload(Ice::ByteSeq, const Ice::Current& current)
{
    _controller->checkCallPause(current);
}

void
BackgroundI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

BackgroundI::BackgroundI(const BackgroundControllerIPtr& controller) :
    _controller(controller)
{
}

void
BackgroundControllerI::pauseCall(string opName, const Ice::Current&)
{
    lock_guard lock(_mutex);
    _pausedCalls.insert(opName);
}

void
BackgroundControllerI::resumeCall(string opName, const Ice::Current&)
{
    lock_guard lock(_mutex);
    _pausedCalls.erase(opName);
    _condition.notify_all();
}

void
BackgroundControllerI::checkCallPause(const Ice::Current& current)
{
    unique_lock lock(_mutex);
    _condition.wait(lock, [this, &current] { return _pausedCalls.find(current.operation) == _pausedCalls.end(); });
}

void
BackgroundControllerI::holdAdapter(const Ice::Current&)
{
    _adapter->hold();
}

void
BackgroundControllerI::resumeAdapter(const Ice::Current&)
{
    _adapter->activate();
}

void
BackgroundControllerI::initializeSocketOperation(int status, const Ice::Current&)
{
    _configuration->initializeSocketOperation(static_cast<IceInternal::SocketOperation>(status));
}

void
BackgroundControllerI::initializeException(bool enable, const Ice::Current&)
{
    _configuration->initializeException(enable ? new Ice::SocketException(__FILE__, __LINE__) : 0);
}

void
BackgroundControllerI::readReady(bool enable, const Ice::Current&)
{
    _configuration->readReady(enable);
}

void
BackgroundControllerI::readException(bool enable, const Ice::Current&)
{
    _configuration->readException(enable ? new Ice::SocketException(__FILE__, __LINE__) : 0);
}

void
BackgroundControllerI::writeReady(bool enable, const Ice::Current&)
{
    _configuration->writeReady(enable);
}

void
BackgroundControllerI::writeException(bool enable, const Ice::Current&)
{
    _configuration->writeException(enable ? new Ice::SocketException(__FILE__, __LINE__) : 0);
}

void
BackgroundControllerI::buffered(bool enable, const Ice::Current&)
{
    _configuration->buffered(enable);
}

BackgroundControllerI::BackgroundControllerI(const Ice::ObjectAdapterPtr& adapter,
                                             const ConfigurationPtr& configuration) :
    _adapter(adapter),
    _configuration(configuration)
{
}
