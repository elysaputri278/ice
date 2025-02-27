//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/OutputStream.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Instance.h>
#include <Ice/Object.h>
#include <Ice/Proxy.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ValueFactory.h>
#include <Ice/LocalException.h>
#include <Ice/Protocol.h>
#include <Ice/TraceUtil.h>
#include <Ice/LoggerUtil.h>
#include <Ice/SlicedData.h>
#include <Ice/StringConverter.h>
#include <iterator>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

class StreamUTF8BufferI : public IceUtil::UTF8Buffer
{
public:

    StreamUTF8BufferI(OutputStream& stream) :
        _stream(stream)
    {
    }

    Ice::Byte* getMoreBytes(size_t howMany, Ice::Byte* firstUnused)
    {
        assert(howMany > 0);

        if(firstUnused != 0)
        {
            //
            // Return unused bytes
            //
            _stream.resize(static_cast<size_t>(firstUnused - _stream.b.begin()));
        }

        //
        // Index of first unused byte
        //
        Buffer::Container::size_type pos = _stream.b.size();

        //
        // Since resize may reallocate the buffer, when firstUnused != 0, the
        // return value can be != firstUnused
        //
        _stream.resize(pos + howMany);

        return &_stream.b[pos];
    }

private:

    OutputStream& _stream;
};

}

Ice::OutputStream::OutputStream() :
    _instance(0),
    _closure(0),
    _encoding(currentEncoding),
    _format(FormatType::CompactFormat),
    _currentEncaps(0)
{
}

Ice::OutputStream::OutputStream(const CommunicatorPtr& communicator) :
    _closure(0),
    _currentEncaps(0)
{
    initialize(communicator);
}

Ice::OutputStream::OutputStream(const CommunicatorPtr& communicator, const EncodingVersion& encoding) :
    _closure(0),
    _currentEncaps(0)
{
    initialize(communicator, encoding);
}

Ice::OutputStream::OutputStream(const CommunicatorPtr& communicator, const EncodingVersion& encoding,
                                const pair<const Byte*, const Byte*>& buf) :
    Buffer(buf.first, buf.second),
    _closure(0),
    _currentEncaps(0)
{
    initialize(communicator, encoding);
    b.reset();
}

Ice::OutputStream::OutputStream(Instance* instance, const EncodingVersion& encoding) :
    _closure(0),
    _currentEncaps(0)
{
    initialize(instance, encoding);
}

void
Ice::OutputStream::initialize(const CommunicatorPtr& communicator)
{
    assert(communicator);
    Instance* instance = getInstance(communicator).get();
    initialize(instance, instance->defaultsAndOverrides()->defaultEncoding);
}

void
Ice::OutputStream::initialize(const CommunicatorPtr& communicator, const EncodingVersion& encoding)
{
    assert(communicator);
    initialize(getInstance(communicator).get(), encoding);
}

void
Ice::OutputStream::initialize(Instance* instance, const EncodingVersion& encoding)
{
    assert(instance);

    _instance = instance;
    _encoding = encoding;

    _format = _instance->defaultsAndOverrides()->defaultFormat;
}

void
Ice::OutputStream::clear()
{
    while(_currentEncaps && _currentEncaps != &_preAllocatedEncaps)
    {
        Encaps* oldEncaps = _currentEncaps;
        _currentEncaps = _currentEncaps->previous;
        delete oldEncaps;
    }
}

void
Ice::OutputStream::setFormat(FormatType fmt)
{
    _format = fmt;
}

void*
Ice::OutputStream::getClosure() const
{
    return _closure;
}

void*
Ice::OutputStream::setClosure(void* p)
{
    void* prev = _closure;
    _closure = p;
    return prev;
}

void
Ice::OutputStream::swap(OutputStream& other)
{
    swapBuffer(other);

    std::swap(_instance, other._instance);
    std::swap(_closure, other._closure);
    std::swap(_encoding, other._encoding);
    std::swap(_format, other._format);

    //
    // Swap is never called for streams that have encapsulations being written. However,
    // encapsulations might still be set in case marshalling failed. We just
    // reset the encapsulations if there are still some set.
    //
    resetEncapsulation();
    other.resetEncapsulation();
}

