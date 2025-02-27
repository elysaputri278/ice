//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_EXCEPTION_H
#define ICE_EXCEPTION_H

#include <IceUtil/Exception.h>
#include <Ice/Config.h>
#include <Ice/Format.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/SlicedDataF.h>

namespace Ice
{

class OutputStream;
class InputStream;

typedef IceUtil::Exception Exception;

/**
 * Base class for all Ice run-time exceptions.
 * \headerfile Ice/Ice.h
 */
class ICE_API LocalException : public IceUtil::Exception
{
public:

    /**
     * The file and line number are required for all local exceptions.
     * @param file The file name in which the exception was raised, typically __FILE__.
     * @param line The line number at which the exception was raised, typically __LINE__.
     */
    LocalException(const char* file, int line);
    LocalException(const LocalException&) = default;
    virtual ~LocalException();

    /**
     * Polymorphically clones this exception.
     * @return A shallow copy of this exception.
     */
    std::unique_ptr<LocalException> ice_clone() const;

    /**
     * Obtains the Slice type ID of this exception.
     * @return The fully-scoped type ID.
     */
    static std::string_view ice_staticId();
};

/**
 * Base class for all Ice user exceptions.
 * \headerfile Ice/Ice.h
 */
class ICE_API UserException : public IceUtil::Exception
{
public:

    /**
     * Polymorphically clones this exception.
     * @return A shallow copy of this exception.
     */
    std::unique_ptr<UserException> ice_clone() const;

    virtual Ice::SlicedDataPtr ice_getSlicedData() const;

    /**
     * Obtains the Slice type ID of this exception.
     * @return The fully-scoped type ID.
     */
    static std::string_view ice_staticId();

    /// \cond STREAM
    virtual void _write(::Ice::OutputStream*) const;
    virtual void _read(::Ice::InputStream*);

    virtual bool _usesClasses() const;
    /// \endcond

protected:

    /// \cond STREAM
    virtual void _writeImpl(::Ice::OutputStream*) const {}
    virtual void _readImpl(::Ice::InputStream*) {}
    /// \endcond
};

/**
 * Base class for all Ice system exceptions.
 *
 * System exceptions are currently Ice internal, non-documented
 * exceptions.
 * \headerfile Ice/Ice.h
 */
class ICE_API SystemException : public IceUtil::Exception
{
public:

    /**
     * The file and line number are required for all local exceptions.
     * @param file The file name in which the exception was raised, typically __FILE__.
     * @param line The line number at which the exception was raised, typically __LINE__.
     */
    SystemException(const char* file, int line);
    SystemException(const SystemException&) = default;
    virtual ~SystemException();

    /**
     * Polymorphically clones this exception.
     * @return A shallow copy of this exception.
     */
    std::unique_ptr<SystemException> ice_clone() const;

    /**
     * Obtains the Slice type ID of this exception.
     * @return The fully-scoped type ID.
     */
    static std::string_view ice_staticId();
};

}

namespace IceInternal
{

namespace Ex
{

ICE_API void throwUOE(const ::std::string&, const std::shared_ptr<Ice::Value>&);
ICE_API void throwMemoryLimitException(const char*, int, size_t, size_t);
ICE_API void throwMarshalException(const char*, int, const std::string&);

}

}

#endif
