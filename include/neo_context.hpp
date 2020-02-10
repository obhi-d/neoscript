#include <string>
#include <string_view>
#include <neo_script.hpp>
#include <neo_command.hpp>
#include <neo_command_template.hpp>
#include <neo_command_instance.hpp>
#include <neo_location.hpp>
#include <vector>

namespace neo {

class context {
public:
  using location_type = neo::location;
  void start_region(std::string_view);
  void consume(neo::command&&);
  void consume(neo::command_template&&);
  void consume(neo::command_instance&&);
  void end_block();
  void start_region(std::string&& region_id, std::string&& content);
  void import(std::string&& file_id);
  void parse();
  void error(location_type const&, std::string const&);

  neo::command_template make_command_template(std::vector<std::string>&&, neo::command&&);
  neo::command_template make_command_template(std::string&&, std::vector<std::string>&&, neo::command&&);
  neo::command make_command(std::string&&, neo::command::parameters&&);
  neo::command make_command(std::string&&, neo::command::parameters&&);
  neo::command_instance make_instance(std::string&&, neo::command_instance&&);
private:
};
}