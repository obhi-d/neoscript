#pragma once
#include <algorithm>
#include <forward_list>
#include <istream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// neo
#include <neo_command.hpp>
#include <neo_command_instance.hpp>
#include <neo_command_template.hpp>
#include <neo_location.hpp>

namespace neo
{

class registry;
struct command_handler;

class NEO_API state_machine
{
public:
  enum options
  {
    f_trace_scan        = 1,
    f_trace_parse       = 2,
    f_continue_on_error = 4,
  };

  using option_flags = std::uint32_t;
  using import_handler =
      std::function<std::shared_ptr<std::istream>(std::string const&)>;

  state_machine(registry const& registry, command_handler* handler,
                option_flags flags = 0);
  using location_type = neo::location;

  void start_region(std::string&&);
  bool consume(neo::command&& cmd);
  void add_template(neo::command_template&&);
  void push_template(neo::command_template const&);
  void record_template(neo::command_template&);
  void remove_template(std::string const&);
  bool consume(neo::command_instance&&);
  void end_block();
  void start_region(std::string&& region_id,
                    std::string&& content);
  void import_script(std::string const& file_id);
  void parse(std::string_view                     src_name,
             std::shared_ptr<std::istream> const& ifile);
  void error(location_type const&, std::string const&);

  // Import handler registration
  void set_import_handler(import_handler&& handler)
  {
    importer_ = std::move(handler);
  }

  // String container
  void               put(char c) { content_ += c; }
  void               put(std::string_view sv) { content_ += sv; }
  std::string const& get() const { return content_; }
  void               start() { content_.clear(); }

  // region id
  void set_current_reg_id(std::string const& name) { region_type_ = name; }

  // stream
  int read(char* buffer, int siz);
  // Content creator
  neo::command_template make_command_template(std::vector<std::string>&&,
                                              neo::command&&);

  neo::command_template make_command_template(std::string&&,
                                              std::vector<std::string>&&,
                                              neo::command&&);

  neo::command make_command(std::string&&, neo::command::parameters&&,
                            bool scope_trigger = false);

  neo::command_instance make_instance(std::string&&, neo::command::param_t&&,
                                      bool scope_trigger = false);

  std::string const*   get_file_name() const { return &source_name_; }
  location_type const& loc() const { return loc_; }
  location_type&       loc() { return loc_; }

  void push_error(location const& l, std::string_view e);
  void resolve(neo::command&);

  neo::command_template const& find_template(std::string const& name)
  {
    auto it = templates_.find(name);
    if (it != templates_.end())
    {
      return it->second.back().get();
    }
    else
    {
      push_error(loc(), "template not found: " + name);
      return null_template_;
    }
  }

  void begin_scan();
  void end_scan();

  void* scanner = nullptr;

  template <typename lambda>
  void for_each_error(lambda&& l)
  {
    std::for_each(errors_.begin(), errors_.end(), l);
  }

  bool fail_bit() const
  {
    return errors_.size() > 0 && !(flags_ & f_continue_on_error);
  }

  bool skip() const { return skip_ > 0; }

  void         enter_skip_scope() { skip_++; }
  std::int32_t exit_skip_scope() { return --skip_; }

private:
  using record_ptr = neo::command_template::record*;
  static std::shared_ptr<std::istream> default_import_handler(
      std::string const&);

  using push_templates =
      std::vector<std::reference_wrapper<neo::command_template const>>;
  using template_map       = std::unordered_map<std::string, push_templates>;
  using root_template_list = std::forward_list<neo::command_template>;

  template_map                  templates_;
  std::vector<std::uint32_t>    block_stack_;
  std::vector<record_ptr>       record_stack_;
  std::vector<std::string>      errors_;
  root_template_list            root_template_storage_;
  registry const&               registry_;
  command_handler*              cmd_handler_;
  resolver*                     resolver_stack_ = nullptr;
  std::string                   region_;
  import_handler                importer_;
  std::shared_ptr<std::istream> current_file_;
  std::string                   content_;
  std::string                   source_name_;
  std::string                   region_type_;
  location_type                 loc_;
  option_flags                  flags_ = 0;
  std::int32_t                  skip_  = 0;

  static const command_template null_template_;
};
} // namespace neo