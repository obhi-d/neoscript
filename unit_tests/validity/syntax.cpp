#include <catch2/catch_test_macros.hpp>
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

TEST_CASE("Allow dash in command name", "[region0]")
{
  neo::registry test_interpreter;
  std::string   test = "--code:Scoped-- \n"
                     "echo-com \"Hello world\" --How -are --you; \n";

  auto root = test_interpreter.ensure_region_root("Scoped");
  test_interpreter.add_command(
      root, "echo-com",
      [](neo::command_handler* _, neo::state_machine const& ctx,
         neo::command const& cmd) noexcept -> neo::retcode
      { return static_cast<test_command_handler*>(_)->generate(ctx, cmd); });
  test_command_handler handler;
  neo::state_machine   state_machine(test_interpreter, &handler, 0);
  state_machine.parse("memory", test);
  REQUIRE(!state_machine.fail_bit());
  REQUIRE(handler.output == "echo-com --> (Hello world, --How, -are, --you)\n");
}

TEST_CASE("Unnamed Template", "[region0]")
{
  neo::registry test_interpreter;
  std::string   test = "$[A, B]\necho A B; ^echo[Hello, World];\n";

  test_interpreter.add_command(
      test_interpreter.root, "echo",
      [](neo::command_handler* _, neo::state_machine const& ctx,
         neo::command const& cmd) noexcept -> neo::retcode
      { return static_cast<test_command_handler*>(_)->generate(ctx, cmd); });
  test_command_handler handler;
  neo::state_machine   state_machine(test_interpreter, &handler, 0);
  state_machine.parse("memory", test);
  REQUIRE(!state_machine.fail_bit());
  REQUIRE(handler.output == "echo --> (Hello, World)\n");
}

