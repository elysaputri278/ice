//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <ServantLocatorI.h>
#include <TestHelper.h>
#include <TestI.h>

using namespace std;
using namespace Ice;
using namespace Test;

namespace
{

class RouterI : public Ice::Router
{
public:

    RouterI() : _nextPort(23456)
    {
    }

    virtual Ice::ObjectPrxPtr
    getClientProxy(optional<bool>&, const Ice::Current&) const
    {
        return nullopt;
    }

    virtual Ice::ObjectPrxPtr
    getServerProxy(const Ice::Current& c) const
    {
        ostringstream os;
        os << "dummy:tcp -h localhost -p " << _nextPort++ << " -t 30000";
        return c.adapter->getCommunicator()->stringToProxy(os.str());
    }

    virtual Ice::ObjectProxySeq
    addProxies(Ice::ObjectProxySeq, const Ice::Current&)
    {
        return Ice::ObjectProxySeq();
    }

private:

    mutable int _nextPort;
};

}

ServantLocatorI::ServantLocatorI() : _deactivated(false), _router(make_shared<RouterI>())
{
}

ServantLocatorI::~ServantLocatorI()
{
    test(_deactivated);
}

std::shared_ptr<Ice::Object>
ServantLocatorI::locate(const Ice::Current& current, std::shared_ptr<void>& cookie)
{
    test(!_deactivated);

    if(current.id.name == "router")
    {
        return _router;
    }

    test(current.id.category == "");
    test(current.id.name == "test");

    cookie = make_shared<Cookie>();

    return make_shared<TestI>();
}

void
ServantLocatorI::finished(const Ice::Current& current, const shared_ptr<Ice::Object>&, const std::shared_ptr<void>& cookie)
{
    test(!_deactivated);
    if(current.id.name == "router")
    {
        return;
    }

    shared_ptr<Cookie> co = static_pointer_cast<Cookie>(cookie);
    test(co);
    test(co->message() == "blahblah");
}

void
ServantLocatorI::deactivate(const string&)
{
    test(!_deactivated);

    _deactivated = true;
}