void
Ice::OutputStream::resetEncapsulation()
{
    while(_currentEncaps && _currentEncaps != &_preAllocatedEncaps)
    {
        Encaps* oldEncaps = _currentEncaps;
        _currentEncaps = _currentEncaps->previous;
        delete oldEncaps;
    }

    _preAllocatedEncaps.reset();
}

void
Ice::OutputStream::startEncapsulation()
{
    //
    // If no encoding version is specified, use the current write
    // encapsulation encoding version if there's a current write
    // encapsulation, otherwise, use the stream encoding version.
    //

    if(_currentEncaps)
    {
        startEncapsulation(_currentEncaps->encoding, _currentEncaps->format);
    }
    else
    {
        startEncapsulation(_encoding, Ice::FormatType::DefaultFormat);
    }
}

void
Ice::OutputStream::writePendingValues()
{
    if(_currentEncaps && _currentEncaps->encoder)
    {
        _currentEncaps->encoder->writePendingValues();
    }
    else if(getEncoding() == Ice::Encoding_1_0)
    {
        //
        // If using the 1.0 encoding and no instances were written, we
        // still write an empty sequence for pending instances if
        // requested (i.e.: if this is called).
        //
        // This is required by the 1.0 encoding, even if no instances
        // are written we do marshal an empty sequence if marshaled
        // data types use classes.
        //
        writeSize(0);
    }
}

void
Ice::OutputStream::writeBlob(const vector<Byte>& v)
{
    if(!v.empty())
    {
        Container::size_type pos = b.size();
        resize(pos + v.size());
        memcpy(&b[pos], &v[0], v.size());
    }
}

void
Ice::OutputStream::write(const Byte* begin, const Byte* end)
{
    int32_t sz = static_cast<int32_t>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + static_cast<size_t>(sz));
        memcpy(&b[pos], begin, static_cast<size_t>(sz));
    }
}

void
Ice::OutputStream::write(const vector<bool>& v)
{
    int32_t sz = static_cast<int32_t>(v.size());
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + static_cast<size_t>(sz));
        copy(v.begin(), v.end(), b.begin() + pos);
    }
}

namespace
{

template<size_t boolSize>
struct WriteBoolHelper
{
    static void write(const bool* begin, OutputStream::Container::size_type pos, OutputStream::Container& b, int32_t sz)
    {
        for(size_t idx = 0; idx < static_cast<size_t>(sz); ++idx)
        {
            b[pos + idx] = static_cast<Byte>(*(begin + idx));
        }
    }
};

template<>
struct WriteBoolHelper<1>
{
    static void write(const bool* begin, OutputStream::Container::size_type pos, OutputStream::Container& b, int32_t sz)
    {
        memcpy(&b[pos], begin, static_cast<size_t>(sz));
    }
};

}

void
Ice::OutputStream::write(const bool* begin, const bool* end)
{
    int32_t sz = static_cast<int32_t>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + static_cast<size_t>(sz));
        WriteBoolHelper<sizeof(bool)>::write(begin, pos, b, sz);
    }
}

void
Ice::OutputStream::write(int16_t v)
{
    Container::size_type pos = b.size();
    resize(pos + sizeof(int16_t));
    Byte* dest = &b[pos];
#ifdef ICE_BIG_ENDIAN
    const Byte* src = reinterpret_cast<const Byte*>(&v) + sizeof(int16_t) - 1;
    *dest++ = *src--;
    *dest = *src;
#else
    const Byte* src = reinterpret_cast<const Byte*>(&v);
    *dest++ = *src++;
    *dest = *src;
#endif
}

void
Ice::OutputStream::write(const int16_t* begin, const int16_t* end)
{
    int32_t sz = static_cast<int32_t>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + static_cast<size_t>(sz) * sizeof(int16_t));
#ifdef ICE_BIG_ENDIAN
        const Byte* src = reinterpret_cast<const Byte*>(begin) + sizeof(int16_t) - 1;
        Byte* dest = &(*(b.begin() + pos));
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest++ = *src--;
            *dest++ = *src--;
            src += 2 * sizeof(int16_t);
        }
#else
        memcpy(&b[pos], reinterpret_cast<const Byte*>(begin), static_cast<size_t>(sz) * sizeof(int16_t));
#endif
    }
}

