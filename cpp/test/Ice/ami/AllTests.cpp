//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceUtil/Random.h>
#include <TestHelper.h>
#include <Test.h>

#include <thread>

using namespace std;

namespace
{

class PingReplyI : public Test::PingReply
{
public:
    PingReplyI() :
        _received(false)
    {
    }

    virtual void reply(const Ice::Current&)
    {
        _received = true;
    }

    bool checkReceived()
    {
        return _received;
    }

private:
    bool _received;
};

using PingReplyIPtr = std::shared_ptr<PingReplyI>;

enum ThrowType { LocalException, UserException, StandardException, OtherException };

ThrowType throwEx[] = { LocalException, UserException, StandardException, OtherException };
auto thrower = [](ThrowType t)
    {
        switch(t)
        {
            case LocalException:
            {
                throw Ice::ObjectNotExistException(__FILE__, __LINE__);
                break;
            }
            case UserException:
            {
                throw Test::TestIntfException();
                break;
            }
            case StandardException:
            {
                throw ::std::bad_alloc();
                break;
            }
            case OtherException:
            {
                throw 99;
                break;
            }
            default:
            {
                assert(false);
                break;
            }
        }
    };
}

void
allTests(Test::TestHelper* helper, bool collocated)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    const string protocol = communicator->getProperties()->getProperty("Ice.Default.Protocol");

    string sref = "test:" + helper->getTestEndpoint();
    auto prx = communicator->stringToProxy(sref);
    test(prx);

    auto p = Ice::uncheckedCast<Test::TestIntfPrx>(prx);

    sref = "testController:" + helper->getTestEndpoint(1);
    prx = communicator->stringToProxy(sref);
    test(prx);

    auto testController = Ice::uncheckedCast<Test::TestIntfControllerPrx>(prx);

    Ice::Context ctx;
    cout << "testing lambda API... " << flush;
    {
        {
            p->ice_isAAsync(Test::TestIntf::ice_staticId(), nullptr);
        }
        {
            promise<bool> promise;
            p->ice_isAAsync(Test::TestIntf::ice_staticId(),
                [&](bool value)
                {
                    promise.set_value(value);
                });
            test(promise.get_future().get());
        }
        {
            promise<bool> promise;
            p->ice_isAAsync(Test::TestIntf::ice_staticId(),
                [&](bool value)
                {
                    promise.set_value(value);
                },
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                });
            test(promise.get_future().get());
        }

        {
            promise<bool> promise;
            p->ice_isAAsync(Test::TestIntf::ice_staticId(),
                [&](bool value)
                {
                    promise.set_value(value);
                },
                nullptr, nullptr, ctx);
            test(promise.get_future().get());
        }

        {
            p->ice_pingAsync(nullptr);
        }
        {
            promise<void> promise;
            p->ice_pingAsync(
                [&]()
                {
                    promise.set_value();
                });
            promise.get_future().get();
        }
        {
            promise<void> promise;
            p->ice_pingAsync(
                [&]()
                {
                    promise.set_value();
                },
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                });
            promise.get_future().get();
        }
        {
            promise<void> promise;
            p->ice_pingAsync(
                [&]()
                {
                    promise.set_value();
                },
                nullptr, nullptr, ctx);
            promise.get_future().get();
        }

        {
            p->ice_idAsync(nullptr);
        }
        {
            promise<string> promise;
            p->ice_idAsync(
                [&](const string& id)
                {
                    promise.set_value(id);
                });
            test(promise.get_future().get() == Test::TestIntf::ice_staticId());
        }
        {
            promise<string> promise;
            p->ice_idAsync(
                [&](const string& id)
                {
                    promise.set_value(id);
                },
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                });
            test(promise.get_future().get() == Test::TestIntf::ice_staticId());
        }
        {
            promise<string> promise;
            p->ice_idAsync(
                [&](const string& id)
                {
                    promise.set_value(id);
                },
                nullptr, nullptr, ctx);
            test(promise.get_future().get() == Test::TestIntf::ice_staticId());
        }

        {
            p->ice_idsAsync(nullptr);
        }
        {
            promise<vector<string>> promise;
            p->ice_idsAsync(
                [&](const vector<string>& ids)
                {
                    promise.set_value(ids);
                });
            test(promise.get_future().get().size() == 2);
        }
        {
            promise<vector<string>> promise;
            p->ice_idsAsync(
                [&](const vector<string>& ids)
                {
                    promise.set_value(ids);
                },
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                });
            test(promise.get_future().get().size() == 2);
        }
        {
            promise<vector<string>> promise;
            p->ice_idsAsync(
                [&](const vector<string>& ids)
                {
                    promise.set_value(ids);
                },
                nullptr, nullptr, ctx);
            test(promise.get_future().get().size() == 2);
        }

        if(!collocated)
        {
            {
                p->ice_getConnectionAsync(nullptr);
            }
            {
                promise<Ice::ConnectionPtr> promise;
                p->ice_getConnectionAsync(
                    [&](const Ice::ConnectionPtr& connection)
                    {
                        promise.set_value(connection);
                    });
                test(promise.get_future().get());
            }
            {
                promise<Ice::ConnectionPtr> promise;
                p->ice_getConnectionAsync(
                    [&](const Ice::ConnectionPtr& connection)
                    {
                        promise.set_value(connection);
                    },
                    [&](exception_ptr ex)
                    {
                        promise.set_exception(ex);
                    });
                test(promise.get_future().get());
            }
        }

        {
            p->opAsync(nullptr);
        }
        {
            promise<void> promise;
            p->opAsync(
                [&]()
                {
                    promise.set_value();
                });
            promise.get_future().get();
        }
        {
            promise<void> promise;
            p->opAsync(
                [&]()
                {
                    promise.set_value();
                },
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                });
            promise.get_future().get();
        }
        {
            promise<void> promise;
            p->opAsync(
                [&]()
                {
                    promise.set_value();
                },
                nullptr, nullptr, ctx);
            promise.get_future().get();
        }

        {
            p->opWithResultAsync(nullptr);
        }
        {
            promise<int> promise;
            p->opWithResultAsync(
                [&](int result)
                {
                    promise.set_value(result);
                });
            test(promise.get_future().get() == 15);
        }
        {
            // Count copies made
            class CopyCounter
            {
            public:
                CopyCounter(int& count) : _count(count)
                {
                }

                CopyCounter(const CopyCounter& o) : _count(o._count)
                {
                    _count++;
                }

                CopyCounter(CopyCounter&&) = default;

            private:
                int& _count;
            };

            int responseCopyCount = 0;
            int errorCopyCount = 0;
            int sentCopyCount = 0;
            CopyCounter responseCopyCounter(responseCopyCount);
            CopyCounter errorCopyCounter(errorCopyCount);
            CopyCounter sentCopyCounter(sentCopyCount);

            promise<int> promise;
            p->opWithResultAsync(
                [&promise, responseCopyCounter](int result)
                {
                    promise.set_value(result);
                },
                [&promise, errorCopyCounter](exception_ptr ex)
                {
                    promise.set_exception(ex);
                },
                [sentCopyCounter](bool)
                {
                    // no-op
                });
            test(promise.get_future().get() == 15);
            test(responseCopyCount == 1);
            test(errorCopyCount == 1);
            test(sentCopyCount == 1);
        }
        {
            promise<int> promise;
            p->opWithResultAsync(
                [&](int result)
                {
                    promise.set_value(result);
                },
                nullptr, nullptr, ctx);
            test(promise.get_future().get() == 15);
        }

        {
            p->opWithUEAsync(nullptr);
        }
        {
            promise<void> promise;
            p->opWithUEAsync(
                nullptr,
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                });

            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Test::TestIntfException&)
            {
            }
        }
        {
            promise<void> promise;
            p->opWithUEAsync(
                [&]()
                {
                    promise.set_value();
                },
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                });

            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Test::TestIntfException&)
            {
            }
        }
        {
            promise<void> promise;
            p->opWithUEAsync(
                nullptr,
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                },
                nullptr, ctx);

            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Test::TestIntfException&)
            {
            }
        }

        {
            p->opWithResultAndUEAsync(nullptr);
        }
        {
            promise<void> promise;
            p->opWithResultAndUEAsync(
                nullptr,
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                });

            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Test::TestIntfException&)
            {
            }
            catch(const Ice::OperationNotExistException&)
            {
            }
        }
        {
            promise<void> promise;
            p->opWithResultAndUEAsync(
                [&](int)
                {
                    promise.set_value();
                },
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                });

            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Test::TestIntfException&)
            {
            }
            catch(const Ice::OperationNotExistException&)
            {
            }
        }
        {
            promise<void> promise;
            p->opWithResultAndUEAsync(
                nullptr,
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                },
                nullptr, ctx);

            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Test::TestIntfException&)
            {
            }
            catch(const Ice::OperationNotExistException&)
            {
            }
        }
    }
    cout << "ok" << endl;

    cout << "testing future API... " << flush;
    {

        test(p->ice_isAAsync(Test::TestIntf::ice_staticId()).get());
        test(p->ice_isAAsync(Test::TestIntf::ice_staticId(), ctx).get());

        p->ice_pingAsync().get();
        p->ice_pingAsync(ctx).get();

        test(p->ice_idsAsync().get().size() == 2);
        test(p->ice_idsAsync(ctx).get().size() == 2);

        if(!collocated)
        {
            test(p->ice_getConnectionAsync().get());
        }

        p->opAsync().get();
        p->opAsync(ctx).get();

        test(p->opWithResultAsync().get() == 15);
        test(p->opWithResultAsync(ctx).get() == 15);

        try
        {
            p->opWithUEAsync().get();
            test(false);
        }
        catch(const Test::TestIntfException&)
        {
        }

        try
        {
            p->opWithUEAsync(ctx).get();
            test(false);
        }
        catch(const Test::TestIntfException&)
        {
        }

        try
        {
            p->opWithResultAndUEAsync().get();
            test(false);
        }
        catch(const Test::TestIntfException&)
        {
        }
        catch(const Ice::OperationNotExistException&)
        {
        }

        try
        {
            p->opWithResultAndUEAsync(ctx).get();
            test(false);
        }
        catch(const Test::TestIntfException&)
        {
        }
        catch(const Ice::OperationNotExistException&)
        {
        }
    }
    cout << "ok" << endl;

    cout << "testing local exceptions with lambda API... " << flush;
    {
        auto indirect = Ice::uncheckedCast<Test::TestIntfPrx>(p->ice_adapterId("dummy"));

        {
            promise<void> promise;
            indirect->opAsync(
                [&]()
                {
                    promise.set_value();
                },
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                });
            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Ice::NoEndpointException&)
            {
            }
        }

        {
            try
            {
                promise<int> promise;
                p->ice_oneway()->opWithResultAsync(
                    [&](int value)
                    {
                        promise.set_value(value);
                    },
                    [&](exception_ptr ex)
                    {
                        promise.set_exception(ex);
                    });
                test(false);
            }
            catch(const Ice::TwowayOnlyException&)
            {
            }
        }

        //
        // Check that CommunicatorDestroyedException is raised directly.
        //
        if(p->ice_getConnection() && protocol != "bt")
        {
            Ice::InitializationData initData;
            initData.properties = communicator->getProperties()->clone();
            Ice::CommunicatorPtr ic = Ice::initialize(initData);
            auto obj = ic->stringToProxy(p->ice_toString());
            auto p2 = Ice::checkedCast<Test::TestIntfPrx>(obj);
            ic->destroy();

            try
            {
                promise<void> promise;
                p2->opAsync(
                    [&]()
                    {
                        promise.set_value();
                    },
                    [&](exception_ptr ex)
                    {
                        promise.set_exception(ex);
                    });
                test(false);
            }
            catch(const Ice::CommunicatorDestroyedException&)
            {
                // Expected.
            }
        }
    }
    cout << "ok" << endl;

    cout << "testing local exceptions with future API... " << flush;
    {
        auto indirect = Ice::uncheckedCast<Test::TestIntfPrx>(p->ice_adapterId("dummy"));
        auto r = indirect->opAsync();
        try
        {
            r.get();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }

        try
        {
            p->ice_oneway()->opWithResultAsync().get();
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }

        //
        // Check that CommunicatorDestroyedException is raised directly.
        //
        if(p->ice_getConnection())
        {
            Ice::InitializationData initData;
            initData.properties = communicator->getProperties()->clone();
            Ice::CommunicatorPtr ic = Ice::initialize(initData);
            auto obj = ic->stringToProxy(p->ice_toString());
            auto p2 = Ice::checkedCast<Test::TestIntfPrx>(obj);
            ic->destroy();

            try
            {
                p2->opAsync();
                test(false);
            }
            catch(const Ice::CommunicatorDestroyedException&)
            {
                // Expected.
            }
        }
    }
    cout << "ok" << endl;

    cout << "testing exception callback with lambda API... " << flush;
    {
        auto i = Ice::uncheckedCast<Test::TestIntfPrx>(p->ice_adapterId("dummy"));

        {
            promise<bool> promise;
            i->ice_isAAsync(Test::TestIntf::ice_staticId(),
                [&](bool value)
                {
                    promise.set_value(value);
                },
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                });
            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Ice::NoEndpointException&)
            {
            }
        }

        {
            promise<void> promise;
            i->opAsync(
                [&]()
                {
                    promise.set_value();
                },
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                });

            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Ice::NoEndpointException&)
            {
            }
        }

        {
            promise<void> promise;
            i->opWithUEAsync(
                [&]()
                {
                    promise.set_value();
                },
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                });

            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Ice::NoEndpointException&)
            {
            }
        }

        // Ensures no exception is called when response is received
        {
            promise<bool> promise;
            p->ice_isAAsync(
                Test::TestIntf::ice_staticId(),
                [&](bool value)
                {
                    promise.set_value(value);
                },
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                });
            try
            {
                test(promise.get_future().get());
            }
            catch(...)
            {
                test(false);
            }
        }

        {
            promise<void> promise;
            p->opAsync(
                [&]()
                {
                    promise.set_value();
                },
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                });
            try
            {
                promise.get_future().get();
            }
            catch(...)
            {
                test(false);
            }
        }

        // If response is a user exception, it should be received.
        {
            promise<void> promise;
            p->opWithUEAsync(
                [&]()
                {
                    promise.set_value();
                },
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                });

            try
            {
                promise.get_future().get();
                test(false);
            }
            catch(const Test::TestIntfException&)
            {
            }
        }
    }
    cout << "ok" << endl;

    cout << "testing sent callback with lambda API... " << flush;
    {
        {
            promise<bool> response;
            promise<bool> sent;

            p->ice_isAAsync("",
                [&](bool value)
                {
                    response.set_value(value);
                },
                [&](exception_ptr ex)
                {
                    response.set_exception(ex);
                },
                [&](bool sentAsync)
                {
                    sent.set_value(sentAsync);
                });

            sent.get_future().get();
            response.get_future().get();
        }

        {
            promise<void> response;
            promise<bool> sent;

            p->ice_pingAsync(
                [&]()
                {
                    response.set_value();
                },
                [&](exception_ptr ex)
                {
                    response.set_exception(ex);
                },
                [&](bool sentAsync)
                {
                    sent.set_value(sentAsync);
                });

            sent.get_future().get();
            response.get_future().get();
        }

        {
            promise<string> response;
            promise<bool> sent;

            p->ice_idAsync(
                [&](string value)
                {
                    response.set_value(value);
                },
                [&](exception_ptr ex)
                {
                    response.set_exception(ex);
                },
                [&](bool sentAsync)
                {
                    sent.set_value(sentAsync);
                });

            sent.get_future().get();
            response.get_future().get();
        }

        {
            promise<vector<string>> response;
            promise<bool> sent;

            p->ice_idsAsync(
                [&](vector<string> value)
                {
                    response.set_value(value);
                },
                [&](exception_ptr ex)
                {
                    response.set_exception(ex);
                },
                [&](bool sentAsync)
                {
                    sent.set_value(sentAsync);
                });

            sent.get_future().get();
            response.get_future().get();
        }

        {
            promise<void> response;
            promise<bool> sent;

            p->opAsync(
                [&]()
                {
                    response.set_value();
                },
                [&](exception_ptr ex)
                {
                    response.set_exception(ex);
                },
                [&](bool sentAsync)
                {
                    sent.set_value(sentAsync);
                });

            sent.get_future().get();
            response.get_future().get();
        }

        vector<future<bool>> futures;
        Ice::ByteSeq seq;
        seq.resize(1024);
        testController->holdAdapter();
        try
        {
            while(true)
            {
                auto s = make_shared<promise<bool>>();
                auto f = s->get_future();
                p->opWithPayloadAsync(
                    seq,
                    [](){},
                    [s](exception_ptr ex)
                    {
                        s->set_exception(ex);
                    },
                    [s](bool value)
                    {
                        s->set_value(value);
                    });

                if(f.wait_for(chrono::seconds(0)) != future_status::ready || !f.get())
                {
                    break;
                }
                futures.push_back(std::move(f));
            }
        }
        catch(...)
        {
            testController->resumeAdapter();
            throw;
        }
        testController->resumeAdapter();
    }
    cout << "ok" << endl;

    cout << "testing unexpected exceptions from callback... " << flush;
    {
        auto q = Ice::uncheckedCast<Test::TestIntfPrx>(p->ice_adapterId("dummy"));

        for(int i = 0; i < 4; ++i)
        {
            {
                promise<void> promise;
                p->opAsync(
                    [&, i]()
                    {
                        promise.set_value();
                        thrower(throwEx[i]);
                    },
                    [&](exception_ptr)
                    {
                        test(false);
                    });

                try
                {
                    promise.get_future().get();
                }
                catch(const exception& ex)
                {
                    cerr << ex.what() << endl;
                    test(false);
                }
            }

            {
                promise<void> promise;
                p->opAsync(nullptr, nullptr,
                    [&, i](bool)
                    {
                        promise.set_value();
                        thrower(throwEx[i]);
                    });

                try
                {
                    promise.get_future().get();
                }
                catch(const exception&)
                {
                    test(false);
                }
            }
        }
    }
    cout << "ok" << endl;

    cout << "testing batch requests with proxy... " << flush;
    {
        {
            test(p->opBatchCount() == 0);
            auto b1 = p->ice_batchOneway();
            b1->opBatchAsync().get();
            b1->opBatch();
            auto id = this_thread::get_id();
            promise<void> promise;
            b1->ice_flushBatchRequestsAsync(
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                },
                [&](bool sentSynchronously)
                {
                    test((sentSynchronously && id == this_thread::get_id()) ||
                         (!sentSynchronously && id != this_thread::get_id()));
                    promise.set_value();
                });
            promise.get_future().get();
            test(p->waitForBatch(2));
        }

        if(p->ice_getConnection() && protocol != "bt")
        {
            test(p->opBatchCount() == 0);
            auto b1 = p->ice_batchOneway();
            b1->opBatch();
            b1->ice_getConnection()->close(Ice::ConnectionClose::GracefullyWithWait);

            auto id = this_thread::get_id();
            promise<void> promise;
            b1->ice_flushBatchRequestsAsync(
                [&](exception_ptr ex)
                {
                    promise.set_exception(ex);
                },
                [&](bool sentSynchronously)
                {
                    test((sentSynchronously && id == this_thread::get_id()) ||
                         (!sentSynchronously && id != this_thread::get_id()));
                    promise.set_value();
                });
            promise.get_future().get();
            test(p->waitForBatch(1));
        }
    }
    cout << "ok" << endl;

    if(p->ice_getConnection()) // No collocation optimization
    {
        cout << "testing batch requests with connection... " << flush;
        {
            {
                test(p->opBatchCount() == 0);
                auto b1 = Ice::uncheckedCast<Test::TestIntfPrx>(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->opBatch();

                auto id = this_thread::get_id();
                promise<void> promise;

                b1->ice_getConnection()->flushBatchRequestsAsync(
                    Ice::CompressBatch::BasedOnProxy,
                    [&](exception_ptr ex)
                    {
                        promise.set_exception(ex);
                    },
                    [&](bool sentSynchronously)
                    {
                        test((sentSynchronously && id == this_thread::get_id()) ||
                            (!sentSynchronously && id != this_thread::get_id()));
                        promise.set_value();
                    });
                promise.get_future().get();
                test(p->waitForBatch(2));
            }

            {
                // Ensure it also works with a twoway proxy
                auto id = this_thread::get_id();
                promise<void> promise;
                p->ice_getConnection()->flushBatchRequestsAsync(
                    Ice::CompressBatch::BasedOnProxy,
                    [&](exception_ptr ex)
                    {
                        promise.set_exception(ex);
                    },
                    [&](bool sentSynchronously)
                    {
                        test((sentSynchronously && id == this_thread::get_id()) ||
                            (!sentSynchronously && id != this_thread::get_id()));
                        promise.set_value();
                    });
                promise.get_future().get();
            }

            if(protocol != "bt")
            {
                test(p->opBatchCount() == 0);
                auto b1 = Ice::uncheckedCast<Test::TestIntfPrx>(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->ice_getConnection()->close(Ice::ConnectionClose::GracefullyWithWait);

                promise<void> promise;
                b1->ice_getConnection()->flushBatchRequestsAsync(
                    Ice::CompressBatch::BasedOnProxy,
                    [&](exception_ptr ex)
                    {
                        promise.set_exception(std::move(ex));
                    },
                    [&](bool)
                    {
                        promise.set_value();
                    });

                try
                {
                    promise.get_future().get();
                    test(false);
                }
                catch(const Ice::LocalException&)
                {
                }
                test(p->opBatchCount() == 0);
            }
        }
        cout << "ok" << endl;

        cout << "testing batch requests with communicator... " << flush;
        {
            {
                //
                // 1 connection. Test future
                //
                test(p->opBatchCount() == 0);
                auto b1 = Ice::uncheckedCast<Test::TestIntfPrx>(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->opBatch();

                communicator->flushBatchRequestsAsync(Ice::CompressBatch::BasedOnProxy).get();
                test(p->waitForBatch(2));
            }

            {
                //
                // 1 connection.
                //
                test(p->opBatchCount() == 0);
                auto b1 = Ice::uncheckedCast<Test::TestIntfPrx>(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->opBatch();

                promise<void> promise;
                auto id = this_thread::get_id();
                communicator->flushBatchRequestsAsync(
                    Ice::CompressBatch::BasedOnProxy,
                    [&](exception_ptr ex)
                    {
                        promise.set_exception(std::move(ex));
                    },
                    [&](bool sentSynchronously)
                    {
                        test((sentSynchronously && id == this_thread::get_id()) ||
                             (!sentSynchronously && id != this_thread::get_id()));
                        promise.set_value();
                    });
                promise.get_future().get();
                test(p->waitForBatch(2));
            }

            if(protocol != "bt")
            {
                //
                // Exception - 1 connection.
                //
                test(p->opBatchCount() == 0);
                auto b1 = Ice::uncheckedCast<Test::TestIntfPrx>(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                b1->opBatch();
                b1->ice_getConnection()->close(Ice::ConnectionClose::GracefullyWithWait);

                promise<void> promise;
                auto id = this_thread::get_id();
                communicator->flushBatchRequestsAsync(
                    Ice::CompressBatch::BasedOnProxy,
                    [&](exception_ptr ex)
                    {
                        promise.set_exception(std::move(ex));
                    },
                    [&](bool sentSynchronously)
                    {
                        test((sentSynchronously && id == this_thread::get_id()) ||
                             (!sentSynchronously && id != this_thread::get_id()));
                        promise.set_value();
                    });
                promise.get_future().get(); // Exceptions are ignored!
                test(p->opBatchCount() == 0);
            }

            if(protocol != "bt")
            {
                //
                // 2 connections.
                //
                test(p->opBatchCount() == 0);
                auto b1 = Ice::uncheckedCast<Test::TestIntfPrx>(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                auto b2 = Ice::uncheckedCast<Test::TestIntfPrx>(
                    p->ice_connectionId("2")->ice_getConnection()->createProxy(
                        p->ice_getIdentity())->ice_batchOneway());
                b2->ice_getConnection(); // Ensure connection is established.
                b1->opBatch();
                b1->opBatch();
                b2->opBatch();
                b2->opBatch();

                promise<void> promise;
                auto id = this_thread::get_id();
                communicator->flushBatchRequestsAsync(
                    Ice::CompressBatch::BasedOnProxy,
                    [&](exception_ptr ex)
                    {
                        promise.set_exception(std::move(ex));
                    },
                    [&](bool sentSynchronously)
                    {
                        test((sentSynchronously && id == this_thread::get_id()) ||
                             (!sentSynchronously && id != this_thread::get_id()));
                        promise.set_value();
                    });
                promise.get_future().get();
                test(p->waitForBatch(4));
            }

            if(protocol != "bt")
            {
                //
                // 2 connections - 2 failures.
                //
                // The sent callback should be invoked even if all connections fail.
                //
                test(p->opBatchCount() == 0);
                auto b1 = Ice::uncheckedCast<Test::TestIntfPrx>(
                    p->ice_getConnection()->createProxy(p->ice_getIdentity())->ice_batchOneway());
                auto b2 = Ice::uncheckedCast<Test::TestIntfPrx>(
                    p->ice_connectionId("2")->ice_getConnection()->createProxy(
                        p->ice_getIdentity())->ice_batchOneway());

                b2->ice_getConnection(); // Ensure connection is established.
                b1->opBatch();
                b2->opBatch();
                b1->ice_getConnection()->close(Ice::ConnectionClose::GracefullyWithWait);
                b2->ice_getConnection()->close(Ice::ConnectionClose::GracefullyWithWait);

                promise<void> promise;
                auto id = this_thread::get_id();
                communicator->flushBatchRequestsAsync(
                    Ice::CompressBatch::BasedOnProxy,
                    [&](exception_ptr ex)
                    {
                        promise.set_exception(std::move(ex));
                    },
                    [&](bool sentSynchronously)
                    {
                        test((sentSynchronously && id == this_thread::get_id()) ||
                             (!sentSynchronously && id != this_thread::get_id()));
                        promise.set_value();
                    });
                promise.get_future().get(); // Exceptions are ignored!
                test(p->opBatchCount() == 0);
            }
        }
        cout << "ok" << endl;

        cout << "testing cancel operations... " << flush;
        {
            if(p->ice_getConnection())
            {
                testController->holdAdapter();
                {
                    promise<void> promise;
                    auto cancel = p->ice_pingAsync(
                        [&]()
                        {
                            promise.set_value();
                        },
                        [&](exception_ptr ex)
                        {
                            promise.set_exception(ex);
                        });
                    cancel();

                    try
                    {
                        promise.get_future().get();
                        test(false);
                    }
                    catch(const Ice::InvocationCanceledException&)
                    {
                    }
                    catch(...)
                    {
                        testController->resumeAdapter();
                        throw;
                    }
                }

                {
                    promise<void> promise;
                    auto cancel = p->ice_idAsync(
                        [&](string)
                        {
                            promise.set_value();
                        },
                        [&](exception_ptr ex)
                        {
                            promise.set_exception(ex);
                        });
                    cancel();

                    try
                    {
                        promise.get_future().get();
                        test(false);
                    }
                    catch(const Ice::InvocationCanceledException&)
                    {
                    }
                    catch(...)
                    {
                        testController->resumeAdapter();
                        throw;
                    }
                }
                testController->resumeAdapter();
            }
        }
        cout << "ok" << endl;

        if(p->ice_getConnection() && protocol != "bt" && p->supportsAMD())
        {
            cout << "testing graceful close connection with wait... " << flush;
            {
                //
                // Local case: begin a request, close the connection gracefully, and make sure it waits
                // for the request to complete.
                //
                auto con = p->ice_getConnection();
                auto sc = make_shared<promise<void>>();
                con->setCloseCallback(
                    [sc](Ice::ConnectionPtr)
                    {
                        sc->set_value();
                    });
                auto fc = sc->get_future();
                auto s = make_shared<promise<void>>();
                p->sleepAsync(100,
                              [s]() { s->set_value(); },
                              [s](exception_ptr ex) { s->set_exception(ex); });
                auto f = s->get_future();
                // Blocks until the request completes.
                con->close(Ice::ConnectionClose::GracefullyWithWait);
                f.get(); // Should complete successfully.
                fc.get();
            }
            {
                //
                // Remote case.
                //
                Ice::ByteSeq seq;
                seq.resize(1024 * 10);
                for(Ice::ByteSeq::iterator q = seq.begin(); q != seq.end(); ++q)
                {
                    *q = static_cast<Ice::Byte>(IceUtilInternal::random(255));
                }

                //
                // Send multiple opWithPayload, followed by a close and followed by multiple opWithPaylod.
                // The goal is to make sure that none of the opWithPayload fail even if the server closes
                // the connection gracefully in between.
                //

                int maxQueue = 2;
                bool done = false;
                while(!done && maxQueue < 5000)
                {
                    done = true;
                    p->ice_ping();
                    vector<future<void>> results;
                    for(int i = 0; i < maxQueue; ++i)
                    {
                        auto s = make_shared<promise<void>>();
                        p->opWithPayloadAsync(seq,
                                              [s]() { s->set_value(); },
                                              [s](exception_ptr ex) { s->set_exception(ex); });
                        results.push_back(s->get_future());
                    }
                    atomic_flag sent = ATOMIC_FLAG_INIT;
                    p->closeAsync(Test::CloseMode::GracefullyWithWait, nullptr, nullptr,
                                  [&sent](bool) { sent.test_and_set(); });
                    if(!sent.test_and_set())
                    {
                        for(int i = 0; i < maxQueue; i++)
                        {
                            auto s = make_shared<promise<void>>();
                            atomic_flag sent2 = ATOMIC_FLAG_INIT;
                            p->opWithPayloadAsync(seq,
                                                  [s]() { s->set_value(); },
                                                  [s](exception_ptr ex) { s->set_exception(ex); },
                                                  [&sent2](bool) { sent2.test_and_set(); });
                            results.push_back(s->get_future());
                            if(sent2.test_and_set())
                            {
                                done = false;
                                maxQueue *= 2;
                                break;
                            }
                        }
                    }
                    else
                    {
                        maxQueue *= 2;
                        done = false;
                    }

                    for(vector<future<void>>::iterator r = results.begin(); r != results.end(); ++r)
                    {
                        try
                        {
                            r->get();
                        }
                        catch(const Ice::LocalException& ex)
                        {
                            cerr << ex << endl;
                            test(false);
                        }
                    }
                }
            }
            cout << "ok" << endl;

            cout << "testing graceful close connection without wait... " << flush;
            {
                //
                // Local case: start an operation and then close the connection gracefully on the client side
                // without waiting for the pending invocation to complete. There will be no retry and we expect the
                // invocation to fail with ConnectionManuallyClosedException.
                //
                p = p->ice_connectionId("CloseGracefully"); // Start with a new connection.
                auto con = p->ice_getConnection();
                auto s = make_shared<promise<void>>();
                auto sent = make_shared<promise<void>>();
                p->startDispatchAsync([s]() { s->set_value(); },
                                      [s](exception_ptr ex) { s->set_exception(ex); },
                                      [sent](bool) { sent->set_value(); });
                auto f = s->get_future();
                sent->get_future().get(); // Ensure the request was sent before we close the connection.
                con->close(Ice::ConnectionClose::Gracefully);
                try
                {
                    f.get();
                    test(false);
                }
                catch(const Ice::ConnectionManuallyClosedException& ex)
                {
                    test(ex.graceful);
                }
                p->finishDispatch();

                //
                // Remote case: the server closes the connection gracefully, which means the connection
                // will not be closed until all pending dispatched requests have completed.
                //
                con = p->ice_getConnection();
                auto sc = make_shared<promise<void>>();
                con->setCloseCallback(
                    [sc](Ice::ConnectionPtr)
                    {
                        sc->set_value();
                    });
                auto fc = sc->get_future();
                s = make_shared<promise<void>>();
                p->sleepAsync(100,
                              [s]() { s->set_value(); },
                              [s](exception_ptr ex) { s->set_exception(ex); });
                f = s->get_future();
                p->close(Test::CloseMode::Gracefully); // Close is delayed until sleep completes.
                fc.get(); // Ensure connection was closed.
                try
                {
                    f.get();
                }
                catch(const Ice::LocalException&)
                {
                    test(false);
                }
            }
            cout << "ok" << endl;

            cout << "testing forceful close connection... " << flush;
            {
                //
                // Local case: start a lengthy operation and then close the connection forcefully on the client side.
                // There will be no retry and we expect the invocation to fail with ConnectionManuallyClosedException.
                //
                p->ice_ping();
                auto con = p->ice_getConnection();
                auto s = make_shared<promise<void>>();
                auto sent = make_shared<promise<void>>();
                p->startDispatchAsync([s]() { s->set_value(); },
                                      [s](exception_ptr ex) { s->set_exception(ex); },
                                      [sent](bool) { sent->set_value(); });
                auto f = s->get_future();
                sent->get_future().get(); // Ensure the request was sent before we close the connection.
                con->close(Ice::ConnectionClose::Forcefully);
                try
                {
                    f.get();
                    test(false);
                }
                catch(const Ice::ConnectionManuallyClosedException& ex)
                {
                    test(!ex.graceful);
                }
                p->finishDispatch();

                //
                // Remote case: the server closes the connection forcefully. This causes the request to fail
                // with a ConnectionLostException. Since the close() operation is not idempotent, the client
                // will not retry.
                //
                try
                {
                    p->close(Test::CloseMode::Forcefully);
                    test(false);
                }
                catch(const Ice::ConnectionLostException&)
                {
                    // Expected.
                }
            }
            cout << "ok" << endl;
        }
    }

    {
        cout << "testing result tuple... " << flush;

        auto q = Ice::uncheckedCast<Test::Outer::Inner::TestIntfPrx>(
            communicator->stringToProxy("test2:" + helper->getTestEndpoint()));

        promise<void> promise;
        q->opAsync(1,
                [&promise](int i, int j)
                {
                    test(i == j);
                    promise.set_value();
                },
                [](exception_ptr ex)
                {
                    try
                    {
                        rethrow_exception(ex);
                    }
                    catch(const std::exception& exc)
                    {
                        cerr << exc.what() << endl;
                    }
                    test(false);
                });
        promise.get_future().get();

        auto f = q->opAsync(1);
        auto r = f.get();
        // TODO: this is not really a good test as we could mix up the two values.
        test(std::get<0>(r) == std::get<1>(r));
        test(std::get<0>(r) == 1);
        cout << "ok" << endl;
    }

    if(p->ice_getConnection())
    {
        cout << "testing bidir... " << flush;
        auto adapter = communicator->createObjectAdapter("");
        auto replyI = make_shared<PingReplyI>();
        auto reply = Ice::uncheckedCast<Test::PingReplyPrx>(adapter->addWithUUID(replyI));
        adapter->activate();

        p->ice_getConnection()->setAdapter(adapter);
        p->pingBiDir(reply);
        test(replyI->checkReceived());
        adapter->destroy();

        cout << "ok" << endl;
    }

    p->shutdown();
}
