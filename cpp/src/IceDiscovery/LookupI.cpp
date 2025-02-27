//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Connection.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/Initialize.h>
#include <Ice/LoggerUtil.h>
#include <Ice/UUID.h>

#include <IceDiscovery/LookupI.h>
#include <iterator>

using namespace std;
using namespace Ice;
using namespace IceDiscovery;

IceDiscovery::Request::Request(const LookupIPtr& lookup, int retryCount) :
    _lookup(lookup), _requestId(Ice::generateUUID()), _retryCount(retryCount), _lookupCount(0), _failureCount(0)
{
}

bool
IceDiscovery::Request::retry()
{
    return --_retryCount >= 0;
}

void
IceDiscovery::Request::invoke(const string& domainId, const vector<pair<LookupPrxPtr, LookupReplyPrxPtr> >& lookups)
{
    _lookupCount = lookups.size();
    _failureCount = 0;
    Ice::Identity id;
    id.name = _requestId;
    for(vector<pair<LookupPrxPtr, LookupReplyPrxPtr> >::const_iterator p = lookups.begin(); p != lookups.end(); ++p)
    {
        invokeWithLookup(domainId, p->first, Ice::uncheckedCast<LookupReplyPrx>(p->second->ice_identity(id)));
    }
}

bool
IceDiscovery::Request::exception()
{
    //
    // If all the invocations on all the lookup proxies failed, report it to the locator.
    //
    if(++_failureCount == _lookupCount)
    {
        finished(nullopt);
        return true;
    }
    return false;
}

string
IceDiscovery::Request::getRequestId() const
{
    return _requestId;
}

AdapterRequest::AdapterRequest(const LookupIPtr& lookup, const std::string& adapterId, int retryCount) :
    RequestT<std::string, AdapterCB>(lookup, adapterId, retryCount),
    _start(chrono::steady_clock::now()),
    _latency(chrono::nanoseconds::zero())
{
}

bool
AdapterRequest::retry()
{
    return _proxies.empty() && --_retryCount >= 0;
}

bool
AdapterRequest::response(const Ice::ObjectPrxPtr& proxy, bool isReplicaGroup)
{
    if(isReplicaGroup)
    {
        if(_latency == chrono::nanoseconds::zero())
        {
            _latency = chrono::duration_cast<chrono::nanoseconds>(chrono::steady_clock::now() - _start) *
                _lookup->latencyMultiplier();
            _lookup->timer()->cancel(shared_from_this());
            _lookup->timer()->schedule(shared_from_this(), _latency);
        }
        _proxies.insert(proxy);
        return false;
    }
    finished(proxy);
    return true;
}

void
AdapterRequest::finished(const ObjectPrxPtr& proxy)
{
    if(proxy || _proxies.empty())
    {
        RequestT<string, AdapterCB>::finished(proxy);
    }
    else if(_proxies.size() == 1)
    {
        RequestT<string, AdapterCB>::finished(*_proxies.begin());
    }
    else
    {
        EndpointSeq endpoints;
        ObjectPrxPtr prx;
        for(set<ObjectPrxPtr>::const_iterator p = _proxies.begin(); p != _proxies.end(); ++p)
        {
            if(!prx)
            {
                prx = *p;
            }
            Ice::EndpointSeq endpts = (*p)->ice_getEndpoints();
            copy(endpts.begin(), endpts.end(), back_inserter(endpoints));
        }
        RequestT<string, AdapterCB>::finished(prx->ice_endpoints(endpoints));
    }
}

void
AdapterRequest::invokeWithLookup(const string& domainId, const LookupPrxPtr& lookup, const LookupReplyPrxPtr& lookupReply)
{
    auto self = shared_from_this();
    lookup->findAdapterByIdAsync(domainId, _id, lookupReply, nullptr, [self](exception_ptr ex)
    {
        self->_lookup->adapterRequestException(self, ex);
    });
}

void
AdapterRequest::runTimerTask()
{
    _lookup->adapterRequestTimedOut(shared_from_this());
}

ObjectRequest::ObjectRequest(const LookupIPtr& lookup, const Ice::Identity& id, int retryCount) :
    RequestT<Ice::Identity, ObjectCB>(lookup, id, retryCount)
{
}

void
ObjectRequest::response(const Ice::ObjectPrxPtr& proxy)
{
    finished(proxy);
}

void
ObjectRequest::invokeWithLookup(const string& domainId, const LookupPrxPtr& lookup, const LookupReplyPrxPtr& lookupReply)
{
    auto self = shared_from_this();
    lookup->findObjectByIdAsync(domainId, _id, lookupReply, nullptr, [self](exception_ptr ex)
    {
        self->_lookup->objectRequestException(self, ex);
    });
}

void
ObjectRequest::runTimerTask()
{
    _lookup->objectRequestTimedOut(shared_from_this());
}

