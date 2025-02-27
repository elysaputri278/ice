//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/FactoryTable.h>
#include <Ice/ValueFactory.h>

using namespace std;

//
// Add a factory to the exception factory table.
// If the factory is present already, increment its reference count.
//
void
IceInternal::FactoryTable::addExceptionFactory(string_view t, Ice::UserExceptionFactory f)
{
    lock_guard lock(_mutex);
    assert(f);
    EFTable::iterator i = _eft.find(t);
    if(i == _eft.end())
    {
        _eft[string{t}] = EFPair(f, 1);
    }
    else
    {
        i->second.second++;
    }
}

//
// Return the exception factory for a given type ID
//
Ice::UserExceptionFactory
IceInternal::FactoryTable::getExceptionFactory(string_view t) const
{
    lock_guard lock(_mutex);
    EFTable::const_iterator i = _eft.find(t);
    return i != _eft.end() ? i->second.first : Ice::UserExceptionFactory();
}

//
// Remove a factory from the exception factory table. If the factory
// is not present, do nothing; otherwise, decrement the factory's
// reference count; if the count drops to zero, remove the factory's
// entry from the table.
//
void
IceInternal::FactoryTable::removeExceptionFactory(string_view t)
{
    lock_guard lock(_mutex);
    EFTable::iterator i = _eft.find(t);
    if(i != _eft.end())
    {
        if(--i->second.second == 0)
        {
            _eft.erase(i);
        }
    }
}

//
// Add a factory to the value factory table.
//
void
IceInternal::FactoryTable::addValueFactory(string_view t, ::Ice::ValueFactoryFunc f)
{
    lock_guard lock(_mutex);
    assert(f);
    VFTable::iterator i = _vft.find(t);
    if(i == _vft.end())
    {
        _vft[string{t}] = VFPair(f, 1);
    }
    else
    {
        i->second.second++;
    }
}

//
// Return the value factory for a given type ID
//
::Ice::ValueFactoryFunc
IceInternal::FactoryTable::getValueFactory(string_view t) const
{
    lock_guard lock(_mutex);
    VFTable::const_iterator i = _vft.find(t);
    return i != _vft.end() ? i->second.first : nullptr;
}

//
// Remove a factory from the value factory table. If the factory
// is not present, do nothing; otherwise, decrement the factory's
// reference count if the count drops to zero, remove the factory's
// entry from the table.
//
void
IceInternal::FactoryTable::removeValueFactory(string_view t)
{
    lock_guard lock(_mutex);
    VFTable::iterator i = _vft.find(t);
    if(i != _vft.end())
    {
        if(--i->second.second == 0)
        {
            _vft.erase(i);
        }
    }
}

//
// Add a factory to the value factory table.
//
void
IceInternal::FactoryTable::addTypeId(int compactId, string_view typeId)
{
    lock_guard lock(_mutex);
    assert(!typeId.empty() && compactId >= 0);
    TypeIdTable::iterator i = _typeIdTable.find(compactId);
    if(i == _typeIdTable.end())
    {
        _typeIdTable[compactId] = TypeIdPair(string{typeId}, 1);
    }
    else
    {
        i->second.second++;
    }
}

//
// Return the type ID for the given compact ID
//
string
IceInternal::FactoryTable::getTypeId(int compactId) const
{
    lock_guard lock(_mutex);
    TypeIdTable::const_iterator i = _typeIdTable.find(compactId);
    return i != _typeIdTable.end() ? i->second.first : string{};
}

void
IceInternal::FactoryTable::removeTypeId(int compactId)
{
    lock_guard lock(_mutex);
    TypeIdTable::iterator i = _typeIdTable.find(compactId);
    if(i != _typeIdTable.end())
    {
        if(--i->second.second == 0)
        {
            _typeIdTable.erase(i);
        }
    }
}
