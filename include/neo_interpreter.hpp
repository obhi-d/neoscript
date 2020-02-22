#pragma once
#include <neo_command.hpp>
#include <neo_context.hpp>
#include <string_view>
#include <unordered_map>

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

  inline RetTy operator()(Args... args) { return ((*_).*(fptr))(args...); }

  template <typename Td>
  static callback_t bind(Td* i_obj, derived_function_t<Td> i_fp) {
    return callback_t(i_obj, i_fp);
  }
};

struct command_handler {};

using command_handler_t =
    callback_t<command_handler, bool, neo::context const&, neo::command const&>;

using command_id = std::uint32_t;

struct interpreter {
  struct block;
  struct handler {
    command_handler_t cbk_;
    std::uint32_t     sub_handlers_;
  };
  struct block {
    std::unordered_map<std::string_view, handler> events_;
    command_handler_t                             any_;
  };

  interpreter() {
    reg_block_mappings_[""] = 0;
    blocks_.emplace_back();
  }

  std::uint32_t execute(std::uint32_t block, neo::context& ctx,
                        neo::command& cmd) {
    auto& block_ref = blocks_[block];
    auto  it        = block_ref.events_.find(cmd.name());
    if (it != block_ref.events_.end()) {
      (*it).second.cbk_(ctx, cmd);
      return (*it).second.sub_handlers_;
    }
    return block;
  }

  std::uint32_t get_region_root(std::string_view name) const {
    auto it = reg_block_mappings_.find(name);
    if (it != reg_block_mappings_.end())
      return (*it).second;
    return 0;
  } // region type to block mapping

  std::uint32_t get_region_root(std::string_view name) {
    auto it = reg_block_mappings_.find(name);
    if (it != reg_block_mappings_.end())
      return (*it).second;
    auto id = static_cast<std::uint32_t>(blocks_.size());
    reg_block_mappings_.emplace(std::move(name), id);
    blocks_.resize(id + 1);
    return id;
  } // region type to block mapping

  // All sub-commands must be registred right after
  // a parent command is registered
  command_id add_command(command_id parent_scope, std::string_view cmd,
                         command_handler_t callback) {
    if (parent_scope >= blocks_.size())
      blocks_.resize(parent_scope + 1);
    auto& block_ref = blocks_[parent_scope];
    block_ref.events_.emplace(std::move(cmd), std::move(callback));
  }

  std::unordered_map<std::string_view, std::uint32_t> reg_block_mappings_;
  // block 0 is always the root
  std::vector<block> blocks_;
};
using interpreter_id = std::uint32_t;

} // namespace neo