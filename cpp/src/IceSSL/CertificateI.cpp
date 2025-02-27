//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/StringUtil.h>
#include <IceSSL/PluginI.h>
#include <IceSSL/Util.h>
#include <IceSSL/RFC2253.h>
#include <IceSSL/CertificateI.h>
#include <Ice/Object.h>
#include <Ice/Base64.h>
#include <Ice/LocalException.h>
#include <Ice/StringConverter.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceSSL;

//
// Map a certificate OID to its alias
//
const CertificateOID IceSSL::certificateOIDS[] =
{
    {"2.5.4.3", "CN"},
    {"2.5.4.4", "SN"},
    {"2.5.4.5", "DeviceSerialNumber"},
    {"2.5.4.6", "C"},
    {"2.5.4.7", "L"},
    {"2.5.4.8", "ST"},
    {"2.5.4.9", "STREET"},
    {"2.5.4.10", "O"},
    {"2.5.4.11", "OU"},
    {"2.5.4.12", "T"},
    {"2.5.4.42", "G"},
    {"2.5.4.43", "I"},
    {"1.2.840.113549.1.9.8", "unstructuredAddress"},
    {"1.2.840.113549.1.9.2", "unstructuredName"},
    {"1.2.840.113549.1.9.1", "emailAddress"},
    {"0.9.2342.19200300.100.1.25", "DC"}
};
const int IceSSL::certificateOIDSSize = sizeof(IceSSL::certificateOIDS) / sizeof(CertificateOID);

CertificateReadException::CertificateReadException(const char* file, int line, const string& r) :
    IceUtil::ExceptionHelper<CertificateReadException>(file, line),
    reason(r)
{
}

string
CertificateReadException::ice_id() const
{
    return "::IceSSL::CertificateReadException";
}

CertificateEncodingException::CertificateEncodingException(const char* file, int line, const string& r) :
    IceUtil::ExceptionHelper<CertificateEncodingException>(file, line),
    reason(r)
{
}

string
CertificateEncodingException::ice_id() const
{
    return "::IceSSL::CertificateEncodingException";
}

ParseException::ParseException(const char* file, int line, const string& r) :
    IceUtil::ExceptionHelper<ParseException>(file, line),
    reason(r)
{
}

string
ParseException::ice_id() const
{
    return "::IceSSL::ParseException";
}

DistinguishedName::DistinguishedName(const string& dn) : _rdns(RFC2253::parseStrict(dn))
{
    unescape();
}

DistinguishedName::DistinguishedName(const list<pair<string, string> >& rdns) : _rdns(rdns)
{
    unescape();
}

namespace IceSSL
{

bool
operator==(const DistinguishedName& lhs, const DistinguishedName& rhs)
{
    return lhs._unescaped == rhs._unescaped;
}

bool
operator<(const DistinguishedName& lhs, const DistinguishedName& rhs)
{
    return lhs._unescaped < rhs._unescaped;
}

}

bool
DistinguishedName::match(const DistinguishedName& other) const
{
    for(list< pair<string, string> >::const_iterator p = other._unescaped.begin(); p != other._unescaped.end(); ++p)
    {
        bool found = false;
        for(list< pair<string, string> >::const_iterator q = _unescaped.begin(); q != _unescaped.end(); ++q)
        {
            if(p->first == q->first)
            {
                found = true;
                if(p->second != q->second)
                {
                    return false;
                }
            }
        }
        if(!found)
        {
            return false;
        }
    }
    return true;
}

bool
DistinguishedName::match(const string& other) const
{
    return match(DistinguishedName(other));
}

//
// This always produces the same output as the input DN -- the type of
// escaping is not changed.
//
std::string
DistinguishedName::toString() const
{
    ostringstream os;
    bool first = true;
    for(list< pair<string, string> >::const_iterator p = _rdns.begin(); p != _rdns.end(); ++p)
    {
        if(!first)
        {
            os << ",";
        }
        first = false;
        os << p->first << "=" << p->second;
    }
    return os.str();
}

void
DistinguishedName::unescape()
{
    for(list< pair<string, string> >::const_iterator q = _rdns.begin(); q != _rdns.end(); ++q)
    {
        pair<string, string> rdn = *q;
        rdn.second = RFC2253::unescape(rdn.second);
        _unescaped.push_back(rdn);
    }
}

bool
CertificateI::operator!=(const IceSSL::Certificate& other) const
{
    return !operator==(other);
}

vector<X509ExtensionPtr>
CertificateI::getX509Extensions() const
{
    loadX509Extensions(); // Lazzy initialize the extensions
    return _extensions;
}

X509ExtensionPtr
CertificateI::getX509Extension(const string& oid) const
{
    loadX509Extensions(); // Lazzy initialize the extensions
    X509ExtensionPtr ext;
    for(vector<X509ExtensionPtr>::const_iterator i = _extensions.begin(); i != _extensions.end(); ++i)
    {
        if((*i)->getOID() == oid)
        {
            ext = *i;
            break;
        }
    }
    return ext;
}

void
CertificateI::loadX509Extensions() const
{
    throw FeatureNotSupportedException(__FILE__, __LINE__);
}

bool
CertificateI::checkValidity() const
{
    auto now = chrono::system_clock::now();
    return now > getNotBefore() && now <= getNotAfter();
}

bool
CertificateI::checkValidity(const chrono::system_clock::time_point& now) const
{
    return now > getNotBefore() && now <= getNotAfter();
}

string
CertificateI::toString() const
{
    ostringstream os;
    os << "serial: " << getSerialNumber() << "\n";
    os << "issuer: " << string(getIssuerDN()) << "\n";
    os << "subject: " << string(getSubjectDN()) << "\n";
    return os.str();
}

unsigned int
Certificate::getKeyUsage() const
{
    const CertificateExtendedInfo* impl = dynamic_cast<const CertificateExtendedInfo*>(this);
    if(impl)
    {
        return impl->getKeyUsage();
    }
    return 0;
}

unsigned int
Certificate::getExtendedKeyUsage() const
{
    const CertificateExtendedInfo* impl = dynamic_cast<const CertificateExtendedInfo*>(this);
    if(impl)
    {
        return impl->getExtendedKeyUsage();
    }
    return 0;
}
