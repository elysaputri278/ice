//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Exception.h>
#include <Ice/LocalException.h>
#include <Ice/Network.h>
#include <Ice/Plugin.h>
#include <Ice/SlicedData.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>
#include <Ice/Initialize.h>
#include <IceUtil/StringUtil.h>
#include <iomanip>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

inline string
socketErrorToString(int error)
{
    if(error == 0)
    {
        return "unknown error";
    }
    return IceUtilInternal::errorToString(error);
}

};

namespace IceInternal
{

namespace Ex
{

void
throwUOE(const string& expectedType, const shared_ptr<Ice::Value>& v)
{
    //
    // If the object is an unknown sliced object, we didn't find an
    // value factory, in this case raise a NoValueFactoryException
    // instead.
    //
    UnknownSlicedValue* usv = dynamic_cast<UnknownSlicedValue*>(v.get());
    if(usv)
    {
        throw NoValueFactoryException(__FILE__, __LINE__, "", usv->ice_id());
    }

    string type = v->ice_id();
    throw Ice::UnexpectedObjectException(__FILE__, __LINE__,
                                         "expected element of type `" + expectedType + "' but received `" +
                                         type + "'", type, expectedType);
}

void
throwMemoryLimitException(const char* file, int line, size_t requested, size_t maximum)
{
    ostringstream s;
    s << "requested " << requested << " bytes, maximum allowed is " << maximum << " bytes (see Ice.MessageSizeMax)";
    throw Ice::MemoryLimitException(file, line, s.str());
}

void
throwMarshalException(const char* file, int line, const string& reason)
{
    throw Ice::MarshalException(file, line, reason);
}

}
}

namespace
{

const string userException_ids[] =
{
    "::Ice::UserException"
};

}

std::string_view
Ice::UserException::ice_staticId()
{
    return userException_ids[0];
}

unique_ptr<Ice::UserException>
Ice::UserException::ice_clone() const
{
    return unique_ptr<UserException>(static_cast<UserException*>(ice_cloneImpl()));
}

Ice::SlicedDataPtr
Ice::UserException::ice_getSlicedData() const
{
    return nullptr;
}

void
Ice::UserException::_write(::Ice::OutputStream* os) const
{
    os->startException(0);
    _writeImpl(os);
    os->endException();
}

void
Ice::UserException::_read(::Ice::InputStream* is)
{
    is->startException();
    _readImpl(is);
    is->endException(false);
}

bool
Ice::UserException::_usesClasses() const
{
    return false;
}

Ice::LocalException::LocalException(const char* file, int line) :
    Exception(file, line)
{
}

Ice::LocalException::~LocalException()
{
   // Out of line to avoid weak vtable
}

unique_ptr<Ice::LocalException>
Ice::LocalException::ice_clone() const
{
    return unique_ptr<LocalException>(static_cast<LocalException*>(ice_cloneImpl()));
}

namespace
{

const string localException_ids[] =
{
    "::Ice::LocalException"
};

}

std::string_view
Ice::LocalException::ice_staticId()
{
    return localException_ids[0];
}

Ice::SystemException::SystemException(const char* file, int line) :
    Exception(file, line)
{
}

Ice::SystemException::~SystemException()
{
}

unique_ptr<Ice::SystemException>
Ice::SystemException::ice_clone() const
{
    return unique_ptr<SystemException>(static_cast<SystemException*>(ice_cloneImpl()));
}

namespace
{

const string systemException_ids[] =
{
    "::Ice::SystemException"
};

}

std::string_view
Ice::SystemException::ice_staticId()
{
    return systemException_ids[0];
}

void
Ice::InitializationException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ninitialization exception";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::UnknownException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown exception";
    if(!unknown.empty())
    {
        out << ":\n" << unknown;
    }
}

void
Ice::UnknownLocalException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown local exception";
    if(!unknown.empty())
    {
        out << ":\n" << unknown;
    }
}

void
Ice::UnknownUserException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown user exception";
    if(!unknown.empty())
    {
        out << ":\n" << unknown;
    }
}

void
Ice::VersionMismatchException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nIce library version mismatch";
}

void
Ice::CommunicatorDestroyedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ncommunicator object destroyed";
}

void
Ice::ObjectAdapterDeactivatedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nobject adapter `" << name << "' deactivated";
}

void
Ice::ObjectAdapterIdInUseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nobject adapter with id `" << id << "' is already in use";
}

void
Ice::NoEndpointException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nno suitable endpoint available for proxy `" << proxy << "'";
}

void
Ice::EndpointParseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nerror while parsing endpoint `" << str << "'";
}

void
Ice::EndpointSelectionTypeParseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nerror while parsing endpoint selection type `" << str << "'";
}

void
Ice::VersionParseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nerror while parsing version `" << str << "'";
}

void
Ice::IdentityParseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nerror while parsing identity `" << str << "'";
}

void
Ice::ProxyParseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nerror while parsing proxy `" << str << "'";
}

void
Ice::IllegalIdentityException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nan identity with an empty name is not allowed";
}

void
Ice::IllegalServantException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nillegal servant: `" << reason << "'";
}

static void
printFailedRequestData(ostream& out, const RequestFailedException& ex)
{
    out << ":\nidentity: `" << identityToString(ex.id, ToStringMode::Unicode) << "'";
    out << "\nfacet: " << ex.facet;
    out << "\noperation: " << ex.operation;
}

