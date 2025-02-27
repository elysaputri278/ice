//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_ENDPOINT_I_H
#define TEST_ENDPOINT_I_H

#include <Ice/EndpointI.h>
#include <Test.h>
#include <Configuration.h>

class EndpointI;
using EndpointIPtr = std::shared_ptr<EndpointI>;

class EndpointI : public IceInternal::EndpointI, public std::enable_shared_from_this<EndpointI>
{
public:

    static std::int16_t TYPE_BASE;

    EndpointI(const IceInternal::EndpointIPtr&);

    // From EndpointI
    virtual void streamWriteImpl(Ice::OutputStream*) const;
    virtual std::int16_t type() const;
    virtual const std::string& protocol() const;
    virtual IceInternal::EndpointIPtr timeout(std::int32_t) const;
    virtual IceInternal::EndpointIPtr connectionId(const ::std::string&) const;
    virtual IceInternal::EndpointIPtr compress(bool) const;
    virtual IceInternal::TransceiverPtr transceiver() const;
    virtual void connectorsAsync(
        Ice::EndpointSelectionType,
        std::function<void(std::vector<IceInternal::ConnectorPtr>)>,
        std::function<void(std::exception_ptr)>) const;
    virtual IceInternal::AcceptorPtr acceptor(const std::string&) const;
    virtual std::vector<IceInternal::EndpointIPtr> expandIfWildcard() const;
    virtual std::vector<IceInternal::EndpointIPtr> expandHost(IceInternal::EndpointIPtr&) const;
    virtual bool equivalent(const IceInternal::EndpointIPtr&) const;

    // From TestEndpoint
    virtual std::string toString() const noexcept;
    virtual Ice::EndpointInfoPtr getInfo() const noexcept;
    virtual std::int32_t timeout() const;
    virtual const std::string& connectionId() const;
    virtual bool compress() const;
    virtual bool datagram() const;
    virtual bool secure() const;

    virtual bool operator==(const Ice::Endpoint&) const;
    virtual bool operator<(const Ice::Endpoint&) const;

    virtual int hash() const;
    virtual std::string options() const;

    IceInternal::EndpointIPtr delegate() const;
    EndpointIPtr endpoint(const IceInternal::EndpointIPtr&) const;

    using IceInternal::EndpointI::connectionId;

private:

    friend class EndpointFactory;

    const IceInternal::EndpointIPtr _endpoint;
    const ConfigurationPtr _configuration;
};

#endif
