//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <SessionI.h>

using namespace std;
using namespace Test;

SessionManagerI::SessionManagerI(const shared_ptr<TestControllerI>& controller):
    _controller(controller)
{
}

Glacier2::SessionPrxPtr
SessionManagerI::create(string, Glacier2::SessionControlPrxPtr sessionControl, const Ice::Current& current)
{
    auto newSession = Ice::uncheckedCast<Glacier2::SessionPrx>(
        current.adapter->addWithUUID(make_shared<SessionI>(sessionControl, _controller)));
    _controller->addSession(SessionTuple(newSession, std::move(sessionControl)));
    return newSession;
}

SessionI::SessionI(Glacier2::SessionControlPrxPtr sessionControl,
                   shared_ptr<TestControllerI> controller) :
    _sessionControl(std::move(sessionControl)),
    _controller(std::move(controller))
{
    assert(_sessionControl);
}

void
SessionI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

void
SessionI::destroy(const Ice::Current& current)
{
    _controller->notifyDestroy(_sessionControl);
    current.adapter->remove(current.id);
}
