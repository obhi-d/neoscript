#include <catch2/catch.hpp>
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

struct test_command_handler : public neo::command_handler
{
  bool generate(neo::state_machine const&, neo::command const& cmd)
  {
    output += cmd.name();
    output += " --> (";
    auto const& params = cmd.params().value();
    bool        first  = true;
    for (auto const& val : params)
    {
      if (!first)
        output += ", ";
      first = false;
      output += to_string(val);
    }
    output += ")\n";
    return true;
  }

  bool print_region(neo::state_machine const& ctx, neo::command const& cmd)
  {
    auto const& params = cmd.params().value();
    bool        first  = true;
    std::string output;
    for (auto const& val : params)
    {
      if (std::visit(neo::overloaded{[&](neo::command::single const& s)
                                     {
                                       if (s.name() == "region")
                                       {
                                         auto it = text.find(s.value());
                                         if (it != text.end())
                                           output = (*it).second;
                                       }
                                       return true;
                                     },
                                     [](auto const&) { return false; }},
                     val))
        break;
    }
    this->output = output;
    return true;
  }

  void add_text(neo::state_machine const&, std::string&& name, std::string&& content)
  {
    text.emplace(std::move(name), std::move(content));
  }

  std::unordered_map<std::string, std::string> text;
  std::string                                  output;
};

TEST_CASE("Scoped command", "[region0]")
{
  neo::interpreter test_interpreter;
  std::string      test = "{{code:Scoped}} \n"
                     "echo \"Hello world\"; \n";
  std::shared_ptr<std::istream> iss =
      std::make_shared<std::istringstream>(test);

  auto root = test_interpreter.ensure_region_root("code:Scoped");
  test_interpreter.add_command(
      root, "echo",
      [](neo::command_handler* _, neo::state_machine const& ctx,
         neo::command const& cmd) -> bool
      { return static_cast<test_command_handler*>(_)->generate(ctx, cmd); });
  test_command_handler handler;
  neo::state_machine         state_machine(test_interpreter, &handler, 0);
  state_machine.parse("memory", iss);
  REQUIRE(!state_machine.fail_bit());
  REQUIRE(handler.output == "echo --> (Hello world)\n");
}

TEST_CASE("Text region", "[region1]")
{
  neo::interpreter test_interpreter;
  std::string      test = "{{text:History}}\n"
                     "The world began when we perished.\n"
                     "But we left our mark to be found again.\n"
                     "In the future, we lived again in memories.\n"
                     "Of time{\n"
                     "\\{\\{unknown}}\n"
                     "{{code:Print}}\n"
                     "print (region = \"text:History\"); \n";
  std::shared_ptr<std::istream> iss =
      std::make_shared<std::istringstream>(test);

  auto root = test_interpreter.ensure_region_root("code:Print");
  test_interpreter.add_command(
      root, "print",
      [](neo::command_handler* _, neo::state_machine const& ctx,
         neo::command const& cmd) -> bool {
        return static_cast<test_command_handler*>(_)->print_region(ctx, cmd);
      });
  test_interpreter.set_text_region_handler(
      [](neo::command_handler* _, neo::state_machine const& ctx, std::string&& name,
         std::string&& content)
      {
        return static_cast<test_command_handler*>(_)->add_text(
            ctx, std::move(name), std::move(content));
      });
  test_command_handler handler;
  neo::state_machine         state_machine(test_interpreter, &handler, 0);
  state_machine.parse("memory", iss);
  REQUIRE(!state_machine.fail_bit());
  REQUIRE(handler.output == "\nThe world began when we perished.\n"
                            "But we left our mark to be found again.\n"
                            "In the future, we lived again in memories.\n"
                            "Of time\n"
                            "{{unknown}}\n");
}

TEST_CASE("Any command", "[region2]")
{
  neo::interpreter test_interpreter;
  std::string      test = "first command [is, executed];\n"
                     "second command [is, stalled];\n"
                     "{{code:Main}}\n"
                     "Third command [is, super = executed];\n"
                     "Fourth command [is, stalled] {\n"
                     "  Four point one command;\n"
                     "  Four point two command;\n"
                     "}\n"
                     "{{code:Print}}\n"
                     "print (region = \"text:History\"); \n";
  std::shared_ptr<std::istream> iss =
      std::make_shared<std::istringstream>(test);

  neo::command_handler_fn lambda = [](neo::command_handler* _,
                                      neo::state_machine const&   ctx,
                                      neo::command const&   cmd) -> bool
  { return static_cast<test_command_handler*>(_)->generate(ctx, cmd); };
  auto root = test_interpreter.ensure_region_root("");
  test_interpreter.add_command(root, "*", lambda);
  root = test_interpreter.ensure_region_root("code:Print");
  test_interpreter.add_command(root, "*", lambda);
  root = test_interpreter.ensure_region_root("code:Main");
  test_interpreter.add_command(root, "*", lambda);
  root = test_interpreter.add_scoped_command(root, "Fourth", lambda);
  test_interpreter.add_command(root, "*", lambda);
  test_command_handler handler;
  neo::state_machine         state_machine(test_interpreter, &handler, 0);
  state_machine.parse("memory", iss);
  REQUIRE(!state_machine.fail_bit());
  REQUIRE(handler.output == "first --> (command, [is, executed])\n"
                            "second --> (command, [is, stalled])\n"
                            "Third --> (command, [is, super = executed])\n"
                            "Fourth --> (command, [is, stalled])\n"
                            "Four --> (point, one, command)\n"
                            "Four --> (point, two, command)\n"
                            "print --> (region = text:History)\n");
}

TEST_CASE("Block test", "[block]")
{
  neo::interpreter test_interpreter;
  std::string      test = "block parameter0 parameter1 {\n"
                     " command parameter2 parameter4;\n"
                     "};\n";
  std::shared_ptr<std::istream> iss =
      std::make_shared<std::istringstream>(test);

  auto root = test_interpreter.ensure_region_root("");
  test_interpreter.add_scoped_command(
      root, "*",
      [](neo::command_handler* _, neo::state_machine const& ctx,
         neo::command const& cmd) -> bool
      { return static_cast<test_command_handler*>(_)->generate(ctx, cmd); });

  test_command_handler handler;
  neo::state_machine         state_machine(test_interpreter, &handler, 0);
  state_machine.parse("memory", iss);
  if (state_machine.fail_bit())
  {
    std::cerr << "[ERROR] While compiling [block]" << std::endl;
    state_machine.for_each_error([](std::string const& err)
                           { std::cerr << "        " << err << std::endl; });
  }
  REQUIRE(!state_machine.fail_bit());
  REQUIRE(handler.output == "block --> (parameter0, parameter1)\n"
                            "command --> (parameter2, parameter4)\n");
}