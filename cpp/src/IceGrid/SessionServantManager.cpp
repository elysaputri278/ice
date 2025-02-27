//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/UUID.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectAdapter.h>
#include <IceGrid/SessionServantManager.h>
#include <iterator>

using namespace std;
using namespace IceGrid;

SessionServantManager::SessionServantManager(const shared_ptr<Ice::ObjectAdapter>& adapter,
                                             const string& instanceName,
                                             bool checkConnection,
                                             const string& serverAdminCategory,
                                             const shared_ptr<Ice::Object>& serverAdminRouter,
                                             const string& nodeAdminCategory,
                                             const shared_ptr<Ice::Object>& nodeAdminRouter,
                                             const string& replicaAdminCategory,
                                             const shared_ptr<Ice::Object>& replicaAdminRouter,
                                             const shared_ptr<AdminCallbackRouter>& adminCallbackRouter) :
    _adapter(adapter),
    _instanceName(instanceName),
    _checkConnection(checkConnection),
    _serverAdminCategory(serverAdminCategory),
    _serverAdminRouter(serverAdminRouter),
    _nodeAdminCategory(nodeAdminCategory),
    _nodeAdminRouter(nodeAdminRouter),
    _replicaAdminCategory(replicaAdminCategory),
    _replicaAdminRouter(replicaAdminRouter),
    _adminCallbackRouter(adminCallbackRouter)
{
}

shared_ptr<Ice::Object>
SessionServantManager::locate(const Ice::Current& current, shared_ptr<void>&)
{
    lock_guard lock(_mutex);

    shared_ptr<Ice::Object> servant;
    bool plainServant = false;

    if(_serverAdminRouter && current.id.category == _serverAdminCategory)
    {
        servant = _serverAdminRouter;
    }
    else if(_nodeAdminRouter && current.id.category == _nodeAdminCategory)
    {
        servant = _nodeAdminRouter;
    }
    else if(_replicaAdminRouter && current.id.category == _replicaAdminCategory)
    {
        servant = _replicaAdminRouter;
    }
    else
    {
        plainServant = true;

        auto p = _servants.find(current.id);
        if(p == _servants.end() || (_checkConnection && p->second.connection != current.con))
        {
            servant = 0;
        }
        else
        {
            servant = p->second.servant;
        }
    }

    if(!plainServant && servant && _checkConnection &&
       _adminConnections.find(current.con) == _adminConnections.end())
    {
        servant = 0;
    }

    return servant;
}

void
SessionServantManager::finished(const Ice::Current&, const shared_ptr<Ice::Object>&, const shared_ptr<void>&)
{
}

void
SessionServantManager::deactivate(const std::string&)
{
    lock_guard lock(_mutex);
    assert(_servants.empty());
    assert(_sessions.empty());
    assert(_adminConnections.empty());
}

Ice::ObjectPrxPtr
SessionServantManager::addSession(const shared_ptr<Ice::Object>& session,
                                  const shared_ptr<Ice::Connection>& con, const string& category)
{
    lock_guard lock(_mutex);
    _sessions.insert({ session, SessionInfo(con, category) });

    //
    // Keep track of all the connections which have an admin session to allow access
    // to server admin objects.
    //
    if(!category.empty() && con != 0)
    {
        _adminConnections.insert(con);
        if(_adminCallbackRouter != 0)
        {
            _adminCallbackRouter->addMapping(category, con);
        }
    }

    return addImpl(session, session); // Register a servant for the session and return its proxy.
}

void
SessionServantManager::setSessionControl(const shared_ptr<Ice::Object>& session,
                                         const Glacier2::SessionControlPrxPtr& ctl,
                                         const Ice::IdentitySeq& ids)
{
    lock_guard lock(_mutex);

    auto p = _sessions.find(session);
    assert(p != _sessions.end());

    p->second.sessionControl = ctl;
    p->second.identitySet = ctl->identities();

    //
    // Allow invocations on the session servants and the given objects.
    //
    Ice::IdentitySeq allIds = ids;
    copy(p->second.identities.begin(), p->second.identities.end(), back_inserter(allIds));
    p->second.identitySet->add(allIds);

    //
    // Allow invocations on server admin objects.
    //
    if(!p->second.category.empty() && _serverAdminRouter)
    {
        Ice::StringSeq seq;
        seq.push_back(_serverAdminCategory);
        ctl->categories()->add(seq);
    }
}

