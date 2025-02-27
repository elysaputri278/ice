//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Glacier2/Instrumentation.h>
#include <IceUtil/PushDisableWarnings.h>
#include <Ice/InputStream.h>
#include <Ice/OutputStream.h>
#include <IceUtil/PopDisableWarnings.h>

#if defined(_MSC_VER)
#   pragma warning(disable:4458) // declaration of ... hides class member
#elif defined(__clang__)
#   pragma clang diagnostic ignored "-Wshadow"
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wshadow"
#endif

Glacier2::Instrumentation::SessionObserver::~SessionObserver()
{
}

Glacier2::Instrumentation::ObserverUpdater::~ObserverUpdater()
{
}

Glacier2::Instrumentation::RouterObserver::~RouterObserver()
{
}
