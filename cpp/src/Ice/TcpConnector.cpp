//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Config.h>

#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0

#include <Ice/TcpConnector.h>
#include <Ice/TcpTransceiver.h>
#include <Ice/TcpEndpointI.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Network.h>
#include <Ice/NetworkProxy.h>
#include <Ice/StreamSocket.h>
#include <Ice/Exception.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

TransceiverPtr
IceInternal::TcpConnector::connect()
{
    return make_shared<TcpTransceiver>(_instance, make_shared<StreamSocket>(_instance, _proxy, _addr, _sourceAddr));
}

int16_t
IceInternal::TcpConnector::type() const
{
    return _instance->type();
}

string
IceInternal::TcpConnector::toString() const
{
    return addrToString(!_proxy ? _addr : _proxy->getAddress());
}

bool
IceInternal::TcpConnector::operator==(const Connector& r) const
{
    const TcpConnector* p = dynamic_cast<const TcpConnector*>(&r);
    if(!p)
    {
        return false;
    }

    if(compareAddress(_addr, p->_addr) != 0)
    {
        return false;
    }

    if(_timeout != p->_timeout)
    {
        return false;
    }

    if(compareAddress(_sourceAddr, p->_sourceAddr) != 0)
    {
        return false;
    }

    if(_connectionId != p->_connectionId)
    {
        return false;
    }

    return true;
}

bool
IceInternal::TcpConnector::operator<(const Connector& r) const
{
    const TcpConnector* p = dynamic_cast<const TcpConnector*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    if(_timeout < p->_timeout)
    {
        return true;
    }
    else if(p->_timeout < _timeout)
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

    if(_connectionId < p->_connectionId)
    {
        return true;
    }
    else if(p->_connectionId < _connectionId)
    {
        return false;
    }
    return compareAddress(_addr, p->_addr) < 0;
}

IceInternal::TcpConnector::TcpConnector(const ProtocolInstancePtr& instance, const Address& addr,
                                        const NetworkProxyPtr& proxy, const Address& sourceAddr,
                                        int32_t timeout, const string& connectionId) :
    _instance(instance),
    _addr(addr),
    _proxy(proxy),
    _sourceAddr(sourceAddr),
    _timeout(timeout),
    _connectionId(connectionId)
{
}

IceInternal::TcpConnector::~TcpConnector()
{
}
#endif
