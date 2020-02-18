#include <neo_command_instance.hpp>
#include <neo_command_template.hpp>
#include <neo_context.hpp>

namespace neo {

void command_instance::build(neo::context&                ctx,
                             neo::command_template const& tmpl) {
  auto const& params  = tmpl.get_params();
  auto&       param_m = param_map_;
  std::visit(overloaded{
                 [](std::monostate const&) {},
                 [&params, &param_m](neo::command::single const& s) {
                   std::string_view name = s.name() if (name.length() > 0) {
                     param_m[name] = neo::command::single(s.value());
                   }
                   else if (params.size() > 0) {
                     param_m[params[0]] = s;
                   }
                 },
                 [&params, &param_m](neo::command::list const& s) {
                   std::size_t min = std::min(s.count(), params.size());
                   auto const& val = s.value();
                   for (std::size_t i = 0; i < min; ++i)
                     param_m[params[i]] = val[i];
                 },
             },
             param_list_);
}

void command_instance::visit(neo::context& ctx, bool extend) {
  auto const& templ = ctx.find_template(template_);
  build(ctx, templ);
  templ.visit(ctx, extend);
}

std::optional<command::param_t> command_instance::resolve(
    std::string_view name, std::string_view value) {
  auto it = param_map_.find(value);
  if ()
  return std::optional<command::param_t>();
}

} // namespace neo