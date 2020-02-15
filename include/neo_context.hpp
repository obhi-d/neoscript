#include <string>
#include <string_view>
#include <neo_script.hpp>
#include <neo_command.hpp>
#include <neo_command_template.hpp>
#include <neo_command_instance.hpp>
#include <neo_location.hpp>
#include <neo_interpreter.hpp>
#include <neo_command.hpp>

#include <vector>

namespace neo {

class context {
public:
  using location_type = neo::location;
  void start_region(std::string&&);
  void consume(neo::command&& cmd);
  void consume(neo::command_template&&);
  void consume(neo::command_instance&&);
  void end_block();
  void start_region(std::string&& region_id, std::string&& content);
  void import_script(std::string&& file_id);
  void parse();
  void error(location_type const&, std::string const&);

  neo::command_template make_command_template(std::vector<std::string>&&, neo::command&&);
  neo::command_template make_command_template(std::string&&, std::vector<std::string>&&, neo::command&&);
  neo::command make_command(std::string&&, neo::command::parameters&&, bool scope_trigger = false);
  neo::command_instance make_instance(std::string&&, neo::command::param_ptr&&, bool scope_trigger = false);

  std::string const* get_file_name() const;

  void push_error(location const& l, std::string const& e);

  void* scanner = nullptr;

private:
  std::vector<neo::command_template> templates_;
  interpreter::block* current_block_;
  interpreter* interpreter_;
  std::string region_;
};
}