void
Ice::OutputStream::write(const int32_t* begin, const int32_t* end)
{
    int32_t sz = static_cast<int32_t>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + static_cast<size_t>(sz) * sizeof(int32_t));
#ifdef ICE_BIG_ENDIAN
        const Byte* src = reinterpret_cast<const Byte*>(begin) + sizeof(int32_t) - 1;
        Byte* dest = &(*(b.begin() + pos));
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            src += 2 * sizeof(int32_t);
        }
#else
        memcpy(&b[pos], reinterpret_cast<const Byte*>(begin), static_cast<size_t>(sz) * sizeof(int32_t));
#endif
    }
}

void
Ice::OutputStream::write(int64_t v)
{
    Container::size_type pos = b.size();
    resize(pos + sizeof(int64_t));
    Byte* dest = &b[pos];
#ifdef ICE_BIG_ENDIAN
    const Byte* src = reinterpret_cast<const Byte*>(&v) + sizeof(Long) - 1;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest = *src;
#else
    const Byte* src = reinterpret_cast<const Byte*>(&v);
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;
#endif
}

void
Ice::OutputStream::write(const int64_t* begin, const int64_t* end)
{
    int32_t sz = static_cast<int32_t>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + static_cast<size_t>(sz) * sizeof(int64_t));
#ifdef ICE_BIG_ENDIAN
        const Byte* src = reinterpret_cast<const Byte*>(begin) + sizeof(int64_t) - 1;
        Byte* dest = &(*(b.begin() + pos));
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            src += 2 * sizeof(int64_t);
        }
#else
        memcpy(&b[pos], reinterpret_cast<const Byte*>(begin), static_cast<size_t>(sz) * sizeof(int64_t));
#endif
    }
}

void
Ice::OutputStream::write(float v)
{
    Container::size_type pos = b.size();
    resize(pos + sizeof(float));
    Byte* dest = &b[pos];
#ifdef ICE_BIG_ENDIAN
    const Byte* src = reinterpret_cast<const Byte*>(&v) + sizeof(float) - 1;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest = *src;
#else
    const Byte* src = reinterpret_cast<const Byte*>(&v);
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;
#endif
}

void
Ice::OutputStream::write(const float* begin, const float* end)
{
    int32_t sz = static_cast<int32_t>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + static_cast<size_t>(sz) * sizeof(float));
#ifdef ICE_BIG_ENDIAN
        const Byte* src = reinterpret_cast<const Byte*>(begin) + sizeof(float) - 1;
        Byte* dest = &(*(b.begin() + pos));
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            src += 2 * sizeof(float);
        }
#else
        memcpy(&b[pos], reinterpret_cast<const Byte*>(begin), static_cast<size_t>(sz) * sizeof(float));
#endif
    }
}

void
Ice::OutputStream::write(double v)
{
    Container::size_type pos = b.size();
    resize(pos + sizeof(double));
    Byte* dest = &b[pos];
#ifdef ICE_BIG_ENDIAN
    const Byte* src = reinterpret_cast<const Byte*>(&v) + sizeof(double) - 1;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest++ = *src--;
    *dest = *src;
#else
    const Byte* src = reinterpret_cast<const Byte*>(&v);
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;
#endif
}

void
Ice::OutputStream::write(const double* begin, const double* end)
{
    int32_t sz = static_cast<int32_t>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        Container::size_type pos = b.size();
        resize(pos + static_cast<size_t>(sz) * sizeof(double));
#ifdef ICE_BIG_ENDIAN
        const Byte* src = reinterpret_cast<const Byte*>(begin) + sizeof(double) - 1;
        Byte* dest = &(*(b.begin() + pos));
        for(int j = 0 ; j < sz ; ++j)
        {
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            src += 2 * sizeof(double);
        }
#else
        memcpy(&b[pos], reinterpret_cast<const Byte*>(begin), static_cast<size_t>(sz) * sizeof(double));
#endif
    }
}

//
// NOTE: This member function is intentionally omitted in order to
// cause a link error if it is used. This is for efficiency reasons:
// writing a const char * requires a traversal of the string to get
// the string length first, which takes O(n) time, whereas getting the
// string length from a std::string takes constant time.
//
/*
void
Ice::OutputStream::write(const char*)
{
}
*/

