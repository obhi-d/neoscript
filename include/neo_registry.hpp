#pragma once

#include "neo_command.hpp"
#include "neo_state_machine.hpp"

#include <string_view>
#include <tuple>
#include <unordered_map>

namespace neo
{
namespace detail
{
class interpreter;
}
struct command_handler
{
  enum class results
  {
    e_success = 0,
    e_skip_block, // for blocked commands, skip this block to next one
    e_skip_rest,  // skip the rest of the commands in current block, and do not
                  // enter the current command if its a block command
    e_fail_and_stop // stop parsing any further
  };
};

using command_hook = command_handler::results (*)(command_handler* obj,
                                                  neo::state_machine const&,
                                                  neo::command const&);
using command_end_hook =
    void (*)(command_handler* obj, neo::state_machine const&,
             std::string_view hook_name); // scope_name is "" at end of scope

using textreg_hook = void (*)(command_handler* obj, neo::state_machine const&,
                              std::string&& name, std::string&& content);

using command_id  = std::uint32_t;
using registry_id = std::uint32_t;

class registry
{
  struct block;
  struct handler
  {
    command_hook  cbk_          = nullptr;
    std::uint32_t sub_handlers_ = 0xffffffff;
    handler()                   = default;
    handler(command_hook cbk, std::uint32_t sub)
        : cbk_(std::move(cbk)), sub_handlers_(sub)
    {}
  };
  struct block
  {
    std::unordered_map<std::string_view, handler> events_;
    command_hook                                  any_;
    command_end_hook                              end_ = nullptr;
  };

public:
  static constexpr std::uint32_t k_invalid_id = 0xffffffff;

  registry()
  {
    reg_block_mappings_[""] = 0;
    blocks_.emplace_back();
  }

  inline std::uint32_t ensure_region_root(std::string_view name)
  {
    auto it = reg_block_mappings_.find(name);
    if (it != reg_block_mappings_.end())
      return (*it).second;
    auto id = static_cast<std::uint32_t>(blocks_.size());
    reg_block_mappings_.emplace(name, id);
    blocks_.resize(id + 1);
    return id;
  } /// region type to block mapping

  /// A non scoped command registration
  inline command_id add_command(command_id parent_scope, std::string_view cmd,
                                command_hook callback = nullptr)
  {
    return internal_add_command(
        parent_scope, cmd, static_cast<command_hook>(callback), false, nullptr);
  }

  /// A scoped command registration
  inline command_id add_scoped_command(command_id       parent_scope,
                                       std::string_view cmd,
                                       command_hook     callback  = nullptr,
                                       command_end_hook block_end = nullptr)
  {
    return internal_add_command(parent_scope, cmd,
                                static_cast<command_hook>(callback), true,
                                static_cast<command_end_hook>(block_end));
  }

  /// Alias the behaviour of an already registered command with a new
  /// command path. Path starts with region name.
  /// For example:
  /// echo { }
  /// {{code:first_reg}}
  /// echo { }
  /// {{code:second_reg}}
  /// upper { echo { } }
  /// To make all 'echo' command behave the same:
  /// @code alias_command("@/echo", "@second_reg/upper/echo");
  /// @code alias_command("@first_reg/echo", "@second_reg/upper/echo");
  inline void alias_command(std::string_view src_path,
                            std::string_view dst_path)
  {
    std::uint32_t par_block = find_parent_block(src_path);
    if (par_block == k_invalid_id)
      return;
    std::uint32_t dst_block              = build_parent_block(dst_path);
    blocks_[dst_block].events_[dst_path] = blocks_[par_block].events_[src_path];
  }

  inline void set_text_region_handler(textreg_hook handler)
  {
    text_reg_handler_ = handler;
  }

  inline std::uint32_t get_region_root(std::string_view name) const
  {
    auto it = reg_block_mappings_.find(name);
    if (it != reg_block_mappings_.end())
      return (*it).second;
    return 0;
  } /// region type to block mapping

private:
  /// All sub-commands must be registred right after
  /// a parent command is registered
  inline command_id internal_add_command(command_id       parent_scope,
                                         std::string_view cmd,
                                         command_hook     callback  = nullptr,
                                         bool             is_scoped = false,
                                         command_end_hook end       = nullptr)
  {
    assert(parent_scope < blocks_.size());
    std::uint32_t id = 0xffffffff;
    if (is_scoped)
    {
      if (cmd == "*")
      {
        id = parent_scope;
      }
      else
      {
        id = static_cast<std::uint32_t>(blocks_.size());
        blocks_.resize(blocks_.size() + 1);
      }
      blocks_[id].end_ = end;
    }
    auto& block_ref = blocks_[parent_scope];
    if (cmd == "*")
      block_ref.any_ = callback;
    else
      block_ref.events_[cmd] = handler(callback, id);
    return id;
  }

  inline std::uint32_t find_parent_block(std::string_view& src_path) const
  {
    std::string_view src_reg = "";
    if (src_path[0] == '@')
    {
      auto pos = src_path.find_first_of('/');
      src_reg  = src_path.substr(1, pos);
      if (pos == std::string_view::npos)
        return k_invalid_id;
      src_path = src_path.substr(pos + 1);
    }

    std::uint32_t root = get_region_root(src_reg);
    std::size_t   pos;

    while ((pos = src_path.find_first_of('/')) != std::string_view::npos)
    {
      auto ss = src_path.substr(0, pos);
      auto it = blocks_[root].events_.find(ss);
      if (it == blocks_[root].events_.end())
        return k_invalid_id;
      root     = (*it).second.sub_handlers_;
      src_path = src_path.substr(pos);
    }
    return root;
  }

  inline std::uint32_t build_parent_block(std::string_view& src_path)
  {
    std::string_view src_reg = "";
    if (src_path[0] == '@')
    {
      auto pos = src_path.find_first_of('/');
      src_reg  = src_path.substr(1, pos);
      if (pos == std::string_view::npos)
        return k_invalid_id;
      src_path = src_path.substr(pos + 1);
    }

    std::uint32_t root = get_region_root(src_reg);
    std::size_t   pos;

    while ((pos = src_path.find_first_of('/')) != std::string_view::npos)
    {
      auto ss = src_path.substr(0, pos);
      auto it = blocks_[root].events_.find(ss);
      if (it == blocks_[root].events_.end())
      {
        root = internal_add_command(root, ss);
      }
      else
      {
        root = (*it).second.sub_handlers_;
      }
      src_path = src_path.substr(pos);
    }
    return root;
  }

  textreg_hook text_reg_handler_ = nullptr;
  std::unordered_map<std::string_view, std::uint32_t> reg_block_mappings_;
  /// block 0 is always the root
  std::vector<block> blocks_;

  friend class ::neo::detail::interpreter;
};

} // namespace neo