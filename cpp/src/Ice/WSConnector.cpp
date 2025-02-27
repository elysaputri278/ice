//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/WSConnector.h>
#include <Ice/WSTransceiver.h>
#include <Ice/WSEndpoint.h>
#include <Ice/HttpParser.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

TransceiverPtr
IceInternal::WSConnector::connect()
{
    return make_shared<WSTransceiver>(_instance, _delegate->connect(), _host, _resource);
}

int16_t
IceInternal::WSConnector::type() const
{
    return _delegate->type();
}

string
IceInternal::WSConnector::toString() const
{
    return _delegate->toString();
}

bool
IceInternal::WSConnector::operator==(const Connector& r) const
{
    const WSConnector* p = dynamic_cast<const WSConnector*>(&r);
    if(!p)
    {
        return false;
    }

    if(this == p)
    {
        return true;
    }

    if(_resource != p->_resource)
    {
        return false;
    }

    return Ice::targetEqualTo(_delegate, p->_delegate);
}

bool
IceInternal::WSConnector::operator<(const Connector& r) const
{
    const WSConnector* p = dynamic_cast<const WSConnector*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    if(this == p)
    {
        return false;
    }

    if(_resource < p->_resource)
    {
        return true;
    }
    else if(p->_resource < _resource)
    {
        return false;
    }

    return Ice::targetLess(_delegate, p->_delegate);
}

IceInternal::WSConnector::WSConnector(const ProtocolInstancePtr& instance, const ConnectorPtr& del, const string& host,
                                      const string& resource) :
    _instance(instance), _delegate(del), _host(host), _resource(resource)
{
}

IceInternal::WSConnector::~WSConnector()
{
}