void
Ice::OutputStream::writeConverted(const char* vdata, size_t vsize)
{
    //
    // What is the size of the resulting UTF-8 encoded string?
    // Impossible to tell, so we guess. If we don't guess correctly,
    // we'll have to fix the mistake afterwards
    //
    try
    {
        int32_t guessedSize = static_cast<int32_t>(vsize);
        writeSize(guessedSize); // writeSize() only writes the size; it does not reserve any buffer space.

        size_t firstIndex = b.size();
        StreamUTF8BufferI buffer(*this);

        Byte* lastByte = nullptr;
        bool converted = false;
        if(_instance)
        {
            const StringConverterPtr& stringConverter = _instance->getStringConverter();
            if(stringConverter)
            {
                lastByte = stringConverter->toUTF8(vdata, vdata + vsize, buffer);
                converted = true;
            }
        }
        else
        {
            StringConverterPtr stringConverter = getProcessStringConverter();
            if(stringConverter)
            {
                lastByte = stringConverter->toUTF8(vdata, vdata + vsize, buffer);
                converted = true;
            }
        }

        if(!converted)
        {
            Container::size_type position = b.size();
            resize(position + vsize);
            memcpy(&b[position], vdata, vsize);
            return;
        }

        if(lastByte != b.end())
        {
            resize(static_cast<size_t>(lastByte - b.begin()));
        }
        size_t lastIndex = b.size();

        int32_t actualSize = static_cast<int32_t>(lastIndex - firstIndex);

        //
        // Check against the guess
        //
        if(guessedSize != actualSize)
        {
            if(guessedSize <= 254 && actualSize > 254)
            {
                //
                // Move the UTF-8 sequence 4 bytes further
                // Use memmove instead of memcpy since the source and destination typically overlap.
                //
                resize(b.size() + 4);
                memmove(b.begin() + firstIndex + 4, b.begin() + firstIndex, static_cast<size_t>(actualSize));
            }
            else if(guessedSize > 254 && actualSize <= 254)
            {
                //
                // Move the UTF-8 sequence 4 bytes back
                //
                memmove(b.begin() + firstIndex - 4, b.begin() + firstIndex, static_cast<size_t>(actualSize));
                resize(b.size() - 4);
            }

            if(guessedSize <= 254)
            {
                rewriteSize(actualSize, b.begin() + firstIndex - 1);
            }
            else
            {
                rewriteSize(actualSize, b.begin() + firstIndex - 1 - 4);
            }
        }
    }
    catch(const IceUtil::IllegalConversionException& ex)
    {
        throw StringConversionException(__FILE__, __LINE__, ex.reason());
    }
}

void
Ice::OutputStream::write(const string* begin, const string* end, bool convert)
{
    int32_t sz = static_cast<int32_t>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        for(int j = 0; j < sz; ++j)
        {
            write(begin[j], convert);
        }
    }
}

void
Ice::OutputStream::write(wstring_view v)
{
    if(v.empty())
    {
        writeSize(0);
        return;
    }

    //
    // What is the size of the resulting UTF-8 encoded string?
    // Impossible to tell, so we guess. If we don't guess correctly,
    // we'll have to fix the mistake afterwards
    //
    try
    {
        int32_t guessedSize = static_cast<int32_t>(v.size());
        writeSize(guessedSize); // writeSize() only writes the size; it does not reserve any buffer space.

        size_t firstIndex = b.size();
        StreamUTF8BufferI buffer(*this);

        Byte* lastByte = nullptr;

        // Note: wstringConverter is never null; when set to null, get returns the default unicode wstring converter
        if(_instance)
        {
            lastByte = _instance->getWstringConverter()->toUTF8(v.data(), v.data() + v.size(), buffer);
        }
        else
        {
            lastByte = getProcessWstringConverter()->toUTF8(v.data(), v.data() + v.size(), buffer);
        }

        if(lastByte != b.end())
        {
            resize(static_cast<size_t>(lastByte - b.begin()));
        }
        size_t lastIndex = b.size();

        int32_t actualSize = static_cast<int32_t>(lastIndex - firstIndex);

        //
        // Check against the guess
        //
        if(guessedSize != actualSize)
        {
            if(guessedSize <= 254 && actualSize > 254)
            {
                //
                // Move the UTF-8 sequence 4 bytes further
                // Use memmove instead of memcpy since the source and destination typically overlap.
                //
                resize(b.size() + 4);
                memmove(b.begin() + firstIndex + 4, b.begin() + firstIndex, static_cast<size_t>(actualSize));
            }
            else if(guessedSize > 254 && actualSize <= 254)
            {
                //
                // Move the UTF-8 sequence 4 bytes back
                //
                memmove(b.begin() + firstIndex - 4, b.begin() + firstIndex, static_cast<size_t>(actualSize));
                resize(b.size() - 4);
            }

            if(guessedSize <= 254)
            {
                rewriteSize(actualSize, b.begin() + firstIndex - 1);
            }
            else
            {
                rewriteSize(actualSize, b.begin() + firstIndex - 1 - 4);
            }
        }
    }
    catch(const IceUtil::IllegalConversionException& ex)
    {
        throw StringConversionException(__FILE__, __LINE__, ex.reason());
    }
}

