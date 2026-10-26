#pragma once
#ifndef PEBO_DIAGNOSTIC
#error "Select diagnostic or compact through build.py --config"
#endif
#if PEBO_DIAGNOSTIC != 0 && PEBO_DIAGNOSTIC != 1
#error "Invalid PEBO_DIAGNOSTIC"
#endif
#ifndef PEBO_TRACE
#define PEBO_TRACE PEBO_DIAGNOSTIC
#endif
#ifndef PEBO_SYNTHETIC
#define PEBO_SYNTHETIC PEBO_DIAGNOSTIC
#endif
#ifndef PEBO_DUMPS
#define PEBO_DUMPS PEBO_DIAGNOSTIC
#endif
#ifndef PEBO_VERBOSE
#define PEBO_VERBOSE PEBO_DIAGNOSTIC
#endif
#if !PEBO_DIAGNOSTIC && (PEBO_TRACE || PEBO_SYNTHETIC || PEBO_DUMPS || PEBO_VERBOSE)
#error "compact cannot enable diagnostic-only features"
#endif
#ifndef PEBO_BUILD_NAME
#error "Missing build identity"
#endif
namespace Build {
constexpr const char *name=PEBO_BUILD_NAME;
constexpr unsigned features=PEBO_TRACE|(PEBO_SYNTHETIC<<1)|(PEBO_DUMPS<<2)|(PEBO_VERBOSE<<3);
}
