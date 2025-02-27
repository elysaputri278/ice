//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/ServantManager.h>
#include <Ice/ServantLocator.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Instance.h>
#include <Ice/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
IceInternal::ServantManager::addServant(const shared_ptr<Object>& object, const Identity& ident, const string& facet)
{
    lock_guard lock(_mutex);

    assert(_instance); // Must not be called after destruction.

    ServantMapMap::iterator p = _servantMapMapHint;

    if(p == _servantMapMap.end() || p->first != ident)
    {
        p = _servantMapMap.find(ident);
    }

    if(p == _servantMapMap.end())
    {
        p = _servantMapMap.insert(_servantMapMapHint, pair<const Identity, FacetMap>(ident, FacetMap()));
    }
    else
    {
        if(p->second.find(facet) != p->second.end())
        {
            ToStringMode toStringMode = _instance->toStringMode();
            ostringstream os;
            os << Ice::identityToString(ident, toStringMode);
            if(!facet.empty())
            {
                os << " -f " << escapeString(facet, "", toStringMode);
            }
            throw AlreadyRegisteredException(__FILE__, __LINE__, "servant", os.str());
        }
    }

    _servantMapMapHint = p;

    p->second.insert(pair<const string, shared_ptr<Object>>(facet, object));
}

void
IceInternal::ServantManager::addDefaultServant(const shared_ptr<Object>& object, const string& category)
{
    lock_guard lock(_mutex);

    assert(_instance); // Must not be called after destruction.

    DefaultServantMap::iterator p = _defaultServantMap.find(category);
    if(p != _defaultServantMap.end())
    {
        throw AlreadyRegisteredException(__FILE__, __LINE__, "default servant", category);
    }

    _defaultServantMap.insert(pair<const string, shared_ptr<Object>>(category, object));
}

shared_ptr<Object>
IceInternal::ServantManager::removeServant(const Identity& ident, const string& facet)
{
    //
    // We return the removed servant to avoid releasing the last reference count
    // with *this locked. We don't want to run user code, such as the servant
    // destructor, with an internal Ice mutex locked.
    //
    shared_ptr<Object> servant = 0;

    lock_guard lock(_mutex);

    assert(_instance); // Must not be called after destruction.

    ServantMapMap::iterator p = _servantMapMapHint;
    FacetMap::iterator q;

    if(p == _servantMapMap.end() || p->first != ident)
    {
        p = _servantMapMap.find(ident);
    }

    if(p == _servantMapMap.end() || (q = p->second.find(facet)) == p->second.end())
    {
        ToStringMode toStringMode = _instance->toStringMode();
        ostringstream os;
        os << Ice::identityToString(ident, toStringMode);
        if(!facet.empty())
        {
            os << " -f " + escapeString(facet, "", toStringMode);
        }
        throw NotRegisteredException(__FILE__, __LINE__, "servant", os.str());
    }

    servant = q->second;
    p->second.erase(q);

    if(p->second.empty())
    {
        if(p == _servantMapMapHint)
        {
            _servantMapMap.erase(p++);
            _servantMapMapHint = p;
        }
        else
        {
            _servantMapMap.erase(p);
        }
    }
    return servant;
}

shared_ptr<Object>
IceInternal::ServantManager::removeDefaultServant(const string& category)
{
    //
    // We return the removed servant to avoid releasing the last reference count
    // with *this locked. We don't want to run user code, such as the servant
    // destructor, with an internal Ice mutex locked.
    //
    shared_ptr<Object> servant = 0;

    lock_guard lock(_mutex);

    assert(_instance); // Must not be called after destruction.

    DefaultServantMap::iterator p = _defaultServantMap.find(category);
    if(p == _defaultServantMap.end())
    {
        throw NotRegisteredException(__FILE__, __LINE__, "default servant", category);
    }

    servant = p->second;
    _defaultServantMap.erase(p);

    return servant;
}

