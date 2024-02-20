//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_FACTORYTABLE_H
#define ICE_FACTORYTABLE_H

#include <Ice/UserExceptionFactory.h>
#include <Ice/ValueFactory.h>

#include <mutex>

namespace Ice
{

/**
 * The base class for a compact ID resolver. Subclasses must implement resolve.
 * The resolver can be installed via InitializationData.
 * \headerfile Ice/Ice.h
 */
class ICE_API CompactIdResolver
{
public:

    /**
     * Called by the Ice run time when a compact ID must be translated into a type ID.
     * @param id The compact ID.
     * @return The fully-scoped Slice type ID, or an empty string if the compact ID is unknown.
     */
    virtual ::std::string resolve(std::int32_t id) const = 0;
};
using CompactIdResolverPtr = ::std::shared_ptr<CompactIdResolver>;

}

namespace IceInternal
{

class ICE_API FactoryTable : private IceUtil::noncopyable
{
public:

    void addExceptionFactory(const ::std::string&, ::Ice::UserExceptionFactory);
    ::Ice::UserExceptionFactory getExceptionFactory(const ::std::string&) const;
    void removeExceptionFactory(const ::std::string&);

    void addValueFactory(const ::std::string&, ::Ice::ValueFactoryFunc);
    ::Ice::ValueFactoryFunc getValueFactory(const ::std::string&) const;
    void removeValueFactory(const ::std::string&);

    void addTypeId(int, const ::std::string&);
    std::string getTypeId(int) const;
    void removeTypeId(int);

private:

    mutable std::mutex _mutex;

    typedef ::std::pair<::Ice::UserExceptionFactory, int> EFPair;
    typedef ::std::map<::std::string, EFPair> EFTable;
    EFTable _eft;

    typedef ::std::pair<::Ice::ValueFactoryFunc, int> VFPair;
    typedef ::std::map<::std::string, VFPair> VFTable;
    VFTable _vft;

    typedef ::std::pair<::std::string, int> TypeIdPair;
    typedef ::std::map<int, TypeIdPair> TypeIdTable;
    TypeIdTable _typeIdTable;
};

}

#endif
