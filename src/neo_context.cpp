#include <neo_context.hpp>
#include <neo_interpreter.hpp>

namespace neo {
context::context() {}
void context::start_region(std::string&& region) {
  if (block_stack_.size() > 1) {
    push_error(loc(), "syntax error, missing '}'");
    return;
  }
  block_stack_.clear();
  if (interpreter_)
    block_stack_.push_back(interpreter_->get_region_root(region));
  else
    block_stack_.push_back(0);
  this->region_ = std::move(region);
}
void context::consume(neo::command&& cmd) {
  if (record_stack_.size() > 0) {
    bool scoped = cmd.is_scoped();
    auto rec    = record_stack_.back();
    rec->sub_.emplace_back(command_template::command_record(std::move(cmd)));
    if (scoped)
      record_stack_.push_back(&rec->sub_.back());
  } else if (interpreter_) {
    if (resolver_stack_)
      resolve(cmd);
    std::uint32_t sub =
        interpreter_->execute(this->block_stack_.back(), *this, cmd);
    if (cmd.is_scoped())
      block_stack_.push_back(sub);
  }
}
void context::add_template(neo::command_template&& cmd_templ) {
  if (record_stack_.size() == 0) {
    neo::command_template::record* new_rec = nullptr;
    auto                           it      = templates_.find(cmd_templ.name());
    if (it != templates_.end()) {
      if ((*it).second.index() == 0) {
        neo::command_template& templ =
            std::get<neo::command_template>((*it).second);
        (*it).second = std::move(std::vector<neo::command_template>{
            std::move(templ), std::move(cmd_templ)});
      } else {
        std::vector<neo::command_template>& v =
            std::get<std::vector<neo::command_template>>((*it).second);
        v.emplace_back(std::move(cmd_templ));
      }
      std::vector<neo::command_template>& v =
          std::get<std::vector<neo::command_template>>((*it).second);
      new_rec = &v.back().main_;
    } else {
      auto                   it = templates_.emplace(cmd_templ.name(),
                                   templ_one_many(std::move(cmd_templ)));
      neo::command_template& templ =
          std::get<neo::command_template>((*it.first).second);
      new_rec = &templ.main_;
    }
    record_stack_.push_back(new_rec);
  } else {
    bool scoped = cmd_templ.is_scoped();
    auto rec    = record_stack_.back();
    rec->sub_.push_back(std::move(cmd_templ.main_));
    if (scoped)
      record_stack_.push_back(&rec->sub_.back());
  }
}
void context::add_template(neo::command_template::template_record&& cmd_templ) {
  auto& it = templates_.find(cmd_templ.name_);
  if (it != templates_.end()) {
    if ((*it).second.index() == 0) {
      neo::command_template& templ =
          std::get<neo::command_template>((*it).second);
      (*it).second = std::move(std::vector<neo::command_template>{
          std::move(templ), neo::command_template(std::move(cmd_templ))});
    } else {
      std::vector<neo::command_template>& v =
          std::get<std::vector<neo::command_template>>((*it).second);
      v.emplace_back(std::move(cmd_templ));
    }
  } else {
    auto it = templates_.emplace(
        cmd_templ.name_,
        templ_one_many(neo::command_template(std::move(cmd_templ))));
  }
}
void context::remove_template(std::string const& name) {
  auto& it = templates_.find(name);
  if (it != templates_.end()) {
    if ((*it).second.index() == 0) {
      templates_.erase(it);
    } else {
      std::vector<neo::command_template>& v =
          std::get<std::vector<neo::command_template>>((*it).second);
      v.pop_back();
    }
  }
}
void context::consume(neo::command_instance&& cmd_inst) {
  if (record_stack_.size() > 0) {
    bool scoped = cmd_inst.is_extended();
    auto rec    = record_stack_.back();
    rec->sub_.emplace_back(
        command_template::instance_record(std::move(cmd_inst)));
    if (scoped)
      record_stack_.push_back(&rec->sub_.back());
  } else if (interpreter_) {
    resolver res;
    res.op_ = std::bind(&command_instance::resolve, &cmd_inst, std::placeholders::_1, std::placeholders::_2);
    res.next_     = resolver_stack_;
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
    else
      block_stack_.pop_back();
  }
}
void context::start_region(std::string&& region_id, std::string&& content) {
  text_regions_.emplace(std::move(region_id), std::move(content));
}
void context::import_script(std::string&& file_id) {}
void context::parse() {}
void context::error(location_type const&, std::string const&) {}
neo::command_template context::make_command_template(std::vector<std::string>&&,
                                                     neo::command&&) {
  return neo::command_template();
}
neo::command_template context::make_command_template(std::string&&,
                                                     std::vector<std::string>&&,
                                                     neo::command&&) {
  return neo::command_template();
}
neo::command context::make_command(std::string&&, neo::command::parameters&&,
                                   bool scope_trigger) {
  return neo::command();
}
neo::command_instance context::make_instance(std::string&&,
                                             neo::command::param_t&&,
                                             bool scope_trigger) {
  return neo::command_instance();
}
std::string const* context::get_file_name() const { return nullptr; }
void context::push_error(location const& l, std::string const& e) {}
void context::resolve(neo::command& cmd) { cmd.resolve(resolver_stack_); }
} // namespace neo