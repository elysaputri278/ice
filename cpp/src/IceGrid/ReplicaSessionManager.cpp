//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceGrid/ReplicaSessionManager.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/Database.h>
#include <IceGrid/WellKnownObjectsManager.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

class MasterDatabaseObserverI : public DatabaseObserver
{
public:

    MasterDatabaseObserverI(const shared_ptr<ReplicaSessionManager::Thread>& thread,
                            const shared_ptr<Database>& database,
                            const ReplicaSessionPrxPtr& session) :
        _thread(thread),
        _database(database),
        _session(session)
    {
    }

    void
    applicationInit(int, ApplicationInfoSeq applications, const Ice::Current& current) override
    {
        int serial = 0;
        _database->syncApplications(std::move(applications), getSerials(current.ctx, serial));
        receivedUpdate(TopicName::ApplicationObserver, serial);
    }

    void
    applicationAdded(int, ApplicationInfo application, const Ice::Current& current) override
    {
        int serial = 0;
        string failure;
        try
        {
            _database->addApplication(std::move(application), nullptr, getSerials(current.ctx, serial));
        }
        catch(const DeploymentException& ex)
        {
            ostringstream os;
            os << ex << ":\n" << ex.reason;
            failure = os.str();
        }
        receivedUpdate(TopicName::ApplicationObserver, serial, failure);
    }

    void
    applicationRemoved(int, std::string name, const Ice::Current& current) override
    {
        int serial = 0;
        string failure;
        try
        {
            _database->removeApplication(name, nullptr, getSerials(current.ctx, serial));
        }
        catch(const ApplicationNotExistException& ex)
        {
            ostringstream os;
            os << ex << ":\napplication: " << ex.name;
            failure = os.str();
        }
        receivedUpdate(TopicName::ApplicationObserver, serial, failure);
    }

    void
    applicationUpdated(int, ApplicationUpdateInfo update, const Ice::Current& current) override
    {
        int serial = 0;
        string failure;
        try
        {
            _database->updateApplication(std::move(update), false, nullptr, getSerials(current.ctx, serial));
        }
        catch(const DeploymentException& ex)
        {
            ostringstream os;
            os << ex << ":\n" << ex.reason;
            failure = os.str();
        }
        catch(const ApplicationNotExistException& ex)
        {
            ostringstream os;
            os << ex << ":\napplication: " << ex.name;
            failure = os.str();
        }
        receivedUpdate(TopicName::ApplicationObserver, serial, failure);
    }

    void
    adapterInit(AdapterInfoSeq adapters, const Ice::Current& current) override
    {
        int serial = 0;
        _database->syncAdapters(adapters, getSerials(current.ctx, serial));
        receivedUpdate(TopicName::AdapterObserver, serial);
    }

    void
    adapterAdded(AdapterInfo info, const Ice::Current& current) override
    {
        int serial = 0;
        string failure;
        try
        {
            _database->setAdapterDirectProxy(info.id, info.replicaGroupId, info.proxy, getSerials(current.ctx, serial));
        }
        catch(const AdapterExistsException&)
        {
            failure = "adapter `" + info.id + "' already exists and belongs to an application";
        }
        receivedUpdate(TopicName::AdapterObserver, serial, failure);
    }

    void
    adapterUpdated(AdapterInfo info, const Ice::Current& current) override
    {
        int serial = 0;
        string failure;
        try
        {
            _database->setAdapterDirectProxy(info.id, info.replicaGroupId, info.proxy, getSerials(current.ctx, serial));
        }
        catch(const AdapterExistsException&)
        {
            failure = "adapter `" + info.id + "' already exists and belongs to an application";
        }
        receivedUpdate(TopicName::AdapterObserver, serial, failure);
    }

    void
    adapterRemoved(std::string id, const Ice::Current& current) override
    {
        int serial = 0;
        string failure;
        try
        {
            _database->setAdapterDirectProxy(id, "", nullopt, getSerials(current.ctx, serial));
        }
        catch(const AdapterExistsException&)
        {
            failure = "adapter `" + id + "' already exists and belongs to an application";
        }
        receivedUpdate(TopicName::AdapterObserver, serial, failure);
    }

