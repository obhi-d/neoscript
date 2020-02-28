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
    last_cmd = cmd.name();
    return true;
  }

  bool enter_scope(neo::context const&, std::string_view name) {
    scope += last_cmd;
    scope += '/';
    return true;
  }

  bool leave_scope(neo::context const&, std::string_view name) {
    scope.pop_back();
    std::size_t pos = scope.find_last_of('/');
    if (pos == std::string::npos)
      pos = 0;
    else
      pos++;
    scope.erase(pos, scope.length() - pos);
    return true;
  }

  std::ofstream file;
  std::string   scope;
  std::string   last_cmd;
};

bool compare_expected(std::string const& name) {
  std::ifstream f1("../output/" + name);
  std::ifstream f2("../expected/" + name);

  if (f1.fail() || f2.fail()) {
    return false; // file problem
  }
  std::string f1_str((std::istreambuf_iterator<char>(f1)),
                  std::istreambuf_iterator<char>());
  std::string f2_str((std::istreambuf_iterator<char>(f2)),
                     std::istreambuf_iterator<char>());

  return f1_str == f2_str;
}

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
    context.set_import_handler([](std::string const& name) {
      return std::make_shared<std::ifstream>("../dataset/" + name);
    });
    std::shared_ptr<std::istream> iss = std::make_shared<std::ifstream>(path);
    context.parse(path.filename().generic_string(), iss);
    if (context.fail_bit()) {
      std::cerr << "[ERROR] Compiling file " << path.filename().generic_string()
                << std::endl;
      context.for_each_error([](std::string const& err) {
        std::cerr << "        " << err << std::endl;
      });
    }
    std::cout << "[INFO] " << path.filename().generic_string() << std::endl;
    CHECK(!context.fail_bit());
    CHECK(compare_expected(path.filename().generic_string()));
  }
}