void
Ice::RequestFailedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nrequest failed";
    printFailedRequestData(out, *this);
}

void
Ice::ObjectNotExistException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nobject does not exist";
    printFailedRequestData(out, *this);
}

void
Ice::FacetNotExistException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nfacet does not exist";
    printFailedRequestData(out, *this);
}

void
Ice::OperationNotExistException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\noperation does not exist";
    printFailedRequestData(out, *this);
}

void
Ice::SyscallException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    if(error != 0)
    {
        out << ":\nsyscall exception: " << IceUtilInternal::errorToString(error);
    }
}

void
Ice::SocketException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nsocket exception: " << socketErrorToString(error);
}

void
Ice::FileException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nfile exception: ";
    if(error == 0)
    {
        out << "couldn't open file";
    }
    else
    {
        out << IceUtilInternal::errorToString(error);
    }
    if(!path.empty())
    {
        out << "\npath: " << path;
    }
}

void
Ice::ConnectFailedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnect failed: " << socketErrorToString(error);
}

void
Ice::ConnectionRefusedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnection refused: " << socketErrorToString(error);
}

void
Ice::ConnectionLostException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnection lost: ";
    if(error == 0)
    {
        out << "recv() returned zero";
    }
    else
    {
        out << socketErrorToString(error);
    }
}

void
Ice::DNSException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nDNS error: ";
    out << errorToStringDNS(error);
    out << "\nhost: " << host;
}

void
Ice::OperationInterruptedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\noperation interrupted";
}

void
Ice::TimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ntimeout while sending or receiving data";
}

void
Ice::ConnectTimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ntimeout while establishing a connection";
}

void
Ice::CloseTimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ntimeout while closing a connection";
}

void
Ice::ConnectionTimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnection has timed out";
}

void
Ice::InvocationTimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ninvocation has timed out";
}

void
Ice::InvocationCanceledException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ninvocation canceled";
}

void
Ice::ProtocolException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol exception";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::BadMagicException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown magic number: ";

    ios_base::fmtflags originalFlags = out.flags();     // Save stream state
    ostream::char_type originalFill = out.fill();

    out.flags(ios_base::hex);                           // Change to hex
    out.fill('0');                                      // Fill with leading zeros

    out << "0x" << setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(badMagic[0])) << ", ";
    out << "0x" << setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(badMagic[1])) << ", ";
    out << "0x" << setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(badMagic[2])) << ", ";
    out << "0x" << setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(badMagic[3]));

    out.fill(originalFill);                             // Restore stream state
    out.flags(originalFlags);

    if(!reason.empty())
    {
        out << "\n" << reason;
    }
}

void
Ice::UnsupportedProtocolException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: unsupported protocol version: " << bad;
    out << "\n(can only support protocols compatible with version " << supported << ")";
}

void
Ice::UnsupportedEncodingException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nencoding error: unsupported encoding version: " << bad;
    out << "\n(can only support encodings compatible with version " << supported << ")";
    if(!reason.empty())
    {
        out << "\n" << reason;
    }
}

void
Ice::UnknownMessageException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: unknown message type";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::ConnectionNotValidatedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: received message over unvalidated connection";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::UnknownRequestIdException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: unknown request id";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::UnknownReplyStatusException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: unknown reply status";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::CloseConnectionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: connection closed";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::ConnectionManuallyClosedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: connection manually closed (" << (graceful ? "gracefully" : "forcefully") << ")";
}

void
Ice::IllegalMessageSizeException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: illegal message size";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::CompressionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: failed to compress or uncompress data";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::DatagramLimitException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: maximum datagram payload size exceeded";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::MarshalException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: error during marshaling or unmarshaling";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::ProxyUnmarshalException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: inconsistent proxy data during unmarshaling";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::UnmarshalOutOfBoundsException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: out of bounds during unmarshaling";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::NoValueFactoryException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: no suitable value factory found for `" << type << "'";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::UnexpectedObjectException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunexpected class instance of type `" << type <<
        "'; expected instance of type `" << expectedType << "'";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::MemoryLimitException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: memory limit exceeded";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::StringConversionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: string conversion failed";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::EncapsulationException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: illegal encapsulation";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::PluginInitializationException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nplug-in initialization failed";
    if(!reason.empty())
    {
        out << ": " << reason;
    }
}

void
Ice::CollocationOptimizationException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nrequested feature not available with collocation optimization";
}

void
Ice::AlreadyRegisteredException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n" << kindOfObject << " with id `" << id << "' is already registered";
}

void
Ice::NotRegisteredException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nno " << kindOfObject << " with id `" << id << "' is registered";
}

void
Ice::TwowayOnlyException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\noperation `" << operation << "' can only be invoked as a twoway request";
}

void
Ice::CloneNotImplementedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nice_clone() must be implemented in classes derived from abstract base classes";
}

void
Ice::FeatureNotSupportedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nfeature `" << unsupportedFeature << "' is not supported";
}

void
Ice::SecurityException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nsecurity exception";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::FixedProxyException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nfixed proxy exception";
}

void
Ice::ResponseSentException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nresponse sent exception";
}

void
Ice::CFNetworkException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nnetwork exception: domain: " << domain << " error: " << error;
}
