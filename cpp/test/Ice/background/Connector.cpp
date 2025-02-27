//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_API_EXPORTS
#   define TEST_API_EXPORTS
#endif

#include <Connector.h>
#include <Transceiver.h>
#include <EndpointI.h>

using namespace std;

IceInternal::TransceiverPtr
Connector::connect()
{
    _configuration->checkConnectException();
    return make_shared<Transceiver>(_connector->connect());
}

int16_t
Connector::type() const
{
    return (int16_t)(EndpointI::TYPE_BASE + _connector->type());
}

string
Connector::toString() const
{
    return _connector->toString();
}

bool
Connector::operator==(const IceInternal::Connector& r) const
{
    const Connector* p = dynamic_cast<const Connector*>(&r);
    if(!p)
    {
        return false;
    }

    return *_connector == *p->_connector;
}

bool
Connector::operator<(const IceInternal::Connector& r) const
{
    const Connector* p = dynamic_cast<const Connector*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    return *_connector < *p->_connector;
}

Connector::Connector(const IceInternal::ConnectorPtr& connector) :
    _connector(connector),
    _configuration(Configuration::getInstance())
{
}