TEST_CASE("Code region with command", "[region0]")
{
  neo::registry test_interpreter;
  std::string   test = "--code:Scoped-- \n"
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

TEST_CASE("Code region with command and spaces", "[region0]")
{
  neo::registry test_interpreter;
  std::string   test = "--code:Scoped-- \n"
                     "  echo \"Hello world\"; \n";
  
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

TEST_CASE("Scoped command with spaces", "[region0]")
{
  neo::registry test_interpreter;
  std::string   test =
                     "scope \"Nice\" { \n"
                     "\t echo \"Hello world\"; } \n";
  

  auto scope = test_interpreter.add_scoped_command(test_interpreter.root, "scope",
                                      [](neo::command_handler* _, neo::state_machine const& ctx,
                                         neo::command const& cmd) noexcept -> neo::retcode
                                      { return static_cast<test_command_handler*>(_)->generate(ctx, cmd); });
  test_interpreter.add_command(
      scope, "echo",
      [](neo::command_handler* _, neo::state_machine const& ctx,
         neo::command const& cmd) noexcept -> neo::retcode
      { return static_cast<test_command_handler*>(_)->generate(ctx, cmd); });
  test_command_handler handler;
  neo::state_machine   state_machine(test_interpreter, &handler, 0);
  state_machine.parse("memory", test);
  REQUIRE(!state_machine.fail_bit());
  REQUIRE(handler.output == "scope --> (Nice)\necho --> (Hello world)\n");
}

TEST_CASE("Text region", "[textregion1]")
{
  neo::registry test_interpreter;
  std::string   test = "--text:History --\n"
                     "The world began when we perished.\n"
                     "But we left our mark to be found again.\n"
                     "In the future, we lived again in memories.\n"
                     "Of time{\n"
                     "{{unknown}}\n"
                     "--code: Print --\n"
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
      "-- text:textreg --\n"
      "The world began when we perished.\n"
      "\\a/a\\a/t\\t\\.a.\n"
      "@.\n"
      "{\n"
      " bracket should appear correctly.\n"
      "}\n"
      "-- text:secondregion--\n"
      "This region {secondregion} follows the previous {textreg}\n"
      "-- code:Print\n"
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
                     "-- code:Main --\n"
                     "Third command [is, super = executed];\n"
                     "Fourth command [is, stalled] {\n"
                     "  Four point one command;\n"
                     "  Four point two command;\n"
                     "}\n"
                     "-- code:Print --\n"
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
  std::string_view   test = "print message;\n"
                     "scope {\n"
                     " print message2;\n"
                     " scope {\n"
                     "    print1 message3;\n"
                     "    print2 message4;\n"
                     "    print3 message5;\n"
                     " };\n"
                     " scope1 {\n"
                     "  scope2 {\n"
                     "    print1 message6;\n"
                     "    print2 message7;\n"
                     "    print3 message8;\n"
                     "  };\n"
                     " };\n"
                     "};\n";

  neo::command_hook lambda =
      [](neo::command_handler* _, neo::state_machine const& ctx,
         neo::command const& cmd) noexcept -> neo::retcode
  { return static_cast<test_command_handler*>(_)->generate(ctx, cmd); };
  auto root = test_interpreter.ensure_region_root("");
  auto id = test_interpreter.add_command(root, "print", lambda);
  auto scope   = test_interpreter.add_scoped_command(root, "scope");
  test_interpreter.alias_command(scope, "print", id);
  auto scope2 = test_interpreter.add_scoped_command(scope, "scope");
  test_interpreter.alias_command("@/print", "@/scope/scope/print1");
  test_interpreter.alias_command("@/print", "@/scope/scope/print2");
  test_interpreter.alias_command("@/print", "@/scope/scope/print3");
  auto scope3 = test_interpreter.add_scoped_command(scope, "scope1");
  auto scope4 = test_interpreter.add_scoped_handler_alias(scope3, "scope2", scope2);
  test_command_handler handler;
  neo::state_machine   state_machine(test_interpreter, &handler, 0);
  state_machine.parse("memory", test);
  REQUIRE(!state_machine.fail_bit());
  REQUIRE(handler.output == "print --> (message)\n"
                            "print --> (message2)\n"
                            "print1 --> (message3)\n"
                            "print2 --> (message4)\n"
                            "print3 --> (message5)\n"
                            "print1 --> (message6)\n"
                            "print2 --> (message7)\n"
                            "print3 --> (message8)\n"
                            );
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

TEST_CASE("Skip test end of commands", "[skip3]")
{
  neo::registry reg;
  std::string   test = "simple skip \n"
                     "{ \n"
                     "  allow next; \n"
                     "  inner command { accept Print; } \n"
                     "  inner command {} \n"
                     "  inner {} \n"
                     "  command next; \n"
                     "  inner { echo strength; echo peace; } \n"
                     "}\n"
                     "simple {}\n"
                     "simple {\n"
                     "  inner command { accept Print; } \n"
                     "}\n";


  auto root  = reg.ensure_region_root("");
  
  struct handler : neo::command_handler
  {
    int inner_call = 0;
    int simple_call = 0;
    int accept_call = 0;
  };
  handler h;
  auto    scope = reg.add_scoped_command(
      root, "simple",
      [](neo::command_handler* _, neo::state_machine const& ctx,
         neo::command const& cmd) noexcept -> neo::retcode
      {
        static_cast<handler*>(_)->simple_call++;
        return neo::retcode::e_success;
      });

  auto scope2 = reg.add_scoped_command(scope, "inner",
                  [](neo::command_handler* _, neo::state_machine const& ctx,
                     neo::command const& cmd) noexcept -> neo::retcode
                  {
                    static_cast<handler*>(_)->inner_call++;
                    return neo::retcode::e_success;
                  },
      [](neo::command_handler* obj, neo::state_machine const&,
         std::string_view cmd_name) noexcept -> neo::retcode
      {
        static_cast<handler*>(obj)->inner_call++;
        return neo::retcode::e_skip_rest;
      });
  
  reg.add_command(scope, "allow",
                  [](neo::command_handler* _, neo::state_machine const& ctx,
                     neo::command const& cmd) noexcept -> neo::retcode
                  { return neo::retcode::e_success; });
  
  reg.add_command(scope2, "accept",
                  [](neo::command_handler* _, neo::state_machine const& ctx,
                     neo::command const& cmd) noexcept -> neo::retcode
                  {
                    static_cast<handler*>(_)->accept_call++;
                    return neo::retcode::e_success;
                  });

  reg.add_command(scope2, "echo",
                  [](neo::command_handler* _, neo::state_machine const& ctx,
                     neo::command const& cmd) noexcept -> neo::retcode
                  {
                    std::cout << "Failed!";
                    return neo::retcode::e_fail_and_stop;
                  });

  reg.add_command(scope, "command",
                  [](neo::command_handler* _, neo::state_machine const& ctx,
                     neo::command const& cmd) noexcept -> neo::retcode
                  {
                    std::cout << "Failed!";
                    return neo::retcode::e_fail_and_stop;
                  });

  neo::state_machine state_machine(reg, &h, 0);
  state_machine.parse("memory", test);
  REQUIRE(!state_machine.fail_bit());
  REQUIRE(h.inner_call == 4);
  REQUIRE(h.accept_call == 2);
  REQUIRE(h.simple_call == 3);
}

TEST_CASE("Stop test end of commands", "[stop]")
{
  neo::registry reg;
  std::string   test = "simple skip \n"
                     "{ \n"
                     "  allow next; \n"
                     "  inner command { accept Print; } \n"
                     "  inner command {} \n"
                     "  inner {} \n"
                     "  command next; \n"
                     "  inner { echo strength; echo peace; } \n"
                     "}\n"
                     "simple {}\n"
                     "simple {}\n";

  auto root = reg.ensure_region_root("");

  struct handler : neo::command_handler
  {
    int inner_call  = 0;
    int simple_call = 0;
    int accept_call = 0;
  };
  handler h;
  auto    scope = reg.add_scoped_command(
      root, "simple",
      [](neo::command_handler* _, neo::state_machine const& ctx,
         neo::command const& cmd) noexcept -> neo::retcode
      {
        static_cast<handler*>(_)->simple_call++;
        return neo::retcode::e_success;
      });

  auto scope2 = reg.add_scoped_command(
      scope, "inner",
      [](neo::command_handler* _, neo::state_machine const& ctx,
         neo::command const& cmd) noexcept -> neo::retcode
      {
        static_cast<handler*>(_)->inner_call++;
        return neo::retcode::e_success;
      },
      [](neo::command_handler* obj, neo::state_machine const&,
         std::string_view      cmd_name) noexcept -> neo::retcode
      {
        static_cast<handler*>(obj)->inner_call++;
        return neo::retcode::e_success_stop;
      });

  reg.add_command(scope, "allow",
                  [](neo::command_handler* _, neo::state_machine const& ctx,
                     neo::command const& cmd) noexcept -> neo::retcode
                  { return neo::retcode::e_success; });

  reg.add_command(scope2, "accept",
                  [](neo::command_handler* _, neo::state_machine const& ctx,
                     neo::command const& cmd) noexcept -> neo::retcode
                  {
                    static_cast<handler*>(_)->accept_call++;
                    return neo::retcode::e_success;
                  });

  reg.add_command(scope2, "echo",
                  [](neo::command_handler* _, neo::state_machine const& ctx,
                     neo::command const& cmd) noexcept -> neo::retcode
                  {
                    std::cout << "Failed!";
                    return neo::retcode::e_fail_and_stop;
                  });

  reg.add_command(scope, "command",
                  [](neo::command_handler* _, neo::state_machine const& ctx,
                     neo::command const& cmd) noexcept -> neo::retcode
                  {
                    std::cout << "Failed!";
                    return neo::retcode::e_fail_and_stop;
                  });

  neo::state_machine state_machine(reg, &h, 0);
  state_machine.parse("memory", test);
  REQUIRE(!state_machine.fail_bit());
  REQUIRE(h.inner_call == 2);
  REQUIRE(h.accept_call == 1);
  REQUIRE(h.simple_call == 1);
}
// Macro compilation test
struct null_h : neo::command_handler
{
};

neo_cmd_handler(example1, null_h, obj, state, cmd)
{
  return neo::retcode::e_success;
}

neo_cmdend_handler(example1, null_h, obj, state, cmd) 
{
  return neo::retcode::e_success;
}

neo_text_handler(example2, null_h, obj, state, type, name, ctx) {}

neo_star_handler(example1, null_h, o, s, c) 
{ return neo::retcode::e_success; }

neo_registry(test) 
{ 
  neo::command_id save;
  neo_handle_text(example2);
  neo_cmd(example1);
  neo_scope_def(example1)
  {
    neo_cmd(example1);
    neo_alias("@/example1/example1", "@/example1/example1/example1");
    neo_aliasid(parent_cmd_id, "example2", current_cmd_id);
  }
  neo_scope_auto(example1)
  { 
    neo_save_scope(save);
    neo_cmd(example1);
    neo_alias("@/example1/example1", "@/example1/example1/example1");
    neo_aliasid(parent_cmd_id, "example2", current_cmd_id);
  }
  neo_subalias_def(example1, save);
  neo_subalias_auto(example1, save);
  neo_subalias_cust(example1, example1, save);

  neo_scope_cust(example1, example1)
  {
    neo_cmd(example1);
    neo_alias("@/example1/example1", "@/example1/example1/example1");
    neo_aliasid(parent_cmd_id, "example2", current_cmd_id);
  }
}


