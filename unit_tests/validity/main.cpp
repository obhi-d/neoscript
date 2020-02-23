#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <filesystem>
#include <fstream>
#include <istream>
#include <neo_script.hpp>
#include <sstream>

std::string to_string(neo::command::param_t const& param);

struct fileout_command_handler : public neo::command_handler {

  fileout_command_handler(std::string const& name) : file("../output/" + name) {
    assert(file.is_open());
  }

  bool print(neo::context const& ctx, neo::command const& cmd) {
    file << scope << cmd.name() << " --> (";
    auto const& params = cmd.params().value();
    bool        first  = true;
    for (auto const& val : params) {
      if (!first)
        file << ", ";
      first = false;
      file << to_string(val);
    }
    file << ")\n";
    return true;
  }

  bool enter_scope(neo::context const&, std::string_view name) {
    scope += '/';
    scope += name;
    return true;
  }

  bool leave_scope(neo::context const&, std::string_view name) {
    std::size_t pos = scope.find_last_of('/');
    assert(pos != std::string::npos);
    assert(std::string_view(scope).substr(pos + 1) == name);
    scope.erase(pos, scope.length() - pos);
    return true;
  }

  std::ofstream file;
  std::string   scope;
};

TEST_CASE("Validate syntax files", "[file]") {
  namespace fs = std::filesystem;

  neo::interpreter test_interpreter;
  std::uint32_t    root = test_interpreter.ensure_region_root("");
  test_interpreter.add_scoped_command(root, "*",
                                      &fileout_command_handler::print,
                                      &fileout_command_handler::enter_scope,
                                      &fileout_command_handler::leave_scope);

  for (auto& p : fs::directory_iterator("../dataset")) {
    auto                          path = p.path();
    fileout_command_handler       handler(path.filename().generic_string());
    neo::context                  context(test_interpreter, handler, 0);
    std::shared_ptr<std::istream> iss = std::make_shared<std::ifstream>(path);
    context.parse(path.filename().generic_string(), iss);
    if (context.fail_bit()) {
      std::cerr << "[ERROR] Compiling file " << path.filename().generic_string()
                << std::endl;
      context.for_each_error([](std::string const& err) {
        std::cerr << "        " << err << std::endl;
      });
    }
    REQUIRE(!context.fail_bit());
  }
}
