//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceSSL/IceSSL.h>

#if defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wredundant-decls"
#endif
#include <mex.h>

typedef struct mxArray_tag mxArray; // Forward declaration to avoid importing mex.h here

namespace IceMatlab
{

mxArray* createStringFromUTF8(const std::string&);
std::string getStringFromUTF16(mxArray*);
mxArray* createEmpty();
mxArray* createBool(bool);
mxArray* createByte(Ice::Byte);
mxArray* createShort(short);
mxArray* createInt(int);
mxArray* createLong(long long);
mxArray* createFloat(float);
mxArray* createDouble(double);
mxArray* createEnumerator(const std::string&, int);
int getEnumerator(mxArray*, const std::string&);
mxArray* createIdentity(const Ice::Identity&);
void getIdentity(mxArray*, Ice::Identity&);
mxArray* createStringMap(const std::map<std::string, std::string>&);
void getStringMap(mxArray*, std::map<std::string, std::string>&);
mxArray* createProtocolVersion(const Ice::ProtocolVersion&);
void getProtocolVersion(mxArray*, Ice::ProtocolVersion&);
mxArray* createEncodingVersion(const Ice::EncodingVersion&);
void getEncodingVersion(mxArray*, Ice::EncodingVersion&);
mxArray* convertException(const std::exception&);
mxArray* createResultValue(mxArray*);
mxArray* createResultException(mxArray*);
mxArray* createOptionalValue(bool, mxArray*);
mxArray* createStringList(const std::vector<std::string>&);
void getStringList(mxArray*, std::vector<std::string>&);
mxArray* createByteArray(const Ice::Byte*, const Ice::Byte*);
mxArray* createByteList(const std::vector<Ice::Byte>&);
mxArray* createCertificateList(const std::vector<IceSSL::CertificatePtr>&);

std::string idToClass(const std::string&);

template<typename T>
std::shared_ptr<T> deref(void* p)
{
    return *reinterpret_cast<std::shared_ptr<T>*>(p);
}

template<typename T>
void* createShared(std::shared_ptr<T> p)
{
    return new std::shared_ptr<T>(std::move(p));
}

inline void* createProxy(Ice::ObjectPrx p)
{
    return new Ice::ObjectPrx(std::move(p));
}

inline void* createProxy(std::optional<Ice::ObjectPrx> p)
{
    return p ? createProxy(std::move(p).value()) : nullptr;
}

inline Ice::ObjectPrx restoreProxy(void* p)
{
    assert(p);
    return *reinterpret_cast<Ice::ObjectPrx*>(p);
}

inline std::optional<Ice::ObjectPrx> restoreNullableProxy(void* p)
{
    if (p)
    {
        return restoreProxy(p);
    }
    else
    {
        return std::nullopt;
    }
}

}
