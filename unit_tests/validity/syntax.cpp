#include <catch2/catch.hpp>
#include <istream>
#include <neo_script.hpp>
#include <sstream>

struct test_command_handler : public neo::command_handler {

  static std::string to_string(neo::command::param_t const& param) {
    std::string output;
    std::visit(neo::overloaded{[&](auto const&) {},
                               [&](neo::command::single const& s) {
                                 if (s.name().length() > 0) {
                                   output += s.name();
                                   output += " = ";
                                 }
                                 output += s.value();
                               },
                               [&](neo::command::list const& s) {
                                 if (s.name().length() > 0) {
                                   output += s.name();
                                   output += " = ";
                                 }
                                 output += " [ ";

                                 bool first = true;

                                 for (auto const& b : s) {
                                   if (!first)
                                     output += ", ";
                                   first = false;
                                   output += to_string(b);
                                 }

                                 output += " ] ";
                               }},
               param);
    return output;
  }

  bool generate(neo::context const&, neo::command const& cmd) {
    output += cmd.name();
    output += " --> (";
    auto const& params = cmd.params().value();
    bool        first  = true;
    for (auto const& val : params) {
      if (!first)
        output += ", ";
      first = false;
      output += to_string(val);
    }
    output += ")\n";
    return true;
  }

  std::string output;
};

TEST_CASE("Region test", "[region]") {

  neo::interpreter test_interpreter;
  std::string      test = " {{code:Scoped}} \n"
                     " echo \"Hello world\"; \n";
  std::shared_ptr<std::istream> iss =
      std::make_shared<std::istringstream>(test);

  auto root = test_interpreter.ensure_region_root("Scoped");
  test_interpreter.add_command(root, "echo", &test_command_handler::generate);
  test_command_handler handler;
  neo::context         context(test_interpreter, handler);
  context.parse("memory", iss);
  REQUIRE(handler.output == "echo --> (Hello world)\n");
}