void
Ice::OutputStream::write(const wstring* begin, const wstring* end)
{
    int32_t sz = static_cast<int32_t>(end - begin);
    writeSize(sz);
    if(sz > 0)
    {
        for(int j = 0; j < sz; ++j)
        {
            write(begin[j]);
        }
    }
}

void
Ice::OutputStream::writeProxy(const ObjectPrx& v)
{
    v->_write(*this);
}

void
Ice::OutputStream::writeNullProxy()
{
    Identity ident;
    write(ident);
}

void
Ice::OutputStream::writeEnum(int32_t v, int32_t maxValue)
{
    if(getEncoding() == Encoding_1_0)
    {
        if(maxValue < 127)
        {
            write(static_cast<Byte>(v));
        }
        else if(maxValue < 32767)
        {
            write(static_cast<int16_t>(v));
        }
        else
        {
            write(v);
        }
    }
    else
    {
        writeSize(v);
    }
}

void
Ice::OutputStream::writeException(const UserException& e)
{
    initEncaps();
    _currentEncaps->encoder->write(e);
}

bool
Ice::OutputStream::writeOptImpl(int32_t tag, OptionalFormat type)
{
    if(getEncoding() == Encoding_1_0)
    {
        return false; // Optional members aren't supported with the 1.0 encoding.
    }

    Byte v = static_cast<Byte>(type);
    if(tag < 30)
    {
        v |= static_cast<Byte>(tag << 3);
        write(v);
    }
    else
    {
        v |= 0xF0; // tag = 30
        write(v);
        writeSize(tag);
    }
    return true;
}

void
Ice::OutputStream::finished(vector<Byte>& bytes)
{
    vector<Byte>(b.begin(), b.end()).swap(bytes);
}

pair<const Byte*, const Byte*>
Ice::OutputStream::finished()
{
    if(b.empty())
    {
        return pair<const Byte*, const Byte*>(reinterpret_cast<Ice::Byte*>(0), reinterpret_cast<Ice::Byte*>(0));
    }
    else
    {
        return pair<const Byte*, const Byte*>(&b[0], &b[0] + b.size());
    }
}

void
Ice::OutputStream::throwEncapsulationException(const char* file, int line)
{
    throw EncapsulationException(file, line);
}

void
Ice::OutputStream::initEncaps()
{
    if(!_currentEncaps) // Lazy initialization.
    {
        _currentEncaps = &_preAllocatedEncaps;
        _currentEncaps->start = b.size();
        _currentEncaps->encoding = _encoding;
    }

    if(_currentEncaps->format == Ice::FormatType::DefaultFormat)
    {
        _currentEncaps->format = _format;
    }

    if(!_currentEncaps->encoder) // Lazy initialization.
    {
        if(_currentEncaps->encoding == Encoding_1_0)
        {
            _currentEncaps->encoder = new EncapsEncoder10(this, _currentEncaps);
        }
        else
        {
            _currentEncaps->encoder = new EncapsEncoder11(this, _currentEncaps);
        }
    }
}

Ice::OutputStream::EncapsEncoder::~EncapsEncoder()
{
    // Out of line to avoid weak vtable
}

