//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace Ice;
using namespace Test;

TestIntfPrxPtr
allTests(Test::TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();
    cout << "testing stringToProxy... " << flush;
    ObjectPrxPtr base = communicator->stringToProxy("test:" + helper->getTestEndpoint());
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestIntfPrxPtr obj = Ice::checkedCast<TestIntfPrx>(base);
    test(obj);
    test(obj == base);
    cout << "ok" << endl;

    {
        if(communicator->getProperties()->getProperty("Ice.Default.Protocol") != "ssl" &&
           communicator->getProperties()->getProperty("Ice.Default.Protocol") != "wss")
        {
            cout << "creating/destroying/recreating object adapter... " << flush;
            ObjectAdapterPtr adpt = communicator->createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
            try
            {
                communicator->createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                test(false);
            }
            catch(const AlreadyRegisteredException&)
            {
            }
            adpt->destroy();

            adpt = communicator->createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
            adpt->destroy();
            cout << "ok" << endl;
        }
    }

    cout << "creating/activating/deactivating object adapter in one operation... " << flush;
    obj->transient();
    obj->transientAsync().get();
    cout << "ok" << endl;

    {
        cout << "testing connection closure... " << flush;
        for(int i = 0; i < 10; ++i)
        {
            Ice::InitializationData initData;
            initData.properties = communicator->getProperties()->clone();
            Ice::CommunicatorHolder comm(initData);
            comm->stringToProxy("test:" + helper->getTestEndpoint())->ice_pingAsync();
        }
        cout << "ok" << endl;
    }

    cout << "testing object adapter published endpoints... " << flush;
    {
        communicator->getProperties()->setProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 30000");
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("PAdapter");
        test(adapter->getPublishedEndpoints().size() == 1);
        Ice::EndpointPtr endpt = adapter->getPublishedEndpoints()[0];
        test(endpt->toString() == "tcp -h localhost -p 12345 -t 30000");
        Ice::ObjectPrxPtr prx =
            communicator->stringToProxy("dummy:tcp -h localhost -p 12346 -t 20000:tcp -h localhost -p 12347 -t 10000");
        adapter->setPublishedEndpoints(prx->ice_getEndpoints());
        test(adapter->getPublishedEndpoints().size() == 2);
        Ice::Identity id;
        id.name = "dummy";
        test(adapter->createProxy(id)->ice_getEndpoints() == prx->ice_getEndpoints());
        test(adapter->getPublishedEndpoints() == prx->ice_getEndpoints());
        adapter->refreshPublishedEndpoints();
        test(adapter->getPublishedEndpoints().size() == 1);
        test(*adapter->getPublishedEndpoints()[0] == *endpt);
        communicator->getProperties()->setProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 20000");
        adapter->refreshPublishedEndpoints();
        test(adapter->getPublishedEndpoints().size() == 1);
        test(adapter->getPublishedEndpoints()[0]->toString() == "tcp -h localhost -p 12345 -t 20000");
        adapter->destroy();
        test(adapter->getPublishedEndpoints().empty());
    }
    cout << "ok" << endl;

    if(obj->ice_getConnection())
    {
        cout << "testing object adapter with bi-dir connection... " << flush;
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("");
        obj->ice_getConnection()->setAdapter(adapter);
        obj->ice_getConnection()->setAdapter(nullptr);
        adapter->deactivate();
        try
        {
            obj->ice_getConnection()->setAdapter(adapter);
            test(false);
        }
        catch(const Ice::ObjectAdapterDeactivatedException&)
        {
        }
        cout << "ok" << endl;
    }

    cout << "testing object adapter with router... " << flush;
    {
        Ice::Identity routerId;
        routerId.name = "router";
        auto router = Ice::uncheckedCast<Ice::RouterPrx>(base->ice_identity(routerId)->ice_connectionId("rc"));
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapterWithRouter("", router);
        test(adapter->getPublishedEndpoints().size() == 1);
        test(adapter->getPublishedEndpoints()[0]->toString() == "tcp -h localhost -p 23456 -t 30000");
        adapter->refreshPublishedEndpoints();
        test(adapter->getPublishedEndpoints().size() == 1);
        test(adapter->getPublishedEndpoints()[0]->toString() == "tcp -h localhost -p 23457 -t 30000");
        try
        {
            adapter->setPublishedEndpoints(router->ice_getEndpoints());
            test(false);
        }
        catch(const invalid_argument&)
        {
            // Expected.
        }
        adapter->destroy();

        try
        {
            routerId.name = "test";
            router = Ice::uncheckedCast<Ice::RouterPrx>(base->ice_identity(routerId));
            communicator->createObjectAdapterWithRouter("", router);
            test(false);
        }
        catch(const Ice::OperationNotExistException&)
        {
            // Expected: the "test" object doesn't implement Ice::Router!
        }

        try
        {
            router = Ice::uncheckedCast<Ice::RouterPrx>(
                communicator->stringToProxy("test:" + helper->getTestEndpoint(1)).value());
            communicator->createObjectAdapterWithRouter("", router);
            test(false);
        }
        catch(const Ice::ConnectFailedException&)
        {
        }
        catch(const Ice::ConnectTimeoutException&)
        {
        }
    }
    cout << "ok" << endl;

    cout << "testing object adapter creation with port in use... " << flush;
    {
        Ice::ObjectAdapterPtr adapter1 =
            communicator->createObjectAdapterWithEndpoints("Adpt1", helper->getTestEndpoint(10));
        try
        {
            communicator->createObjectAdapterWithEndpoints("Adpt2", helper->getTestEndpoint(10));
            test(false);
        }
        catch(const Ice::LocalException&)
        {
            // Expected can't re-use the same endpoint.
        }
        adapter1->destroy();
    }
    cout << "ok" << endl;

    cout << "deactivating object adapter in the server... " << flush;
    obj->deactivate();
    cout << "ok" << endl;

    cout << "testing whether server is gone... " << flush;
    try
    {
#ifdef _WIN32
        obj = obj->ice_timeout(100); // Workaround to speed up testing
#endif
        obj->ice_ping();
        test(false);
    }
    catch(const LocalException&)
    {
        cout << "ok" << endl;
    }

    return obj;
}
