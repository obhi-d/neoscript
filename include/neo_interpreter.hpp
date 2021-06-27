#pragma once
#include <neo_command.hpp>
#include <neo_state_machine.hpp>
#include <string_view>
#include <tuple>
#include <unordered_map>

namespace neo
{
struct command_handler
{
};
using command_hook = bool (*)(command_handler* obj, neo::state_machine const&,
                              neo::command const&);
using block_hook =
    bool (*)(command_handler* obj, neo::state_machine const&,
             std::string_view hook_name); // scope_name is "" at end of scope

using textreg_hook = void (*)(command_handler* obj, neo::state_machine const&,
                              std::string&& name, std::string&& content);

using command_id = std::uint32_t;

class interpreter
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
    block_hook                                    begin_ = nullptr;
    block_hook                                    end_   = nullptr;
  };

public:
  enum : std::uint32_t
  {
    k_failure = 0xffffffff
  };

  inline interpreter()
  {
    reg_block_mappings_[""] = 0;
    blocks_.emplace_back();
  }

  inline bool begin_scope(neo::state_machine& ctx, command_handler* obj,
                          std::uint32_t block, std::string_view scope_id)
  {
    return blocks_[block].begin_
               ? std::invoke(blocks_[block].begin_, obj, ctx, scope_id)
               : true;
  }

  inline bool end_scope(neo::state_machine& ctx, command_handler* obj,
                        std::uint32_t block, std::string_view scope_id)
  {
    return blocks_[block].end_
               ? std::invoke(blocks_[block].end_, obj, ctx, scope_id)
               : true;
  }

  inline std::tuple<std::uint32_t, bool> execute(neo::state_machine& ctx,
                                                 command_handler*    obj,
                                                 std::uint32_t       block,
                                                 neo::command&       cmd)
  {
    auto& block_ref = blocks_[block];
    auto  it        = block_ref.events_.find(cmd.name());
    if (it != block_ref.events_.end())
    {
      if ((*it).second.cbk_)
      {
        if (!std::invoke((*it).second.cbk_, obj, ctx, cmd))
          return {k_failure, false};
      }
      if (cmd.is_scoped())
        block = (*it).second.sub_handlers_;
    }
    else
    {
      if (!std::invoke(block_ref.any_, obj, ctx, cmd))
        return {k_failure, false};
    }
    return {block, cmd.is_scoped()};
  }

  inline std::uint32_t get_region_root(std::string_view name) const
  {
    auto it = reg_block_mappings_.find(name);
    if (it != reg_block_mappings_.end())
      return (*it).second;
    return 0;
  } /// region type to block mapping

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
    return internal_add_command(parent_scope, cmd,
                                static_cast<command_hook>(callback), false,
                                nullptr, nullptr);
  }

  /// A scoped command registration
  inline command_id add_scoped_command(command_id       parent_scope,
                                       std::string_view cmd,
                                       command_hook     callback    = nullptr,
                                       block_hook       block_begin = nullptr,
                                       block_hook       block_end   = nullptr)
  {
    return internal_add_command(parent_scope, cmd,
                                static_cast<command_hook>(callback), true,
                                static_cast<block_hook>(block_begin),
                                static_cast<block_hook>(block_end));
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
    if (par_block == k_failure)
      return;
    std::uint32_t dst_block              = build_parent_block(dst_path);
    blocks_[dst_block].events_[dst_path] = blocks_[par_block].events_[src_path];
  }

  inline void set_text_region_handler(textreg_hook handler)
  {
    text_reg_handler_ = handler;
  }

  inline void handle_text_region(neo::command_handler*     obj,
                                 neo::state_machine const& ctx,
                                 std::string&& region_id, std::string&& content)
  {
    if (text_reg_handler_)
      text_reg_handler_(obj, ctx, std::move(region_id), std::move(content));
  }

private:
  /// All sub-commands must be registred right after
  /// a parent command is registered
  inline command_id internal_add_command(command_id       parent_scope,
                                         std::string_view cmd,
                                         command_hook     callback  = nullptr,
                                         bool             is_scoped = false,
                                         block_hook       begin     = nullptr,
                                         block_hook       end       = nullptr)
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
      blocks_[id].begin_ = begin;
      blocks_[id].end_   = end;
    }
    auto& block_ref = blocks_[parent_scope];
    if (cmd == "*")
      block_ref.any_ = callback;
    else
      block_ref.events_[cmd] = handler(callback, id);
    return id;
  }

  inline void internal_add_scope(command_id parent_scope, command_id new_scope,
                                 block_hook begin, block_hook end)
  {
    if (parent_scope >= blocks_.size())
      blocks_.resize(parent_scope + 1);
    if (new_scope >= blocks_.size())
      blocks_.resize(new_scope + 1);
    auto& block_ref = blocks_[parent_scope];
  }

  inline std::uint32_t find_parent_block(std::string_view& src_path) const
  {
    std::string_view src_reg = "";
    if (src_path[0] == '@')
    {
      auto pos = src_path.find_first_of('/');
      src_reg  = src_path.substr(1, pos);
      if (pos == std::string_view::npos)
        return k_failure;
      src_path = src_path.substr(pos + 1);
    }

    std::uint32_t root = get_region_root(src_reg);
    std::size_t   pos;

    while ((pos = src_path.find_first_of('/')) != std::string_view::npos)
    {
      auto ss = src_path.substr(0, pos);
      auto it = blocks_[root].events_.find(ss);
      if (it == blocks_[root].events_.end())
        return k_failure;
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
        return k_failure;
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

private:
  textreg_hook text_reg_handler_ = nullptr;
  std::unordered_map<std::string_view, std::uint32_t> reg_block_mappings_;
  /// block 0 is always the root
  std::vector<block> blocks_;
};
using interpreter_id = std::uint32_t;
} // namespace neo