#include <unordered_map>
#include <neo_command.hpp>
#include <neo_context.hpp>
#include <string_view>

namespace neo {

template <typename Ts, typename RetTy, typename... Args>
struct callback_t {

  using function_t = RetTy (Ts::*)(Args...);
  template <typename Td>
  using derived_function_t = RetTy (Td::*)(Args...);
  Ts*        _             = nullptr;
  function_t fptr          = nullptr;

  template <typename Td>
  callback_t(Td* i_obj, derived_function_t<Td> i_fp)
      : _(static_cast<Td*>(i_obj)), fptr(static_cast<function_t>(i_pf)) {}
  callback_t(callback_t const& i_cbk) = default;
  callback_t(callback_t&& i_cbk)      = default;

  callback_t& operator=(callback_t const& i_cbk) = default;
  callback_t& operator=(callback_t&& i_cbk) = default;

  inline RetTy operator()(Args... args) { return _->(*fptr)(args...); }

  template <typename Td>
  static callback_t bind(Td* i_obj, derived_function_t<Td> i_fp) {
    return callback_t(i_obj, i_fp);
  }
};

struct command_handler {};

using command_handler_t = callback_t<command_handler, bool, neo::context const&, neo::command const&>;

struct interpreter {
  struct block;
  struct handler {
    command_handler_t callback_;
    std::uint32_t     sub_handlers_;
  };
  struct block {
    std::uint32_t                                 parent_ = nullptr;
    std::unordered_map<std::string_view, handler> events_;
  };

  // block 0 is always the root
  std::vector<block> blocks_;
};
}