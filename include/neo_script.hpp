#pragma once

#include <neo_command.hpp>
#include <neo_registry.hpp>
#include <neo_state_machine.hpp>

#ifdef NEO_HEADER_ONLY_IMPL
#include <detail/neo_command_instance.cpp>
#include <detail/neo_command_template.cpp>
#include <detail/neo_state_machine.cpp>
#include <detail/lex.neo.cpp>
#include <detail/parse.neo.cpp>
#endif

