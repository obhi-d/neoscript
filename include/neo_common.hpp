#pragma once
#include <cassert>
#include <vector>
#include <string_view>
#include <string>
#include <variant>
#include <memory>

#ifdef NEO_DYN_LIB_
#if defined _WIN32 || defined __CYGWIN__
# ifdef NEO_EXPORT_
# if defined(__GNUC__) || defined(__clang__)
#     define NEO_API __attribute__((dllexport))
#   else
#     define NEO_API __declspec(dllexport)
#   endif
# else
#   if defined(__GNUC__) || defined(__clang__)
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

#if defined(__GNUC__) || defined(__clang__)
# define NEO_FORCEINLINE inline __attribute__((always_inline))
#else
# define NEO_FORCEINLINE __forceinline
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
struct fixed_string
{
  std::unique_ptr<char[]> fixed_;
  std::size_t             length_ = 0;

  fixed_string(std::unique_ptr<char[]> other, std::size_t length)
      : fixed_(std::move(other)), length_(length)
  {}
  fixed_string(std::string_view other)
      : fixed_(new char[other.length()]), length_(other.length())
  {
    std::memcpy(fixed_.get(), other.data(), other.length());
  }
  fixed_string() = default;
  fixed_string(fixed_string const&) = default;
  fixed_string(fixed_string &&) = default;
  fixed_string& operator=(fixed_string const&) = default;
  fixed_string& operator=(fixed_string&&) = default;

  std::size_t length() const { return length_; }
  std::string_view as_view() const
  {
    return std::string_view(fixed_.get(), length_);
  }
};

using flex_string = std::variant<std::string_view, fixed_string>;
} // namespace neo