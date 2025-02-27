//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef GLACIER2_ROUTER_I_H
#define GLACIER2_ROUTER_I_H

#include <Ice/Ice.h>
#include <Glacier2/Router.h>
#include <Glacier2/ClientBlobject.h>
#include <Glacier2/ServerBlobject.h>

namespace Glacier2
{

class RoutingTable;
class RouterI;
class FilterManager;

class RouterI final : public Router
{
public:

    RouterI(std::shared_ptr<Instance>, std::shared_ptr<Ice::Connection>, const std::string&,
            SessionPrxPtr, const Ice::Identity&, std::shared_ptr<FilterManager>, const Ice::Context&);

    void destroy(std::function<void(std::exception_ptr)>);

    Ice::ObjectPrxPtr getClientProxy(std::optional<bool>&, const Ice::Current&) const override;
    Ice::ObjectPrxPtr getServerProxy(const Ice::Current&) const override;
    Ice::ObjectProxySeq addProxies(Ice::ObjectProxySeq, const Ice::Current&) override;
    std::string getCategoryForClient(const Ice::Current&) const override;
    void createSessionAsync(std::string, std::string,
                            std::function<void(const SessionPrxPtr& returnValue)>,
                            std::function<void(std::exception_ptr)>, const Ice::Current&) override;
    void createSessionFromSecureConnectionAsync(
        std::function<void(const SessionPrxPtr& returnValue)>,
        std::function<void(std::exception_ptr)>, const Ice::Current&) override;
    void refreshSessionAsync(std::function<void()>,
                             std::function<void(std::exception_ptr)>,
                             const Ice::Current&) override;
    void destroySession(const Ice::Current&) override;
    std::int64_t getSessionTimeout(const Ice::Current&) const override;
    int getACMTimeout(const Ice::Current&) const override;

    std::shared_ptr<ClientBlobject> getClientBlobject() const;
    std::shared_ptr<ServerBlobject> getServerBlobject() const;

    SessionPrxPtr getSession() const;

    std::chrono::steady_clock::time_point getTimestamp() const;
    void updateTimestamp() const;

    void updateObserver(const std::shared_ptr<Glacier2::Instrumentation::RouterObserver>&);

    std::string toString() const;

private:

    const std::shared_ptr<Instance> _instance;
    const std::shared_ptr<RoutingTable> _routingTable;
    const Ice::ObjectPrxPtr _clientProxy;
    const Ice::ObjectPrxPtr _serverProxy;
    const std::shared_ptr<ClientBlobject> _clientBlobject;
    const std::shared_ptr<ServerBlobject> _serverBlobject;
    const bool _clientBlobjectBuffered;
    const bool _serverBlobjectBuffered;
    const std::shared_ptr<Ice::Connection> _connection;
    const std::string _userId;
    const SessionPrxPtr _session;
    const Ice::Identity _controlId;
    const Ice::Context _context;
    const std::mutex _timestampMutex;
    mutable std::chrono::steady_clock::time_point _timestamp;

    std::shared_ptr<Glacier2::Instrumentation::SessionObserver> _observer;
};

}

#endif
