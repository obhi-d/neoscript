#pragma once
#include <algorithm>
#include <forward_list>
#include <istream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
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

  using option_flags   = std::uint32_t;
  using import_handler = std::function<std::string_view(std::string_view)>;

  state_machine(registry const& registry, command_handler* handler,
                option_flags flags = 0) noexcept;
  using location_type = neo::location;

  void start_region(std::string_view) noexcept;
  bool consume(neo::command&& cmd) noexcept;
  void add_template(neo::command_template&&) noexcept;
  void push_template(neo::command_template const&) noexcept;
  void record_template(neo::command_template&) noexcept;
  void remove_template(std::string_view) noexcept;
  bool consume(neo::command_instance&&) noexcept;
  void end_block() noexcept;
  void start_region(std::string_view region_id,
                    text_content&& content) noexcept;
  void import_script(std::string_view file_id) noexcept;
  void parse(std::string_view src_name, std::string_view content) noexcept;
  void error(location_type const&, std::string const&) noexcept;

  // Import handler registration
  void set_import_handler(import_handler&& handler) noexcept
  {
    importer_ = std::move(handler);
  }

  // String container
  void put(std::int32_t len) noexcept { read_len(len); }

  void skip_len(std::int32_t len) noexcept
  {
    assert(len_reading_ == 0);
    pos_commit_ += len;
  }

  std::string_view get() const noexcept

  {
    return current_file_.substr(pos_commit_, len_reading_);
  }

  void commit() noexcept
  {
    pos_commit_ += len_reading_;
    len_reading_ = 0;
  }

  std::string_view make_token() noexcept

  {
    auto m = current_file_.substr(pos_commit_, len_reading_);
    commit();
    return m;
  }

  void append_to_esqstr(std::string_view ss) noexcept 
  { 
    esq_string_ += ss;
  }

  std::string& manage_esq_string() noexcept 
  { 
    return esq_string_;
  }

  void push_content() noexcept
  {
    content_.fragments.push_back(make_token());
  }
  // region id
  void set_current_reg_id(std::string_view name) noexcept
  {
    region_type_ = name;
  }

  text_content retrieve_content() noexcept 
  {
    return std::move(content_); 
  }

  // stream
  int read(char* buffer, int siz) noexcept;
  // Content creator
  neo::command_template make_command_template(std::vector<std::string_view>&&,
                                              neo::command&&) noexcept;

  neo::command_template make_command_template(std::string_view,
                                              std::vector<std::string_view>&&,
                                              neo::command&&) noexcept;

  neo::command make_command(std::string_view, neo::command::parameters&&,
                            bool scope_trigger = false) noexcept;

  neo::command_instance make_instance(std::string_view, neo::command::param_t&&,
                                      bool scope_trigger = false) noexcept;

  std::string_view     get_file_name() const noexcept { return source_name_; }
  location_type const& loc() const noexcept { return loc_; }
  location_type&       loc() noexcept { return loc_; }

  void push_error(location const& l, std::string_view e) noexcept;
  void resolve(neo::command&) noexcept;

  neo::command_template const& find_template(std::string_view name) noexcept
  {
    auto it = templates_.find(name);
    if (it != templates_.end())
    {
      return it->second.back().get();
    }
    else
    {
      push_error(loc(), "template not found -> ");
      push_error(loc(), name);
      return null_template_;
    }
  }

  void begin_scan() noexcept;
  void end_scan() noexcept;

  void* scanner = nullptr;

  template <typename lambda>
  void for_each_error(lambda&& l) noexcept
  {
    std::for_each(errors_.begin(), errors_.end(), l);
  }

  bool fail_bit() const noexcept

  {
    return errors_.size() > 0 && !(flags_ & f_continue_on_error);
  }

  bool skip() const noexcept { return skip_ > 0; }

  void         enter_skip_scope() noexcept { skip_++; }
  std::int32_t exit_skip_scope() noexcept { return --skip_; }

  inline void start_read_len(int len) noexcept { len_reading_ = len; }

  inline void read_len(int l) noexcept 
  { 
    len_reading_ += l; 
  }

  inline int flush_read_len() noexcept

  {
    int r        = len_reading_;
    len_reading_ = 0;
    return r;
  }

private:
  using record_ptr = neo::command_template::record*;
  std::string_view default_import_handler(std::string_view) noexcept;

  using push_templates =
      std::vector<std::reference_wrapper<neo::command_template const>>;
  using template_map = std::unordered_map<std::string_view, push_templates>;
  using root_template_list = std::forward_list<neo::command_template>;

  using def_imported_string_map =
      std::unordered_map<std::string_view, std::string>;

  template_map                  templates_;
  std::vector<std::uint32_t>    block_stack_;
  std::vector<record_ptr>       record_stack_;
  std::vector<std::string>      errors_;
  def_imported_string_map       imported_;
  root_template_list            root_template_storage_;
  registry const&               registry_;
  command_handler*              cmd_handler_;
  resolver*                     resolver_stack_ = nullptr;
  std::string_view              region_;
  import_handler                importer_;
  std::string_view              current_file_;
  std::string                   source_name_;
  std::string_view              region_type_;
  std::string                   esq_string_;
  location_type                 loc_;
  option_flags                  flags_       = 0;
  std::int32_t                  skip_        = 0;
  std::int32_t                  pos_         = 0;
  std::int32_t                  pos_commit_  = 0;
  std::int32_t                  len_reading_ = 0;
  text_content                  content_;
  static const command_template null_template_;
};
} // namespace neo