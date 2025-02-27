//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_BT_CONNECTOR_I_H
#define ICE_BT_CONNECTOR_I_H

#include <IceBT/Config.h>
#include <IceBT/Engine.h>
#include <IceBT/InstanceF.h>

#include <Ice/TransceiverF.h>
#include <Ice/Connector.h>

namespace IceBT
{

class ConnectorI final : public IceInternal::Connector
{
public:

    ConnectorI(const InstancePtr&, const std::string&, const std::string&, std::int32_t, const std::string&);
    IceInternal::TransceiverPtr connect() final;

    std::int16_t type() const final;
    std::string toString() const final;

    bool operator==(const IceInternal::Connector&) const final;
    bool operator<(const IceInternal::Connector&) const final;

private:

    const InstancePtr _instance;
    const std::string _addr;
    const std::string _uuid;
    const std::int32_t _timeout;
    const std::string _connectionId;
};

}

#endif
