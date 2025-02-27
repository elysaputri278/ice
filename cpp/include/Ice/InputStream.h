//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_INPUT_STREAM_H
#define ICE_INPUT_STREAM_H

#include <Ice/CommunicatorF.h>
#include <Ice/InstanceF.h>
#include <Ice/Object.h>
#include <Ice/ValueF.h>
#include "ProxyF.h"
#include <Ice/LoggerF.h>
#include <Ice/ValueFactory.h>
#include <Ice/Buffer.h>
#include <Ice/Protocol.h>
#include <Ice/SlicedDataF.h>
#include <Ice/UserExceptionFactory.h>
#include <Ice/StreamHelpers.h>
#include <Ice/FactoryTable.h>
#include "ReferenceF.h"

namespace Ice
{

class UserException;

/// \cond INTERNAL
template<typename T> inline void
patchValue(void* addr, const std::shared_ptr<Value>& v)
{
    std::shared_ptr<T>* ptr = static_cast<::std::shared_ptr<T>*>(addr);
    *ptr = ::std::dynamic_pointer_cast<T>(v);
    if(v && !(*ptr))
    {
        IceInternal::Ex::throwUOE(std::string{T::ice_staticId()}, v);
    }
}
/// \endcond

/**
 * Interface for input streams used to extract Slice types from a sequence of bytes.
 * \headerfile Ice/Ice.h
 */
class ICE_API InputStream : public IceInternal::Buffer
{
public:

    typedef size_t size_type;

    /**
     * Signature for a patch function, used to receive an unmarshaled value.
     * @param addr The target address.
     * @param v The unmarshaled value.
     */
    using PatchFunc = std::function<void(void* addr, const std::shared_ptr<Value>& v)>;

    /**
     * Constructs a stream using the latest encoding version but without a communicator.
     * This stream will not be able to unmarshal a proxy. For other unmarshaling tasks,
     * you can provide Helpers for objects that are normally provided by a communicator.
     * You can supply a communicator later by calling initialize().
     */
    InputStream();

    /**
     * Constructs a stream using the latest encoding version but without a communicator.
     * This stream will not be able to unmarshal a proxy. For other unmarshaling tasks,
     * you can provide Helpers for objects that are normally provided by a communicator.
     * You can supply a communicator later by calling initialize().
     * @param bytes The encoded data.
     */
    InputStream(const std::vector<Byte>& bytes);

    /**
     * Constructs a stream using the latest encoding version but without a communicator.
     * This stream will not be able to unmarshal a proxy. For other unmarshaling tasks,
     * you can provide Helpers for objects that are normally provided by a communicator.
     * You can supply a communicator later by calling initialize().
     * @param bytes The encoded data.
     */
    InputStream(const std::pair<const Byte*, const Byte*>& bytes);

    /// \cond INTERNAL
    InputStream(IceInternal::Buffer&, bool = false);
    /// \endcond

    /**
     * Constructs a stream using the communicator's default encoding version.
     * @param communicator The communicator to use for unmarshaling tasks.
     */
    InputStream(const CommunicatorPtr& communicator);

    /**
     * Constructs a stream using the communicator's default encoding version.
     * @param communicator The communicator to use for unmarshaling tasks.
     * @param bytes The encoded data.
     */
    InputStream(const CommunicatorPtr& communicator, const std::vector<Byte>& bytes);

    /**
     * Constructs a stream using the communicator's default encoding version.
     * @param communicator The communicator to use for unmarshaling tasks.
     * @param bytes The encoded data.
     */
    InputStream(const CommunicatorPtr& communicator, const std::pair<const Byte*, const Byte*>& bytes);

    /// \cond INTERNAL
    InputStream(const CommunicatorPtr& communicator, IceInternal::Buffer&, bool = false);
    /// \endcond

    /**
     * Constructs a stream using the given encoding version but without a communicator.
     * This stream will not be able to unmarshal a proxy. For other unmarshaling tasks,
     * you can provide Helpers for objects that are normally provided by a communicator.
     * You can supply a communicator later by calling initialize().
     * @param version The encoding version used to encode the data to be unmarshaled.
     */
    InputStream(const EncodingVersion& version);

    /**
     * Constructs a stream using the given encoding version but without a communicator.
     * This stream will not be able to unmarshal a proxy. For other unmarshaling tasks,
     * you can provide Helpers for objects that are normally provided by a communicator.
     * You can supply a communicator later by calling initialize().
     * @param version The encoding version used to encode the data to be unmarshaled.
     * @param bytes The encoded data.
     */
    InputStream(const EncodingVersion& version, const std::vector<Byte>& bytes);

