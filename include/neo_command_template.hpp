#pragma once
#include <neo_command.hpp>
#include <neo_command_instance.hpp>
#include <variant>
#include <vector>

namespace neo
{
class state_machine;
class NEO_API command_template
{
public:
  struct NEO_API command_record
  {
    neo::command cmd_;
    command_record() = default;
    command_record(command_record const& _) : cmd_(_.cmd_) {}
    command_record(command_record&&) = default;
    command_record& operator         =(command_record const& _)
    {
      cmd_ = _.cmd_;
      return *this;
    }
    command_record& operator=(command_record&&) = default;
    command_record(neo::command&& cmd) : cmd_(std::move(cmd)) {}
    command_record(neo::command const& cmd) : cmd_(cmd) {}
  };

  struct NEO_API template_record
  {
    std::string              name_;
    neo::command             cmd_;
    std::vector<std::string> params_;
    template_record()                  = default;
    template_record(template_record&&) = default;
    template_record(template_record const& _)
        : name_(_.name_), cmd_(_.cmd_), params_(_.params_)
    {}
    template_record& operator=(template_record&&) = default;
    template_record& operator                     =(template_record const& _)
    {
      name_   = _.name_;
      cmd_    = _.cmd_;
      params_ = _.params_;
      return *this;
    }
    template_record(std::string&& name, std::vector<std::string>&& vec,
                    neo::command&& cmd)
        : name_(std::move(name)), cmd_(std::move(cmd)), params_(std::move(vec))
    {}
  };

  struct NEO_API instance_record
  {
    neo::command_instance instance_;
    instance_record() = default;
    instance_record(instance_record const& _) : instance_(_.instance_) {}
    instance_record(instance_record&&) = default;
    instance_record& operator          =(instance_record const& _)
    {
      instance_ = _.instance_;
      return *this;
    }
    instance_record& operator=(instance_record&&) = default;
    instance_record(neo::command_instance&& cmd) : instance_(std::move(cmd)) {}
  };

  using command_template_ref = std::reference_wrapper<command_template const>;
  using node = std::variant<std::monostate, command_record, template_record,
                            instance_record, command_template_ref>;
  struct NEO_API record
  {
    node                node_;
    std::vector<record> sub_;

    bool pre_visit(neo::state_machine&) const;
    bool visit(neo::state_machine&) const;
    void post_visit(neo::state_machine&) const;

    record() = default;
    record(record const& rec) : node_(rec.node_), sub_(rec.sub_) {}

    record& operator=(record&&) = default;
    record& operator            =(record const& rec)
    {
      node_ = rec.node_;
      sub_  = rec.sub_;
      return *this;
    }
    record(command_record&& rec) : node_(std::move(rec)) {}
    record(command_record const& rec) : node_(rec) {}
    record(template_record&& rec) : node_(std::move(rec)) {}
    record(instance_record&& rec) : node_(std::move(rec)) {}
    record(command_template_ref&& rec) : node_(std::move(rec)) {}
    record(template_record const& rec) : node_(rec) {}
    record(instance_record const& rec) : node_(rec) {}
    record(command_template_ref const& rec) : node_(rec) {}
    record(std::string&& name, std::vector<std::string>&& params,
           neo::command&& cmd)
        : node_(std::in_place_type_t<template_record>{}, std::move(name),
                std::move(params), std::move(cmd))
    {}
  };

  command_template()                        = default;
  command_template(command_template const&) = default;
  command_template(command_template&&)      = default;
  command_template& operator=(command_template const&) = default;
  command_template& operator=(command_template&&) = default;

  command_template(template_record const& rec) : main_(rec) {}
  command_template(std::string&& name, std::vector<std::string>&& params,
                   neo::command&& cmd)
      : main_(std::move(name), std::move(params), std::move(cmd))
  {}

  std::string const& name() const
  {
    return std::get<template_record>(main_.node_).name_;
  }

  bool is_scoped() const
  {
    return std::get<template_record>(main_.node_).cmd_.is_scoped();
  }

  bool visit(neo::state_machine&, bool extended) const;

  std::vector<std::string> const& get_params() const
  {
    return std::get<template_record>(main_.node_).params_;
  }

  record main_;
};
} // namespace neo