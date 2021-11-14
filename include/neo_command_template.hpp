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
    command_record() noexcept = default;
    command_record(command_record const& _) noexcept : cmd_(_.cmd_) {}
    command_record(command_record&&) noexcept = default;
    command_record& operator=(command_record const& _) noexcept 
    {
      cmd_ = _.cmd_;
      return *this;
    }
    command_record& operator=(command_record&&) noexcept = default;
    command_record(neo::command&& cmd) noexcept : cmd_(std::move(cmd)) {}
    command_record(neo::command const& cmd) noexcept : cmd_(cmd) {}
  };

  struct NEO_API template_record
  {
    std::string_view              name_;
    neo::command                  cmd_;
    std::vector<std::string_view> params_;
    template_record() noexcept         = default;
    template_record(template_record&&) noexcept = default;
    template_record(template_record const& _) noexcept 
        : name_(_.name_), cmd_(_.cmd_), params_(_.params_)
    {}
    template_record& operator=(template_record&&) noexcept = default;
    template_record& operator=(template_record const& _) noexcept 
    {
      name_   = _.name_;
      cmd_    = _.cmd_;
      params_ = _.params_;
      return *this;
    }
    template_record(std::string_view name, std::vector<std::string_view>&& vec,
                    neo::command&& cmd) noexcept 
        : name_(std::move(name)), cmd_(std::move(cmd)), params_(std::move(vec))
    {}
  };

  struct NEO_API instance_record
  {
    neo::command_instance instance_;
    instance_record() = default;
    instance_record(instance_record const& _) noexcept : instance_(_.instance_)
    {}
    instance_record(instance_record&&) noexcept = default;
    instance_record& operator=(instance_record const& _) noexcept 
    {
      instance_ = _.instance_;
      return *this;
    }
    instance_record& operator=(instance_record&&) noexcept = default;
    instance_record(neo::command_instance&& cmd) noexcept
        : instance_(std::move(cmd))
    {}
  };

  using command_template_ref = std::reference_wrapper<command_template const>;
  using node = std::variant<std::monostate, command_record, template_record,
                            instance_record, command_template_ref>;
  struct NEO_API record
  {
    node                node_;
    std::vector<record> sub_;

    bool pre_visit(neo::state_machine&) const noexcept;
    bool visit(neo::state_machine&) const noexcept;
    void post_visit(neo::state_machine&) const noexcept;

    record() noexcept = default;
    record(record const& rec) noexcept : node_(rec.node_), sub_(rec.sub_) {}

    record& operator=(record&&) noexcept = default;
    record& operator                     =(record const& rec) noexcept 
    {
      node_ = rec.node_;
      sub_  = rec.sub_;
      return *this;
    }
    record(command_record&& rec) noexcept : node_(std::move(rec)) {}
    record(command_record const& rec) noexcept : node_(rec) {}
    record(template_record&& rec) noexcept : node_(std::move(rec)) {}
    record(instance_record&& rec) noexcept : node_(std::move(rec)) {}
    record(command_template_ref&& rec) noexcept : node_(std::move(rec)) {}
    record(template_record const& rec) noexcept : node_(rec) {}
    record(instance_record const& rec) noexcept : node_(rec) {}
    record(command_template_ref const& rec) noexcept : node_(rec) {}
    record(std::string_view name, std::vector<std::string_view>&& params,
           neo::command&& cmd) noexcept 
        : node_(std::in_place_type_t<template_record>{}, std::move(name),
                std::move(params), std::move(cmd))
    {}
  };

  command_template() noexcept                   = default;
  command_template(command_template const&) noexcept = default;
  command_template(command_template&&) noexcept = default;
  command_template& operator=(command_template const&) noexcept = default;
  command_template& operator=(command_template&&) noexcept = default;

  command_template(template_record const& rec) noexcept : main_(rec) {}
  command_template(std::string_view                name,
                   std::vector<std::string_view>&& params,
                   neo::command&&                  cmd) noexcept 
      : main_(std::move(name), std::move(params), std::move(cmd))
  {}

  std::string_view const& name() const noexcept 
  {
    return std::get<template_record>(main_.node_).name_;
  }

  bool is_scoped() const noexcept 
  {
    return std::get<template_record>(main_.node_).cmd_.is_scoped();
  }

  bool visit(neo::state_machine&, bool extended) const noexcept;

  std::vector<std::string_view> const& get_params() const noexcept 
  {
    return std::get<template_record>(main_.node_).params_;
  }

  record main_;
};
} // namespace neo