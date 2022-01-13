#include <fstream>

// neo
#include <neo_registry.hpp>
#include <neo_state_machine.hpp>

// interpreter
#include <detail/neo_interpreter.hpp>

namespace neo
{

const command_template state_machine::null_template_;
state_machine::state_machine(registry const& reg, command_handler* handler,
                             state_machine::option_flags flags) noexcept
    : flags_(flags), 
      registry_(reg), cmd_handler_(handler)
{
  importer_ = [this](std::string_view src)
  { return default_import_handler(src); };
}

void state_machine::start_region(std::string_view region) noexcept
{
  if (block_stack_.size() > 1 || skip_ > 0)
  {
    push_error(loc(), "syntax error, missing '}'");
    return;
  }

  block_stack_.clear();
  auto block = registry_.get_region_root(region);
  block_stack_.push_back(block);
  this->region_ = std::move(region);
}
bool state_machine::end_block() noexcept
{
  if (record_stack_.size() > 0)
  {
    record_stack_.pop_back();
  }
  else
  {
    if (block_stack_.size() <= 1)
      push_error(loc(), "syntax error, unexpected '}'");
    else
    {
      switch(neo::detail::interpreter::end_scope(registry_, *this, cmd_handler_,
                                          block_stack_.back(), ""))
      {
      case neo::retcode::e_success:
        break;
      case neo::retcode::e_success_stop:
        return false;
      case neo::retcode::e_fail_and_stop:
        push_error(loc(), "command end failure");
        return true;
      case neo::retcode::e_skip_block:
        push_error(loc(), "block already executed");
        break;
      case neo::retcode::e_skip_rest:
        skip_++;
        block_stack_.pop_back();
        break;
      }
      block_stack_.pop_back();
    }
  }
  return true;
}
bool state_machine::consume(neo::command&& cmd) noexcept
{
  if (record_stack_.size() > 0)
  {
    bool scoped = cmd.is_scoped();
    auto rec    = record_stack_.back();
    rec->sub_.emplace_back(command_template::command_record(std::move(cmd)));
    if (scoped)
      record_stack_.push_back(&rec->sub_.back());
  }
  else
  {
    if (resolver_stack_)
      resolve(cmd);
    auto [block, result] = neo::detail::interpreter::execute(
        registry_, *this, cmd_handler_, this->block_stack_.back(), cmd);
    switch (result)
    {
    case neo::retcode::e_success:
      if (cmd.is_scoped() && block != registry::k_invalid_id)
        block_stack_.push_back(block);
      break;
    case neo::retcode::e_success_stop:
      return false;
    case neo::retcode::e_cmd_not_found:
      {
        std::string err = "command not found: ";
        err += cmd.name();
        push_error(loc(), err);
      }
      return true;
    case neo::retcode::e_fail_and_stop:
      push_error(loc(), "command execution failure");
      return true;
    case neo::retcode::e_skip_block:
      if (!cmd.is_scoped())
        return true;
      [[fallthrough]];
    case neo::retcode::e_skip_rest:
      skip_++;
      break;
    }
  }
  return true;
}
void state_machine::add_template(neo::command_template&& cmd_templ) noexcept
{
  root_template_storage_.emplace_front(std::move(cmd_templ));
  record_template(root_template_storage_.front());
}
void state_machine::record_template(neo::command_template& cmd_templ) noexcept
{
  if (record_stack_.size() == 0)
  {
    record_ptr new_rec   = &(cmd_templ.main_);
    bool       is_scoped = cmd_templ.is_scoped();
    templates_[cmd_templ.name()].emplace_back(std::cref(cmd_templ));

    if (is_scoped)
      record_stack_.push_back(new_rec);
  }
  else
  {
    bool scoped = cmd_templ.is_scoped();
    auto rec    = record_stack_.back();
    // TODO
    // We need a DOM
    rec->sub_.emplace_back(std::cref(cmd_templ));
    if (scoped)
      record_stack_.push_back(&cmd_templ.main_);
  }
}
void state_machine::push_template(
    neo::command_template const& cmd_templ) noexcept
{
  templates_[cmd_templ.name()].emplace_back(std::cref(cmd_templ));
}
void state_machine::remove_template(std::string_view name) noexcept
{
  auto it = templates_.find(name);
  if (it != templates_.end())
  {
    it->second.pop_back();
  }
}
bool state_machine::consume(neo::command_instance&& cmd_inst) noexcept
{
  if (record_stack_.size() > 0)
  {
    bool  scoped = cmd_inst.is_extended();
    auto& rec    = record_stack_.back();
    rec->sub_.emplace_back(
        command_template::instance_record(std::move(cmd_inst)));
    if (scoped)
      record_stack_.push_back(&rec->sub_.back());
  }
  else
  {
    resolver res;
    res.op_         = std::bind(&command_instance::resolve, &cmd_inst,
                        std::placeholders::_1, std::placeholders::_2);
    res.next_       = resolver_stack_;
    resolver_stack_ = &res;
    bool result     = cmd_inst.visit(*this, cmd_inst.is_extended());
    resolver_stack_ = resolver_stack_->next_;
    return result;
  }
  return true;
}
void state_machine::start_region(std::string_view region_id,
                                 text_content&& content) noexcept
{
  neo::detail::interpreter::handle_text_region(
      registry_, cmd_handler_, *this, region_type_,
      region_id,
      std::move(content));
}
void state_machine::import_script(std::string_view file_id) noexcept
{
  auto file = importer_(file_id);
  parse(file_id, file);
}
void state_machine::error(location_type const&, std::string const&) noexcept {}
neo::command_template state_machine::make_command_template(
    std::vector<std::string_view>&& params, neo::command&& cmd) noexcept
{
  return neo::command_template(cmd.name(), std::move(params),
                               std::move(cmd));
}

neo::command_template state_machine::make_command_template(
    std::string_view name, std::vector<std::string_view>&& params,
    neo::command&& cmd) noexcept
{
  return neo::command_template(std::move(name), std::move(params),
                               std::move(cmd));
}
neo::command state_machine::make_command(std::string_view              name,
                                         neo::command::parameters&& params,
                                         bool scope_trigger) noexcept
{
  return neo::command(std::move(name), std::move(params), scope_trigger);
}
neo::command_instance state_machine::make_instance(
    std::string_view name, neo::command::param_t&& param,
    bool scope_trigger) noexcept
{
  return neo::command_instance(std::move(name), std::move(param),
                               scope_trigger);
}
void state_machine::push_error(location const& l, std::string_view e) noexcept
{
  std::string loc = l;
  loc += e;
  errors_.emplace_back(std::move(loc));
}

void state_machine::resolve(neo::command& cmd) noexcept
{
  cmd.resolve(resolver_stack_);
}
int state_machine::read(char* data, int size) noexcept
{
  auto min = std::min<std::int32_t>(
      static_cast<std::int32_t>(current_file_.size() - pos_), size);
  if (min)
  {
    std::memcpy(data, current_file_.data() + pos_,
                static_cast<std::size_t>(min));
    pos_ += min;
    if (min < size)
      data[min] = 0;
    
    assert(min <= size);
    return min;
  }
  data[0] = 0;
  return 0;
}
std::string_view neo::state_machine::default_import_handler(
    std::string_view file) noexcept
{
  auto it = imported_.find(file);
  if (it != imported_.end())
  {
    return std::string_view(it->second);
  }
  std::ifstream t{std::string(file)};
  t.seekg(0, std::ios::end);
  size_t      size = t.tellg();
  std::string buffer(size, '\0');
  t.seekg(0);
  t.read(&buffer[0], size); 
  auto r = imported_.emplace(file, std::move(buffer));
  return r.first->second;
}
} // namespace neo