//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_SCANNER_CONFIG_H
#define ICE_UTIL_SCANNER_CONFIG_H

#include <IceUtil/Config.h> // Required by generated Scanners.

//
// COMPILERFIX: VC compilers does not provide stdint.h header until v100
// the header must be included before that macros for integral types
// in flex generated Scanners are defined.
//
// in C99 conformant compilers we don't need to include it because the
// header is included by inttypes.h, that is included by the generated
// Scanners.
//
#if defined(_MSC_VER)
#   include <stdint.h>
#endif

//
// Clang++ >= 5.1 and VC++ using C++17 standard deprecate 'register' storage
// class specifier used by lex generated Scanners.
//
#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wdeprecated-register"
#elif defined(_MSC_VER)
#   pragma warning(disable:5033)
#endif

#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wconversion"
#   pragma clang diagnostic ignored "-Wsign-conversion"
#   pragma clang diagnostic ignored "-Wdocumentation"
#   pragma clang diagnostic ignored "-Wunused-but-set-variable"
#endif

#ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wunused-function"
#  pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

#endif
