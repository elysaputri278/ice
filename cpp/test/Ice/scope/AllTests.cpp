//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Ice/Locator.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    //
    // Scoped types
    //
    {
        Test::IPrxPtr i = Ice::checkedCast<Test::IPrx>(communicator->stringToProxy("i1:" + helper->getTestEndpoint()));

        Test::S s1;
        s1.v = 0;
        Test::S s2;
        Test::S s3 = i->opS(s1, s2);
        test(s1 == s2);
        test(s1 == s3);

        Test::SSeq sseq1;
        sseq1.push_back(s1);
        Test::SSeq sseq2;
        Test::SSeq sseq3 = i->opSSeq(sseq1, sseq2);
        test(sseq2 == sseq1);
        test(sseq3 == sseq1);

        Test::SMap smap1;
        smap1["a"] = s1;
        Test::SMap smap2;
        Test::SMap smap3 = i->opSMap(smap1, smap2);
        test(smap2 == smap1);
        test(smap3 == smap1);

        Test::CPtr c1 = make_shared<Test::C>(s1);
        Test::CPtr c2;
        Test::CPtr c3 = i->opC(c1, c2);
        test(c2->s == c1->s);
        test(c3->s == c1->s);

        Test::CSeq cseq1;
        cseq1.push_back(c1);
        Test::CSeq cseq2;
        Test::CSeq cseq3 = i->opCSeq(cseq1, cseq2);
        test(cseq2[0]->s == c1->s);
        test(cseq3[0]->s == c1->s);

        Test::CMap cmap1;
        cmap1["a"] = c1;
        Test::CMap cmap2;
        Test::CMap cmap3 = i->opCMap(cmap1, cmap2);
        test(cmap2["a"]->s == c1->s);
        test(cmap3["a"]->s == c1->s);

        Test::E1 e = i->opE1(Test::E1::v1);
        test(e == Test::E1::v1);

        Test::S1 s;
        s.s = "S1";
        s = i->opS1(s);
        test(s.s == "S1");

        Test::C1Ptr c = i->opC1(make_shared<Test::C1>("C1"));
        test(c->s == "C1");
    }

    //
    // C++ 11 Future-Based Async Function
    //
    {
        Test::IPrxPtr i = Ice::checkedCast<Test::IPrx>(communicator->stringToProxy("i1:" + helper->getTestEndpoint()));

        Test::S s1;
        s1.v = 0;
        {
            auto result = i->opSAsync(s1).get();
            test(std::get<0>(result) == s1);
            test(std::get<1>(result) == s1);
        }

        Test::SSeq sseq1;
        sseq1.push_back(s1);
        {
            auto result = i->opSSeqAsync(sseq1).get();
            test(std::get<0>(result) == sseq1);
            test(std::get<1>(result) == sseq1);
        }

        Test::SMap smap1;
        smap1["a"] = s1;
        {
            auto result = i->opSMapAsync(smap1).get();
            test(std::get<0>(result) == smap1);
            test(std::get<1>(result) == smap1);
        }

        Test::CPtr c1 =  make_shared<Test::C>(s1);
        {
            auto result = i->opCAsync(c1).get();
            test(Ice::targetEqualTo(std::get<0>(result), c1));
            test(Ice::targetEqualTo(std::get<1>(result), c1));
        }

        Test::CSeq cseq1;
        cseq1.push_back(c1);
        {
            auto result = i->opCSeqAsync(cseq1).get();
            test(Ice::targetEqualTo(std::get<0>(result)[0], c1));
            test(Ice::targetEqualTo(std::get<1>(result)[0], c1));
        }

        Test::CMap cmap1;
        cmap1["a"] = c1;
        {
            auto result = i->opCMapAsync(cmap1).get();
            test(Ice::targetEqualTo(std::get<0>(result)["a"], c1));
            test(Ice::targetEqualTo(std::get<1>(result)["a"], c1));
        }

        {
            auto result = i->opE1Async(Test::E1::v1).get();
            test(result == Test::E1::v1);
        }

        {
            Test::S1 s;
            s.s = "S1";
            s = i->opS1Async(s).get();
            test(s.s == "S1");
        }

        {
            auto result = i->opC1Async(make_shared<Test::C1>("C1")).get();
            test(result->s == "C1");
        }
    }

    //
    // C++11 Callback-Based Async Function
    //
    {
        Test::IPrxPtr i = Ice::checkedCast<Test::IPrx>(communicator->stringToProxy("i1:" + helper->getTestEndpoint()));

        Test::S s1;
        s1.v = 0;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSAsync(s1,
                                      [&p, &s1](Test::S s2, Test::S s3)
                                      {
                                          test(s2 == s1);
                                          test(s3 == s1);
                                          p.set_value();
                                      },
                                      [&p](exception_ptr e)
                                      {
                                          p.set_exception(e);
                                      });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::SSeq sseq1;
        sseq1.push_back(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSSeqAsync(sseq1,
                                         [&p, &sseq1](Test::SSeq s2, Test::SSeq s3)
                                         {
                                             test(s2 == sseq1);
                                             test(s3 == sseq1);
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::SMap smap1;
        smap1["a"] = s1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSMapAsync(smap1,
                                         [&p, &smap1](Test::SMap s2, Test::SMap s3)
                                         {
                                             test(s2 == smap1);
                                             test(s3 == smap1);
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::CPtr c1 = make_shared<Test::C>(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCAsync(c1,
                                      [&p, &c1](Test::CPtr c2, Test::CPtr c3)
                                      {
                                          test(Ice::targetEqualTo(c2, c1));
                                          test(Ice::targetEqualTo(c3, c1));
                                          p.set_value();
                                      },
                                      [&p](exception_ptr e)
                                      {
                                          p.set_exception(e);
                                      });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::CSeq cseq1;
        cseq1.push_back(c1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCSeqAsync(cseq1,
                                         [&p, c1](Test::CSeq c2, Test::CSeq c3)
                                         {
                                             test(Ice::targetEqualTo(c2[0], c1));
                                             test(Ice::targetEqualTo(c3[0], c1));
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::CMap cmap1;
        cmap1["a"] = c1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCMapAsync(cmap1,
                                         [&p, c1](Test::CMap c2, Test::CMap c3)
                                         {
                                             test(Ice::targetEqualTo(c2["a"], c1));
                                             test(Ice::targetEqualTo(c3["a"], c1));
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opE1Async(Test::E1::v1,
                                       [&p](Test::E1 v)
                                       {
                                           test(v == Test::E1::v1);
                                           p.set_value();
                                       },
                                       [&p](exception_ptr e)
                                       {
                                           p.set_exception(e);
                                       });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        {
            Test::S1 s;
            s.s = "S1";
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opS1Async(s,
                                       [&p](Test::S1 v)
                                       {
                                           test(v.s == "S1");
                                           p.set_value();
                                       },
                                       [&p](exception_ptr e)
                                       {
                                           p.set_exception(e);
                                       });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opC1Async(make_shared<Test::C1>("C1"),
                                       [&p](Test::C1Ptr v)
                                       {
                                           test(v->s == "C1");
                                           p.set_value();
                                       },
                                       [&p](exception_ptr e)
                                       {
                                           p.set_exception(e);
                                       });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }
    }

    {
        Test::Inner::Inner2::IPrxPtr i =
            Ice::checkedCast<Test::Inner::Inner2::IPrx>(communicator->stringToProxy("i2:" + helper->getTestEndpoint()));

        Test::Inner::Inner2::S s1;
        s1.v = 0;
        Test::Inner::Inner2::S s2;
        Test::Inner::Inner2::S s3 = i->opS(s1, s2);
        test(s1 == s2);
        test(s1 == s3);

        Test::Inner::Inner2::SSeq sseq1;
        sseq1.push_back(s1);
        Test::Inner::Inner2::SSeq sseq2;
        Test::Inner::Inner2::SSeq sseq3 = i->opSSeq(sseq1, sseq2);
        test(sseq2 == sseq1);
        test(sseq3 == sseq1);

        Test::Inner::Inner2::SMap smap1;
        smap1["a"] = s1;
        Test::Inner::Inner2::SMap smap2;
        Test::Inner::Inner2::SMap smap3 = i->opSMap(smap1, smap2);
        test(smap2 == smap1);
        test(smap3 == smap1);

        Test::Inner::Inner2::CPtr c1 = make_shared<Test::Inner::Inner2::C>(s1);
        Test::Inner::Inner2::CPtr c2;
        Test::Inner::Inner2::CPtr c3 = i->opC(c1, c2);
        test(c2->s == c1->s);
        test(c3->s == c1->s);

        Test::Inner::Inner2::CSeq cseq1;
        cseq1.push_back(c1);
        Test::Inner::Inner2::CSeq cseq2;
        Test::Inner::Inner2::CSeq cseq3 = i->opCSeq(cseq1, cseq2);
        test(cseq2[0]->s == c1->s);
        test(cseq3[0]->s == c1->s);

        Test::Inner::Inner2::CMap cmap1;
        cmap1["a"] = c1;
        Test::Inner::Inner2::CMap cmap2;
        Test::Inner::Inner2::CMap cmap3 = i->opCMap(cmap1, cmap2);
        test(cmap2["a"]->s == c1->s);
        test(cmap3["a"]->s == c1->s);
    }

    //
    // C++ 11 Future-Based Async Function
    //
    {
        Test::Inner::Inner2::IPrxPtr i =
            Ice::checkedCast<Test::Inner::Inner2::IPrx>(communicator->stringToProxy("i2:" + helper->getTestEndpoint()));

        Test::Inner::Inner2::S s1;
        s1.v = 0;
        {
            auto result = i->opSAsync(s1).get();
            test(std::get<0>(result) == s1);
            test(std::get<1>(result) == s1);
        }

        Test::Inner::Inner2::SSeq sseq1;
        sseq1.push_back(s1);
        {
            auto result = i->opSSeqAsync(sseq1).get();
            test(std::get<0>(result) == sseq1);
            test(std::get<1>(result) == sseq1);
        }

        Test::Inner::Inner2::SMap smap1;
        smap1["a"] = s1;
        {
            auto result = i->opSMapAsync(smap1).get();
            test(std::get<0>(result) == smap1);
            test(std::get<1>(result) == smap1);
        }

        Test::Inner::Inner2::CPtr c1 = make_shared<Test::Inner::Inner2::C>(s1);
        {
            auto result = i->opCAsync(c1).get();
            test(Ice::targetEqualTo(std::get<0>(result), c1));
            test(Ice::targetEqualTo(std::get<1>(result), c1));
        }

        Test::Inner::Inner2::CSeq cseq1;
        cseq1.push_back(c1);
        {
            auto result = i->opCSeqAsync(cseq1).get();
            test(Ice::targetEqualTo(std::get<0>(result)[0], c1));
            test(Ice::targetEqualTo(std::get<1>(result)[0], c1));
        }

        Test::Inner::Inner2::CMap cmap1;
        cmap1["a"] = c1;
        {
            auto result = i->opCMapAsync(cmap1).get();
            test(Ice::targetEqualTo(std::get<0>(result)["a"], c1));
            test(Ice::targetEqualTo(std::get<1>(result)["a"], c1));
        }
    }

    //
    // C++11 Callback-Based Async Function
    //
    {
        Test::Inner::Inner2::IPrxPtr i =
            Ice::checkedCast<Test::Inner::Inner2::IPrx>(communicator->stringToProxy("i2:" + helper->getTestEndpoint()));

        Test::Inner::Inner2::S s1;
        s1.v = 0;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSAsync(s1,
                                      [&p, &s1](Test::Inner::Inner2::S s2, Test::Inner::Inner2::S s3)
                                      {
                                          test(s2 == s1);
                                          test(s3 == s1);
                                          p.set_value();
                                      },
                                      [&p](exception_ptr e)
                                      {
                                          p.set_exception(e);
                                      });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::SSeq sseq1;
        sseq1.push_back(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSSeqAsync(sseq1,
                                         [&p, &sseq1](Test::Inner::Inner2::SSeq s2, Test::Inner::Inner2::SSeq s3)
                                         {
                                             test(s2 == sseq1);
                                             test(s3 == sseq1);
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::SMap smap1;
        smap1["a"] = s1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSMapAsync(smap1,
                                         [&p, &smap1](Test::Inner::Inner2::SMap s2, Test::Inner::Inner2::SMap s3)
                                         {
                                             test(s2 == smap1);
                                             test(s3 == smap1);
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        auto c1 = make_shared<Test::Inner::Inner2::C>(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCAsync(c1,
                                      [&p, &c1](shared_ptr<Test::Inner::Inner2::C> c2,
                                                shared_ptr<Test::Inner::Inner2::C> c3)
                                      {
                                          test(Ice::targetEqualTo(c2, c1));
                                          test(Ice::targetEqualTo(c3, c1));
                                          p.set_value();
                                      },
                                      [&p](exception_ptr e)
                                      {
                                          p.set_exception(e);
                                      });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::CSeq cseq1;
        cseq1.push_back(c1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCSeqAsync(cseq1,
                                         [&p, c1](Test::Inner::Inner2::CSeq c2, Test::Inner::Inner2::CSeq c3)
                                         {
                                             test(Ice::targetEqualTo(c2[0], c1));
                                             test(Ice::targetEqualTo(c3[0], c1));
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::CMap cmap1;
        cmap1["a"] = c1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCMapAsync(cmap1,
                                         [&p, c1](Test::Inner::Inner2::CMap c2, Test::Inner::Inner2::CMap c3)
                                         {
                                             test(Ice::targetEqualTo(c2["a"], c1));
                                             test(Ice::targetEqualTo(c3["a"], c1));
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }
    }

    {
        Test::Inner::IPrxPtr i =
            Ice::checkedCast<Test::Inner::IPrx>(communicator->stringToProxy("i3:" + helper->getTestEndpoint()));

        Test::Inner::Inner2::S s1;
        s1.v = 0;
        Test::Inner::Inner2::S s2;
        Test::Inner::Inner2::S s3 = i->opS(s1, s2);
        test(s1 == s2);
        test(s1 == s3);

        Test::Inner::Inner2::SSeq sseq1;
        sseq1.push_back(s1);
        Test::Inner::Inner2::SSeq sseq2;
        Test::Inner::Inner2::SSeq sseq3 = i->opSSeq(sseq1, sseq2);
        test(sseq2 == sseq1);
        test(sseq3 == sseq1);

        Test::Inner::Inner2::SMap smap1;
        smap1["a"] = s1;
        Test::Inner::Inner2::SMap smap2;
        Test::Inner::Inner2::SMap smap3 = i->opSMap(smap1, smap2);
        test(smap2 == smap1);
        test(smap3 == smap1);

        Test::Inner::Inner2::CPtr c1 = make_shared<Test::Inner::Inner2::C>(s1);
        Test::Inner::Inner2::CPtr c2;
        Test::Inner::Inner2::CPtr c3 = i->opC(c1, c2);
        test(c2->s == c1->s);
        test(c3->s == c1->s);

        Test::Inner::Inner2::CSeq cseq1;
        cseq1.push_back(c1);
        Test::Inner::Inner2::CSeq cseq2;
        Test::Inner::Inner2::CSeq cseq3 = i->opCSeq(cseq1, cseq2);
        test(cseq2[0]->s == c1->s);
        test(cseq3[0]->s == c1->s);

        Test::Inner::Inner2::CMap cmap1;
        cmap1["a"] = c1;
        Test::Inner::Inner2::CMap cmap2;
        Test::Inner::Inner2::CMap cmap3 = i->opCMap(cmap1, cmap2);
        test(cmap2["a"]->s == c1->s);
        test(cmap3["a"]->s == c1->s);
    }

    //
    // C++ 11 Future-Based Async Function
    //
    {
        Test::Inner::IPrxPtr i =
            Ice::checkedCast<Test::Inner::IPrx>(communicator->stringToProxy("i3:" + helper->getTestEndpoint()));

        Test::Inner::Inner2::S s1;
        s1.v = 0;
        {
            auto result = i->opSAsync(s1).get();
            test(std::get<0>(result) == s1);
            test(std::get<1>(result) == s1);
        }

        Test::Inner::Inner2::SSeq sseq1;
        sseq1.push_back(s1);
        {
            auto result = i->opSSeqAsync(sseq1).get();
            test(std::get<0>(result) == sseq1);
            test(std::get<1>(result) == sseq1);
        }

        Test::Inner::Inner2::SMap smap1;
        smap1["a"] = s1;
        {
            auto result = i->opSMapAsync(smap1).get();
            test(std::get<0>(result) == smap1);
            test(std::get<1>(result) == smap1);
        }

        Test::Inner::Inner2::CPtr c1 = make_shared<Test::Inner::Inner2::C>(s1);
        {
            auto result = i->opCAsync(c1).get();
            test(Ice::targetEqualTo(std::get<0>(result), c1));
            test(Ice::targetEqualTo(std::get<1>(result), c1));
        }

        Test::Inner::Inner2::CSeq cseq1;
        cseq1.push_back(c1);
        {
            auto result = i->opCSeqAsync(cseq1).get();
            test(Ice::targetEqualTo(std::get<0>(result)[0], c1));
            test(Ice::targetEqualTo(std::get<1>(result)[0], c1));
        }

        Test::Inner::Inner2::CMap cmap1;
        cmap1["a"] = c1;
        {
            auto result = i->opCMapAsync(cmap1).get();
            test(Ice::targetEqualTo(std::get<0>(result)["a"], c1));
            test(Ice::targetEqualTo(std::get<1>(result)["a"], c1));
        }
    }

    //
    // C++11 Callback-Based Async Function
    //
    {
        Test::Inner::IPrxPtr i =
            Ice::checkedCast<Test::Inner::IPrx>(communicator->stringToProxy("i3:" + helper->getTestEndpoint()));

        Test::Inner::Inner2::S s1;
        s1.v = 0;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSAsync(s1,
                                      [&p, &s1](Test::Inner::Inner2::S s2, Test::Inner::Inner2::S s3)
                                      {
                                          test(s2 == s1);
                                          test(s3 == s1);
                                          p.set_value();
                                      },
                                      [&p](exception_ptr e)
                                      {
                                          p.set_exception(e);
                                      });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::SSeq sseq1;
        sseq1.push_back(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSSeqAsync(sseq1,
                                         [&p, &sseq1](Test::Inner::Inner2::SSeq s2, Test::Inner::Inner2::SSeq s3)
                                         {
                                             test(s2 == sseq1);
                                             test(s3 == sseq1);
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::SMap smap1;
        smap1["a"] = s1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSMapAsync(smap1,
                                         [&p, &smap1](Test::Inner::Inner2::SMap s2, Test::Inner::Inner2::SMap s3)
                                         {
                                             test(s2 == smap1);
                                             test(s3 == smap1);
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        auto c1 = make_shared<Test::Inner::Inner2::C>(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCAsync(c1,
                                      [&p, &c1](shared_ptr<Test::Inner::Inner2::C> c2,
                                                shared_ptr<Test::Inner::Inner2::C> c3)
                                      {
                                          test(Ice::targetEqualTo(c2, c1));
                                          test(Ice::targetEqualTo(c3, c1));
                                          p.set_value();
                                      },
                                      [&p](exception_ptr e)
                                      {
                                          p.set_exception(e);
                                      });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::CSeq cseq1;
        cseq1.push_back(c1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCSeqAsync(cseq1,
                                         [&p, c1](Test::Inner::Inner2::CSeq c2, Test::Inner::Inner2::CSeq c3)
                                         {
                                             test(Ice::targetEqualTo(c2[0], c1));
                                             test(Ice::targetEqualTo(c3[0], c1));
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::Inner::Inner2::CMap cmap1;
        cmap1["a"] = c1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCMapAsync(cmap1,
                                         [&p, c1](Test::Inner::Inner2::CMap c2, Test::Inner::Inner2::CMap c3)
                                         {
                                             test(Ice::targetEqualTo(c2["a"], c1));
                                             test(Ice::targetEqualTo(c3["a"], c1));
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }
    }

    {
        Inner::Test::Inner2::IPrxPtr i =
            Ice::checkedCast<Inner::Test::Inner2::IPrx>(communicator->stringToProxy("i4:" + helper->getTestEndpoint()));

        Test::S s1;
        s1.v = 0;
        Test::S s2;
        Test::S s3 = i->opS(s1, s2);
        test(s1 == s2);
        test(s1 == s3);

        Test::SSeq sseq1;
        sseq1.push_back(s1);
        Test::SSeq sseq2;
        Test::SSeq sseq3 = i->opSSeq(sseq1, sseq2);
        test(sseq2 == sseq1);
        test(sseq3 == sseq1);

        Test::SMap smap1;
        smap1["a"] = s1;
        Test::SMap smap2;
        Test::SMap smap3 = i->opSMap(smap1, smap2);
        test(smap2 == smap1);
        test(smap3 == smap1);

        Test::CPtr c1 = make_shared<Test::C>(s1);
        Test::CPtr c2;
        Test::CPtr c3 = i->opC(c1, c2);
        test(c2->s == c1->s);
        test(c3->s == c1->s);

        Test::CSeq cseq1;
        cseq1.push_back(c1);
        Test::CSeq cseq2;
        Test::CSeq cseq3 = i->opCSeq(cseq1, cseq2);
        test(cseq2[0]->s == c1->s);
        test(cseq3[0]->s == c1->s);

        Test::CMap cmap1;
        cmap1["a"] = c1;
        Test::CMap cmap2;
        Test::CMap cmap3 = i->opCMap(cmap1, cmap2);
        test(cmap2["a"]->s == c1->s);
        test(cmap3["a"]->s == c1->s);
    }

    //
    // C++ 11 Future-Based Async Function
    //
    {
        Inner::Test::Inner2::IPrxPtr i =
            Ice::checkedCast<Inner::Test::Inner2::IPrx>(communicator->stringToProxy("i4:" + helper->getTestEndpoint()));

        Test::S s1;
        s1.v = 0;
        {
            auto result = i->opSAsync(s1).get();
            test(std::get<0>(result) == s1);
            test(std::get<1>(result) == s1);
        }

        Test::SSeq sseq1;
        sseq1.push_back(s1);
        {
            auto result = i->opSSeqAsync(sseq1).get();
            test(std::get<0>(result) == sseq1);
            test(std::get<1>(result) == sseq1);
        }

        Test::SMap smap1;
        smap1["a"] = s1;
        {
            auto result = i->opSMapAsync(smap1).get();
            test(std::get<0>(result) == smap1);
            test(std::get<1>(result) == smap1);
        }

        Test::CPtr c1 = make_shared<Test::C>(s1);
        {
            auto result = i->opCAsync(c1).get();
            test(Ice::targetEqualTo(std::get<0>(result), c1));
            test(Ice::targetEqualTo(std::get<1>(result), c1));
        }

        Test::CSeq cseq1;
        cseq1.push_back(c1);
        {
            auto result = i->opCSeqAsync(cseq1).get();
            test(Ice::targetEqualTo(std::get<0>(result)[0], c1));
            test(Ice::targetEqualTo(std::get<1>(result)[0], c1));
        }

        Test::CMap cmap1;
        cmap1["a"] = c1;
        {
            auto result = i->opCMapAsync(cmap1).get();
            test(Ice::targetEqualTo(std::get<0>(result)["a"], c1));
            test(Ice::targetEqualTo(std::get<1>(result)["a"], c1));
        }
    }

    //
    // C++11 Callback-Based Async Function
    //
    {
        Inner::Test::Inner2::IPrxPtr i =
            Ice::checkedCast<Inner::Test::Inner2::IPrx>(communicator->stringToProxy("i4:" + helper->getTestEndpoint()));

        Test::S s1;
        s1.v = 0;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSAsync(s1,
                                      [&p, &s1](Test::S s2, Test::S s3)
                                      {
                                          test(s2 == s1);
                                          test(s3 == s1);
                                          p.set_value();
                                      },
                                      [&p](exception_ptr e)
                                      {
                                          p.set_exception(e);
                                      });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::SSeq sseq1;
        sseq1.push_back(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSSeqAsync(sseq1,
                                         [&p, &sseq1](Test::SSeq s2, Test::SSeq s3)
                                         {
                                             test(s2 == sseq1);
                                             test(s3 == sseq1);
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::SMap smap1;
        smap1["a"] = s1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opSMapAsync(smap1,
                                         [&p, &smap1](Test::SMap s2, Test::SMap s3)
                                         {
                                             test(s2 == smap1);
                                             test(s3 == smap1);
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        auto c1 = make_shared<Test::C>(s1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCAsync(c1,
                                      [&p, &c1](shared_ptr<Test::C> c2,
                                                shared_ptr<Test::C> c3)
                                      {
                                          test(Ice::targetEqualTo(c2, c1));
                                          test(Ice::targetEqualTo(c3, c1));
                                          p.set_value();
                                      },
                                      [&p](exception_ptr e)
                                      {
                                          p.set_exception(e);
                                      });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::CSeq cseq1;
        cseq1.push_back(c1);
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCSeqAsync(cseq1,
                                         [&p, c1](Test::CSeq c2, Test::CSeq c3)
                                         {
                                             test(Ice::targetEqualTo(c2[0], c1));
                                             test(Ice::targetEqualTo(c3[0], c1));
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }

        Test::CMap cmap1;
        cmap1["a"] = c1;
        {
            promise<void> p;
            auto f = p.get_future();
            auto result = i->opCMapAsync(cmap1,
                                         [&p, c1](Test::CMap c2, Test::CMap c3)
                                         {
                                             test(Ice::targetEqualTo(c2["a"], c1));
                                             test(Ice::targetEqualTo(c3["a"], c1));
                                             p.set_value();
                                         },
                                         [&p](exception_ptr e)
                                         {
                                             p.set_exception(e);
                                         });

            try
            {
                f.get();
            }
            catch(const exception& ex)
            {
                cerr << ex.what() << endl;
                test(false);
            }
        }
    }

    Test::IPrxPtr i = Ice::checkedCast<Test::IPrx>(communicator->stringToProxy("i1:" + helper->getTestEndpoint()));
    i->shutdown();
}