    /**
     * Constructs a stream using the given encoding version but without a communicator.
     * This stream will not be able to unmarshal a proxy. For other unmarshaling tasks,
     * you can provide Helpers for objects that are normally provided by a communicator.
     * You can supply a communicator later by calling initialize().
     * @param version The encoding version used to encode the data to be unmarshaled.
     * @param bytes The encoded data.
     */
    InputStream(const EncodingVersion& version, const std::pair<const Byte*, const Byte*>& bytes);

    /// \cond INTERNAL
    InputStream(const EncodingVersion&, IceInternal::Buffer&, bool = false);
    /// \endcond

    /**
     * Constructs a stream using the given communicator and encoding version.
     * @param communicator The communicator to use for unmarshaling tasks.
     * @param version The encoding version used to encode the data to be unmarshaled.
     */
    InputStream(const CommunicatorPtr& communicator, const EncodingVersion& version);

    /**
     * Constructs a stream using the given communicator and encoding version.
     * @param communicator The communicator to use for unmarshaling tasks.
     * @param version The encoding version used to encode the data to be unmarshaled.
     * @param bytes The encoded data.
     */
    InputStream(const CommunicatorPtr& communicator, const EncodingVersion& version, const std::vector<Byte>& bytes);

    /**
     * Constructs a stream using the given communicator and encoding version.
     * @param communicator The communicator to use for unmarshaling tasks.
     * @param version The encoding version used to encode the data to be unmarshaled.
     * @param bytes The encoded data.
     */
    InputStream(const CommunicatorPtr& communicator, const EncodingVersion& version,
                const std::pair<const Byte*, const Byte*>& bytes);

    /// \cond INTERNAL
    InputStream(const CommunicatorPtr&, const EncodingVersion&, IceInternal::Buffer&, bool = false);
    /// \endcond

    ~InputStream()
    {
        // Inlined for performance reasons.

        if(_currentEncaps != &_preAllocatedEncaps)
        {
            clear(); // Not inlined.
        }

        for(auto d : _deleters)
        {
            d();
        }
    }

    /**
     * Initializes the stream to use the communicator's default encoding version.
     * Use initialize() if you originally constructed the stream without a communicator.
     * @param communicator The communicator to use for unmarshaling tasks.
     */
    void initialize(const CommunicatorPtr& communicator);

    /**
     * Initializes the stream to use the given communicator and encoding version.
     * Use initialize() if you originally constructed the stream without a communicator.
     * @param communicator The communicator to use for unmarshaling tasks.
     * @param version The encoding version used to encode the data to be unmarshaled.
     */
    void initialize(const CommunicatorPtr& communicator, const EncodingVersion& version);

    /**
     * Releases any data retained by encapsulations.
     */
    void clear();

    /// \cond INTERNAL
    //
    // Must return Instance*, because we don't hold an InstancePtr for
    // optimization reasons (see comments below).
    //
    IceInternal::Instance* instance() const { return _instance; } // Inlined for performance reasons.
    /// \endcond

    /**
     * Sets the value factory manager to use when unmarshaling value instances. If the stream
     * was initialized with a communicator, the communicator's value factory manager will
     * be used by default.
     *
     * @param vfm The value factory manager.
     */
    void setValueFactoryManager(const ValueFactoryManagerPtr& vfm);

    /**
     * Sets the logger to use when logging trace messages. If the stream
     * was initialized with a communicator, the communicator's logger will
     * be used by default.
     *
     * @param logger The logger to use for logging trace messages.
     */
    void setLogger(const LoggerPtr& logger);

    /**
     * Sets the compact ID resolver to use when unmarshaling value and exception
     * instances. If the stream was initialized with a communicator, the communicator's
     * resolver will be used by default.
     *
     * @param r The compact ID resolver.
     */
    void setCompactIdResolver(std::function<std::string(int)> r);

    /**
     * Indicates whether to slice instances of Slice classes to a known Slice type when a more
     * derived type is unknown. An instance is "sliced" when no static information is available
     * for a Slice type ID and no factory can be found for that type, resulting in the creation
     * of an instance of a less-derived type. If slicing is disabled in this situation, the
     * stream raises the exception NoValueFactoryException. The default behavior is to allow slicing.
     * @param b True to enable slicing, false otherwise.
     */
    void setSliceValues(bool b);

    /**
     * Indicates whether to log messages when instances of Slice classes are sliced. If the stream
     * is initialized with a communicator, this setting defaults to the value of the Ice.Trace.Slicing
     * property, otherwise the setting defaults to false.
     * @param b True to enable logging, false otherwise.
     */
    void setTraceSlicing(bool b);

    /**
     * Sets an upper limit on the depth of a class graph. If this limit is exceeded during
     * unmarshaling, the stream raises MarshalException.
     * @param n The maximum depth.
     */
    void setClassGraphDepthMax(size_t n);

    /**
     * Obtains the closure data associated with this stream.
     * @return The data as a void pointer.
     */
    void* getClosure() const;

    /**
     * Associates closure data with this stream.
     * @param p The data as a void pointer.
     * @return The previous closure data, or nil.
     */
    void* setClosure(void* p);

