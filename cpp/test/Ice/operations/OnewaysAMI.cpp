//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

namespace
{

class CallbackBase
{
public:

    CallbackBase() :
        _called(false)
    {
    }

    virtual ~CallbackBase()
    {
    }

    void check()
    {
        unique_lock lock(_mutex);
        _condition.wait(lock, [this] { return _called; });
        _called = false;
    }

protected:

    void called()
    {
        lock_guard lock(_mutex);
        assert(!_called);
        _called = true;
        _condition.notify_one();
    }

private:

    mutex _mutex;
    condition_variable _condition;
    bool _called;
};

class Callback : public CallbackBase
{
public:

    Callback()
    {
    }

    void sent(bool)
    {
        called();
    }

    void noException(const Ice::Exception&)
    {
        test(false);
    }
};
using CallbackPtr = std::shared_ptr<Callback>;

}

void
onewaysAMI(const Ice::CommunicatorPtr&, const Test::MyClassPrxPtr& proxy)
{
    Test::MyClassPrxPtr p = Ice::uncheckedCast<Test::MyClassPrx>(proxy->ice_oneway());

    {
        CallbackPtr cb = std::make_shared<Callback>();
        p->ice_pingAsync(
            nullptr,
            [](exception_ptr)
            {
                test(false);
            },
            [&](bool sent)
            {
                cb->sent(sent);
            });
        cb->check();
    }

    {
        try
        {
            p->ice_isAAsync(Test::MyClass::ice_staticId(),
                [&](bool)
                {
                    test(false);
                });
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        try
        {
            p->ice_idAsync(
                [&](string)
                {
                    test(false);
                });
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        try
        {
            p->ice_idsAsync(
                [&](vector<string>)
                {
                });
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        CallbackPtr cb = std::make_shared<Callback>();
        p->opVoidAsync(
            nullptr,
            [](exception_ptr)
            {
                test(false);
            },
            [&](bool sent)
            {
                cb->sent(sent);
            });
        cb->check();
    }

    {
        CallbackPtr cb = std::make_shared<Callback>();
        p->opIdempotentAsync(
            nullptr,
            [](exception_ptr)
            {
                test(false);
            },
            [&](bool sent)
            {
                cb->sent(sent);
            });
        cb->check();
    }

    {
        CallbackPtr cb = std::make_shared<Callback>();
        p->opNonmutatingAsync(
            nullptr,
            [](exception_ptr)
            {
                test(false);
            },
            [&](bool sent)
            {
                cb->sent(sent);
            });
        cb->check();
    }

    {
        try
        {
            p->opByteAsync(Ice::Byte(0xff), Ice::Byte(0x0f),
                [](Ice::Byte, Ice::Byte)
                {
                    test(false);
                });
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        CallbackPtr cb = std::make_shared<Callback>();
        p->ice_pingAsync(nullptr,
                        [=](exception_ptr e)
                        {
                            try
                            {
                                rethrow_exception(e);
                            }
                            catch(const Ice::Exception& ex)
                            {
                                cb->noException(ex);
                            }
                        },
                        [=](bool sent)
                        {
                            cb->sent(sent);
                        });
        cb->check();

    }
    {
        try
        {
            p->ice_isAAsync(Test::MyClass::ice_staticId());
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        try
        {
            p->ice_idAsync();
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        try
        {
            p->ice_idsAsync();
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }
}
