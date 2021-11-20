#include <catch2/catch.hpp>
#include <iostream>
#include <istream>
#include <neo_script.hpp>
#include <sstream>

std::string to_string(neo::command::param_t const& param)
{
  std::string output;
  std::visit(neo::overloaded{[&](std::monostate const&) {},
                             [&](auto const& s)
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
  neo::retcode generate(neo::state_machine const&, neo::command const& cmd)
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
    return neo::retcode::e_success;
  }

  neo::retcode print_region(neo::state_machine const& ctx,
                            neo::command const&       cmd)
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
                                           (*it).second.append_to(output);
                                       }
                                       return true;
                                     },
                                     [](auto const&) { return false; }},
                     val))
        break;
    }
    this->output += output;
    return neo::retcode::e_success;
  }

  void add_text(neo::state_machine const&, std::string_view name,
                neo::text_content&& content)
  {
    text.emplace(std::move(name), std::move(content));
  }

  std::unordered_map<std::string_view, neo::text_content> text;
  std::string                                  output;
};

TEST_CASE("Scoped command", "[region0]")
{
  neo::registry test_interpreter;
  std::string   test = "{{code:Scoped}} \n"
                     "echo \"Hello world\"; \n";
  
  auto root = test_interpreter.ensure_region_root("Scoped");
  test_interpreter.add_command(
      root, "echo",
      [](neo::command_handler* _, neo::state_machine const& ctx,
         neo::command const& cmd) noexcept -> neo::retcode
      { return static_cast<test_command_handler*>(_)->generate(ctx, cmd); });
  test_command_handler handler;
  neo::state_machine   state_machine(test_interpreter, &handler, 0);
  state_machine.parse("memory", test);
  REQUIRE(!state_machine.fail_bit());
  REQUIRE(handler.output == "echo --> (Hello world)\n");
}

TEST_CASE("Text region", "[textregion1]")
{
  neo::registry test_interpreter;
  std::string   test = "{{text:History}}\n"
                     "The world began when we perished.\n"
                     "But we left our mark to be found again.\n"
                     "In the future, we lived again in memories.\n"
                     "Of time{\n"
                     "\\{\\{unknown}}\n"
                     "{{code:Print}}\n"
                     "print (region = \"History\"); \n";
  
  auto root = test_interpreter.ensure_region_root("Print");
  test_interpreter.add_command(
      root, "print",
      [](neo::command_handler* _, neo::state_machine const& ctx,
         neo::command const& cmd) noexcept -> neo::retcode {
        return static_cast<test_command_handler*>(_)->print_region(ctx, cmd);
      });
  test_interpreter.set_text_region_handler(
      [](neo::command_handler* _, neo::state_machine const& ctx,
         std::string_view id, 
         std::string_view name,
         neo::text_content&& content) noexcept
      {
        return static_cast<test_command_handler*>(_)->add_text(
            ctx, std::move(name), std::move(content));
      });
  test_command_handler handler;
  neo::state_machine   state_machine(test_interpreter, &handler, 0);
  state_machine.parse("memory", test);
  REQUIRE(!state_machine.fail_bit());
  REQUIRE(handler.output == "\nThe world began when we perished.\n"
                            "But we left our mark to be found again.\n"
                            "In the future, we lived again in memories.\n"
                            "Of time{\n"
                            "{{unknown}}\n");
}

TEST_CASE("Text region 2", "[textregion2]")
{
  neo::registry test_interpreter;
  std::string   test =
      "{{text:textreg}}\n"
      "The world began when we perished.\n"
      "\\a/a\\a/t\\t\\.a.\n"
      "@.\n"
      "{\n"
      " bracket should appear correctly.\n"
      "}\n"
      "{{text:secondregion}}\n"
      "This region {secondregion} follows the previous \\{textreg}\n"
      "{{code:Print}}\n"
      "print (region = \"textreg\"); \n"
      "print (region = \"secondregion\"); \n";

  auto root = test_interpreter.ensure_region_root("Print");
  test_interpreter.add_command(
      root, "print",
      [](neo::command_handler* _, neo::state_machine const& ctx,
         neo::command const& cmd) noexcept -> neo::retcode {
        return static_cast<test_command_handler*>(_)->print_region(ctx, cmd);
      });
  test_interpreter.set_text_region_handler(
      [](neo::command_handler* _, neo::state_machine const& ctx,
         std::string_view id, std::string_view name, neo::text_content&& content) noexcept
      {
        return static_cast<test_command_handler*>(_)->add_text(
            ctx, std::move(name), std::move(content));
      });
  test_command_handler handler;
  neo::state_machine   state_machine(test_interpreter, &handler, 0);
  state_machine.parse("memory", test);
  REQUIRE(!state_machine.fail_bit());
  REQUIRE(handler.output ==
          "\nThe world began when we perished.\n"
          "\\a/a\\a/t\\t\\.a.\n"
          "@.\n"
          "{\n"
          " bracket should appear correctly.\n"
          "}\n\n"
          "This region {secondregion} follows the previous {textreg}\n");
}