    void
    objectInit(ObjectInfoSeq objects, const Ice::Current& current) override
    {
        int serial = 0;
        _database->syncObjects(std::move(objects), getSerials(current.ctx, serial));
        receivedUpdate(TopicName::ObjectObserver, serial);
    }

    void
    objectAdded(ObjectInfo info, const Ice::Current& current) override
    {
        int serial = 0;
        string failure;
        try
        {
            _database->addOrUpdateObject(info, getSerials(current.ctx, serial));
        }
        catch(const ObjectExistsException& ex)
        {
            ostringstream os;
            os << ex << ":\n";
            os << "id: " << _database->getCommunicator()->identityToString(info.proxy->ice_getIdentity());
            failure = os.str();
        }
        receivedUpdate(TopicName::ObjectObserver, serial, failure);
    }

    void
    objectUpdated(ObjectInfo info, const Ice::Current& current) override
    {
        int serial = 0;
        string failure;
        try
        {
            _database->addOrUpdateObject(info, getSerials(current.ctx, serial));
        }
        catch(const ObjectExistsException& ex)
        {
            ostringstream os;
            os << ex << ":\n";
            os << "id: " << _database->getCommunicator()->identityToString(info.proxy->ice_getIdentity());
            failure = os.str();
        }
        catch(const DeploymentException& ex)
        {
            ostringstream os;
            os << ex << ":\n" << ex.reason;
            failure = os.str();
        }
        receivedUpdate(TopicName::ObjectObserver, serial, failure);
    }

    void
    objectRemoved(Ice::Identity id, const Ice::Current& current) override
    {
        int serial = 0;
        string failure;
        try
        {
            _database->removeObject(id, getSerials(current.ctx, serial));
        }
        catch(const DeploymentException& ex)
        {
            ostringstream os;
            os << ex << ":\n" << ex.reason;
            failure = os.str();
        }
        catch(const ObjectNotRegisteredException&)
        {
        }
        receivedUpdate(TopicName::ObjectObserver, serial, failure);
    }

private:

    int64_t
    getSerials(const Ice::Context& context, int& serial)
    {
        auto p = context.find("serial");
        if(p != context.end())
        {
            istringstream is(p->second);
            is >> serial;
        }
        else
        {
            serial = -1;
        }

        p = context.find("dbSerial");
        if(p != context.end())
        {
            int64_t dbSerial;
            istringstream is(p->second);
            is >> dbSerial;
            return dbSerial;
        }
        else
        {
            return -1;
        }
    }

    void
    receivedUpdate(TopicName name, int serial, const string& failure = string())
    {
        try
        {
            _session->receivedUpdate(name, serial, failure);
        }
        catch(const Ice::LocalException&)
        {
        }
        if(!failure.empty())
        {
            _thread->destroyActiveSession();
        }
    }

    const shared_ptr<ReplicaSessionManager::Thread> _thread;
    const shared_ptr<Database> _database;
    const ReplicaSessionPrxPtr _session;
};

};

void
ReplicaSessionManager::create(const string& name,
                              const shared_ptr<InternalReplicaInfo>& info,
                              const shared_ptr<Database>& database,
                              const shared_ptr<WellKnownObjectsManager>& wellKnownObjects,
                              const InternalRegistryPrxPtr& internalRegistry)
{
    {
        lock_guard lock(_mutex);

        _name = name;
        _info = info;
        _internalRegistry = internalRegistry;
        _database = database;
        _wellKnownObjects = wellKnownObjects;
        _traceLevels = _database->getTraceLevels();

        _thread = make_shared<Thread>(*this, _master, _traceLevels->logger);
        _condVar.notify_all();
    }

    _thread->tryCreateSession();
    _thread->waitTryCreateSession();
}

