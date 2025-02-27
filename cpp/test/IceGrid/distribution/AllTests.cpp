//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceGrid/IceGrid.h>
#include <TestHelper.h>
#include <Test.h>

#include <iterator>

using namespace std;
using namespace Test;
using namespace IceGrid;

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    IceGrid::RegistryPrxPtr registry = Ice::checkedCast<IceGrid::RegistryPrx>(
        communicator->stringToProxy(communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry"));
    test(registry);
    AdminSessionPrxPtr session = registry->createAdminSession("foo", "bar");

    session->ice_getConnection()->setACM(registry->getACMTimeout(),
                                         nullopt,
                                         Ice::ACMHeartbeat::HeartbeatAlways);

    AdminPrxPtr admin = session->getAdmin();
    test(admin);

    cout << "testing distributions... " << flush;
    {
        admin->startServer("Test.IcePatch2");
        admin->startServer("IcePatch2-Direct");

        TestIntfPrxPtr test = Ice::uncheckedCast<TestIntfPrx>(communicator->stringToProxy("server-all"));
        test(test->getServerFile("rootfile") == "");

        try
        {
            admin->patchServer("server-all", true);
        }
        catch(const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }

        test(test->getServerFile("rootfile") == "rootfile");
        test(test->getServerFile("dir1/file1") == "dummy-file1");
        test(test->getServerFile("dir1/file2") == "dummy-file2");
        test(test->getServerFile("dir2/file3") == "dummy-file3");

        test(test->getApplicationFile("rootfile") == "");
        test(test->getApplicationFile("dir1/file1") == "");
        test(test->getApplicationFile("dir1/file2") == "");
        test(test->getApplicationFile("dir2/file3") == "dummy-file3");

        test = Ice::uncheckedCast<TestIntfPrx>(communicator->stringToProxy("server-all-direct"));

        test(test->getServerFile("rootfile") == "");
        test(test->getServerFile("dir1/file1") == "");
        test(test->getServerFile("dir1/file2") == "");
        test(test->getServerFile("dir2/file3") == "");

        test(test->getApplicationFile("rootfile") == "");
        test(test->getApplicationFile("dir1/file1") == "");
        test(test->getApplicationFile("dir1/file2") == "");
        test(test->getApplicationFile("dir2/file3") == "dummy-file3");

        try
        {
            admin->patchServer("server-all-direct", true);
        }
        catch(const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }

        test(test->getServerFile("rootfile") == "rootfile");
        test(test->getServerFile("dir1/file1") == "dummy-file1");
        test(test->getServerFile("dir1/file2") == "dummy-file2");
        test(test->getServerFile("dir2/file3") == "dummy-file3");

        test(test->getApplicationFile("rootfile") == "");
        test(test->getApplicationFile("dir1/file1") == "");
        test(test->getApplicationFile("dir1/file2") == "");
        test(test->getApplicationFile("dir2/file3") == "dummy-file3");

        try
        {
            admin->patchApplication("Test", true);
        }
        catch(const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }
        test = Ice::uncheckedCast<TestIntfPrx>(communicator->stringToProxy("server-dir1"));

        test(test->getServerFile("rootfile") == "");
        test(test->getServerFile("dir1/file1") == "dummy-file1");
        test(test->getServerFile("dir1/file2") == "dummy-file2");
        test(test->getServerFile("dir2/file3") == "");

        test(test->getApplicationFile("rootfile") == "");
        test(test->getApplicationFile("dir1/file1") == "");
        test(test->getApplicationFile("dir1/file2") == "");
        test(test->getApplicationFile("dir2/file3") == "dummy-file3");

        admin->stopServer("Test.IcePatch2");
        admin->stopServer("IcePatch2-Direct");
    }
    cout << "ok" << endl;

    cout << "testing distributions after update... " << flush;
    {
        ApplicationUpdateDescriptor update;
        update.name = "Test";
        update.variables["icepatch.directory"] = "${test.dir}/data/updated";
        admin->updateApplication(update);

        admin->startServer("Test.IcePatch2");
        admin->startServer("IcePatch2-Direct");

        TestIntfPrxPtr test = Ice::uncheckedCast<TestIntfPrx>(communicator->stringToProxy("server-all"));
        test(test->getServerFile("rootfile") == "rootfile");

        try
        {
            admin->patchServer("server-all", true);
        }
        catch(const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }

        test(test->getServerFile("rootfile") == "rootfile-updated!");
        test(test->getServerFile("dir1/file1") == "");
        test(test->getServerFile("dir1/file2") == "dummy-file2-updated!");
        test(test->getServerFile("dir2/file3") == "dummy-file3");
        test(test->getServerFile("dir2/file4") == "dummy-file4");

        test(test->getApplicationFile("rootfile") == "");
        test(test->getApplicationFile("dir1/file1") == "");
        test(test->getApplicationFile("dir1/file2") == "");
        test(test->getApplicationFile("dir2/file3") == "dummy-file3");
        test(test->getApplicationFile("dir2/file4") == "dummy-file4");

        try
        {
            admin->patchServer("server-all-direct", true);
        }
        catch(const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }
        test = Ice::uncheckedCast<TestIntfPrx>(communicator->stringToProxy("server-all-direct"));

        test(test->getServerFile("rootfile") == "rootfile-updated!");
        test(test->getServerFile("dir1/file1") == "");
        test(test->getServerFile("dir1/file2") == "dummy-file2-updated!");
        test(test->getServerFile("dir2/file3") == "dummy-file3");
        test(test->getServerFile("dir2/file4") == "dummy-file4");

        test(test->getApplicationFile("rootfile") == "");
        test(test->getApplicationFile("dir1/file1") == "");
        test(test->getApplicationFile("dir1/file2") == "");
        test(test->getApplicationFile("dir2/file3") == "dummy-file3");
        test(test->getApplicationFile("dir2/file4") == "dummy-file4");

        try
        {
            admin->patchApplication("Test", true);
        }
        catch(const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }
        test = Ice::uncheckedCast<TestIntfPrx>(communicator->stringToProxy("server-dir1"));

        test(test->getServerFile("rootfile") == "");
        test(test->getServerFile("dir1/file1") == "");
        test(test->getServerFile("dir1/file2") == "dummy-file2-updated!");
        test(test->getServerFile("dir2/file3") == "");
        test(test->getServerFile("dir2/file4") == "");

        test(test->getApplicationFile("rootfile") == "");
        test(test->getApplicationFile("dir1/file1") == "");
        test(test->getApplicationFile("dir1/file2") == "");
        test(test->getApplicationFile("dir2/file3") == "dummy-file3");
        test(test->getApplicationFile("dir2/file4") == "dummy-file4");

        admin->stopServer("Test.IcePatch2");
        admin->stopServer("IcePatch2-Direct");
    }
    cout << "ok" << endl;

    cout << "testing application distrib configuration... " << flush;
    try
    {
        ApplicationDescriptor app = admin->getApplicationInfo("Test").descriptor;
        admin->removeApplication("Test");

        app.variables["icepatch.directory"] = "${test.dir}/data/original";
        test(app.nodes["localnode"].servers[2]->id == "server-dir1");
        app.nodes["localnode"].servers[2]->applicationDistrib = false;

        admin->addApplication(app);
        admin->startServer("Test.IcePatch2");

        try
        {
            admin->patchServer("server-dir1", true);
        }
        catch(const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }

        TestIntfPrxPtr test = Ice::uncheckedCast<TestIntfPrx>(communicator->stringToProxy("server-dir1"));

        test(test->getServerFile("rootfile") == "");
        test(test->getServerFile("dir1/file1") == "dummy-file1");
        test(test->getServerFile("dir1/file2") == "dummy-file2");
        test(test->getServerFile("dir2/file3") == "");

        test(test->getApplicationFile("rootfile") == "");
        test(test->getApplicationFile("dir1/file1") == "");
        test(test->getApplicationFile("dir1/file2") == "");
        test(test->getApplicationFile("dir2/file3") == "");

        admin->removeApplication("Test");

        admin->addApplication(app);
        admin->startServer("Test.IcePatch2");
        admin->startServer("IcePatch2-Direct");

        try
        {
            admin->patchApplication("Test", true);
        }
        catch(const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }

        test = Ice::uncheckedCast<TestIntfPrx>(communicator->stringToProxy("server-dir1"));

        test(test->getServerFile("rootfile") == "");
        test(test->getServerFile("dir1/file1") == "dummy-file1");
        test(test->getServerFile("dir1/file2") == "dummy-file2");
        test(test->getServerFile("dir2/file3") == "");

        test(test->getApplicationFile("rootfile") == "");
        test(test->getApplicationFile("dir1/file1") == "");
        test(test->getApplicationFile("dir1/file2") == "");
        test(test->getApplicationFile("dir2/file3") == "dummy-file3");

        admin->removeApplication("Test");

        app.distrib.icepatch = "";

        admin->addApplication(app);
        admin->startServer("Test.IcePatch2");
        admin->startServer("IcePatch2-Direct");

        try
        {
            admin->patchServer("server-dir1", true);
        }
        catch(const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }

        test = Ice::uncheckedCast<TestIntfPrx>(communicator->stringToProxy("server-dir1"));

        test(test->getServerFile("rootfile") == "");
        test(test->getServerFile("dir1/file1") == "dummy-file1");
        test(test->getServerFile("dir1/file2") == "dummy-file2");
        test(test->getServerFile("dir2/file3") == "");

        test(test->getApplicationFile("rootfile") == "");
        test(test->getApplicationFile("dir1/file1") == "");
        test(test->getApplicationFile("dir1/file2") == "");
        test(test->getApplicationFile("dir2/file3") == "");

        test = Ice::uncheckedCast<TestIntfPrx>(communicator->stringToProxy("server-all"));

        test(test->getServerFile("rootfile") == "");
        test(test->getServerFile("dir1/file1") == "");
        test(test->getServerFile("dir1/file2") == "");
        test(test->getServerFile("dir2/file3") == "");

        try
        {
            admin->patchApplication("Test", true);
        }
        catch(const PatchException& ex)
        {
            copy(ex.reasons.begin(), ex.reasons.end(), ostream_iterator<string>(cerr, "\n"));
            test(false);
        }

        test(test->getServerFile("rootfile") == "rootfile");
        test(test->getServerFile("dir1/file1") == "dummy-file1");
        test(test->getServerFile("dir1/file2") == "dummy-file2");
        test(test->getServerFile("dir2/file3") == "dummy-file3");
    }
    catch(const DeploymentException& ex)
    {
        cerr << ex << ":\n" << ex.reason << endl;
        test(false);
    }
    cout << "ok" << endl;

    session->destroy();
}