    /**
     * Swaps the contents of one stream with another.
     *
     * @param other The other stream.
     */
    void swap(InputStream& other);

    /// \cond INTERNAL
    void resetEncapsulation();
    /// \endcond

    /**
     * Resizes the stream to a new size.
     *
     * @param sz The new size.
     */
    void resize(Container::size_type sz)
    {
        b.resize(sz);
        i = b.end();
    }

    /**
     * Marks the start of a class instance.
     */
    void startValue()
    {
        assert(_currentEncaps && _currentEncaps->decoder);
        _currentEncaps->decoder->startInstance(ValueSlice);
    }

    /**
     * Marks the end of a class instance.
     *
     * @param preserve Pass true and the stream will preserve the unknown slices of the instance, or false
     * to discard the unknown slices.
     * @return An object that encapsulates the unknown slice data.
     */
    SlicedDataPtr endValue(bool preserve)
    {
        assert(_currentEncaps && _currentEncaps->decoder);
        return _currentEncaps->decoder->endInstance(preserve);
    }

    /**
     * Marks the start of a user exception.
     */
    void startException()
    {
        assert(_currentEncaps && _currentEncaps->decoder);
        _currentEncaps->decoder->startInstance(ExceptionSlice);
    }

    /**
     * Marks the end of a user exception.
     *
     * @param preserve Pass true and the stream will preserve the unknown slices of the exception, or false
     * to discard the unknown slices.
     * @return An object that encapsulates the unknown slice data.
     */
    SlicedDataPtr endException(bool preserve)
    {
        assert(_currentEncaps && _currentEncaps->decoder);
        return _currentEncaps->decoder->endInstance(preserve);
    }

    /**
     * Reads the start of an encapsulation.
     *
     * @return The encoding version used by the encapsulation.
     */
    const EncodingVersion& startEncapsulation()
    {
        Encaps* oldEncaps = _currentEncaps;
        if(!oldEncaps) // First allocated encaps?
        {
            _currentEncaps = &_preAllocatedEncaps;
        }
        else
        {
            _currentEncaps = new Encaps();
            _currentEncaps->previous = oldEncaps;
        }
        _currentEncaps->start = static_cast<size_t>(i - b.begin());

        //
        // I don't use readSize() and writeSize() for encapsulations,
        // because when creating an encapsulation, I must know in advance
        // how many bytes the size information will require in the data
        // stream. If I use an Int, it is always 4 bytes. For
        // readSize()/writeSize(), it could be 1 or 5 bytes.
        //
        std::int32_t sz;
        read(sz);
        if(sz < 6)
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        if(i - sizeof(std::int32_t) + sz > b.end())
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        _currentEncaps->sz = sz;

        read(_currentEncaps->encoding);
        IceInternal::checkSupportedEncoding(_currentEncaps->encoding); // Make sure the encoding is supported

        return _currentEncaps->encoding;
    }

    /**
     * Ends the current encapsulation.
     */
    void endEncapsulation()
    {
        assert(_currentEncaps);

        if(_currentEncaps->encoding != Encoding_1_0)
        {
            skipOptionals();
            if(i != b.begin() + _currentEncaps->start + _currentEncaps->sz)
            {
                throwEncapsulationException(__FILE__, __LINE__);
            }
        }
        else if(i != b.begin() + _currentEncaps->start + _currentEncaps->sz)
        {
            if(i + 1 != b.begin() + _currentEncaps->start + _currentEncaps->sz)
            {
                throwEncapsulationException(__FILE__, __LINE__);
            }

            //
            // Ice version < 3.3 had a bug where user exceptions with
            // class members could be encoded with a trailing byte
            // when dispatched with AMD. So we tolerate an extra byte
            // in the encapsulation.
            //
            ++i;
        }

        Encaps* oldEncaps = _currentEncaps;
        _currentEncaps = _currentEncaps->previous;
        if(oldEncaps == &_preAllocatedEncaps)
        {
            oldEncaps->reset();
        }
        else
        {
            delete oldEncaps;
        }
    }

    /**
     * Skips an empty encapsulation.
     *
     * @return The encapsulation's encoding version.
     */
    EncodingVersion skipEmptyEncapsulation()
    {
        std::int32_t sz;
        read(sz);
        if(sz < 6)
        {
            throwEncapsulationException(__FILE__, __LINE__);
        }
        if(i - sizeof(std::int32_t) + sz > b.end())
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        Ice::EncodingVersion encoding;
        read(encoding);
        IceInternal::checkSupportedEncoding(encoding); // Make sure the encoding is supported

        if(encoding == Ice::Encoding_1_0)
        {
            if(sz != static_cast<std::int32_t>(sizeof(std::int32_t)) + 2)
            {
                throwEncapsulationException(__FILE__, __LINE__);
            }
        }
        else
        {
            // Skip the optional content of the encapsulation if we are expecting an
            // empty encapsulation.
            i += static_cast<size_t>(sz) - sizeof(std::int32_t) - 2;
        }
        return encoding;
    }