void
ReplicaSessionManager::create(const InternalRegistryPrxPtr& replica)
{
    {
        unique_lock lock(_mutex);

        // Wait to be initialized.
        _condVar.wait(lock, [this] { return _master; });
    }

    if(replica->ice_getIdentity() != _master->ice_getIdentity())
    {
        _database->getTraceLevels()->logger->error("can only create sessions with the master replica");
        return;
    }

    _thread->setRegistry(replica);
    _thread->tryCreateSession();
    _thread->waitTryCreateSession();
}

NodePrxSeq
ReplicaSessionManager::getNodes(const NodePrxSeq& nodes) const
{
    assert(_thread && _thread->getRegistry());
    if(_thread->getSession())
    {
        try
        {
            return _thread->getRegistry()->getNodes();
        }
        catch(const Ice::LocalException&)
        {
            return nodes;
        }
    }
    else
    {
        return nodes;
    }
}

void
ReplicaSessionManager::destroy()
{
    shared_ptr<Thread> thread;
    {
        lock_guard lock(_mutex);
        if(!_communicator)
        {
            return;
        }
        thread = _thread;
        _thread = nullptr;

        _communicator = nullptr;
    }

    if(thread)
    {
        thread->terminate();
        thread->join();
    }

    _database = nullptr;
    _wellKnownObjects = nullptr;
}

void
ReplicaSessionManager::registerAllWellKnownObjects()
{
    //
    // Always register first the well-known objects with the
    // database. Then, if there's a session, we register them with the
    // session and this will eventually override the ones with just
    // registered with the ones from the master.
    //
    _wellKnownObjects->registerAll();

    //
    // If there's an active session, register the well-known objects
    // with the session.
    //
    auto session = _thread->getSession();
    if(session)
    {
        try
        {
            _wellKnownObjects->registerAll(session);
            return;
        }
        catch(const Ice::LocalException&)
        {
        }
    }
}

InternalRegistryPrxPtr
ReplicaSessionManager::findInternalRegistryForReplica(const Ice::Identity& id)
{
    vector<future<Ice::ObjectPrxPtr>> results;
    for(const auto& obj : findAllQueryObjects(true))
    {
        results.push_back(obj->findObjectByIdAsync(id));
    }

    for(auto& result : results)
    {
        try
        {
            return Ice::checkedCast<InternalRegistryPrx>(result.get());
        }
        catch(const Ice::Exception&)
        {
        }
    }

    return nullopt;
}

bool
ReplicaSessionManager::keepAlive(const ReplicaSessionPrxPtr& session)
{
    try
    {
        if(_traceLevels && _traceLevels->replica > 2)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
            out << "sending keep alive message to master replica";
        }

        session->keepAlive();
        return true;
    }
    catch(const Ice::LocalException& ex)
    {
        if(_traceLevels && _traceLevels->replica > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
            out << "lost session with master replica:\n" << ex;
        }
        return false;
    }
}