FacetMap
IceInternal::ServantManager::removeAllFacets(const Identity& ident)
{
    lock_guard lock(_mutex);

    assert(_instance); // Must not be called after destruction.

    ServantMapMap::iterator p = _servantMapMapHint;

    if(p == _servantMapMap.end() || p->first != ident)
    {
        p = _servantMapMap.find(ident);
    }

    if(p == _servantMapMap.end())
    {
        throw NotRegisteredException(__FILE__, __LINE__, "servant",
                                     Ice::identityToString(ident, _instance->toStringMode()));
    }

    FacetMap result = p->second;

    if(p == _servantMapMapHint)
    {
        _servantMapMap.erase(p++);
        _servantMapMapHint = p;
    }
    else
    {
        _servantMapMap.erase(p);
    }

    return result;
}

shared_ptr<Object>
IceInternal::ServantManager::findServant(const Identity& ident, const string& facet) const
{
    lock_guard lock(_mutex);

    //
    // This assert is not valid if the adapter dispatch incoming
    // requests from bidir connections. This method might be called if
    // requests are received over the bidir connection after the
    // adapter was deactivated.
    //
    //assert(_instance); // Must not be called after destruction.

    ServantMapMap::iterator p = _servantMapMapHint;
    FacetMap::iterator q;

    ServantMapMap& servantMapMap = const_cast<ServantMapMap&>(_servantMapMap);

    if(p == servantMapMap.end() || p->first != ident)
    {
        p = servantMapMap.find(ident);
    }

    if(p == servantMapMap.end() || (q = p->second.find(facet)) == p->second.end())
    {
        DefaultServantMap::const_iterator d = _defaultServantMap.find(ident.category);
        if(d == _defaultServantMap.end())
        {
            d = _defaultServantMap.find("");
            if(d == _defaultServantMap.end())
            {
                return 0;
            }
            else
            {
                return d->second;
            }
        }
        else
        {
            return d->second;
        }
    }
    else
    {
        _servantMapMapHint = p;
        return q->second;
    }
}

shared_ptr<Object>
IceInternal::ServantManager::findDefaultServant(const string& category) const
{
    lock_guard lock(_mutex);

    DefaultServantMap::const_iterator p = _defaultServantMap.find(category);
    if(p == _defaultServantMap.end())
    {
        return 0;
    }
    else
    {
        return p->second;
    }
}

FacetMap
IceInternal::ServantManager::findAllFacets(const Identity& ident) const
{
    lock_guard lock(_mutex);

    assert(_instance); // Must not be called after destruction.

    ServantMapMap::iterator p = _servantMapMapHint;

    ServantMapMap& servantMapMap = const_cast<ServantMapMap&>(_servantMapMap);

    if(p == servantMapMap.end() || p->first != ident)
    {
        p = servantMapMap.find(ident);
    }

    if(p == servantMapMap.end())
    {
        return FacetMap();
    }
    else
    {
        _servantMapMapHint = p;
        return p->second;
    }
}

bool
IceInternal::ServantManager::hasServant(const Identity& ident) const
{
    lock_guard lock(_mutex);

    //
    // This assert is not valid if the adapter dispatch incoming
    // requests from bidir connections. This method might be called if
    // requests are received over the bidir connection after the
    // adapter was deactivated.
    //
    //assert(_instance); // Must not be called after destruction.

    ServantMapMap::iterator p = _servantMapMapHint;
    ServantMapMap& servantMapMap = const_cast<ServantMapMap&>(_servantMapMap);

    if(p == servantMapMap.end() || p->first != ident)
    {
        p = servantMapMap.find(ident);
    }

    if(p == servantMapMap.end())
    {
        return false;
    }
    else
    {
        _servantMapMapHint = p;
        assert(!p->second.empty());
        return true;
    }
}

void
IceInternal::ServantManager::addServantLocator(const shared_ptr<ServantLocator>& locator, const string& category)
{
    lock_guard lock(_mutex);

    assert(_instance); // Must not be called after destruction.

    if((_locatorMapHint != _locatorMap.end() && _locatorMapHint->first == category)
       || _locatorMap.find(category) != _locatorMap.end())
    {
        throw AlreadyRegisteredException(__FILE__, __LINE__, "servant locator", category);
    }

    _locatorMapHint = _locatorMap.insert(_locatorMapHint, pair<const string, shared_ptr<ServantLocator>>(category, locator));
}

