//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <MyObjectI.h>
#include <TestHelper.h>
#include <IceUtil/IceUtil.h>

#include <thread>

using namespace IceUtil;
using namespace std;

MySystemException::MySystemException(const char* file, int line) :
    Ice::SystemException(file, line)
{
}

string
MySystemException::ice_id() const
{
    return "::MySystemException";
}

IceUtil::Exception*
MySystemException::ice_cloneImpl() const
{
    return new MySystemException(*this);
}

void
MySystemException::ice_throw() const
{
    throw *this;
}

int
MyObjectI::add(int x, int y, const Ice::Current&)
{
    return x + y;
}

int
MyObjectI::addWithRetry(int x, int y, const Ice::Current& current)
{
    Ice::Context::const_iterator p = current.ctx.find("retry");

    if(p == current.ctx.end() || p->second != "no")
    {
        throw MyRetryException();
    }
    return x + y;
}

int
MyObjectI::badAdd(int, int, const Ice::Current&)
{
    throw Test::InvalidInputException();
}

int
MyObjectI::notExistAdd(int, int, const Ice::Current&)
{
    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
}

int
MyObjectI::badSystemAdd(int, int, const Ice::Current&)
{
    throw MySystemException(__FILE__, __LINE__);
}

void
MyObjectI::amdAddAsync(int x,
                       int y,
                       function<void(int)> response,
                       function<void(exception_ptr)>,
                       const Ice::Current& current)
{
    Ice::Context::const_iterator p = current.ctx.find("retry");
    bool retry = p != current.ctx.end();
    std::thread t(
        [x, y, response, retry]()
        {
            this_thread::sleep_for(chrono::milliseconds(10));
            try
            {
                response(x + y);
            }
            catch(const Ice::ResponseSentException&)
            {
                test(retry);
            }
        });
    t.detach();
}

void
MyObjectI::amdAddWithRetryAsync(int x,
                                int y,
                                function<void(int)> response,
                                function<void(exception_ptr)> error,
                                const Ice::Current& current)
{
    std::thread t(
        [x, y, response]()
        {
            try
            {
                this_thread::sleep_for(chrono::milliseconds(10));
                response(x + y);
            }
            catch(const Ice::ResponseSentException&)
            {
            }
        });
    t.detach();

    Ice::Context::const_iterator p = current.ctx.find("retry");

    if(p == current.ctx.end() || p->second != "no")
    {
        error(make_exception_ptr(MyRetryException()));
    }
}

void
MyObjectI::amdBadAddAsync(int,
                          int,
                          function<void(int)>,
                          function<void(exception_ptr)> error,
                          const Ice::Current&)
{
    std::thread t(
        [error]()
        {
            this_thread::sleep_for(chrono::milliseconds(10));
            error(make_exception_ptr(Test::InvalidInputException()));
        });
    t.detach();
}

void
MyObjectI::amdNotExistAddAsync(int,
                               int,
                               function<void(int)>,
                               function<void(exception_ptr)> error,
                               const Ice::Current&)
{
    std::thread t(
        [error]()
        {
            this_thread::sleep_for(chrono::milliseconds(10));
            error(make_exception_ptr(Ice::ObjectNotExistException(__FILE__, __LINE__)));
        });
    t.detach();
}

void
MyObjectI::amdBadSystemAddAsync(int,
                                int,
                                function<void(int)>,
                                function<void(exception_ptr)> error,
                                const Ice::Current&)
{
    std::thread t(
        [error]()
        {
            this_thread::sleep_for(chrono::milliseconds(10));
            error(make_exception_ptr(MySystemException(__FILE__, __LINE__)));
        });
    t.detach();
}
