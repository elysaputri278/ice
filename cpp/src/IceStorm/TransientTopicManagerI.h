//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TRANSIENT_TOPIC_MANAGER_I_H
#define TRANSIENT_TOPIC_MANAGER_I_H

#include <IceStorm/IceStormInternal.h>

namespace IceStorm
{

class Instance;
class TransientTopicImpl;

class TransientTopicManagerImpl final : public TopicManagerInternal
{
public:

    TransientTopicManagerImpl(std::shared_ptr<Instance>);

    // TopicManager methods.
    TopicPrxPtr create(std::string, const Ice::Current&) override;
    TopicPrxPtr retrieve(std::string, const Ice::Current&) override;
    TopicDict retrieveAll(const Ice::Current&) override;
    IceStormElection::NodePrxPtr getReplicaNode(const Ice::Current&) const override;

    void reap();
    void shutdown();

private:

    const std::shared_ptr<Instance> _instance;
    std::map<std::string, std::shared_ptr<TransientTopicImpl>> _topics;

    std::mutex _mutex;
};

} // End namespace IceStorm

#endif
