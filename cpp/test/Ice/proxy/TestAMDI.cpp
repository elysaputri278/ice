//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestAMDI.h>
#include <TestHelper.h>

using namespace std;

MyDerivedClassI::MyDerivedClassI()
{
}

void
MyDerivedClassI::echoAsync(
    Ice::ObjectPrxPtr obj,
    function<void(const Ice::ObjectPrxPtr&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(obj);
}

void
MyDerivedClassI::shutdownAsync(
    function<void()> response,
    function<void(exception_ptr)>,
    const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
    response();
}

void
MyDerivedClassI::getContextAsync(
    function<void(const Ice::Context&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(_ctx);
}

bool
MyDerivedClassI::ice_isA(string s, const Ice::Current& current) const
{
    _ctx = current.ctx;
    return Test::MyDerivedClass::ice_isA(std::move(s), current);
}
