#pragma once
#include <neo_parameter.hpp>

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

  using list       = neo::list;
  using single     = neo::single;
  using esq_string = neo::esq_string;
  using param_t    = list::node;

  inline static std::string_view const& as_string(
      param_t const& source, std::string_view default_val = "",
      std::uint32_t pref_index = 0) noexcept
  {
    param_t const* p_source = &source;
    while (true)
    {
      switch (source.index())
      {
      default:
        return default_val;
      case 1:
        return std::get<1>(*p_source).value();
      case 2:
      {
        list const& l = std::get<2>(*p_source);
        if (l.count() > pref_index)
          p_source = &l.at(pref_index);
        else
          return default_val;
      }
      case 3:
        return std::get<3>(*p_source).value();
      }
    }
  }

  struct resolver
  {
    std::function<std::optional<param_t>(std::string_view name,
                                         std::string_view value)>
              op_;
    resolver* next_ = nullptr;
  };

  inline static void resolve(resolver const* stack, list::vector& vec) noexcept
  {
    for (auto& e : vec)
      std::visit(overloaded{[](std::monostate&) {},
                            [&stack](list& l) { resolve(stack, l.value()); },
                            [&stack, &e](auto& s)
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
    inline parameters() noexcept = default;
    inline parameters(parameters const& _) noexcept : value_(_.value_) {}
    inline parameters(parameters&&) noexcept = default;

    inline parameters& operator=(parameters const& _) noexcept
    {
      value_ = _.value_;
      return *this;
    }
    inline parameters& operator=(parameters&&) noexcept = default;

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
    inline static void set_name(param_t& _, std::string_view name) noexcept
    {
      std::visit(overloaded{[](std::monostate& s) {},
                            [name](auto& other) { other.set_name(name); }},
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
  inline command(std::string_view name, parameters&& params,
                 bool scoped) noexcept
      : name_(std::move(name)), params_(std::move(params)), scoped_(scoped)
  {}

  inline command& operator=(command const& _)
  {
    name_   = _.name_;
    params_ = _.params_;
    scoped_ = _.scoped_;
    return *this;
  }

  inline command& operator=(command&& _) noexcept
  {
    name_   = std::move(_.name_);
    params_ = std::move(_.params_);
    scoped_ = std::move(_.scoped_);
    return *this;
  }

  inline std::string_view  name() const noexcept { return name_; }
  inline parameters const& params() const noexcept { return params_; }
  inline parameters&       params() noexcept { return params_; }

  inline operator bool() const noexcept { return name_.length() > 0; }

  inline bool is_scoped() const noexcept { return scoped_; }
  inline void resolve(resolver const* stack) noexcept
  {
    params_.resolve(stack);
  }

  
  inline static std::string_view const& as_string(
      parameters const& source, std::string_view default_val = "",
      std::uint32_t pref_index = 0) noexcept
  {
    auto const& p_source = source.value();
    if (pref_index < p_source.size())
    {
      auto& source = p_source[pref_index];
      switch (source.index())
      {
      case 1:
        return std::get<1>(source).value();
      }
    }
    return default_val;
  }

private:
  std::string_view name_;
  parameters       params_;
  bool             scoped_ = false;
};

using resolver = neo::command::resolver;
} // namespace neo