shared_ptr<ServantLocator>
IceInternal::ServantManager::removeServantLocator(const string& category)
{
    lock_guard lock(_mutex);

    assert(_instance); // Must not be called after destruction.

    map<string, shared_ptr<ServantLocator>>::iterator p = _locatorMap.end();
    if(_locatorMapHint != p)
    {
        if(_locatorMapHint->first == category)
        {
            p = _locatorMapHint;
        }
    }

    if(p == _locatorMap.end())
    {
        p = _locatorMap.find(category);
    }

    if(p == _locatorMap.end())
    {
        throw NotRegisteredException(__FILE__, __LINE__, "servant locator", category);
    }

    shared_ptr<ServantLocator> locator = p->second;
    _locatorMap.erase(p);
    _locatorMapHint = _locatorMap.begin();
    return locator;
}

shared_ptr<ServantLocator>
IceInternal::ServantManager::findServantLocator(const string& category) const
{
    lock_guard lock(_mutex);

    //
    // This assert is not valid if the adapter dispatch incoming
    // requests from bidir connections. This method might be called if
    // requests are received over the bidir connection after the
    // adapter was deactivated.
    //
    //assert(_instance); // Must not be called after destruction.

    map<string, shared_ptr<ServantLocator>>& locatorMap =
        const_cast<map<string, shared_ptr<ServantLocator>>&>(_locatorMap);

    map<string, shared_ptr<ServantLocator>>::iterator p = locatorMap.end();
    if(_locatorMapHint != locatorMap.end())
    {
        if(_locatorMapHint->first == category)
        {
            p = _locatorMapHint;
        }
    }

    if(p == locatorMap.end())
    {
        p = locatorMap.find(category);
    }

    if(p != locatorMap.end())
    {
        _locatorMapHint = p;
        return p->second;
    }
    else
    {
        return 0;
    }
}

IceInternal::ServantManager::ServantManager(const InstancePtr& instance, const string& adapterName)
    : _instance(instance),
      _adapterName(adapterName),
      _servantMapMapHint(_servantMapMap.end()),
      _locatorMapHint(_locatorMap.end())
{
}

IceInternal::ServantManager::~ServantManager()
{
    //
    // Don't check whether destroy() has been called. It might have
    // not been called if the associated object adapter was not
    // properly deactivated.
    //
    //assert(!_instance);
}

void
IceInternal::ServantManager::destroy()
{
    ServantMapMap servantMapMap;
    DefaultServantMap defaultServantMap;
    map<string, shared_ptr<ServantLocator>> locatorMap;
    Ice::LoggerPtr logger;

    {
        lock_guard lock(_mutex);
        //
        // If the ServantManager has already been destroyed, we're done.
        //
        if(!_instance)
        {
            return;
        }

        logger = _instance->initializationData().logger;

        servantMapMap.swap(_servantMapMap);
        _servantMapMapHint = _servantMapMap.end();

        defaultServantMap.swap(_defaultServantMap);

        locatorMap.swap(_locatorMap);
        _locatorMapHint = _locatorMap.end();

        _instance = 0;
    }

    for(map<string, shared_ptr<ServantLocator>>::const_iterator p = locatorMap.begin(); p != locatorMap.end(); ++p)
    {
        try
        {
            p->second->deactivate(p->first);
        }
        catch(const Exception& ex)
        {
            Error out(logger);
            out << "exception during locator deactivation:\n"
                << "object adapter: `" << _adapterName << "'\n"
                << "locator category: `" << p->first << "'\n"
                << ex;
        }
        catch(...)
        {
            Error out(logger);
            out << "unknown exception during locator deactivation:\n"
                << "object adapter: `" << _adapterName << "'\n"
                << "locator category: `" << p->first << "'";
        }
    }

    //
    // We clear the maps outside the synchronization as we don't want to
    // hold any internal Ice mutex while running user code (such as servant
    // or servant locator destructors).
    //
    servantMapMap.clear();
    locatorMap.clear();
    defaultServantMap.clear();
}
