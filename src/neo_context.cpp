#include <fstream>
#include <neo_context.hpp>
#include <neo_interpreter.hpp>

namespace neo {

const command_template context::null_template_;
context::context(interpreter& interp, command_handler& handler,
                 context::option_flags flags)
    : flags_(flags), importer_(&context::default_import_handler),
      interpreter_(interp), cmd_handler_(handler) {}

void context::start_region(std::string&& region) {
  if (block_stack_.size() > 1) {
    push_error(loc(), "syntax error, missing '}'");
    return;
  }
  block_stack_.clear();
  block_stack_.push_back(interpreter_.get_region_root(region));
  this->region_ = std::move(region);
}
void context::consume(neo::command&& cmd) {
  if (record_stack_.size() > 0) {
    bool scoped = cmd.is_scoped();
    auto rec    = record_stack_.back();
    rec->sub_.emplace_back(command_template::command_record(std::move(cmd)));
    if (scoped)
      record_stack_.push_back(&rec->sub_.back());
  } else {
    if (resolver_stack_)
      resolve(cmd);
    auto sub = interpreter_.execute(*this, cmd_handler_,
                                    this->block_stack_.back(), cmd);
    if (std::get<0>(sub) == interpreter::k_failure) {
      push_error(loc(), "command execution failure");
      return;
    }
    if (cmd.is_scoped() && std::get<1>(sub)) {
      block_stack_.push_back(std::get<0>(sub));
      interpreter_.begin_scope(*this, cmd_handler_, std::get<0>(sub));
    }
  }
}
void context::add_template(neo::command_template&& cmd_templ) {
  root_template_storage_.emplace_front(std::move(cmd_templ));
  record_template(root_template_storage_.front());
}
void context::record_template(neo::command_template& cmd_templ) {
  if (record_stack_.size() == 0) {
    record_ptr new_rec   = &(cmd_templ.main_);
    bool       is_scoped = cmd_templ.is_scoped();
    templates_[cmd_templ.name()].emplace_back(std::cref(cmd_templ));

    if (is_scoped)
      record_stack_.push_back(new_rec);
  } else {
    bool scoped = cmd_templ.is_scoped();
    auto rec    = record_stack_.back();
    // TODO
    // We need a DOM
    rec->sub_.emplace_back(std::cref(cmd_templ));
    if (scoped)
      record_stack_.push_back(&cmd_templ.main_);
  }
}
void context::push_template(neo::command_template const& 
                                cmd_templ) {
  templates_[cmd_templ.name()].emplace_back(std::cref(cmd_templ));
}
void context::remove_template(std::string const& name) {
  auto it = templates_.find(name);
  if (it != templates_.end()) {
    it->second.pop_back();
  }
}
void context::consume(neo::command_instance&& cmd_inst) {
  if (record_stack_.size() > 0) {
    bool  scoped = cmd_inst.is_extended();
    auto& rec    = record_stack_.back();
    rec->sub_.emplace_back(
        command_template::instance_record(std::move(cmd_inst)));
    if (scoped)
      record_stack_.push_back(&rec->sub_.back());
  } else {
    resolver res;
    res.op_         = std::bind(&command_instance::resolve, &cmd_inst,
                        std::placeholders::_1, std::placeholders::_2);
    res.next_       = resolver_stack_;
    resolver_stack_ = &res;
    cmd_inst.visit(*this, cmd_inst.is_extended());
    resolver_stack_ = resolver_stack_->next_;
  }
}
void context::end_block() {
  if (record_stack_.size() > 0) {
    record_stack_.pop_back();
  } else {
    if (block_stack_.size() <= 1)
      push_error(loc(), "syntax error, unexpected '}'");
    else {
      interpreter_.end_scope(*this, cmd_handler_, block_stack_.back());
      block_stack_.pop_back();
    }
  }
}
void context::start_region(std::string&& region_id, std::string&& content) {
  text_regions_.emplace(std::move(region_id), std::move(content));
}
void context::import_script(std::string const& file_id) {
  auto file = importer_(file_id);
  parse(file_id, file);
}
void context::error(location_type const&, std::string const&) {}
neo::command_template context::make_command_template(
    std::vector<std::string>&& params, neo::command&& cmd) {
  std::string name(cmd.name());
  return neo::command_template(std::move(name), std::move(params),
                               std::move(cmd));
}

neo::command_template context::make_command_template(
    std::string&& name, std::vector<std::string>&& params, neo::command&& cmd) {
  return neo::command_template(std::move(name), std::move(params),
                               std::move(cmd));
}
neo::command context::make_command(std::string&&              name,
                                   neo::command::parameters&& params,
                                   bool                       scope_trigger) {
  return neo::command(std::move(name), std::move(params), scope_trigger);
}
neo::command_instance context::make_instance(std::string&&           name,
                                             neo::command::param_t&& param,
                                             bool scope_trigger) {
  return neo::command_instance(std::move(name), std::move(param),
                               scope_trigger);
}
void context::push_error(location const& l, std::string_view e) {
  std::string loc = l;
  loc += e;
  errors_.emplace_back(std::move(loc));
}

void context::resolve(neo::command& cmd) { cmd.resolve(resolver_stack_); }
int  context::read(char* buffer, int size) {
  current_file_->read(buffer, static_cast<std::streamsize>(size - 1));
  int read     = static_cast<int>(current_file_->gcount());
  buffer[read] = 0;
  return read;
}
std::shared_ptr<std::istream> neo::context::default_import_handler(
    std::string const& file) {
  return std::make_shared<std::ifstream>(file);
}
} // namespace neo