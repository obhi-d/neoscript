
#include <neo_command.hpp>
#include <neo_command_instance.hpp>
#include <variant>
#include <vector>

namespace neo {

class command_template {
public:
  command_template()                        = default;
  command_template(command_template const&) = default;
  command_template(command_template&&)      = default;
  command_template& operator=(command_template const&) = default;
  command_template& operator=(command_template&&) = default;

  using node = std::variant<std::monostate, command_template, command, command_instance>;
  neo::command             cmd_;
  std::vector<std::string> params_;
  std::vector<node>        subs_;
};
} // namespace neo