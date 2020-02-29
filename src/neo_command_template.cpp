#include <neo_command_template.hpp>
#include <neo_common.hpp>
#include <neo_context.hpp>

namespace neo
{
void command_template::record::pre_visit(neo::context& ctx) const
{
  if (ctx.fail_bit())
    return;
  std::visit(overloaded{[&ctx](auto const& oth) {},
                        [&ctx](command_template_ref const& rec)
                        { ctx.push_template(rec.get()); },
                        [&ctx](command_record const& rec)
                        {
                          command cmd = rec.cmd_;
                          ctx.consume(std::move(cmd));
                        },
                        [&ctx](instance_record const& rec)
                        {
                          neo::command_instance inst = rec.instance_;
                          ctx.consume(std::move(inst));
                        }},
             node_);
}
void command_template::record::visit(neo::context& ctx) const
{
  if (sub_.size() > 0)
  {
    for (auto const& e : sub_)
    {
      e.pre_visit(ctx);
    }
    for (auto const& e : sub_)
    {
      e.visit(ctx);
    }
    for (auto const& e : sub_)
    {
      e.post_visit(ctx);
    }
  }
}
void command_template::record::post_visit(neo::context& ctx) const
{
  std::visit(overloaded{[&ctx](auto const& oth) {},
                        [&ctx](command_template_ref const& rec)
                        { ctx.remove_template(rec.get().name()); },
                        [&ctx](command_record const& rec) {},
                        [&ctx](instance_record const& rec) {}},
             node_);
}
void command_template::visit(neo::context& ctx, bool extend) const
{
  assert(main_.node_.index() == 2);
  template_record const& rec = std::get<template_record>(main_.node_);
  command                cmd = rec.cmd_;
  ctx.consume(std::move(cmd));
  main_.visit(ctx);
  if (!extend && cmd.is_scoped())
    ctx.end_block();
}
} // namespace neo