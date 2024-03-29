#pragma once
#include <functional>
#include <neo_command.hpp>
#include <neo_common.hpp>
#include <string_view>
#include <unordered_map>

namespace neo
{
class state_machine;
class command_template;
class NEO_API command_instance
{
public:
  command_instance() noexcept = default;
  command_instance(command_instance const& _) noexcept
      : template_(_.template_), param_list_(_.param_list_),
        param_map_(_.param_map_), is_extended_(_.is_extended_)
  {}
  command_instance(command_instance&&) noexcept = default;
  command_instance& operator=(command_instance const&) noexcept = default;
  command_instance& operator=(command_instance&&) noexcept = default;
  command_instance(std::string_view name, neo::command::param_t&& param,
                   bool extended) noexcept
      : template_(std::move(name)), param_list_(std::move(param)),
        is_extended_(extended)
  {}

  bool is_extended() const noexcept { return is_extended_; }
  void build(neo::state_machine&, neo::command_template const&) noexcept;
  bool visit(neo::state_machine&, bool extend) noexcept;
  std::optional<command::param_t> resolve(std::string_view name,
                                          std::string_view value) noexcept;

  std::string_view                                            template_;
  neo::command::param_t                                       param_list_;
  std::unordered_map<std::string_view, neo::command::param_t> param_map_;
  bool is_extended_ = false;
};

} // namespace neo