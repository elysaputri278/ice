//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/UdpConnector.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/UdpTransceiver.h>
#include <Ice/UdpEndpointI.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

TransceiverPtr
IceInternal::UdpConnector::connect()
{
    return make_shared<UdpTransceiver>(_instance, _addr, _sourceAddr, _mcastInterface, _mcastTtl);
}

int16_t
IceInternal::UdpConnector::type() const
{
    return _instance->type();
}

string
IceInternal::UdpConnector::toString() const
{
    return addrToString(_addr);
}

bool
IceInternal::UdpConnector::operator==(const Connector& r) const
{
    const UdpConnector* p = dynamic_cast<const UdpConnector*>(&r);
    if(!p)
    {
        return false;
    }
    if(compareAddress(_addr, p->_addr) != 0)
    {
        return false;
    }

    if(_connectionId != p->_connectionId)
    {
        return false;
    }

    if(_mcastTtl != p->_mcastTtl)
    {
        return false;
    }

    if(_mcastInterface != p->_mcastInterface)
    {
        return false;
    }

    if(compareAddress(_sourceAddr, p->_sourceAddr) != 0)
    {
        return false;
    }

    return true;
}

bool
IceInternal::UdpConnector::operator<(const Connector& r) const
{
    const UdpConnector* p = dynamic_cast<const UdpConnector*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    if(_connectionId < p->_connectionId)
    {
        return true;
    }
    else if(p->_connectionId < _connectionId)
    {
        return false;
    }

    if(_mcastTtl < p->_mcastTtl)
    {
        return true;
    }
    else if(p->_mcastTtl < _mcastTtl)
    {
        return false;
    }

    if(_mcastInterface < p->_mcastInterface)
    {
        return true;
    }
    else if(p->_mcastInterface < _mcastInterface)
    {
        return false;
    }

    int rc = compareAddress(_sourceAddr, p->_sourceAddr);
    if(rc < 0)
    {
        return true;
    }
    else if(rc > 0)
    {
        return false;
    }
    return compareAddress(_addr, p->_addr) == -1;
}

IceInternal::UdpConnector::UdpConnector(const ProtocolInstancePtr& instance, const Address& addr,
                                        const Address& sourceAddr, const string& mcastInterface, int mcastTtl,
                                        const std::string& connectionId) :
    _instance(instance),
    _addr(addr),
    _sourceAddr(sourceAddr),
    _mcastInterface(mcastInterface),
    _mcastTtl(mcastTtl),
    _connectionId(connectionId)
{
}

IceInternal::UdpConnector::~UdpConnector()
{
}
