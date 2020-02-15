%language "C++"
%skeleton "lalr1.cc"
%name-prefix "neo_"

%defines
%define parser_class_name {parser_impl}
%define api.namespace {neo}
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define parse.trace
%define parse.error verbose

%code requires
{
namespace neo {
  class location;
  class context;
}
#ifndef YY_NULLPTR
#  define YY_NULLPTR nullptr
#endif
#define YY_DECL extern neo::parser_impl::symbol_type neo_lex(neo::context& _, void* yyscanner)

}

%define api.location.type {neo::location}
%param { context& _ }
%lex-param { void* SCANNER_PARAM  }
%locations
%initial-action
{
  @$.source_name = _.get_file_name();
}

%code
{
#include "neo_context.hpp"
#define SCANNER_PARAM _.scanner
YY_DECL;
}

%token 
	END 0 "end of file"
	SEMICOLON  ";"
	LBRACKET   "{"
	RBRACKET   "}"
	LSQBRACKET "["
	RSQBRACKET "]"
	LABRACKET  "<"
	RABRACKET  ">"
	COMMA      ","
	LBRACES    "("
	RBRACES    ")"
	ASSIGN     "="
	TEMPLATE   "template"
	INVOKE     "invoke"
	IMPORT	   "import"
	;

%token <std::string> REGION_ID TEXT_REGION_ID TEXT_CONTENTS IDENTIFIER STRING_LITERAL

// Types
%type <script> script
%type <command> commanddecl
%type <command_instance> instancedecl
%type <command::parameters> parameters.0.N 
%type <command::param_t> special_parameter special_parameters.0.N  parameter list.0.N list
%type <std::vector<std::string>> template_args.0.N
%type <command_template> templatedecl
%printer { yyoutput << $$; } <std::string>

%start script

%%
/*============================================================================*/
script:                                      {                                                         }
		| REGION_ID script                       { _.start_region(std::move($1));                          }
		| commanddecl script                     { _.consume(std::move($1));                               }
		| templatedecl script                    { _.consume(std::move($1));                               }
		| instancedecl script                    { _.consume(std::move($1));                               }
		| RBRACKET script                        { _.end_block();                                          }
		| TEXT_REGION_ID TEXT_CONTENTS script    { _.start_region(std::move($1), std::move($2));           }
		| IMPORT IDENTIFIER SEMICOLON script     { _.import_script(std::move($1));                                }

template_args.0.N:			{}
					| IDENTIFIER	{ 
						std::vector<std::string> args; 
						args.push_back($1); 
						$$ = std::move(args); 
					}
					| template_args.0.N COMMA IDENTIFIER  { 
						$$ = std::move($1);
						$$.push_back($3); 
					}
					;

templatedecl: TEMPLATE LABRACKET template_args.0.N RABRACKET commanddecl {
								$$ = _.make_command_template(
											std::move($3), std::move($5));
						 }
			   | TEMPLATE IDENTIFIER LABRACKET template_args.0.N RABRACKET commanddecl {
				 				$$ = _.make_command_template(std::move($2),
							        std::move($3), std::move($5));
				     }
				;

commanddecl: SEMICOLON                          {  }
		 | IDENTIFIER parameters.0.N SEMICOLON      { $$ = _.make_command(std::move($1), std::move($2)); }
		 | IDENTIFIER parameters.0.N LBRACKET       { $$ = _.make_command(std::move($1), std::move($2), true); }
		 
instancedecl: INSTANCE IDENTIFIER LABRACKET list.0.N RABRACKET SEMICOLON { $$ = _.make_instance(std::move($2), std::move($4)); }
		 | INSTANCE IDENTIFIER LABRACKET list.0.N RABRACKET LBRACKET         { $$ = _.make_instance(std::move($2), std::move($4), true); }

parameters.0.N:                       { }
				  | parameters.0.N parameter  { $1.append(std::move($2)); $$ = std::move($1); }
				  | parameters.0.N LBRACES special_parameters.0.N RBRACES { 
							$1.append_expanded(std::move($3)); $$ = std::move($1); 
						}
				  ;

special_parameters.0.N:            {  }
			| special_parameter       { 
					command::list l; 
					l.emplace_back(std::move($1)); 
					$$ = std::move(l); 
				}
			| special_parameters.0.N COMMA special_parameter 
			{ 
				command::list list;
				if ($1.index() == 1) 
					list.emplace_back(std::move($1));
				else if ($1.index() == 2) 
					list = std::get<command::list>(std::move($1)));

				list.emplace_back(std::move($3));
				$$ = std::move(list);
			}
			;

special_parameter: IDENTIFIER  ASSIGN  parameter 
			{ 
				$3->set_name($1); 
				$$ = std::move($3);	
			}
			;

parameter: STRING_LITERAL						      { $$ = command::single(std::move($1)); }
			| IDENTIFIER                        { $$ = command::single(std::move($1)); }
			| LSQBRACKET list.0.N RSQBRACKET    { $$ = std::move($2); }
			;
			
list: parameter					  { $$ = std::move($1); }
	| special_parameter			{ $$ = std::move($1); }
    ;
	
list.0.N:								{ }
		|  list                         { 
				command::list list; 
				list.emplace_back(std::move($1)); 
				$$ = std::move(list);
			}
	  |  list.0.N COMMA list          { 
				command::list list; 
				if ($1.index() == 1) 
					list.emplace_back(std::move($1));
				else if ($1.index() == 2) 
					list = std::get<command::list>(std::move($1)));
				list.emplace_back(std::move($3));
				$$ = std::move(list);
		  }
	  ;


%%
/*============================================================================*/

namespace neo {

void parser_impl::error(location_type const& l,
												std::string const & e) {
  _.push_error(l, e.c_str());
}

void context::parse() {
	begin_scan();
	parser_impl parser(*this);
	parser.set_debug_level(trace_parsing);
	int res = parser.parse();
	end_scan();
}

}
