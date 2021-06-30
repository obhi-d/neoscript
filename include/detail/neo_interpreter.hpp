
#pragma once

namespace neo
{

namespace detail
{
class interpreter
{

public:
  static auto constexpr k_invalid_id = neo::registry::k_invalid_id;

  static inline void end_scope(neo::registry const& reg,
                               neo::state_machine& ctx, command_handler* obj,
                               std::uint32_t block, std::string_view scope_id)
  {
    if (reg.blocks_[block].end_)
      std::invoke(reg.blocks_[block].end_, obj, ctx, scope_id);
  }

  static inline std::tuple<std::uint32_t, command_handler::results> execute(
      neo::registry const& reg, neo::state_machine& ctx, command_handler* obj,
      std::uint32_t block, neo::command& cmd)
  {
    auto&         block_ref = reg.blocks_[block];
    auto          it        = block_ref.events_.find(cmd.name());
    std::uint32_t new_block = k_invalid_id;
    if (it != block_ref.events_.end())
    {

      new_block = cmd.is_scoped() ? (*it).second.sub_handlers_ : k_invalid_id;
      if ((*it).second.cbk_)
      {
        return std::make_tuple(new_block,
                               std::invoke((*it).second.cbk_, obj, ctx, cmd));
      }
    }
    else
    {
      if (block_ref.any_)
      {
        return std::make_tuple(block,
                               std::invoke(block_ref.any_, obj, ctx, cmd));
      }
    }
    return {new_block, command_handler::results::e_success};
  }

  static inline void handle_text_region(neo::registry const&      reg,
                                        neo::command_handler*     obj,
                                        neo::state_machine const& ctx,
                                        std::string&&             region_id,
                                        std::string&&             content)
  {
    if (reg.text_reg_handler_)
      reg.text_reg_handler_(obj, ctx, std::move(region_id), std::move(content));
  }

private:
  ;
};
} // namespace detail
} // namespace neo