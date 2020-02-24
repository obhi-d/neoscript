#pragma once
#include <algorithm>
#include <istream>
#include <memory>
#include <neo_command.hpp>
#include <neo_command_instance.hpp>
#include <neo_command_template.hpp>
#include <neo_location.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace neo {
class interpreter;
struct command_handler;
class context {
public:
  enum options {
    f_trace_scan        = 1,
    f_trace_parse       = 2,
    f_continue_on_error = 4,
  };

  using option_flags = std::uint32_t;
  using import_handler =
      std::function<std::shared_ptr<std::istream>(std::string const&)>;

  context(interpreter& interp, command_handler& handler,
          option_flags flags = 0);
  using location_type = neo::location;

  void start_region(std::string&&);
  void consume(neo::command&& cmd);
  void add_template(neo::command_template&&);
  void add_template(neo::command_template::template_record&&);
  void remove_template(std::string const&);
  void consume(neo::command_instance&&);
  void end_block();
  void start_region(std::string&& region_id, std::string&& content);
  void import_script(std::string const& file_id);
  void parse(std::string_view src_name, std::shared_ptr<std::istream>& ifile);
  void error(location_type const&, std::string const&);

  // Import handler registration
  void set_import_handler(import_handler&& handler) {
    importer_ = std::move(handler);
  }

  // String container
  void               put(char c) { content_ += c; }
  void               put(std::string_view sv) { content_ += sv; }
  std::string const& get() const { return content_; }
  void               start() { content_.clear(); }

  // stream
  int read(char* buffer, int siz);
  // Content creator
  neo::command_template make_command_template(std::vector<std::string>&&,
                                              neo::command&&);
  neo::command_template make_command_template(std::string&&,
                                              std::vector<std::string>&&,
                                              neo::command&&);
  neo::command          make_command(std::string&&, neo::command::parameters&&,
                                     bool scope_trigger = false);
  neo::command_instance make_instance(std::string&&, neo::command::param_t&&,
                                      bool scope_trigger = false);

  std::string const*   get_file_name() const { return &source_name_; }
  location_type const& loc() const { return loc_; }
  location_type&       loc() { return loc_; }

  void push_error(location const& l, std::string_view e);
  void resolve(neo::command&);

  neo::command_template const& find_template(std::string const& name) {
    auto it = templates_.find(name);
    if (it != templates_.end()) {
      if ((*it).second.index() == 0)
        return std::get<command_template>((*it).second);
      else {
        std::vector<neo::command_template>& v =
            std::get<std::vector<neo::command_template>>((*it).second);
        return v.back();
      }
    } else {
      push_error(loc(), "template not found: " + name);
      return null_template_;
    }
  }

  void begin_scan();
  void end_scan();

  void* scanner = nullptr;

  template <typename lambda>
  void for_each_error(lambda&& l) {
    std::for_each(errors_.begin(), errors_.end(), l);
  }

  bool fail_bit() const {
    return errors_.size() > 0 && !(flags_ & f_continue_on_error);
  }

  std::string_view get_text_content(std::string const& name) const {
    auto it = text_regions_.find(name);
    if (it != text_regions_.end())
      return (*it).second;
    return "";
  }

  std::string& get_text_content(std::string const& name) {
    return text_regions_[name];
  }

private:
  static std::shared_ptr<std::istream> default_import_handler(
      std::string const&);

  using templ_one_many =
      std::variant<neo::command_template, std::vector<neo::command_template>>;
  using template_map    = std::unordered_map<std::string, templ_one_many>;
  using text_region_map = std::unordered_map<std::string, std::string>;
  template_map                                templates_;
  text_region_map                             text_regions_;
  std::vector<std::uint32_t>                  block_stack_;
  std::vector<neo::command_template::record*> record_stack_;
  std::vector<std::string>                    errors_;
  interpreter&                                interpreter_;
  command_handler&                            cmd_handler_;
  resolver*                                   resolver_stack_ = nullptr;
  std::string                                 region_;
  import_handler                              importer_;
  std::shared_ptr<std::istream>               current_file_;
  static const command_template               null_template_;
  std::string                                 content_;
  std::string                                 source_name_;
  location_type                               loc_;
  option_flags                                flags_ = 0;
};
} // namespace neo