    /**
     * Returns a blob of bytes representing an encapsulation.
     *
     * @param v A pointer into the internal marshaling buffer representing the start of the encoded encapsulation.
     * @param sz The number of bytes in the encapsulation.
     * @return encoding The encapsulation's encoding version.
     */
    EncodingVersion readEncapsulation(const Byte*& v, std::int32_t& sz)
    {
        EncodingVersion encoding;
        v = i;
        read(sz);
        if(sz < 6)
        {
            throwEncapsulationException(__FILE__, __LINE__);
        }
        if(i - sizeof(std::int32_t) + sz > b.end())
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }

        read(encoding);
        i += static_cast<size_t>(sz) - sizeof(std::int32_t) - 2;
        return encoding;
    }

    /**
     * Determines the current encoding version.
     *
     * @return The encoding version.
     */
    const EncodingVersion& getEncoding() const
    {
        return _currentEncaps ? _currentEncaps->encoding : _encoding;
    }

    /**
     * Determines the size of the current encapsulation, excluding the encapsulation header.
     *
     * @return The size of the encapsulated data.
     */
    std::int32_t getEncapsulationSize();

    /**
     * Skips over an encapsulation.
     *
     * @return The encoding version of the skipped encapsulation.
     */
    EncodingVersion skipEncapsulation();

    /**
     * Reads the start of a value or exception slice.
     *
     * @return The Slice type ID for this slice.
     */
    std::string startSlice()
    {
        assert(_currentEncaps && _currentEncaps->decoder);
        return std::string{_currentEncaps->decoder->startSlice()};
    }

    /**
     * Indicates that the end of a value or exception slice has been reached.
     */
    void endSlice()
    {
        assert(_currentEncaps && _currentEncaps->decoder);
        _currentEncaps->decoder->endSlice();
    }

    /**
     * Skips over a value or exception slice.
     */
    void skipSlice()
    {
        assert(_currentEncaps && _currentEncaps->decoder);
        _currentEncaps->decoder->skipSlice();
    }

    /**
     * Indicates that unmarshaling is complete, except for any class instances. The application must call this method
     * only if the stream actually contains class instances. Calling readPendingValues triggers the
     * patch callbacks to inform the application that unmarshaling of an instance is complete.
     */
    void readPendingValues();

    /**
     * Extracts a size from the stream.
     *
     * @return The extracted size.
     */
    std::int32_t readSize() // Inlined for performance reasons.
    {
        Byte byte;
        read(byte);
        unsigned char val = static_cast<unsigned char>(byte);
        if(val == 255)
        {
            std::int32_t v;
            read(v);
            if(v < 0)
            {
                throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
            }
            return v;
        }
        else
        {
            return static_cast<std::int32_t>(static_cast<unsigned char>(byte));
        }
    }

    /**
     * Reads and validates a sequence size.
     *
     * @param minSize The minimum size required by the sequence type.
     * @return The extracted size.
     */
    std::int32_t readAndCheckSeqSize(int minSize);

    /**
     * Reads a blob of bytes from the stream.
     *
     * @param bytes The vector to hold a copy of the bytes from the marshaling buffer.
     * @param sz The number of bytes to read.
     */
    void readBlob(std::vector<Byte>& bytes, std::int32_t sz);

    /**
     * Reads a blob of bytes from the stream.
     *
     * @param v A pointer into the internal marshaling buffer representing the start of the blob.
     * @param sz The number of bytes to read.
     */
    void readBlob(const Byte*& v, Container::size_type sz)
    {
        if(sz > 0)
        {
            v = i;
            if(static_cast<Container::size_type>(b.end() - i) < sz)
            {
                throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
            }
            i += sz;
        }
        else
        {
            v = i;
        }
    }

    /**
     * Reads a data value from the stream.
     * @param v Holds the extracted data.
     */
    template<typename T> void read(T& v)
    {
        StreamHelper<T, StreamableTraits<T>::helper>::read(this, v);
    }

    /**
     * Reads an optional data value from the stream. For all types except proxies.
     * @param tag The tag ID.
     * @param v Holds the extracted data (if any).
     */
    template<typename T, std::enable_if_t<!std::is_base_of<ObjectPrx, T>::value, bool> = true>
    void read(std::int32_t tag, std::optional<T>& v)
    {
        if(readOptional(tag, StreamOptionalHelper<T,
                                             StreamableTraits<T>::helper,
                                             StreamableTraits<T>::fixedLength>::optionalFormat))
        {
            v.emplace();
            StreamOptionalHelper<T,
                                 StreamableTraits<T>::helper,
                                 StreamableTraits<T>::fixedLength>::read(this, *v);
        }
        else
        {
            v = std::nullopt;
        }
    }

