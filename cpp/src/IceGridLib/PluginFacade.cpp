//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEGRID_API_EXPORTS
#   define ICEGRID_API_EXPORTS
#endif
#include <IceGrid/PluginFacade.h>
#include <IceUtil/PushDisableWarnings.h>
#include <Ice/LocalException.h>
#include <Ice/ValueFactory.h>
#include <Ice/InputStream.h>
#include <Ice/OutputStream.h>
#include <Ice/LocalException.h>
#include <Ice/SlicedData.h>
#include <IceUtil/PopDisableWarnings.h>

#if defined(_MSC_VER)
#   pragma warning(disable:4458) // declaration of ... hides class member
#elif defined(__clang__)
#   pragma clang diagnostic ignored "-Wshadow"
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wshadow"
#endif

IceGrid::ReplicaGroupFilter::~ReplicaGroupFilter()
{
}

IceGrid::TypeFilter::~TypeFilter()
{
}

IceGrid::RegistryPluginFacade::~RegistryPluginFacade()
{
}
