// No pragma once here
// This file can be included multiple times, and only once when NEO_HEADER_ONLY_IMPL is defined will it include all source files and undef afterwards.

#include <neo_common.hpp>
#include <neo_command.hpp>
#include <neo_registry.hpp>
#include <neo_state_machine.hpp>

#ifdef NEO_HEADER_ONLY_IMPL
#include <detail/neo_command_instance.cpp>
#include <detail/neo_command_template.cpp>
#include <detail/neo_state_machine.cpp>
#include <detail/lex.neo.cpp>
#include <detail/parse.neo.cpp>
#undef NEO_HEADER_ONLY_IMPL
#endif

