#pragma once
#include <cassert>

#ifdef NEO_DYN_LIB_
#if defined _WIN32 || defined __CYGWIN__
#ifdef NEO_EXPORT_
// Exporting...
#ifdef __GNUC__
#define NEO_API __attribute__((dllexport))
#else
#define NEO_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define NEO_API __attribute__((dllimport))
#else
#define NEO_API __declspec(dllimport)
#endif
#endif
#else
#if __GNUC__ >= 4
#define NEO_API __attribute__((visibility("default")))
#else
#define NEO_API
#endif
#endif
#else
#define NEO_API
#endif

namespace neo
{
// helper type for the visitor #4
template <class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
} // namespace neo