int32_t
Ice::OutputStream::EncapsEncoder::registerTypeId(string_view typeId)
{
    TypeIdMap::const_iterator p = _typeIdMap.find(typeId);
    if(p != _typeIdMap.end())
    {
        return p->second;
    }
    else
    {
        _typeIdMap.insert(make_pair(string{typeId}, ++_typeIdIndex));
        return -1;
    }
}

void
Ice::OutputStream::EncapsEncoder10::write(const shared_ptr<Value>& v)
{
    //
    // Object references are encoded as a negative integer in 1.0.
    //
    if(v)
    {
        _stream->write(-registerValue(v));
    }
    else
    {
        _stream->write(0);
    }
}

void
Ice::OutputStream::EncapsEncoder10::write(const UserException& v)
{
    //
    // User exception with the 1.0 encoding start with a boolean
    // flag that indicates whether or not the exception uses
    // classes.
    //
    // This allows reading the pending instances even if some part of
    // the exception was sliced.
    //
    bool usesClasses = v._usesClasses();
    _stream->write(usesClasses);
    v._write(_stream);
    if(usesClasses)
    {
        writePendingValues();
    }
}

void
Ice::OutputStream::EncapsEncoder10::startInstance(SliceType sliceType, const SlicedDataPtr&)
{
    _sliceType = sliceType;
}

void
Ice::OutputStream::EncapsEncoder10::endInstance()
{
    if(_sliceType == ValueSlice)
    {
        //
        // Write the Object slice.
        //
        startSlice(Value::ice_staticId(), -1, true);
        _stream->writeSize(0); // For compatibility with the old AFM.
        endSlice();
    }
    _sliceType = NoSlice;
}

void
Ice::OutputStream::EncapsEncoder10::startSlice(string_view typeId, int, bool /*last*/)
{
    //
    // For instance slices, encode a boolean to indicate how the type ID
    // is encoded and the type ID either as a string or index. For
    // exception slices, always encode the type ID as a string.
    //
    if(_sliceType == ValueSlice)
    {
        int32_t index = registerTypeId(typeId);
        if(index < 0)
        {
            _stream->write(false);
            _stream->write(typeId, false);
        }
        else
        {
            _stream->write(true);
            _stream->writeSize(index);
        }
    }
    else
    {
        _stream->write(typeId, false);
    }

    _stream->write(int32_t(0)); // Placeholder for the slice length.

    _writeSlice = _stream->b.size();
}

void
Ice::OutputStream::EncapsEncoder10::endSlice()
{
    //
    // Write the slice length.
    //
    int32_t sz = static_cast<int32_t>(_stream->b.size() - _writeSlice + sizeof(int32_t));
    Byte* dest = &(*(_stream->b.begin() + _writeSlice - sizeof(int32_t)));
    _stream->write(sz, dest);
}

void
Ice::OutputStream::EncapsEncoder10::writePendingValues()
{
    while(!_toBeMarshaledMap.empty())
    {
        //
        // Consider the to be marshalled instances as marshalled now,
        // this is necessary to avoid adding again the "to be
        // marshalled instances" into _toBeMarshaledMap while writing
        // instances.
        //
        _marshaledMap.insert(_toBeMarshaledMap.begin(), _toBeMarshaledMap.end());

        PtrToIndexMap savedMap;
        savedMap.swap(_toBeMarshaledMap);
        _stream->writeSize(static_cast<int32_t>(savedMap.size()));
        for(PtrToIndexMap::iterator p = savedMap.begin(); p != savedMap.end(); ++p)
        {
            //
            // Ask the instance to marshal itself. Any new class
            // instances that are triggered by the classes marshaled
            // are added to toBeMarshaledMap.
            //
            _stream->write(p->second);

            try
            {
                p->first->ice_preMarshal();
            }
            catch(const std::exception& ex)
            {
                Warning out(_stream->instance()->initializationData().logger);
                out << "std::exception raised by ice_preMarshal:\n" << ex;
            }
            catch(...)
            {
                Warning out(_stream->instance()->initializationData().logger);
                out << "unknown exception raised by ice_preMarshal";
            }

            p->first->_iceWrite(_stream);
        }
    }
    _stream->writeSize(0); // Zero marker indicates end of sequence of sequences of instances.
}

