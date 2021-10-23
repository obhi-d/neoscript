#include <filesystem>
#include <fstream>
#include <iostream>
#include <istream>
#include <neo_script.hpp>
#include <sstream>

std::string to_string(neo::command::param_t const& param)
{
  std::string output;
  std::visit(neo::overloaded{[&](auto const&) {},
                             [&](neo::command::single const& s)
                             {
                               if (s.name().length() > 0)
                               {
                                 output += s.name();
                                 output += " = ";
                               }
                               output += s.value();
                             },
                             [&](neo::command::list const& s)
                             {
                               if (s.name().length() > 0)
                               {
                                 output += s.name();
                                 output += " = ";
                               }
                               output += "[";

                               bool first = true;

                               for (auto const& b : s)
                               {
                                 if (!first)
                                   output += ", ";
                                 first = false;
                                 output += to_string(b);
                               }

                               output += "]";
                             }},
             param);
  return output;
}

struct fileout_command_handler : public neo::command_handler
{
  fileout_command_handler(std::string const& name) : file(name + ".out.txt")
  {
    assert(file.is_open());
  }

  static neo::retcode print(neo::command_handler*     _,
                            neo::state_machine const& ctx,
                            neo::command const&       cmd)
  {
    return static_cast<fileout_command_handler*>(_)->print_m(ctx, cmd);
  }

  static neo::retcode print_enter_scope(neo::command_handler*     _,
                                        neo::state_machine const& ctx,
                                        neo::command const&       cmd)
  {
    auto result = static_cast<fileout_command_handler*>(_)->print_m(ctx, cmd);
    if (result == neo::retcode::e_success && cmd.is_scoped())
      return static_cast<fileout_command_handler*>(_)->enter_scope_m(
          ctx, cmd.name());
    else
      return result;
  }

  static void leave_scope(neo::command_handler*     _,
                          neo::state_machine const& ctx, std::string_view name)
  {
    static_cast<fileout_command_handler*>(_)->leave_scope_m(ctx, name);
  }

  neo::retcode print_m(neo::state_machine const& ctx, neo::command const& cmd)
  {
    file << scope << cmd.name() << " --> (";
    auto const& params = cmd.params().value();
    bool        first  = true;
    for (auto const& val : params)
    {
      if (!first)
        file << ", ";
      first = false;
      file << to_string(val);
    }
    file << ")\n";
    last_cmd = cmd.name();
    return neo::retcode::e_success;
  }

  neo::retcode enter_scope_m(neo::state_machine const&, std::string_view name)
  {
    scope += last_cmd;
    scope += '/';
    return neo::retcode::e_success;
  }

  void leave_scope_m(neo::state_machine const&, std::string_view name)
  {
    scope.pop_back();
    std::size_t pos = scope.find_last_of('/');
    if (pos == std::string::npos)
      pos = 0;
    else
      pos++;
    scope.erase(pos, scope.length() - pos);
  }

  std::ofstream file;
  std::string   scope;
  std::string   last_cmd;
};

int main(int argc, char** argv)
{
  int rc = 0;
  if (argc > 1)
  {
    neo::registry test_interpreter;
    std::uint32_t root = test_interpreter.ensure_region_root("");
    test_interpreter.add_scoped_command(
        root, "*", &fileout_command_handler::print_enter_scope,
        &fileout_command_handler::leave_scope);

    for (int i = 1; i < argc; ++i)
    {
      std::string             file_name = argv[1];
      fileout_command_handler handler(file_name);
      neo::state_machine      state_machine(test_interpreter, &handler, 0);
      std::filesystem::path   p(file_name);
      if (std::filesystem::exists(p))
      {
        auto working_dir = p.parent_path();
        state_machine.set_import_handler(
            [working_dir](std::string const& name)
            {
              auto path = working_dir / name;
              return std::make_shared<std::ifstream>(path);
            });
        std::shared_ptr<std::istream> iss = std::make_shared<std::ifstream>(p);
        state_machine.parse(file_name, iss);
        if (state_machine.fail_bit())
        {
          std::cerr << "[ERROR] Compiling file " << file_name << std::endl;
          state_machine.for_each_error(
              [](std::string const& err)
              { std::cerr << "        " << err << std::endl; });
          rc--;
        }
      }
    }
  }
  else
  {
    std::cout
        << "Usage:\nvalidate_script first second third ...\n  Will generate a "
           "new file called first.out.txt, second.out.txt ... with command "
           "calls."
        << std::endl;
  }
  return rc;
}
