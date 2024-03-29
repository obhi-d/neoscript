#include <algorithm>
#include <neo_command_instance.hpp>
#include <neo_command_template.hpp>
#include <neo_state_machine.hpp>

namespace neo
{

void command_instance::build(neo::state_machine&          ctx,
                             neo::command_template const& tmpl) noexcept
{
  auto const& params  = tmpl.get_params();
  auto&       param_m = param_map_;
  std::visit(
      overloaded{
          [](std::monostate const&) {},
          [&params, &param_m](auto const& s)
          {
            std::string_view name = s.name();
            if (name.length() > 0)
            {
              param_m[name] = neo::command::single(s.value());
            }
            else if (params.size() > 0)
            {
              param_m[params[0]] = s;
            }
          },
          [&params, &param_m](neo::command::list const& s)
          {
            std::size_t min_v = std::min(s.count(), params.size());
            auto const& val   = s.value();
            for (std::size_t i = 0; i < min_v; ++i)
              param_m[params[i]] = val[i];
          },
      },
      param_list_);
}

bool command_instance::visit(neo::state_machine& ctx, bool extend) noexcept
{
  auto const& templ = ctx.find_template(template_);
  if (ctx.fail_bit())
    return true;
  build(ctx, templ);
  return templ.visit(ctx, extend);
}

std::optional<command::param_t> command_instance::resolve(
    std::string_view name, std::string_view value) noexcept
{
  auto it = param_map_.find(value);
  if (it != param_map_.end())
    return {(*it).second};
  return std::optional<command::param_t>();
}

} // namespace neo