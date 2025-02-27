//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef SERVANT_LOCATOR_I_H
#define SERVANT_LOCATOR_I_H

#include <Ice/Ice.h>

class ServantLocatorI final : public Ice::ServantLocator
{
public:

    ServantLocatorI();
    ~ServantLocatorI() final;

    std::shared_ptr<Ice::Object> locate(const Ice::Current&, ::std::shared_ptr<void>&) final;
    void finished(const Ice::Current&, const std::shared_ptr<Ice::Object>&, const ::std::shared_ptr<void>&) final;
    void deactivate(const std::string&) final;

public:

    bool _deactivated;
    std::shared_ptr<Ice::Object> _router;
};

#endif
