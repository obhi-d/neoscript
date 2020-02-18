
#include <neo_command.hpp>
#include <neo_command_instance.hpp>
#include <variant>
#include <vector>

namespace neo {

class context;
class command_template {
public:
  struct command_record {
    neo::command cmd_;
    command_record() = default;
    command_record(command_record const&) = default;
    command_record(command_record &&) = default;
    command_record& operator=(command_record const&) = default;
    command_record& operator=(command_record&&) = default;
    command_record(neo::command&& cmd) : cmd_(std::move(cmd)) {}

  };

  struct template_record {
    std::string              name_;
    neo::command             cmd_;
    std::vector<std::string> params_;
  };

  struct instance_record {
    neo::command_instance instance_;
    instance_record()                     = default;
    instance_record(instance_record const&) = default;
    instance_record(instance_record&&)      = default;
    instance_record& operator=(instance_record const&) = default;
    instance_record& operator=(instance_record&&) = default;
    instance_record(neo::command_instance&& cmd) : cmd_(std::move(cmd)) {}
  };

  using node = std::variant<std::monostate, command_record, template_record,
                            instance_record>;
  struct record {
    node                node_;
    std::vector<record> sub_;

    void visit(neo::context&, bool extend) const;

    record()              = default;
    record(record const&) = default;
    record(record&&)      = default;
    record(command_record&& rec) : node_(std::move(rec)) {}
    record(command_record const& rec) : node_(rec) {}
    record(template_record&& rec) : node_(std::move(rec)) {}
    record(instance_record&& rec) : node_(std::move(rec)) {}
  };

  command_template()                        = default;
  command_template(command_template const&) = default;
  command_template(command_template&&)      = default;
  command_template& operator=(command_template const&) = default;
  command_template& operator=(command_template&&) = default;

  command_template(template_record const&);

  std::string const& name() const {
    return std::get<template_record>(main_.node_).name_;
  }

  bool is_scoped() const {
    return std::get<template_record>(main_.node_).cmd_.is_scoped();
  }

  void visit(neo::context&, bool extended) const;
  std::vector<std::string> const& get_params() const {
    return std::get<template_record>(main_.node_).params_;
  }

  record main_;
};
} // namespace neo