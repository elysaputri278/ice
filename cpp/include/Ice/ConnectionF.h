//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __Ice_ConnectionF_h__
#define __Ice_ConnectionF_h__

#include <IceUtil/PushDisableWarnings.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <optional>
#include <IceUtil/UndefSysMacros.h>

#ifndef ICE_API
#   if defined(ICE_STATIC_LIBS)
#       define ICE_API /**/
#   elif defined(ICE_API_EXPORTS)
#       define ICE_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace Ice
{

class ConnectionInfo;
class WSConnectionInfo;
class Connection;

}

/// \cond INTERNAL
namespace Ice
{

using ConnectionInfoPtr = ::std::shared_ptr<ConnectionInfo>;
using WSConnectionInfoPtr = ::std::shared_ptr<WSConnectionInfo>;
using ConnectionPtr = ::std::shared_ptr<Connection>;

}
/// \endcond

#include <IceUtil/PopDisableWarnings.h>
#endif
