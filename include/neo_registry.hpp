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
enum class retcode
{
  e_success = 0,
  e_success_stop,
  e_skip_block,   // for blocked commands, skip this block to next one
  e_skip_rest,    // skip the rest of the commands in current block, and do not
                  // enter the current command if its a block command
  e_fail_and_stop // stop parsing any further
};

struct command_handler
{
};

//
// @remarks Command hook that is executed at the start of the command,
// parameters are stored inside command
using command_hook = neo::retcode (*)(command_handler* obj,
                                      neo::state_machine const&,
                                      neo::command const&) noexcept;
//
// @remarks Command hook that is executed at the end of the command
using command_end_hook = void (*)(
    command_handler* obj, neo::state_machine const&,
    std::string_view cmd_name) noexcept; // scope_name is "" at end of scope

///
/// @remarks Text region hook/callback for regions marked as
/// {{text_type:SegmentName}}
/// @param obj       Command handler interface
/// @param sm        State machine that holds the parser state
/// @param text_type This corresponds to the text just before ':' in the region
/// which is 'text_type' in the above example
/// @param name      This corresponds to the text just after ':' in the region
/// which is 'SegmentName' in the above example
/// @param content   The text content in the region
using textreg_hook = void (*)(command_handler*          obj,
                              neo::state_machine const& sm,
                              std::string_view text_type, std::string_view name,
                              text_content&& content) noexcept;

using command_id  = std::pair<std::uint32_t, std::uint32_t>;
using registry_id = std::uint32_t;

class registry
{
  struct block;
  struct handler
  {
    command_hook  cbk_          = nullptr;
    std::uint32_t sub_handlers_ = 0xffffffff;
    std::uint32_t iid_          = 0xffffffff;
    handler() noexcept          = default;
    handler(command_hook cbk, std::uint32_t sub, std::uint32_t id) noexcept

        : cbk_(std::move(cbk)), sub_handlers_(sub), iid_(id)
    {}
  };

  struct block
  {
    std::unordered_map<std::string_view, handler> events_;
    command_hook                                  any_ = nullptr;
    command_end_hook                              end_ = nullptr;
  };

public:
  static constexpr std::uint32_t k_np_id_mask = 0x80000000;
  static constexpr std::uint32_t k_np_cl_mask = ~k_np_id_mask;
  static constexpr std::uint32_t k_invalid_id = 0xffffffff;

  registry() noexcept

  {
    reg_block_mappings_[""] = 0;
    blocks_.emplace_back();
  }

  inline command_id ensure_region_root(std::string_view name) noexcept

  {
    auto it = reg_block_mappings_.find(name);
    if (it != reg_block_mappings_.end())
      return command_id{(*it).second, 0};
    auto id = static_cast<std::uint32_t>(blocks_.size());
    reg_block_mappings_.emplace(name, id);
    blocks_.resize(static_cast<std::size_t>(id + 1));
    return command_id{id, 0};
  } /// region type to block mapping

  /// A non scoped command registration
  inline command_id add_command(command_id parent_scope, std::string_view cmd,
                                command_hook callback = nullptr) noexcept

  {
    return internal_add_command(
        parent_scope, cmd, static_cast<command_hook>(callback), false, nullptr);
  }

  /// A scoped command registration
  inline command_id add_scoped_command(
      command_id parent_scope, std::string_view cmd,
      command_hook     callback  = nullptr,
      command_end_hook block_end = nullptr) noexcept

  {
    return internal_add_command(parent_scope, cmd,
                                static_cast<command_hook>(callback), true,
                                static_cast<command_end_hook>(block_end));
  }

  inline void alias_command(command_id new_parent_scope, std::string_view name,
                            command_id existing)
  {
    auto&   old_parent = blocks_[existing.first & k_np_cl_mask];
    handler h;
    for (auto& e : old_parent.events_)
    {
      if (e.second.iid_ == existing.second)
      {
        h = e.second;
        break;
      }
    }
    blocks_[new_parent_scope.first].events_[name] = h;
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
                            std::string_view dst_path) noexcept

  {
    std::uint32_t par_block = find_parent_block(src_path);
    if (par_block & k_np_id_mask)
      return;
    std::uint32_t dst_block              = build_parent_block(dst_path);
    blocks_[dst_block].events_[dst_path] = blocks_[par_block].events_[src_path];
  }

  inline void set_text_region_handler(textreg_hook handler) noexcept

  {
    text_reg_handler_ = handler;
  }

  inline std::uint32_t get_region_root(std::string_view name) const noexcept

  {
    auto it = reg_block_mappings_.find(name);
    if (it != reg_block_mappings_.end())
      return (*it).second;
    return 0;
  } /// region type to block mapping

private:
  /// All sub-commands must be registred right after
  /// a parent command is registered
  inline command_id internal_add_command(
      command_id parent_scope, std::string_view cmd,
      command_hook callback = nullptr, bool is_scoped = false,
      command_end_hook end = nullptr) noexcept

  {
    assert(parent_scope.first < blocks_.size());
    std::uint32_t iid = k_np_id_mask;
    std::uint32_t id  = 0xffffffff;
    if (is_scoped)
    {
      iid = 0;
      if (cmd == "*")
      {
        id = parent_scope.first;
      }
      else
      {
        id = static_cast<std::uint32_t>(blocks_.size());
        blocks_.resize(blocks_.size() + 1);
      }
      blocks_[id].end_ = end;
    }
    else
      id = k_np_id_mask | parent_scope.first;
    auto& block_ref = blocks_[parent_scope.first];
    iid |= static_cast<std::uint32_t>(block_ref.events_.size());
    if (cmd == "*")
      block_ref.any_ = callback;
    else
      block_ref.events_[cmd] = handler(callback, id, iid);
    return std::make_pair(id, iid);
  }