int32_t
Ice::OutputStream::EncapsEncoder10::registerValue(const shared_ptr<Value>& v)
{
    assert(v);

    //
    // Look for this instance in the to-be-marshaled map.
    //
    PtrToIndexMap::const_iterator p = _toBeMarshaledMap.find(v);
    if(p != _toBeMarshaledMap.end())
    {
        return p->second;
    }

    //
    // Didn't find it, try the marshaled map next.
    //
    PtrToIndexMap::const_iterator q = _marshaledMap.find(v);
    if(q != _marshaledMap.end())
    {
        return q->second;
    }

    //
    // We haven't seen this instance previously, create a new
    // index, and insert it into the to-be-marshaled map.
    //
    _toBeMarshaledMap.insert(make_pair(v, ++_valueIdIndex));
    return _valueIdIndex;
}

void
Ice::OutputStream::EncapsEncoder11::write(const shared_ptr<Value>& v)
{
    if(!v)
    {
        _stream->writeSize(0); // Nil reference.
    }
    else if(_current && _encaps->format == FormatType::SlicedFormat)
    {
        //
        // If writing an instance within a slice and using the sliced
        // format, write an index from the instance indirection
        // table. The indirect instance table is encoded at the end of
        // each slice and is always read (even if the Slice is
        // unknown).
        //
        PtrToIndexMap::const_iterator p = _current->indirectionMap.find(v);
        if(p == _current->indirectionMap.end())
        {
            _current->indirectionTable.push_back(v);
            int32_t idx = static_cast<int32_t>(_current->indirectionTable.size()); // Position + 1 (0 is reserved for nil)
            _current->indirectionMap.insert(make_pair(v, idx));
            _stream->writeSize(idx);
        }
        else
        {
            _stream->writeSize(p->second);
        }
    }
    else
    {
        writeInstance(v); // Write the instance or a reference if already marshaled.
    }
}

void
Ice::OutputStream::EncapsEncoder11::write(const UserException& v)
{
    v._write(_stream);
}

void
Ice::OutputStream::EncapsEncoder11::startInstance(SliceType sliceType, const SlicedDataPtr& data)
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
    _current->firstSlice = true;

    if(data)
    {
        writeSlicedData(data);
    }
}

void
Ice::OutputStream::EncapsEncoder11::endInstance()
{
    _current = _current->previous;
}

void
Ice::OutputStream::EncapsEncoder11::startSlice(string_view typeId, int compactId, bool last)
{
    assert(_current->indirectionTable.empty() && _current->indirectionMap.empty());

    _current->sliceFlagsPos = _stream->b.size();

    _current->sliceFlags = 0;
    if(_encaps->format == FormatType::SlicedFormat)
    {
        _current->sliceFlags |= FLAG_HAS_SLICE_SIZE; // Encode the slice size if using the sliced format.
    }
    if(last)
    {
        _current->sliceFlags |= FLAG_IS_LAST_SLICE; // This is the last slice.
    }

    _stream->write(Byte(0)); // Placeholder for the slice flags

    //
    // For instance slices, encode the flag and the type ID either as a
    // string or index. For exception slices, always encode the type
    // ID a string.
    //
    if(_current->sliceType == ValueSlice)
    {
        //
        // Encode the type ID (only in the first slice for the compact
        // encoding).
        //
        if(_encaps->format == FormatType::SlicedFormat || _current->firstSlice)
        {
            if(compactId >= 0)
            {
                _current->sliceFlags |= FLAG_HAS_TYPE_ID_COMPACT;
                _stream->writeSize(compactId);
            }
            else
            {
                int32_t index = registerTypeId(typeId);
                if(index < 0)
                {
                    _current->sliceFlags |= FLAG_HAS_TYPE_ID_STRING;
                    _stream->write(typeId, false);
                }
                else
                {
                    _current->sliceFlags |= FLAG_HAS_TYPE_ID_INDEX;
                    _stream->writeSize(index);
                }
            }
        }
    }
    else
    {
        _stream->write(typeId, false);
    }

    if(_current->sliceFlags & FLAG_HAS_SLICE_SIZE)
    {
        _stream->write(int32_t(0)); // Placeholder for the slice length.
    }

    _current->writeSlice = _stream->b.size();
    _current->firstSlice = false;
}

