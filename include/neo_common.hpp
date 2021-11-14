#pragma once
#include <cassert>
#include <vector>
#include <string_view>

#ifdef NEO_DYN_LIB_
#if defined _WIN32 || defined __CYGWIN__
# ifdef NEO_EXPORT_
#   ifdef __GNUC__
#     define NEO_API __attribute__((dllexport))
#   else
#     define NEO_API __declspec(dllexport)
#   endif
# else
#   ifdef __GNUC__
#     define NEO_API __attribute__((dllimport))
#   else
#     define NEO_API __declspec(dllimport)
#   endif
# endif
#else
# if __GNUC__ >= 4
#   define NEO_API __attribute__((visibility("default")))
# elif defined(__clang__)
#   define NEO_API __attribute__((visibility("default")))
# else
#   define NEO_API
# endif
#endif
#else
# define NEO_API
#endif

namespace neo
{

#ifdef NEO_SMALL_VECTOR
template <typename T, unsigned N>
using vector = PPR_SMALL_VECTOR<T, N>;
#else
template <typename T, unsigned N>
using vector = std::vector<T>;
#endif

// helper type for the visitor #4
template <class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

struct text_content
{
  vector<std::string_view, 4> fragments;

  text_content() noexcept = default;
  text_content(text_content const&) noexcept = default;
  text_content(text_content&&) noexcept = default;
  text_content& operator=(text_content const&) noexcept = default;
  text_content& operator=(text_content&&) noexcept = default;

  void append_to(std::string& o) const
  {
    for (auto f : fragments)
      o += f;
  }
};

} // namespace neo