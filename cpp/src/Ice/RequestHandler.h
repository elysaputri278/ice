//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_REQUEST_HANDLER_H
#define ICE_REQUEST_HANDLER_H

#include <Ice/RequestHandlerF.h>
#include <Ice/ReferenceF.h>
#include <Ice/ProxyF.h>
#include <Ice/ConnectionIF.h>

namespace Ice
{

class LocalException;

}

namespace IceInternal
{

class OutgoingAsyncBase;
class ProxyOutgoingAsyncBase;

using OutgoingAsyncBasePtr = std::shared_ptr<OutgoingAsyncBase>;
using ProxyOutgoingAsyncBasePtr = std::shared_ptr<ProxyOutgoingAsyncBase>;

//
// An exception wrapper, which is used to notify that the request
// handler should be cleared and the invocation retried.
//
class RetryException
{
public:

    RetryException(std::exception_ptr);
    RetryException(const RetryException&);

    std::exception_ptr get() const;

private:

    std::exception_ptr _ex;
};

class CancellationHandler
{
public:

    virtual void asyncRequestCanceled(const OutgoingAsyncBasePtr&, std::exception_ptr) = 0;
};

class RequestHandler : public CancellationHandler
{
public:

    RequestHandler(const ReferencePtr&);

    virtual AsyncStatus sendAsyncRequest(const ProxyOutgoingAsyncBasePtr&) = 0;

    virtual Ice::ConnectionIPtr getConnection() = 0;
    virtual Ice::ConnectionIPtr waitForConnection() = 0;

protected:

    const ReferencePtr _reference;
    const bool _response;
};

}

#endif
