//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace Ice;
using namespace Test;

void
testExceptions(const TestIntfPrxPtr& obj)
{
    try
    {
        obj->requestFailedException();
        test(false);
    }
    catch(const ObjectNotExistException& ex)
    {
        test(ex.id == obj->ice_getIdentity());
        test(ex.facet == obj->ice_getFacet());
        test(ex.operation == "requestFailedException");
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->unknownUserException();
        test(false);
    }
    catch(const UnknownUserException& ex)
    {
        test(ex.unknown == "reason");
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->unknownLocalException();
        test(false);
    }
    catch(const UnknownLocalException& ex)
    {
        test(ex.unknown == "reason");
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->unknownException();
        test(false);
    }
    catch(const UnknownException& ex)
    {
        test(ex.unknown == "reason");
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->userException();
        test(false);
    }
    catch(const UnknownUserException& ex)
    {
        test(ex.unknown == "::Test::TestIntfUserException");
    }
    catch(const Ice::OperationNotExistException&)
    {
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->localException();
        test(false);
    }
    catch(const UnknownLocalException& ex)
    {
        test(ex.unknown.find("Ice::SocketException") != string::npos ||
             ex.unknown.find("Ice.SocketException") != string::npos);
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->stdException();
        test(false);
    }
    catch(const Ice::OperationNotExistException&)
    {
    }
    catch(const UnknownException& ex)
    {
        test(ex.unknown == "c++ exception: Hello");
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->cppException();
        test(false);
    }
    catch(const UnknownException& ex)
    {
        test(ex.unknown == "c++ exception: unknown");
    }
    catch(const Ice::OperationNotExistException&)
    {
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->unknownExceptionWithServantException();
        test(false);
    }
    catch(const UnknownException& ex)
    {
        test(ex.unknown == "reason");
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->impossibleException(false);
        test(false);
    }
    catch(const UnknownUserException&)
    {
        // Operation doesn't throw, but locate() and finished() throw TestIntfUserException.
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->impossibleException(true);
        test(false);
    }
    catch(const UnknownUserException&)
    {
        // Operation doesn't throw, but locate() and finished() throw TestIntfUserException.
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->intfUserException(false);
        test(false);
    }
    catch(const TestImpossibleException&)
    {
        // Operation doesn't throw, but locate() and finished() throw TestImpossibleException.
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->intfUserException(true);
        test(false);
    }
    catch(const TestImpossibleException&)
    {
        // Operation throws TestIntfUserException, but locate() and finished() throw TestImpossibleException.
    }
    catch(...)
    {
        test(false);
    }
}

TestIntfPrxPtr
allTests(Test::TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();
    const string endp = helper->getTestEndpoint();
    cout << "testing stringToProxy... " << flush;
    ObjectPrxPtr base = communicator->stringToProxy("asm:" + endp);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestIntfPrxPtr obj = Ice::checkedCast<TestIntfPrx>(base);
    test(obj);
    test(obj == base);
    cout << "ok" << endl;

    cout << "testing ice_ids... " << flush;
    try
    {
        ObjectPrxPtr o = communicator->stringToProxy("category/locate:" + endp);
        o->ice_ids();
        test(false);
    }
    catch(const UnknownUserException& ex)
    {
        test(ex.unknown == "::Test::TestIntfUserException");
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        ObjectPrxPtr o = communicator->stringToProxy("category/finished:" + endp);
        o->ice_ids();
        test(false);
    }
    catch(const UnknownUserException& ex)
    {
        test(ex.unknown == "::Test::TestIntfUserException");
    }
    catch(...)
    {
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing servant locator..." << flush;
    base = communicator->stringToProxy("category/locate:" + endp);
    obj = Ice::checkedCast<TestIntfPrx>(base);
    try
    {
        Ice::checkedCast<TestIntfPrx>(communicator->stringToProxy("category/unknown:" + endp));
    }
    catch(const ObjectNotExistException&)
    {
    }
    cout << "ok" << endl;

    cout << "testing default servant locator..." << flush;
    base = communicator->stringToProxy("anothercategory/locate:" + endp);
    obj = Ice::checkedCast<TestIntfPrx>(base);
    base = communicator->stringToProxy("locate:" + endp);
    obj = Ice::checkedCast<TestIntfPrx>(base);
    try
    {
        Ice::checkedCast<TestIntfPrx>(communicator->stringToProxy("anothercategory/unknown:" + endp));
    }
    catch(const ObjectNotExistException&)
    {
    }
    try
    {
        Ice::checkedCast<TestIntfPrx>(communicator->stringToProxy("unknown:" + endp));
    }
    catch(const Ice::ObjectNotExistException&)
    {
    }
    cout << "ok" << endl;

    cout << "testing locate exceptions... " << flush;
    base = communicator->stringToProxy("category/locate:" + endp);
    obj = Ice::checkedCast<TestIntfPrx>(base);
    testExceptions(obj);
    cout << "ok" << endl;

    cout << "testing finished exceptions... " << flush;
    base = communicator->stringToProxy("category/finished:" + endp);
    obj = Ice::checkedCast<TestIntfPrx>(base);
    testExceptions(obj);

    //
    // Only call these for category/finished.
    //
    try
    {
        obj->asyncResponse();
    }
    catch(const TestIntfUserException&)
    {
        test(false);
    }
    catch(const TestImpossibleException&)
    {
        //
        // Called by finished().
        //
    }

    try
    {
        obj->asyncException();
    }
    catch(const TestIntfUserException&)
    {
        test(false);
    }
    catch(const TestImpossibleException&)
    {
        //
        // Called by finished().
        //
    }

    cout << "ok" << endl;

    cout << "testing servant locator removal... " << flush;
    base = communicator->stringToProxy("test/activation:" + endp);
    TestActivationPrxPtr activation = Ice::checkedCast<TestActivationPrx>(base);
    activation->activateServantLocator(false);
    try
    {
        obj->ice_ping();
        test(false);
    }
    catch(const Ice::ObjectNotExistException&)
    {
        cout << "ok" << endl;
    }
    cout << "testing servant locator addition... " << flush;
    activation->activateServantLocator(true);
    try
    {
        obj->ice_ping();
        cout << "ok" << endl;
    }
    catch(...)
    {
        test(false);
    }
    return obj;
}