    /**
     * Reads an optional proxy from the stream.
     * @param tag The tag ID.
     * @param v The proxy unmarshaled by this function. If nullopt, the proxy was not present in the stream or
     * was set to nullopt (set to nullopt is supported for backward compatibility with Ice 3.7 and earlier releases).
     */
    template<typename T, std::enable_if_t<std::is_base_of<ObjectPrx, T>::value, bool> = true>
    void read(std::int32_t tag, std::optional<T>& v)
    {
        if (readOptional(tag, OptionalFormat::FSize))
        {
            skip(4); // the fixed-length size on 4 bytes
            read(v); // can be nullopt
        }
        else
        {
            v = std::nullopt;
        }
    }

    /**
     * Extracts a sequence of data values from the stream.
     * @param v A pair of pointers representing the beginning and end of the sequence elements.
     */
    template<typename T> void read(std::pair<const T*, const T*>& v)
    {
        auto holder = new std::vector<T>;
        _deleters.push_back([holder] { delete holder; });
        read(*holder);
        if(holder->size() > 0)
        {
            v.first = holder->data();
            v.second = holder->data() + holder->size();
        }
        else
        {
            v.first = 0;
            v.second = 0;
        }
    }

    /**
     * Reads a list of mandatory data values.
     */
    template<typename T> void readAll(T& v)
    {
        read(v);
    }

    /**
     * Reads a list of mandatory data values.
     */
    template<typename T, typename... Te> void readAll(T& v, Te&... ve)
    {
        read(v);
        readAll(ve...);
    }

    /**
     * Reads a list of optional data values.
     */
    template<typename T>
    void readAll(std::initializer_list<std::int32_t> tags, std::optional<T>& v)
    {
        read(*(tags.begin() + tags.size() - 1), v);
    }

    /**
     * Reads a list of optional data values.
     */
    template<typename T, typename... Te>
    void readAll(std::initializer_list<std::int32_t> tags, std::optional<T>& v, std::optional<Te>&... ve)
    {
        size_t index = tags.size() - sizeof...(ve) - 1;
        read(*(tags.begin() + index), v);
        readAll(tags, ve...);
    }

    /**
     * Determine if an optional value is available for reading.
     *
     * @param tag The tag associated with the value.
     * @param expectedFormat The optional format for the value.
     * @return True if the value is present, false otherwise.
     */
    bool readOptional(std::int32_t tag, OptionalFormat expectedFormat)
    {
        assert(_currentEncaps);
        if(_currentEncaps->decoder)
        {
            return _currentEncaps->decoder->readOptional(tag, expectedFormat);
        }
        else
        {
            return readOptImpl(tag, expectedFormat);
        }
    }

    /**
     * Reads a byte from the stream.
     * @param v The extracted byte.
     */
    void read(Byte& v)
    {
        if(i >= b.end())
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        v = *i++;
    }

    /**
     * Reads a sequence of bytes from the stream.
     * @param v A vector to hold a copy of the bytes.
     */
    void read(std::vector<Byte>& v);

    /**
     * Reads a sequence of bytes from the stream.
     * @param v A pair of pointers into the internal marshaling buffer representing the start and end of the
     * sequence elements.
     */
    void read(std::pair<const Byte*, const Byte*>& v);

    /**
     * Reads a bool from the stream.
     * @param v The extracted bool.
     */
    void read(bool& v)
    {
        if(i >= b.end())
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        v = (0 != *i++);
    }

    /**
     * Reads a sequence of boolean values from the stream.
     * @param v A vector to hold a copy of the boolean values.
     */
    void read(std::vector<bool>& v);

    /**
     * Reads a sequence of boolean values from the stream.
     * @param v A pair of pointers into the internal marshaling buffer representing the start and end of the
     * sequence elements.
     */
    void read(std::pair<const bool*, const bool*>& v);

    /**
     * Unmarshals a Slice short into an int16_t.
     * @param v The extracted int16_t.
     */
    void read(std::int16_t& v);

    /**
     * Unmarshals a sequence of Slice shorts into a vector of int16_t.
     * @param v A vector to hold a copy of the int16_t values.
     */
    void read(std::vector<std::int16_t>& v);

    /**
     * Unmarshals a sequence of Slice shorts into a pair of int16_t pointers representing the start and end of the
     * sequence elements.
     * @param v A pair of pointers representing the start and end of the sequence elements.
     */
    void read(std::pair<const short*, const short*>& v);

