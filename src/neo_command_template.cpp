#include <neo_command_template.hpp>
#include <neo_common.hpp>
#include <neo_context.hpp>

namespace neo {

void command_template::record::visit(neo::context& ctx, bool extend) const {
  if (ctx.fail_bit())
    return;
  std::visit(
      overloaded{[&ctx](auto const& oth) {},
                 [&ctx](template_record const& rec) { ctx.add_template(template_record(rec)); },
                 [&ctx](command_record const& rec) {
                   command cmd = rec.cmd_;
                   ctx.consume(std::move(cmd));
                 },
                 [&ctx](instance_record const& rec) {
                   neo::command_instance inst = rec.instance_;
                   ctx.consume(std::move(inst));
                 }

      },
      node_);
  if (sub_.size() > 0) {
    for (auto const& e : sub_) {
      e.visit(ctx, false);
    }
    if (!extend)
      ctx.end_block();
  }
  std::visit(overloaded{[&ctx](auto const& oth) {},
                        [&ctx](template_record const& rec) {
                          ctx.remove_template(rec.name_);
                        },
                        [&ctx](command_record const& rec) {},
                        [&ctx](instance_record const& rec) {}

             },
             node_);
}
void command_template::visit(neo::context& ctx, bool extend) const {
  assert(main_.node_.index() == 2);
  template_record const& rec = std::get<template_record>(main_.node_);
  command                cmd = rec.cmd_;
  ctx.consume(std::move(cmd));
  if (main_.sub_.size() > 0) {
    for (auto const& e : main_.sub_) {
      e.visit(ctx, false);
    }
    if (!extend)
      ctx.end_block();
  }
}

} // namespace neo