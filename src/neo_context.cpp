
#include <neo_context.hpp>

namespace neo {

void context::start_region(std::string&& region) {
  this->region = std::move(region);
}
void context::consume(neo::command&& cmd) { 
  if (current_block_)
    current_block_ = current_block_->execute(cmd);
}
void context::consume(neo::command_template&& cmd_templ) {
  templates_.emplace_back(std::move(cmd_templ));
}
void context::consume(neo::command_instance&& cmd_inst) {
}
void context::end_block() {
  if (current_block_)
    current_block_ = current_block_->parent_;
}
void context::start_region(std::string&& region_id, std::string&& content) {}
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
                                             neo::command_instance&&,
                                             bool scope_trigger) {
  return neo::command_instance();
}
std::string const* context::get_file_name() const { return nullptr; }
void context::push_error(location const& l, std::string const& e) {}
} // namespace neo