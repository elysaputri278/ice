//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEBOX_API_EXPORTS
#   define ICEBOX_API_EXPORTS
#endif
#include <IceBox/Service.h>
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

IceBox::FailureException::~FailureException()
{
}

std::string_view
IceBox::FailureException::ice_staticId()
{
    static constexpr std::string_view typeId = "::IceBox::FailureException";
    return typeId;
}

IceBox::Service::~Service()
{
}
