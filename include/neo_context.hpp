#pragma once
#include <neo_command.hpp>
#include <neo_command_instance.hpp>
#include <neo_command_template.hpp>
#include <neo_location.hpp>
#include <neo_script.hpp>
#include <string>
#include <string_view>
#include <vector>
#include <optional>

namespace neo {
struct interpreter;
class context {
public:
  context();
  using location_type = neo::location;
  void start_region(std::string&&);
  void consume(neo::command&& cmd);
  void add_template(neo::command_template&&);
  void add_template(neo::command_template::template_record&&);
  void remove_template(std::string const&);
  void consume(neo::command_instance&&);
  void end_block();
  void start_region(std::string&& region_id, std::string&& content);
  void import_script(std::string&& file_id);
  void parse();
  void error(location_type const&, std::string const&);

  neo::command_template make_command_template(std::vector<std::string>&&,
                                              neo::command&&);
  neo::command_template make_command_template(std::string&&,
                                              std::vector<std::string>&&,
                                              neo::command&&);
  neo::command          make_command(std::string&&, neo::command::parameters&&,
                                     bool scope_trigger = false);
  neo::command_instance make_instance(std::string&&, neo::command::param_t&&,
                                      bool scope_trigger = false);

  std::string const* get_file_name() const;
  location_type      loc() const;

  void push_error(location const& l, std::string const& e);

  void* scanner = nullptr;

  void resolve(neo::command&);

  neo::command_template const& find_template(std::string const& name) {
    auto it = templates_.find(name);
    if (it != templates_.end()) {
      if ((*it).second.index() == 0)
        return std::get<command_template>((*it).second);
      else {
        std::vector<neo::command_template>& v = std::get<std::vector<neo::command_template>>((*it).second);
        return v.back(); 

      }    
    } else {
      push_error(loc(), "template not found: " + name);
      return command_template();
    }
  }

private:
  using templ_one_many =
      std::variant<neo::command_template, std::vector<neo::command_template>>;
  using template_map    = std::unordered_map<std::string, templ_one_many>;
  using text_region_map = std::unordered_map<std::string, std::string>;
  template_map                                templates_;
  text_region_map                             text_regions_;
  std::vector<std::uint32_t>                  block_stack_;
  std::vector<neo::command_template::record*> record_stack_;
  interpreter*                                interpreter_ = nullptr;
  resolver*                       resolver_stack_;
  std::string                                 region_;
};
} // namespace neo