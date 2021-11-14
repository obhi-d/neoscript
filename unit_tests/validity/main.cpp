#define CATCH_CONFIG_MAIN
#define NEO_IMPLEMENT
#include <catch2/catch.hpp>
#include <filesystem>
#include <fstream>
#include <istream>
#include <neo_script.hpp>
#include <sstream>

std::string to_string(neo::command::param_t const& param);

struct fileout_command_handler : public neo::command_handler
{
  fileout_command_handler(std::string const& name) : file("../output/" + name)
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
                                        neo::command const&       cmd) noexcept
  {
    auto result = static_cast<fileout_command_handler*>(_)->print_m(ctx, cmd);
    if (result == neo::retcode::e_success && cmd.is_scoped())
      return static_cast<fileout_command_handler*>(_)->enter_scope_m(
          ctx, cmd.name());
    else
      return result;
  }

  static void leave_scope(neo::command_handler*     _,
                          neo::state_machine const& ctx,
                          std::string_view          name) noexcept
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

std::tuple<std::string, std::string> compare_expected(std::string const& name)
{
  std::ifstream f1("../output/" + name);
  std::ifstream f2("../expected/" + name);

  if (f1.fail() || f2.fail())
  {
    return {"file issue", "error in open"}; // file problem
  }
  std::string f1_str((std::istreambuf_iterator<char>(f1)),
                     std::istreambuf_iterator<char>());
  std::string f2_str((std::istreambuf_iterator<char>(f2)),
                     std::istreambuf_iterator<char>());

  return {f1_str, f2_str};
}

std::string read_file(std::filesystem::path path)
{
  std::ifstream t{path};
  t.seekg(0, std::ios::end);
  size_t      size = t.tellg();
  std::string buffer(size, '\0');
  t.seekg(0);
  t.read(&buffer[0], size);
  return buffer;
}

TEST_CASE("Validate syntax files", "[file]")
{
  namespace fs = std::filesystem;

  neo::registry test_interpreter;
  std::uint32_t root = test_interpreter.ensure_region_root("");
  test_interpreter.add_scoped_command(
      root, "*", &fileout_command_handler::print_enter_scope,
      &fileout_command_handler::leave_scope);

  std::unordered_map<std::string_view, std::string> sources;
  for (auto& p : fs::directory_iterator("../dataset"))
  {
    auto path = p.path();
    {
      fileout_command_handler handler(path.filename().generic_string());
      neo::state_machine      state_machine(test_interpreter, &handler, 0);
      state_machine.set_import_handler(
          [&sources](std::string_view name) noexcept -> std::string_view
          {
            std::string base{"../dataset/"};
            base += name;
            std::filesystem::path p{base};
            auto                  buffer = read_file(p);
            auto                  r = sources.emplace(name, std::move(buffer));
            return r.first->second;
          });
      auto buffer = read_file(path);
      state_machine.parse(path.filename().generic_string(), buffer);
      if (state_machine.fail_bit())
      {
        std::cerr << "[ERROR] Compiling file "
                  << path.filename().generic_string() << std::endl;
        state_machine.for_each_error(
            [](std::string const& err)
            { std::cerr << "        " << err << std::endl; });
      }
      std::cout << "[INFO] " << path.filename().generic_string() << std::endl;
      CHECK(!state_machine.fail_bit());
    }
    auto [first, second] = compare_expected(path.filename().generic_string());
    CHECK(first == second);
  }
}