LookupI::LookupI(const LocatorRegistryIPtr& registry, const LookupPrxPtr& lookup, const Ice::PropertiesPtr& properties) :
    _registry(registry),
    _lookup(lookup),
    _timeout(chrono::milliseconds(properties->getPropertyAsIntWithDefault("IceDiscovery.Timeout", 300))),
    _retryCount(properties->getPropertyAsIntWithDefault("IceDiscovery.RetryCount", 3)),
    _latencyMultiplier(properties->getPropertyAsIntWithDefault("IceDiscovery.LatencyMultiplier", 1)),
    _domainId(properties->getProperty("IceDiscovery.DomainId")),
    _timer(IceInternal::getInstanceTimer(lookup->ice_getCommunicator())),
    _warnOnce(true)
{
    //
    // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast
    // datagram on each endpoint.
    //
    EndpointSeq endpoints = lookup->ice_getEndpoints();
    for(vector<EndpointPtr>::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    {
        EndpointSeq single;
        single.push_back(*p);
        _lookups.push_back(make_pair(lookup->ice_endpoints(single), LookupReplyPrxPtr()));
    }
    assert(!_lookups.empty());
}

LookupI::~LookupI()
{
}

void
LookupI::destroy()
{
    lock_guard lock(_mutex);
    for(map<Identity, ObjectRequestPtr>::const_iterator p = _objectRequests.begin(); p != _objectRequests.end(); ++p)
    {
        p->second->finished(nullopt);
        _timer->cancel(p->second);
    }
    _objectRequests.clear();

    for(map<string, AdapterRequestPtr>::const_iterator p = _adapterRequests.begin(); p != _adapterRequests.end(); ++p)
    {
        p->second->finished(nullopt);
        _timer->cancel(p->second);
    }
    _adapterRequests.clear();
}

void
LookupI::setLookupReply(const LookupReplyPrxPtr& lookupReply)
{
    //
    // Use a lookup reply proxy whose adress matches the interface used to send multicast datagrams.
    //
    for(vector<pair<LookupPrxPtr, LookupReplyPrxPtr> >::iterator p = _lookups.begin(); p != _lookups.end(); ++p)
    {
        auto info = dynamic_pointer_cast<UDPEndpointInfo>(p->first->ice_getEndpoints()[0]->getInfo());
        if(info && !info->mcastInterface.empty())
        {
            EndpointSeq endpts = lookupReply->ice_getEndpoints();
            for(EndpointSeq::const_iterator q = endpts.begin(); q != endpts.end(); ++q)
            {
                auto r = dynamic_pointer_cast<IPEndpointInfo>((*q)->getInfo());
                if(r && r->host == info->mcastInterface)
                {
                    EndpointSeq single;
                    single.push_back(*q);
                    p->second = lookupReply->ice_endpoints(single);
                }
            }
        }

        if(!p->second)
        {
            p->second = lookupReply; // Fallback: just use the given lookup reply proxy if no matching endpoint found.
        }
    }
}

void
LookupI::findObjectById(string domainId, Ice::Identity id, LookupReplyPrxPtr reply, const Ice::Current&)
{
    if(domainId != _domainId)
    {
        return; // Ignore.
    }

    Ice::ObjectPrxPtr proxy = _registry->findObject(id);
    if(proxy)
    {
        //
        // Reply to the mulicast request using the given proxy.
        //
        try
        {
            reply->foundObjectByIdAsync(id, proxy);
        }
        catch(const Ice::LocalException&)
        {
            // Ignore.
        }
    }
}

void
LookupI::findAdapterById(string domainId, string adapterId, LookupReplyPrxPtr reply,
                         const Ice::Current&)
{
    if(domainId != _domainId)
    {
        return; // Ignore.
    }

    bool isReplicaGroup;
    Ice::ObjectPrxPtr proxy = _registry->findAdapter(adapterId, isReplicaGroup);
    if(proxy)
    {
        //
        // Reply to the multicast request using the given proxy.
        //
        try
        {
            reply->foundAdapterByIdAsync(adapterId, proxy, isReplicaGroup);
        }
        catch(const Ice::LocalException&)
        {
            // Ignore.
        }
    }
}

void
LookupI::findObject(const ObjectCB& cb, const Ice::Identity& id)
{
    lock_guard lock(_mutex);
    map<Ice::Identity, ObjectRequestPtr>::iterator p = _objectRequests.find(id);
    if(p == _objectRequests.end())
    {
        p = _objectRequests.insert(make_pair(id, make_shared<ObjectRequest>(LookupIPtr(shared_from_this()),
                                                                            id,
                                                                            _retryCount))).first;
    }

    if(p->second->addCallback(cb))
    {
        try
        {
            p->second->invoke(_domainId, _lookups);
            _timer->schedule(p->second, _timeout);
        }
        catch(const Ice::LocalException&)
        {
            p->second->finished(nullopt);
            _objectRequests.erase(p);
        }
    }
}

void
LookupI::findAdapter(const AdapterCB& cb, const std::string& adapterId)
{
    lock_guard lock(_mutex);
    map<string, AdapterRequestPtr>::iterator p = _adapterRequests.find(adapterId);
    if(p == _adapterRequests.end())
    {
        p = _adapterRequests.insert(make_pair(adapterId, make_shared<AdapterRequest>(LookupIPtr(shared_from_this()),
                                                                                     adapterId,
                                                                                     _retryCount))).first;
    }

    if(p->second->addCallback(cb))
    {
        try
        {
            p->second->invoke(_domainId, _lookups);
            _timer->schedule(p->second, _timeout);
        }
        catch(const Ice::LocalException&)
        {
            p->second->finished(nullopt);
            _adapterRequests.erase(p);
        }
    }
}

void
LookupI::foundObject(const Ice::Identity& id, const string& requestId, const Ice::ObjectPrxPtr& proxy)
{
    lock_guard lock(_mutex);
    map<Ice::Identity, ObjectRequestPtr>::iterator p = _objectRequests.find(id);
    if(p != _objectRequests.end() && p->second->getRequestId() == requestId) // Ignore responses from old requests
    {
        p->second->response(proxy);
        _timer->cancel(p->second);
        _objectRequests.erase(p);
    }
}

void
LookupI::foundAdapter(const string& adapterId, const string& requestId, const Ice::ObjectPrxPtr& proxy,
                      bool isReplicaGroup)
{
    lock_guard lock(_mutex);
    map<string, AdapterRequestPtr>::iterator p = _adapterRequests.find(adapterId);
    if(p != _adapterRequests.end() && p->second->getRequestId() == requestId) // Ignore responses from old requests
    {
        if(p->second->response(proxy, isReplicaGroup))
        {
            _timer->cancel(p->second);
            _adapterRequests.erase(p);
        }
    }
}

void
LookupI::objectRequestTimedOut(const ObjectRequestPtr& request)
{
    lock_guard lock(_mutex);
    map<Ice::Identity, ObjectRequestPtr>::iterator p = _objectRequests.find(request->getId());
    if(p == _objectRequests.end() || p->second.get() != request.get())
    {
        return;
    }

    if(request->retry())
    {
        try
        {
            request->invoke(_domainId, _lookups);
            _timer->schedule(request, _timeout);
            return;
        }
        catch(const Ice::LocalException&)
        {
        }
    }

    request->finished(nullopt);
    _objectRequests.erase(p);
    _timer->cancel(request);
}

void
LookupI::adapterRequestException(const AdapterRequestPtr& request, exception_ptr ex)
{
    lock_guard lock(_mutex);
    map<string, AdapterRequestPtr>::iterator p = _adapterRequests.find(request->getId());
    if(p == _adapterRequests.end() || p->second.get() != request.get())
    {
        return;
    }

    if(request->exception())
    {
        if(_warnOnce)
        {
            try
            {
                rethrow_exception(ex);
            }
            catch (const std::exception& e)
            {
                Warning warn(_lookup->ice_getCommunicator()->getLogger());
                warn << "failed to lookup adapter `" << p->first << "' with lookup proxy `" << _lookup << "':\n" << e;
                _warnOnce = false;
            }

        }
        _timer->cancel(request);
        _adapterRequests.erase(p);
    }
}

void
LookupI::adapterRequestTimedOut(const AdapterRequestPtr& request)
{
    lock_guard lock(_mutex);
    map<string, AdapterRequestPtr>::iterator p = _adapterRequests.find(request->getId());
    if(p == _adapterRequests.end() || p->second.get() != request.get())
    {
        return;
    }

    if(request->retry())
    {
        try
        {
            request->invoke(_domainId, _lookups);
            _timer->schedule(request, _timeout);
            return;
        }
        catch(const Ice::LocalException&)
        {
        }
    }

    request->finished(nullopt);
    _adapterRequests.erase(p);
    _timer->cancel(request);
}

void
LookupI::objectRequestException(const ObjectRequestPtr& request, exception_ptr ex)
{
    lock_guard lock(_mutex);
    map<Ice::Identity, ObjectRequestPtr>::iterator p = _objectRequests.find(request->getId());
    if(p == _objectRequests.end() || p->second.get() != request.get())
    {
        return;
    }

    if(request->exception())
    {
        if(_warnOnce)
        {
            try
            {
                rethrow_exception(ex);
            }
            catch (const std::exception& e)
            {
                Warning warn(_lookup->ice_getCommunicator()->getLogger());
                string id = _lookup->ice_getCommunicator()->identityToString(p->first);
                warn << "failed to lookup object `" << id << "' with lookup proxy `" << _lookup << "':\n" << e;
                _warnOnce = false;
            }
        }
        _timer->cancel(request);
        _objectRequests.erase(p);
    }
}

LookupReplyI::LookupReplyI(const LookupIPtr& lookup) : _lookup(lookup)
{
}

void
LookupReplyI::foundObjectById(Identity id, ObjectPrxPtr proxy, const Current& current)
{
    _lookup->foundObject(id, current.id.name, proxy);
}

void
LookupReplyI::foundAdapterById(string adapterId, ObjectPrxPtr proxy, bool isReplicaGroup,
                               const Current& current)
{
    _lookup->foundAdapter(adapterId, current.id.name, proxy, isReplicaGroup);
}