void
Ice::OutputStream::EncapsEncoder11::endSlice()
{
    //
    // Write the optional member end marker if some optional members
    // were encoded. Note that the optional members are encoded before
    // the indirection table and are included in the slice size.
    //
    if(_current->sliceFlags & FLAG_HAS_OPTIONAL_MEMBERS)
    {
        _stream->write(OPTIONAL_END_MARKER);
    }

    //
    // Write the slice length if necessary.
    //
    if(_current->sliceFlags & FLAG_HAS_SLICE_SIZE)
    {
        int32_t sz = static_cast<int32_t>(_stream->b.size() - _current->writeSlice + sizeof(int32_t));
        Byte* dest = &(*(_stream->b.begin() + _current->writeSlice - sizeof(int32_t)));
        _stream->write(sz, dest);
    }

    //
    // Only write the indirection table if it contains entries.
    //
    if(!_current->indirectionTable.empty())
    {
        assert(_encaps->format == FormatType::SlicedFormat);
        _current->sliceFlags |= FLAG_HAS_INDIRECTION_TABLE;

        //
        // Write the indirect instance table.
        //
        _stream->writeSize(static_cast<int32_t>(_current->indirectionTable.size()));
        ValueList::const_iterator p;
        for(p = _current->indirectionTable.begin(); p != _current->indirectionTable.end(); ++p)
        {
            writeInstance(*p);
        }
        _current->indirectionTable.clear();
        _current->indirectionMap.clear();
    }

    //
    // Finally, update the slice flags.
    //
    Byte* dest = &(*(_stream->b.begin() + _current->sliceFlagsPos));
    *dest = _current->sliceFlags;
}

bool
Ice::OutputStream::EncapsEncoder11::writeOptional(int32_t tag, Ice::OptionalFormat format)
{
    if(!_current)
    {
        return _stream->writeOptImpl(tag, format);
    }
    else
    {
        if(_stream->writeOptImpl(tag, format))
        {
            _current->sliceFlags |= FLAG_HAS_OPTIONAL_MEMBERS;
            return true;
        }
        else
        {
            return false;
        }
    }
}

void
Ice::OutputStream::EncapsEncoder11::writeSlicedData(const SlicedDataPtr& slicedData)
{
    assert(slicedData);

    //
    // We only remarshal preserved slices if we are using the sliced
    // format. Otherwise, we ignore the preserved slices, which
    // essentially "slices" the instance into the most-derived type
    // known by the sender.
    //
    if(_encaps->format != FormatType::SlicedFormat)
    {
        return;
    }

    for(SliceInfoSeq::const_iterator p = slicedData->slices.begin(); p != slicedData->slices.end(); ++p)
    {
        startSlice((*p)->typeId, (*p)->compactId, (*p)->isLastSlice);

        //
        // Write the bytes associated with this slice.
        //
        _stream->writeBlob((*p)->bytes);

        if((*p)->hasOptionalMembers)
        {
            _current->sliceFlags |= FLAG_HAS_OPTIONAL_MEMBERS;
        }

        //
        // Make sure to also re-write the instance indirection table.
        //
        _current->indirectionTable = (*p)->instances;

        endSlice();
    }
}

void
Ice::OutputStream::EncapsEncoder11::writeInstance(const shared_ptr<Value>& v)
{
    assert(v);

    //
    // If the instance was already marshaled, just write it's ID.
    //
    PtrToIndexMap::const_iterator q = _marshaledMap.find(v);
    if(q != _marshaledMap.end())
    {
        _stream->writeSize(q->second);
        return;
    }

    //
    // We haven't seen this instance previously, create a new ID,
    // insert it into the marshaled map, and write the instance.
    //
    _marshaledMap.insert(make_pair(v, ++_valueIdIndex));

    try
    {
        v->ice_preMarshal();
    }
    catch(const std::exception& ex)
    {
        Warning out(_stream->instance()->initializationData().logger);
        out << "std::exception raised by ice_preMarshal:\n" << ex;
    }
    catch(...)
    {
        Warning out(_stream->instance()->initializationData().logger);
        out << "unknown exception raised by ice_preMarshal";
    }

    _stream->writeSize(1); // Object instance marker.
    v->_iceWrite(_stream);
}