  inline std::uint32_t find_parent_block(
      std::string_view& src_path) const noexcept

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

  inline std::uint32_t build_parent_block(std::string_view& src_path) noexcept

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

    std::uint32_t root = get_region_root(src_reg) & k_np_cl_mask;
    std::size_t   pos;

    while ((pos = src_path.find_first_of('/')) != std::string_view::npos)
    {
      auto ss = src_path.substr(0, pos);
      auto it = blocks_[root].events_.find(ss);
      if (it == blocks_[root].events_.end())
      {
        root = internal_add_command(command_id{root, 0}, ss).first;
      }
      else
      {
        root = (*it).second.sub_handlers_;
      }
      src_path = src_path.substr(pos + 1);
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

//// Macro helpers
#define neo_tp_(FnName, Ext) FnName##Ext
#define neo_tp(FnName, Ext)  neo_tp_(FnName, Ext)
#define neo_cmd_handler(FnName, Ty, iObj, iState, iCmd)                        \
  neo::retcode neo_tp(call_, FnName)(                                          \
      [[maybe_unused]] Ty & iObj,                                              \
      [[maybe_unused]] neo::state_machine const& iState,                       \
      [[maybe_unused]] neo::command const&       iCmd) noexcept;                     \
  neo::retcode neo_tp(cmd_, FnName)(neo::command_handler * iObj,               \
                                    neo::state_machine const& iState,          \
                                    neo::command const&       iCmd) noexcept         \
  {                                                                            \
    return neo_tp(call_, FnName)(static_cast<Ty&>(*iObj), iState, iCmd);       \
  }                                                                            \
  neo::retcode neo_tp(call_, FnName)(                                          \
      [[maybe_unused]] Ty & iObj,                                              \
      [[maybe_unused]] neo::state_machine const& iState,                       \
      [[maybe_unused]] neo::command const&       iCmd) noexcept

#define neo_cmdend_handler(FnName, Ty, iObj, iState, iName)                    \
  void neo_tp(call_,                                                           \
              FnName)([[maybe_unused]] Ty & iObj,                              \
                      [[maybe_unused]] neo::state_machine const& iState,       \
                      [[maybe_unused]] std::string_view iName) noexcept;       \
  void neo_tp(cmd_, FnName)(neo::command_handler * iObj,                       \
                            neo::state_machine const& iState,                  \
                            std::string_view          iName) noexcept                   \
  {                                                                            \
    neo_tp(call_, FnName)(static_cast<Ty&>(*iObj), iState, iName);             \
  }                                                                            \
  void neo_tp(call_,                                                           \
              FnName)([[maybe_unused]] Ty & iObj,                              \
                      [[maybe_unused]] neo::state_machine const& iState,       \
                      [[maybe_unused]] std::string_view          iName) noexcept

#define neo_text_handler(FnName, Ty, iObj, iState, iType, iName, iContent)     \
  void neo_tp(call_,                                                           \
              FnName)([[maybe_unused]] Ty & iObj,                              \
                      [[maybe_unused]] neo::state_machine const& iState,       \
                      [[maybe_unused]] std::string&&             iType,        \
                      [[maybe_unused]] std::string_view          iName,        \
                      [[maybe_unused]] std::string_view          iContent);             \
  void neo_tp(cmd_, FnName)(                                                   \
      neo::command_handler * iObj, neo::state_machine const& iState,           \
      std::string&& iType, std::string_view iName, std::string_view iContent)  \
  {                                                                            \
    neo_tp(call_, FnName)(static_cast<Ty&>(*iObj), iState, std::move(iType),   \
                          iName, iContent);                                    \
  }                                                                            \
  void neo_tp(call_,                                                           \
              FnName)([[maybe_unused]] Ty & iObj,                              \
                      [[maybe_unused]] neo::state_machine const& iState,       \
                      [[maybe_unused]] std::string&&             iType,        \
                      [[maybe_unused]] std::string_view          iName,        \
                      [[maybe_unused]] std::string_view          iContent)

#define neo_star_handler(FnName, Ty, iObj, iState, iCmd)                       \
  neo_cmd_handler(neo_tp(FnName, _star), Ty, iObj, iState, iCmd)

#define neo_registry(name)                                                     \
  void neo_tp(registry_, name)(neo::registry & r, neo::command_id p = {},       \
                               neo::command_id c = {})

#define neo_star(name) c = r.add_command(p, "*", neo_tp(neo_tp(cmd_, name), _star))
#define neo_cmd(name)  c = r.add_command(p, #name, neo_tp(cmd_, name))
#define neo_scope(name)                                                        \
  if (auto p =                                                        \
          r.add_scoped_command(p, #name, neo_tp(cmd_, name), nullptr))
#define neo_blk(name)                                                          \
  if (auto p = r.add_scoped_command(p, #name, neo_tp(cmd_, name),     \
                                             neo_tp(cmd_, name)))
#define neo_alias(src, dst) r.alias_command(src, dst)
#define neo_aliasid(par_scope, name, ex) r.alias_command(par_scope, name, ex)
#define neo_savecmd(as) auto as = c