Glacier2::IdentitySetPrxPtr
SessionServantManager::getGlacier2IdentitySet(const shared_ptr<Ice::Object>& session)
{
    lock_guard lock(_mutex);
    auto p = _sessions.find(session);
    if(p != _sessions.end() && p->second.sessionControl)
    {
        if(!p->second.identitySet) // Cache the identity set proxy
        {
            p->second.identitySet = p->second.sessionControl->identities();
        }
        return p->second.identitySet;
    }
    else
    {
        return nullopt;
    }
}

Glacier2::StringSetPrxPtr
SessionServantManager::getGlacier2AdapterIdSet(const shared_ptr<Ice::Object>& session)
{
    lock_guard lock(_mutex);
    auto p = _sessions.find(session);
    if(p != _sessions.end() && p->second.sessionControl)
    {
        if(!p->second.adapterIdSet) // Cache the adapterId set proxy
        {
            p->second.adapterIdSet = p->second.sessionControl->adapterIds();
        }
        return p->second.adapterIdSet;
    }
    else
    {
        return nullopt;
    }
}

void
SessionServantManager::removeSession(const shared_ptr<Ice::Object>& session)
{
    lock_guard lock(_mutex);

    auto p = _sessions.find(session);
    assert(p != _sessions.end());

    //
    // Remove all the servants associated with the session.
    //
    for(auto q = p->second.identities.cbegin(); q != p->second.identities.cend(); ++q)
    {
        _servants.erase(*q);
    }

    //
    // If this is an admin session, remove its connection from the admin connections.
    //

    if(!p->second.category.empty() && p->second.connection)
    {
        assert(_adminConnections.find(p->second.connection) != _adminConnections.end());
        _adminConnections.erase(_adminConnections.find(p->second.connection));

        if(_adminCallbackRouter != nullptr)
        {
            _adminCallbackRouter->removeMapping(p->second.category);
        }
    }

    _sessions.erase(p);
}

Ice::ObjectPrxPtr
SessionServantManager::add(const shared_ptr<Ice::Object>& servant, const shared_ptr<Ice::Object>& session)
{
    lock_guard lock(_mutex);
    return addImpl(servant, session);
}

void
SessionServantManager::remove(const Ice::Identity& id)
{
    lock_guard lock(_mutex);
    map<Ice::Identity, ServantInfo>::iterator p = _servants.find(id);
    assert(p != _servants.end());

    //
    // Find the session associated to the servant and remove the servant identity from the
    // session identities.
    //
    map<shared_ptr<Ice::Object>, SessionInfo>::iterator q = _sessions.find(p->second.session);
    assert(q != _sessions.end());
    q->second.identities.erase(id);

    //
    // Remove the identity from the Glacier2 identity set.
    //
    if(q->second.identitySet)
    {
        try
        {
            q->second.identitySet->remove({id});
        }
        catch(const Ice::LocalException&)
        {
        }
    }

    //
    // Remove the servant from the servant map.
    //
    _servants.erase(p);
}

Ice::ObjectPrxPtr
SessionServantManager::addImpl(const shared_ptr<Ice::Object>& servant, const shared_ptr<Ice::Object>& session)
{
    auto p = _sessions.find(session);
    assert(p != _sessions.end());

    Ice::Identity id;
    id.name = Ice::generateUUID();
    id.category = _instanceName;

    //
    // Add the identity to the session identities.
    //
    p->second.identities.insert(id);

    //
    // Add the identity to the Glacier2 identity set.
    //
    if(p->second.identitySet)
    {
        try
        {
            p->second.identitySet->add({id});
        }
        catch(const Ice::LocalException&)
        {
        }
    }

    //
    // Add the servant to the servant map and return its proxy.
    //
    _servants.insert(make_pair(id, ServantInfo(servant, p->second.connection, session)));
    return _adapter->createProxy(id);
}