ReplicaSessionPrxPtr
ReplicaSessionManager::createSession(InternalRegistryPrxPtr& registry, chrono::seconds& timeout)
{
    ReplicaSessionPrxPtr session;
    string exceptionMsg = "";
    try
    {
        if(_traceLevels && _traceLevels->replica > 1)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
            out << "trying to establish session with master replica";
        }

        set<InternalRegistryPrxPtr> used;
        if(!registry->ice_getEndpoints().empty())
        {
            try
            {
                session = createSessionImpl(registry, timeout);
            }
            catch(const Ice::LocalException& ex)
            {
                exceptionMsg = ex.what();
                used.insert(registry);
                registry = Ice::uncheckedCast<InternalRegistryPrx>(registry->ice_endpoints({}));
            }
        }

        if(!session)
        {
            vector<future<Ice::ObjectPrxPtr>> results;
            for(const auto& obj : findAllQueryObjects(false))
            {
                results.push_back(obj->findObjectByIdAsync(registry->ice_getIdentity()));
            }

            for(auto& result : results)
            {
                if(isDestroyed())
                {
                    break;
                }

                InternalRegistryPrxPtr newRegistry;
                try
                {
                    auto obj = result.get();
                    newRegistry = Ice::uncheckedCast<InternalRegistryPrx>(obj);
                    if(newRegistry && used.find(newRegistry) == used.end())
                    {
                        session = createSessionImpl(newRegistry, timeout);
                        registry = newRegistry;
                        break;
                    }
                }
                catch(const Ice::LocalException& ex)
                {
                    exceptionMsg = ex.what();
                    if(newRegistry)
                    {
                        used.insert(newRegistry);
                    }
                }
            }
        }
    }
    catch(const ReplicaActiveException& ex)
    {
        if(_traceLevels)
        {
            _traceLevels->logger->error("a replica with the same name is already registered and active");
        }
        exceptionMsg = ex.what();
    }
    catch(const DeploymentException& ex)
    {
        if(_traceLevels)
        {
            _traceLevels->logger->error("database synchronization with master failed:\n" + ex.reason);
        }
        exceptionMsg = ex.what();
    }
    catch(const PermissionDeniedException& ex)
    {
        if(_traceLevels)
        {
            _traceLevels->logger->error("connection to master was denied:\n" + ex.reason);
        }
        exceptionMsg = ex.what();
    }
    catch(const Ice::Exception& ex)
    {
        exceptionMsg = ex.what();
    }

    if(session)
    {
        //
        // Register all the well-known objects with the replica session.
        //
        _wellKnownObjects->registerAll(session);
    }
    else
    {
        //
        // Re-register all the well known objects with the local database.
        //
        _wellKnownObjects->registerAll();
    }

    if(_traceLevels && _traceLevels->replica > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
        if(session)
        {
            out << "established session with master replica";
        }
        else
        {
            out << "failed to establish session with master replica:\n";
            if(exceptionMsg.empty())
            {
                out << "failed to get replica proxy";
            }
            else
            {
                out << exceptionMsg;
            }
        }
    }
    return session;
}

ReplicaSessionPrxPtr
ReplicaSessionManager::createSessionImpl(const InternalRegistryPrxPtr& registry, chrono::seconds& timeout)
{
    ReplicaSessionPrxPtr session;
    try
    {
        session = registry->registerReplica(_info, _internalRegistry);
        auto t = session->getTimeout();
        if(t > 0)
        {
            timeout = chrono::seconds(t / 2);
        }

        //
        // Create a new database observer servant and give its proxy
        // to the session so that it can subscribe it. This call only
        // returns once the observer is subscribed and initialized.
        //
        auto servant = make_shared<MasterDatabaseObserverI>(_thread, _database, session);
        _observer = Ice::uncheckedCast<DatabaseObserverPrx>(_database->getInternalAdapter()->addWithUUID(servant));
        StringLongDict serials = _database->getSerials();
        optional<StringLongDict> serialsOpt;
        if(!serials.empty())
        {
            serialsOpt = serials; // Don't provide serials parameter if serials aren't supported.
        }
        session->setDatabaseObserver(_observer, serialsOpt);
        return session;
    }
    catch(const Ice::Exception&)
    {
        destroySession(session);
        throw;
    }
}

void
ReplicaSessionManager::destroySession(const ReplicaSessionPrxPtr& session)
{
    if(session)
    {
        try
        {
            session->destroy();

            if(_traceLevels && _traceLevels->replica > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
                out << "destroyed master replica session";
            }
        }
        catch(const Ice::LocalException& ex)
        {
            if(_traceLevels && _traceLevels->replica > 1)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
                out << "couldn't destroy master replica session:\n" << ex;
            }
        }
    }

    if(_observer)
    {
        try
        {
            _database->getInternalAdapter()->remove(_observer->ice_getIdentity());
        }
        catch(const Ice::LocalException&)
        {
        }
        _observer = nullopt;
    }
}
