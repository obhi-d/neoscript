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
                             state_machine::option_flags flags)
    : flags_(flags), importer_(&state_machine::default_import_handler),
      registry_(reg), cmd_handler_(handler)
{}

void state_machine::start_region(std::string&& region)
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
void state_machine::consume(neo::command&& cmd)
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
    case command_handler::results::e_success:
      if (cmd.is_scoped() && block != registry::k_invalid_id)
        block_stack_.push_back(block);
      break;
    case command_handler::results::e_fail_and_stop:
      push_error(loc(), "command execution failure");
      return;
    case command_handler::results::e_skip_block:
      if (!cmd.is_scoped())
        return;
      [[fallthrough]];
    case command_handler::results::e_skip_rest:
      skip_++;
      break;
    }
  }
}
void state_machine::add_template(neo::command_template&& cmd_templ)
{
  root_template_storage_.emplace_front(std::move(cmd_templ));
  record_template(root_template_storage_.front());
}
void state_machine::record_template(neo::command_template& cmd_templ)
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
void state_machine::push_template(neo::command_template const& cmd_templ)
{
  templates_[cmd_templ.name()].emplace_back(std::cref(cmd_templ));
}
void state_machine::remove_template(std::string const& name)
{
  auto it = templates_.find(name);
  if (it != templates_.end())
  {
    it->second.pop_back();
  }
}
void state_machine::consume(neo::command_instance&& cmd_inst)
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
    cmd_inst.visit(*this, cmd_inst.is_extended());
    resolver_stack_ = resolver_stack_->next_;
  }
}
void state_machine::end_block()
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
      neo::detail::interpreter::end_scope(registry_, *this, cmd_handler_,
                                          block_stack_.back(), "");
      block_stack_.pop_back();
    }
  }
}
void state_machine::start_region(std::string&& region_id, std::string&& content)
{
  neo::detail::interpreter::handle_text_region(
      registry_, cmd_handler_, *this, std::move(region_id), std::move(content));
}
void state_machine::import_script(std::string const& file_id)
{
  auto file = importer_(file_id);
  parse(file_id, file);
}
void state_machine::error(location_type const&, std::string const&) {}
neo::command_template state_machine::make_command_template(
    std::vector<std::string>&& params, neo::command&& cmd)
{
  std::string name(cmd.name());
  return neo::command_template(std::move(name), std::move(params),
                               std::move(cmd));
}

neo::command_template state_machine::make_command_template(
    std::string&& name, std::vector<std::string>&& params, neo::command&& cmd)
{
  return neo::command_template(std::move(name), std::move(params),
                               std::move(cmd));
}
neo::command state_machine::make_command(std::string&&              name,
                                         neo::command::parameters&& params,
                                         bool scope_trigger)
{
  return neo::command(std::move(name), std::move(params), scope_trigger);
}
neo::command_instance state_machine::make_instance(
    std::string&& name, neo::command::param_t&& param, bool scope_trigger)
{
  return neo::command_instance(std::move(name), std::move(param),
                               scope_trigger);
}
void state_machine::push_error(location const& l, std::string_view e)
{
  std::string loc = l;
  loc += e;
  errors_.emplace_back(std::move(loc));
}

void state_machine::resolve(neo::command& cmd) { cmd.resolve(resolver_stack_); }
int  state_machine::read(char* buffer, int size)
{
  current_file_->read(buffer, static_cast<std::streamsize>(size - 1));
  int read     = static_cast<int>(current_file_->gcount());
  buffer[read] = 0;
  return read;
}
std::shared_ptr<std::istream> neo::state_machine::default_import_handler(
    std::string const& file)
{
  return std::make_shared<std::ifstream>(file);
}
} // namespace neo