    /**
     * Reads an int from the stream.
     * @param v The extracted int.
     */
    void read(std::int32_t& v) // Inlined for performance reasons.
    {
        if(b.end() - i < static_cast<int>(sizeof(std::int32_t)))
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        const Byte* src = &(*i);
        i += sizeof(std::int32_t);
#ifdef ICE_BIG_ENDIAN
        Byte* dest = reinterpret_cast<Byte*>(&v) + sizeof(std::int32_t) - 1;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest-- = *src++;
        *dest = *src;
#else
        Byte* dest = reinterpret_cast<Byte*>(&v);
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest = *src;
#endif
    }

    /**
     * Reads a sequence of ints from the stream.
     * @param v A vector to hold a copy of the int values.
     */
    void read(std::vector<std::int32_t>& v);

    /**
     * Reads a sequence of ints from the stream.
     * @param v A pair of pointers representing the start and end of the sequence elements.
     */
    void read(std::pair<const int*, const int*>& v);

    /**
     * Reads a long from the stream.
     * @param v The extracted long.
     */
    void read(std::int64_t& v);

    /**
     * Reads a sequence of longs from the stream.
     * @param v A vector to hold a copy of the long values.
     */
    void read(std::vector<std::int64_t>& v);

    /**
     * Reads a sequence of longs from the stream.
     * @param v A pair of pointers representing the start and end of the sequence elements.
     */
    void read(std::pair<const std::int64_t*, const std::int64_t*>& v);

    /**
     * Unmarshals a Slice float into a float.
     * @param v The extracted float.
     */
    void read(float& v);

    /**
     * Unmarshals a sequence of Slice floats into a vector of float.
     * @param v An output vector filled by this function.
     */
    void read(std::vector<float>& v);

    /**
     * Unmarshals a sequence of Slice floats into a pair of float pointers representing the start and end of the
     * sequence elements.
     * @param v A pair of pointers representing the start and end of the sequence elements.
     */
    void read(std::pair<const float*, const float*>& v);

    /**
     * Unmarshals a Slice double into a double.
     * @param v The extracted double.
     */
    void read(double& v);

    /**
     * Unmarshals a sequence of Slice doubles into a vector of double.
     * @param v An output vector filled by this function.
     */
    void read(std::vector<double>& v);

    /**
     * Unmarshals a sequence of Slice doubles into a pair of double pointers representing the start and end of the
     * sequence elements.
     * @param v A pair of pointers representing the start and end of the sequence elements.
     */
    void read(std::pair<const double*, const double*>& v);

    /**
     * Reads a string from the stream.
     * @param v The extracted string.
     * @param convert Determines whether the string is processed by the string converter, if one
     * is installed. The default behavior is to convert strings.
     */
    void read(std::string& v, bool convert = true);

    /**
     * Reads a string from the stream.
     * @param vdata A pointer to the beginning of the string.
     * @param vsize The number of bytes in the string.
     * @param convert Determines whether the string is processed by the string converter, if one
     * is installed. The default behavior is to convert strings.
     */
    void read(const char*& vdata, size_t& vsize, bool convert = true);

    /**
     * Reads a sequence of strings from the stream.
     * @param v The extracted string sequence.
     * @param convert Determines whether strings are processed by the string converter, if one
     * is installed. The default behavior is to convert the strings.
     */
    void read(std::vector<std::string>& v, bool convert = true);

    /**
     * Reads a wide string from the stream.
     * @param v The extracted string.
     */
    void read(std::wstring& v);

    /**
     * Reads a sequence of wide strings from the stream.
     * @param v The extracted sequence.
     */
    void read(std::vector<std::wstring>& v);
    /**
     * Reads a typed proxy from the stream.
     * @param v The proxy as a user-defined type.
     */
    template<typename Prx, std::enable_if_t<std::is_base_of<ObjectPrx, Prx>::value, bool> = true>
    void read(std::optional<Prx>& v)
    {
        IceInternal::ReferencePtr ref = readReference();
        if (ref)
        {
            v = Prx::_fromReference(ref);
        }
        else
        {
            v = std::nullopt;
        }
    }

    /**
     * Reads a value (instance of a Slice class) from the stream (New mapping).
     * @param v The instance.
     */
    template<typename T, typename ::std::enable_if<::std::is_base_of<Value, T>::value>::type* = nullptr>
    void read(::std::shared_ptr<T>& v)
    {
        read(patchValue<T>, &v);
    }

    /**
     * Reads a value (instance of a Slice class) from the stream.
     * @param patchFunc The patch callback function.
     * @param patchAddr Closure data passed to the callback.
     */
    void read(PatchFunc patchFunc, void* patchAddr)
    {
        initEncaps();
        _currentEncaps->decoder->read(patchFunc, patchAddr);
    }

    /**
     * Reads an enumerator from the stream.
     * @param maxValue The maximum enumerator value in the definition.
     * @return The enumerator value.
     */
    std::int32_t readEnum(std::int32_t maxValue);

