#include <neo_command_template.hpp>
#include <neo_common.hpp>
#include <neo_context.hpp>

namespace neo {

void command_template::record::visit(neo::context& ctx, bool extend) const {
  if (ctx.fail_bit())
    return;
  std::visit(
      overloaded{[&ctx](auto const& oth) {},
                 [&ctx](template_record const& rec) { ctx.add_template(rec); },
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
  main_.visit(ctx, extend);
}

} // namespace neo