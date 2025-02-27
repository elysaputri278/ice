//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceStorm/Instrumentation.h>
#include <IceUtil/PushDisableWarnings.h>
#include <Ice/LocalException.h>
#include <Ice/ValueFactory.h>
#include <Ice/InputStream.h>
#include <Ice/OutputStream.h>
#include <Ice/LocalException.h>
#include <IceUtil/PopDisableWarnings.h>

#if defined(_MSC_VER)
#   pragma warning(disable:4458) // declaration of ... hides class member
#elif defined(__clang__)
#   pragma clang diagnostic ignored "-Wshadow"
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wshadow"
#endif

IceStorm::Instrumentation::TopicObserver::~TopicObserver()
{
}

IceStorm::Instrumentation::SubscriberObserver::~SubscriberObserver()
{
}

IceStorm::Instrumentation::ObserverUpdater::~ObserverUpdater()
{
}

IceStorm::Instrumentation::TopicManagerObserver::~TopicManagerObserver()
{
}
