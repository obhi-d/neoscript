
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace neo {

/// Parameter Layout
class command {
public:
  class single {
  public:
    single()              = default;
    single(single const&) = default;
    single(single&&)      = default;

    single(std::string const& value) : value_(value) {}
    single(std::string&& value) : value_(std::move(value)) {}
    single(std::string const& name, std::string const& value)
        : name_(name), value_(value) {}
    single(std::string&& name, std::string&& value) : name_(std::move(name)), value_(std::move(value)) {}

    single& operator=(single const&) = default;
    single& operator=(single&&) = default;

    void set_name(std::string&& name) { name_ = std::move(name); }
    void set_value(std::string&& value) { value_ = std::move(value); }

    std::string_view name() const noexcept { return name_; }
    std::string_view value() const noexcept { return name_; }

  private:
    std::string name_;
    std::string value_;
  };

  class list {
  public:
    list()            = default;
    list(list const&) = default;
    list(list&&)      = default;

    list& operator=(list const&) = default;
    list& operator=(list&&) = default;

    using node           = std::variant<std::monostate, single, list>;
    using vector         = std::vector<node>;
    using iterator       = vector::iterator;
    using const_iterator = vector::const_iterator;

    iterator       begin() noexcept { return value_.begin(); }
    const_iterator begin() const noexcept { return value_.begin(); }
    iterator       end() noexcept { return value_.end(); }
    const_iterator end() const noexcept { return value_.end(); }

    void set_name(std::string&& name) noexcept { name_ = std::move(name); }
    void set_value(std::string&&) noexcept {}

    void emplace_back(node&& i_param) noexcept {
      value_.emplace_back(std::move(i_param));
    }

    std::size_t count() const noexcept { return value_.size(); }
    node const& at(std::uint32_t i) const noexcept { return *value_[i].get(); }
    std::string_view name() const { return name_; }

  private:
    vector      value_;
    std::string name_;
  };

  using param_t = list::node;
  struct parameters {
    parameters()                  = default;
    parameters(parameters const&) = default;
    parameters(parameters&&)      = default;

    parameters& operator=(parameters const&) = default;
    parameters& operator=(parameters&&) = default;

    inline void append(param_t&& i_param) noexcept {
      value_.emplace_back(std::move(i_param));
    }
    inline void append_expanded(param_t&& i_param) noexcept {
      if (i_param.index() == 2) {
        list& l = std::get<list>(i_param);
        value_.reserve(value_.size() + l.size());
        value_.insert(value_.end(), l.begin(), l.end());
      } else
        value_.emplace_back(std::move(i_param));
    }

    list::vector value_;
  };

  command() = default;
  command(std::string&& name, parameters&& params)
      : name_(std::move(name)), params_(std::move(params)) {}

  std::string_view    name() const { return name_; }
  list::vector const& params() const { return params_; }
  list::vector&       params() { return params_; }

  operator bool() const { return name_.length() > 0; }

private:
  std::string name_;
  parameters  params_;
  bool scoped_;
};

} // namespace neo