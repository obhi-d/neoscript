#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <neo_common.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace neo
{

class NEO_API single
{
public:
  inline single() noexcept = default;
  inline single(single const& _) noexcept : name_(_.name_), value_(_.value_) {}
  inline single(single&&) noexcept = default;

  inline single(std::string_view value) noexcept : value_(value) {}
  inline single(std::string_view name, std::string_view value) noexcept
      : name_(name), value_(value)
  {}

  inline single& operator=(single const& _) noexcept
  {
    name_  = _.name_;
    value_ = _.value_;
    return *this;
  }
  inline single& operator=(single&&) noexcept = default;

  inline void set_name(std::string_view name) noexcept
  {
    name_ = std::move(name);
  }
  inline void set_value(std::string_view value) noexcept
  {
    value_ = std::move(value);
  }

  inline std::string_view name() const noexcept { return name_; }
  inline std::string_view value() const noexcept { return value_; }
  // Return either name or value iff name is empty.
  inline std::string_view nonempty() const noexcept
  {
    return name_.empty() ? value_ : name_;
  }

private:
  std::string_view name_;
  std::string_view value_;
};

// A string which had \" character, this character was replaced by "
// hence it was stored in a std::string
class NEO_API esq_string
{
public:
  inline esq_string() noexcept = default;
  inline esq_string(esq_string const& _) noexcept
      : name_(_.name_), value_(std::move(fixed_string(_.value_.as_view())))
  {}
  inline esq_string(esq_string&&) noexcept = default;

  inline esq_string(neo::fixed_string value) noexcept : value_(std::move(value)) {}
  inline esq_string(std::string_view name, neo::fixed_string value) noexcept
      : name_(name), value_(std::move(value))
  {}

  inline esq_string& operator=(esq_string const& _) noexcept
  {
    name_  = _.name_;
    value_ = std::move(fixed_string( _.value_.as_view()));
    return *this;
  }
  inline esq_string& operator=(esq_string&&) noexcept = default;

  inline void set_name(std::string_view name) noexcept
  {
    name_ = std::move(name);
  }
  inline void set_value(std::string_view value) noexcept
  {
    value_ = std::move(fixed_string(value));
  }

  inline std::string_view name() const noexcept { return name_; }
  inline std::string_view value() const noexcept { return value_.as_view(); }
  // Return either name or value iff name is empty.
  inline std::string_view nonempty() const noexcept
  {
    return name_.empty() ? value_.as_view() : name_;
  }

private:
  std::string_view name_;
  fixed_string     value_;
};

class NEO_API list
{
public:
  inline list() noexcept = default;
  inline list(list const& _) noexcept : value_(_.value_), name_(_.name_) {}
  inline list(list&&) noexcept = default;

  inline list& operator=(list const& _) noexcept
  {
    value_ = _.value_;
    name_  = _.name_;
    return *this;
  }
  inline list& operator=(list&&) noexcept = default;

  using node           = std::variant<std::monostate, single, list, esq_string>;
  using vector         = std::vector<node>;
  using iterator       = vector::iterator;
  using const_iterator = vector::const_iterator;

  inline iterator       begin() noexcept { return value_.begin(); }
  inline const_iterator begin() const noexcept { return value_.begin(); }
  inline iterator       end() noexcept { return value_.end(); }
  inline const_iterator end() const noexcept { return value_.end(); }

  inline void set_name(std::string_view name) noexcept
  {
    name_ = std::move(name);
  }

  inline void set_value(std::string_view) noexcept {}

  inline void emplace_back(node&& i_param) noexcept
  {
    value_.emplace_back(std::move(i_param));
  }

  inline std::size_t size() const noexcept { return value_.size(); }
  inline std::size_t count() const noexcept { return value_.size(); }
  inline node const& at(std::uint32_t i) const noexcept { return value_[i]; }
  inline std::string_view name() const noexcept { return name_; }

  inline list::vector&       value() noexcept { return value_; }
  inline list::vector const& value() const noexcept { return value_; }

private:
  vector           value_;
  std::string_view name_;
};
} // namespace neo
