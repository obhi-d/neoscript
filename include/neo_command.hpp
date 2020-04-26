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
/// Parameter Layout
class NEO_API command
{
public:
  enum
  {
    k_param_single = 1,
    k_param_list   = 2
  };

  class NEO_API single
  {
  public:
    inline single() = default;
    inline single(single const& _) : name_(_.name_), value_(_.value_) {}
    inline single(single&&) = default;

    inline single(std::string_view value) : value_(value) {}
    inline single(std::string const& value) : value_(value) {}
    inline single(std::string&& value) : value_(std::move(value)) {}
    inline single(std::string const& name, std::string const& value)
        : name_(name), value_(value)
    {}
    inline single(std::string&& name, std::string&& value)
        : name_(std::move(name)), value_(std::move(value))
    {}

    inline single& operator=(single const& _)
    {
      name_  = _.name_;
      value_ = _.value_;
      return *this;
    }
    inline single& operator=(single&&) = default;

    inline void set_name(std::string&& name) { name_ = std::move(name); }
    inline void set_value(std::string&& value) { value_ = std::move(value); }

    inline std::string const& name() const noexcept { return name_; }
    inline std::string const& value() const noexcept { return value_; }

  private:
    std::string name_;
    std::string value_;
  };

  class NEO_API list
  {
  public:
    inline list() = default;
    inline list(list const& _) : value_(_.value_), name_(_.name_) {}
    inline list(list&&) = default;

    inline list& operator=(list const& _)
    {
      value_ = _.value_;
      name_  = _.name_;
      return *this;
    }
    inline list& operator=(list&&) = default;

    using node           = std::variant<std::monostate, single, list>;
    using vector         = std::vector<node>;
    using iterator       = vector::iterator;
    using const_iterator = vector::const_iterator;

    inline iterator       begin() noexcept { return value_.begin(); }
    inline const_iterator begin() const noexcept { return value_.begin(); }
    inline iterator       end() noexcept { return value_.end(); }
    inline const_iterator end() const noexcept { return value_.end(); }

    inline void set_name(std::string&& name) noexcept
    {
      name_ = std::move(name);
    }

    inline void set_value(std::string&&) noexcept {}

    inline void emplace_back(node&& i_param) noexcept
    {
      value_.emplace_back(std::move(i_param));
    }

    inline std::size_t count() const noexcept { return value_.size(); }
    inline node const& at(std::uint32_t i) const noexcept { return value_[i]; }
    inline std::string const& name() const { return name_; }

    inline list::vector&       value() { return value_; }
    inline list::vector const& value() const { return value_; }

  private:
    vector      value_;
    std::string name_;
  };

  using param_t = list::node;

  struct resolver
  {
    std::function<std::optional<param_t>(std::string_view name,
                                         std::string_view value)>
              op_;
    resolver* next_ = nullptr;
  };

  inline static void resolve(resolver const* stack, list::vector& vec)
  {
    for (auto& e : vec)
      std::visit(overloaded{[](std::monostate&) {},
                            [&stack](list& l) { resolve(stack, l.value()); },
                            [&stack, &e](single& s)
                            {
                              for (auto it = stack; it; it = it->next_)
                              {
                                std::optional<list::node> res =
                                    it->op_(s.name(), s.value());
                                if (res)
                                {
                                  e = res.value();
                                  break;
                                }
                              }
                            }},
                 e);
  }

  struct NEO_API parameters
  {
    inline parameters() = default;
    inline parameters(parameters const& _) : value_(_.value_) {}
    inline parameters(parameters&&) = default;

    inline parameters& operator=(parameters const& _)
    {
      value_ = _.value_;
      return *this;
    }
    inline parameters& operator=(parameters&&) = default;

    inline void append(param_t&& i_param) noexcept
    {
      value_.emplace_back(std::move(i_param));
    }
    inline void append_expanded(param_t&& i_param) noexcept
    {
      if (i_param.index() == 2)
      {
        list& l = std::get<list>(i_param);
        value_.reserve(value_.size() + l.count());
        value_.insert(value_.end(), l.begin(), l.end());
      }
      else
        value_.emplace_back(std::move(i_param));
    }
    inline void resolve(resolver const* stack) noexcept
    {
      command::resolve(stack, value_);
    }
    inline static void set_name(param_t& _, std::string&& name) noexcept
    {
      std::visit(overloaded{[](std::monostate& s) {}, [&name](auto& other)
                            { other.set_name(std::move(name)); }},
                 _);
    }

    list::vector&       value() noexcept { return value_; }
    list::vector const& value() const noexcept { return value_; }

    list::vector value_;
  };

  inline command() noexcept = default;
  inline command(command const& _)
      : name_(_.name_), params_(_.params_), scoped_(_.scoped_)
  {}
  inline command(command&&) noexcept = default;
  inline command(std::string&& name, parameters&& params, bool scoped) noexcept
      : name_(std::move(name)), params_(std::move(params)), scoped_(scoped)
  {}

  inline command& operator=(command const& _)
  {
    name_   = _.name_;
    params_ = _.params_;
    scoped_ = _.scoped_;
    return *this;
  }

  inline command& operator=(command&& _)
  {
    name_   = std::move(_.name_);
    params_ = std::move(_.params_);
    scoped_ = std::move(_.scoped_);
    return *this;
  }

  inline std::string_view  name() const { return name_; }
  inline parameters const& params() const { return params_; }
  inline parameters&       params() { return params_; }

  inline operator bool() const { return name_.length() > 0; }

  inline bool is_scoped() const { return scoped_; }
  inline void resolve(resolver const* stack) { params_.resolve(stack); }

private:
  std::string name_;
  parameters  params_;
  bool        scoped_ = false;
};

using resolver = neo::command::resolver;
} // namespace neo