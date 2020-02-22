
#include <neo_interpreter.hpp>

namespace neo {
interpreter_id get_interpreter(std::string_view);
command_id     add_command_handler(interpreter_id interpreter,
                                   command_id parent_scope, std::string_view cmd,
                                   command_handler_t callback);
command_id     add_root_command(interpreter_id   interpreter,
                                std::string_view region_scope);
interpreter*   get_interpreter(interpreter_id);

}