    /**
     * Extracts a user exception from the stream and throws it.
     * @param factory If provided, this factory is given the first opportunity to instantiate
     * the exception. If not provided, or if the factory does not throw an exception when invoked,
     * the stream will attempt to instantiate the exception using static type information.
     * @throws UserException The user exception that was unmarshaled.
     */
    void throwException(UserExceptionFactory factory = nullptr);

    /**
     * Skips one optional value with the given format.
     * @param format The expected format of the optional, if present.
     */
    void skipOptional(OptionalFormat format);

    /**
     * Skips all remaining optional values.
     */
    void skipOptionals();

    /**
     * Advances the current stream position by the given number of bytes.
     * @param size The number of bytes to skip.
     */
    void skip(size_type size)
    {
        if(i + size > b.end())
        {
            throwUnmarshalOutOfBoundsException(__FILE__, __LINE__);
        }
        i += size;
    }

    /**
     * Reads a size at the current position and skips that number of bytes.
     */
    void skipSize()
    {
        Byte bt;
        read(bt);
        if(static_cast<unsigned char>(bt) == 255)
        {
            skip(4);
        }
    }

    /**
     * Obtains the current position of the stream.
     * @return The current position.
     */
    size_type pos()
    {
        return static_cast<size_t>(i - b.begin());
    }

    /**
     * Sets a new position for the stream.
     * @param p The new position.
     */
    void pos(size_type p)
    {
        i = b.begin() + p;
    }

    /// \cond INTERNAL
    InputStream(IceInternal::Instance*, const EncodingVersion&);
    InputStream(IceInternal::Instance*, const EncodingVersion&, IceInternal::Buffer&, bool = false);

    void initialize(IceInternal::Instance*, const EncodingVersion&);

    bool readOptImpl(std::int32_t, OptionalFormat);
    /// \endcond

private:

    void initialize(const EncodingVersion&);

    // Reads a reference from the stream; the return value can be null.
    IceInternal::ReferencePtr readReference();

    //
    // String
    //
    bool readConverted(std::string&, std::int32_t);

    //
    // We can't throw these exception from inline functions from within
    // this file, because we cannot include the header with the
    // exceptions. Doing so would screw up the whole include file
    // ordering.
    //
    void throwUnmarshalOutOfBoundsException(const char*, int);
    void throwEncapsulationException(const char*, int);

    std::string resolveCompactId(int) const;

    void postUnmarshal(const std::shared_ptr<Value>&) const;

    class Encaps;
    enum SliceType { NoSlice, ValueSlice, ExceptionSlice };

    void traceSkipSlice(std::string_view, SliceType) const;

    ValueFactoryManagerPtr valueFactoryManager() const;

    LoggerPtr logger() const;

    std::function<std::string(int)> compactIdResolver() const;

    using ValueList = std::vector<std::shared_ptr<Value>>;

    class ICE_API EncapsDecoder : private ::IceUtil::noncopyable
    {
    public:

        virtual ~EncapsDecoder();

        virtual void read(PatchFunc, void*) = 0;
        virtual void throwException(UserExceptionFactory) = 0;

        virtual void startInstance(SliceType) = 0;
        virtual SlicedDataPtr endInstance(bool) = 0;
        virtual std::string_view startSlice() = 0;
        virtual void endSlice() = 0;
        virtual void skipSlice() = 0;

        virtual bool readOptional(std::int32_t, OptionalFormat)
        {
            return false;
        }

        virtual void readPendingValues()
        {
        }

    protected:

        EncapsDecoder(InputStream* stream, Encaps* encaps, bool sliceValues, size_t classGraphDepthMax,
                      const Ice::ValueFactoryManagerPtr& f) :
            _stream(stream), _encaps(encaps), _sliceValues(sliceValues), _classGraphDepthMax(classGraphDepthMax),
            _classGraphDepth(0), _valueFactoryManager(f), _typeIdIndex(0)
        {
        }

        std::string readTypeId(bool);
        std::shared_ptr<Value> newInstance(std::string_view);

        void addPatchEntry(std::int32_t, PatchFunc, void*);
        void unmarshal(std::int32_t, const std::shared_ptr<Value>&);

        typedef std::map<std::int32_t, std::shared_ptr<Value>> IndexToPtrMap;
        typedef std::map<std::int32_t, std::string> TypeIdMap;

        struct PatchEntry
        {
            PatchFunc patchFunc;
            void* patchAddr;
            size_t classGraphDepth;
        };
        typedef std::vector<PatchEntry> PatchList;
        typedef std::map<std::int32_t, PatchList> PatchMap;

        InputStream* _stream;
        Encaps* _encaps;
        const bool _sliceValues;
        const size_t _classGraphDepthMax;
        size_t _classGraphDepth;
        Ice::ValueFactoryManagerPtr _valueFactoryManager;

        // Encapsulation attributes for object un-marshalling
        PatchMap _patchMap;

    private:

