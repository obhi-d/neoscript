#include <neo_command.hpp>
#include <functional>
#include <unordered_map>
#include <string_view>

namespace neo {
class command_template;
class command_instance {
public:
  command_instance() = default;
  command_instance(command_instance const&) = default;
  command_instance(command_instance&&)      = default;
  command_instance& operator=(command_instance const&) = default;
  command_instance& operator=(command_instance&&) = default;

  std::reference_wrapper<neo::command_template>    template_;
  std::unordered_map<std::string_view, neo::command::param_ptr> params_;
};

}