TEST_CASE("Any command", "[region2]")
{
  neo::registry test_interpreter;
  std::string   test = "first command [is, executed];\n"
                     "second command [is, stalled];\n"
                     "{{code:Main}}\n"
                     "Third command [is, super = executed];\n"
                     "Fourth command [is, stalled] {\n"
                     "  Four point one command;\n"
                     "  Four point two command;\n"
                     "}\n"
                     "{{code:Print}}\n"
                     "print (region = \"text:History\"); \n";
  
  neo::command_hook lambda = [](neo::command_handler*     _,
                                neo::state_machine const& ctx,
                                neo::command const&       cmd) noexcept -> neo::retcode
  { return static_cast<test_command_handler*>(_)->generate(ctx, cmd); };
  auto root = test_interpreter.ensure_region_root("");
  test_interpreter.add_command(root, "*", lambda);
  root = test_interpreter.ensure_region_root("Print");
  test_interpreter.add_command(root, "*", lambda);
  root = test_interpreter.ensure_region_root("Main");
  test_interpreter.add_command(root, "*", lambda);
  root = test_interpreter.add_scoped_command(root, "Fourth", lambda);
  test_interpreter.add_command(root, "*", lambda);
  test_command_handler handler;
  neo::state_machine   state_machine(test_interpreter, &handler, 0);
  state_machine.parse("memory", test);
  REQUIRE(!state_machine.fail_bit());
  REQUIRE(handler.output == "first --> (command, [is, executed])\n"
                            "second --> (command, [is, stalled])\n"
                            "Third --> (command, [is, super = executed])\n"
                            "Fourth --> (command, [is, stalled])\n"
                            "Four --> (point, one, command)\n"
                            "Four --> (point, two, command)\n"
                            "print --> (region = text:History)\n");
}

TEST_CASE("Alias command", "[alias]")
{
  neo::registry test_interpreter;
  std::string   test = "print message;\n"
                     "scope {\n"
                     " print message2;\n"
                     " scope {\n"
                     "    print message3;\n"
                     "}};\n";

  neo::command_hook lambda =
      [](neo::command_handler* _, neo::state_machine const& ctx,
         neo::command const& cmd) noexcept -> neo::retcode
  { return static_cast<test_command_handler*>(_)->generate(ctx, cmd); };
  auto root = test_interpreter.ensure_region_root("");
  auto id = test_interpreter.add_command(root, "print", lambda);
  auto scope   = test_interpreter.add_scoped_command(root, "scope");
  test_interpreter.alias_command(scope, "print", id);
  auto scope2 = test_interpreter.add_scoped_command(scope, "scope");
  test_interpreter.alias_command("@/print", "@/scope/scope/print");
  test_command_handler handler;
  neo::state_machine   state_machine(test_interpreter, &handler, 0);
  state_machine.parse("memory", test);
  REQUIRE(!state_machine.fail_bit());
  REQUIRE(handler.output == "print --> (message)\n"
                            "print --> (message2)\n"
                            "print --> (message3)\n");
}

TEST_CASE("Block test", "[block]")
{
  neo::registry test_interpreter;
  std::string   test = "block parameter0 parameter1 {\n"
                     " command parameter2 parameter4;\n"
                     "};\n";
  
  auto root = test_interpreter.ensure_region_root("");
  test_interpreter.add_scoped_command(
      root, "*",
      [](neo::command_handler* _, neo::state_machine const& ctx,
         neo::command const& cmd) noexcept -> neo::retcode
      { return static_cast<test_command_handler*>(_)->generate(ctx, cmd); });

  test_command_handler handler;
  neo::state_machine   state_machine(test_interpreter, &handler, 0);
  state_machine.parse("memory", test);
  if (state_machine.fail_bit())
  {
    std::cerr << "[ERROR] While compiling [block]" << std::endl;
    state_machine.for_each_error(
        [](std::string const& err)
        { std::cerr << "        " << err << std::endl; });
  }
  REQUIRE(!state_machine.fail_bit());
  REQUIRE(handler.output == "block --> (parameter0, parameter1)\n"
                            "command --> (parameter2, parameter4)\n");
}

TEST_CASE("Skip test", "[skip]")
{
  neo::registry reg;
  std::string   test =
      "simple skip { inner command; inner strength; inner peace; };\n";
  
  auto root  = reg.ensure_region_root("");
  auto scope = reg.add_scoped_command(
      root, "simple",
      [](neo::command_handler* _, neo::state_machine const& ctx,
         neo::command const& cmd) noexcept -> neo::retcode
      { return neo::retcode::e_skip_block; });

  reg.add_command(scope, "inner",
                  [](neo::command_handler* _, neo::state_machine const& ctx,
                     neo::command const& cmd) noexcept -> neo::retcode
                  { return neo::retcode::e_fail_and_stop; });

  neo::state_machine state_machine(reg, nullptr, 0);
  state_machine.parse("memory", test);
  REQUIRE(!state_machine.fail_bit());
}

TEST_CASE("Skip test rest of commands", "[skip2]")
{
  neo::registry reg;
  std::string   test = "simple skip { inner command; next command; porchain "
                     "command; inner strength; inner peace; };\n";
  
  auto root  = reg.ensure_region_root("");
  auto scope = reg.add_scoped_command(
      root, "simple",
      [](neo::command_handler* _, neo::state_machine const& ctx,
         neo::command const& cmd) noexcept -> neo::retcode
      { return neo::retcode::e_success; });

  struct handler : neo::command_handler
  {
    int inner_call = 0;
  };
  handler h;
  reg.add_command(scope, "inner",
                  [](neo::command_handler* _, neo::state_machine const& ctx,
                     neo::command const& cmd) noexcept -> neo::retcode
                  {
                    static_cast<handler*>(_)->inner_call++;
                    return neo::retcode::e_skip_rest;
                  });

  reg.add_command(scope, "next",
                  [](neo::command_handler* _, neo::state_machine const& ctx,
                     neo::command const& cmd) noexcept -> neo::retcode
                  { return neo::retcode::e_fail_and_stop; });

  reg.add_command(scope, "porchain",
                  [](neo::command_handler* _, neo::state_machine const& ctx,
                     neo::command const& cmd) noexcept -> neo::retcode
                  { return neo::retcode::e_fail_and_stop; });

  neo::state_machine state_machine(reg, &h, 0);
  state_machine.parse("memory", test);
  REQUIRE(!state_machine.fail_bit());
  REQUIRE(h.inner_call == 1);
}