        // Encapsulation attributes for object un-marshalling
        IndexToPtrMap _unmarshaledMap;
        TypeIdMap _typeIdMap;
        std::int32_t _typeIdIndex;
        ValueList _valueList;
    };

    class ICE_API EncapsDecoder10 : public EncapsDecoder
    {
    public:

        EncapsDecoder10(InputStream* stream, Encaps* encaps, bool sliceValues, size_t classGraphDepthMax,
                        const Ice::ValueFactoryManagerPtr& f) :
            EncapsDecoder(stream, encaps, sliceValues, classGraphDepthMax, f),
            _sliceType(NoSlice)
        {
        }

        virtual void read(PatchFunc, void*);
        virtual void throwException(UserExceptionFactory);

        virtual void startInstance(SliceType);
        virtual SlicedDataPtr endInstance(bool);
        virtual std::string_view startSlice();
        virtual void endSlice();
        virtual void skipSlice();

        virtual void readPendingValues();

    private:

        void readInstance();

        // Instance attributes
        SliceType _sliceType;
        bool _skipFirstSlice;

        // Slice attributes
        std::int32_t _sliceSize;
        std::string _typeId;
    };

    class ICE_API EncapsDecoder11 : public EncapsDecoder
    {
    public:

        EncapsDecoder11(InputStream* stream, Encaps* encaps, bool sliceValues, size_t classGraphDepthMax,
                        const Ice::ValueFactoryManagerPtr& f) :
            EncapsDecoder(stream, encaps, sliceValues, classGraphDepthMax, f),
            _preAllocatedInstanceData(0), _current(0), _valueIdIndex(1)
        {
        }

        virtual void read(PatchFunc, void*);
        virtual void throwException(UserExceptionFactory);

        virtual void startInstance(SliceType);
        virtual SlicedDataPtr endInstance(bool);
        virtual std::string_view startSlice();
        virtual void endSlice();
        virtual void skipSlice();

        virtual bool readOptional(std::int32_t, OptionalFormat);

    private:

        std::int32_t readInstance(std::int32_t, PatchFunc, void*);
        SlicedDataPtr readSlicedData();

        struct IndirectPatchEntry
        {
            std::int32_t index;
            PatchFunc patchFunc;
            void* patchAddr;
        };
        typedef std::vector<IndirectPatchEntry> IndirectPatchList;

        typedef std::vector<std::int32_t> IndexList;
        typedef std::vector<IndexList> IndexListList;

        struct InstanceData
        {
            InstanceData(InstanceData* p) : previous(p), next(0)
            {
                if(previous)
                {
                    previous->next = this;
                }
            }

            ~InstanceData()
            {
                if(next)
                {
                    delete next;
                }
            }

            // Instance attributes
            SliceType sliceType;
            bool skipFirstSlice;
            SliceInfoSeq slices; // Preserved slices.
            IndexListList indirectionTables;

            // Slice attributes
            Byte sliceFlags;
            std::int32_t sliceSize;
            std::string typeId;
            int compactId;
            IndirectPatchList indirectPatchList;

            InstanceData* previous;
            InstanceData* next;
        };
        InstanceData _preAllocatedInstanceData;
        InstanceData* _current;

        void push(SliceType sliceType)
        {
            if(!_current)
            {
                _current = &_preAllocatedInstanceData;
            }
            else
            {
                _current = _current->next ? _current->next : new InstanceData(_current);
            }
            _current->sliceType = sliceType;
            _current->skipFirstSlice = false;
        }

        std::int32_t _valueIdIndex; // The ID of the next value to unmarshal.
    };

    class Encaps : private ::IceUtil::noncopyable
    {
    public:

        Encaps() : start(0), decoder(0), previous(0)
        {
            // Inlined for performance reasons.
        }
        ~Encaps()
        {
            // Inlined for performance reasons.
            delete decoder;
        }
        void reset()
        {
            // Inlined for performance reasons.
            delete decoder;
            decoder = 0;

            previous = 0;
        }

        Container::size_type start;
        std::int32_t sz;
        EncodingVersion encoding;

        EncapsDecoder* decoder;

        Encaps* previous;
    };

    //
    // Optimization. The instance may not be deleted while a
    // stack-allocated stream still holds it.
    //
    IceInternal::Instance* _instance;

    //
    // The encoding version to use when there's no encapsulation to
    // read from. This is for example used to read message headers.
    //
    EncodingVersion _encoding;

    Encaps* _currentEncaps;

    void initEncaps();

    Encaps _preAllocatedEncaps;

    bool _traceSlicing;

    size_t _classGraphDepthMax;

    void* _closure;

    bool _sliceValues;

    int _startSeq;
    int _minSeqSize;

    ValueFactoryManagerPtr _valueFactoryManager;
    LoggerPtr _logger;
    std::function<std::string(int)> _compactIdResolver;
    std::vector<std::function<void()>> _deleters;

};

} // End namespace Ice

#endif
