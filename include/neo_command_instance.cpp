#include "neo_command_instance.hpp"

void neo::command_instance::build(neo::context&) {}

void neo::command_instance::visit(neo::context&, bool extend) {}

std::optional<command::param_t> neo::command_instance::resolve(
    std::string_view name, std::string_view value) {
  return std::optional<command::param_t>();
}
