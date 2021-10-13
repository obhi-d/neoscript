%language "C++"
%skeleton "lalr1.cc"

%defines
%define api.parser.class {parser_impl}
%define api.namespace {neo}
%define api.prefix {neo_}
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define parse.trace
%define parse.error verbose

%code requires
{
#include <neo_state_machine.hpp>

namespace neo {
  class location;
  class state_machine;
}
#ifndef YY_NULLPTR
#  define YY_NULLPTR nullptr
#endif
#define YY_DECL extern neo::parser_impl::symbol_type neo_lex(neo::state_machine& _, void* yyscanner)

}

%define api.location.type {neo::location}
%param { state_machine& _ }
%lex-param { void* SCANNER_PARAM  }
%locations
%initial-action
{
  @$.source_name = _.get_file_name();
}

%code
{
#include "neo_state_machine.hpp"
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
	USING      "using"
	IMPORT	   "import"
	;

%token <std::string> REGION_ID TEXT_REGION_ID TEXT_CONTENTS IDENTIFIER STRING_LITERAL

// Types
%type <std::string> commandname
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
script: statement           
		| statement script      

statement:                                   
		  SEMICOLON                              {                                                                      }
		| commanddecl                            { if (!_.skip()) { if(!_.consume(std::move($1))) { YYACCEPT; }   }     }
		| templatedecl                           { if (!_.skip()) _.add_template(std::move($1));                        }
		| instancedecl                           { if (!_.skip()) { if(!_.consume(std::move($1))) { YYACCEPT; }   }     }
		| RBRACKET                               { if (!_.skip()) _.end_block(); else _.exit_skip_scope();              }
		| REGION_ID                              { _.start_region(std::move($1));                                       }
		| TEXT_REGION_ID TEXT_CONTENTS           { _.start_region(std::move($1), std::move($2));                        }
		| IMPORT STRING_LITERAL SEMICOLON        { _.import_script(std::move($2));                                      }


template_args.0.N:	/* empty string */		
					| IDENTIFIER	{ 
						if (!_.skip())
						{
							std::vector<std::string> args; 
							args.push_back($1); 
							$$ = std::move(args); 
						}
					}
					| template_args.0.N COMMA IDENTIFIER  { 
						if (!_.skip())
						{
							$$ = std::move($1);
							$$.push_back($3); 
						}
					}
					;

templatedecl: TEMPLATE LABRACKET template_args.0.N RABRACKET commanddecl {
						if (!_.skip())
								$$ = std::move(_.make_command_template(
											std::move($3), std::move($5)));
						 }
			   | TEMPLATE IDENTIFIER LABRACKET template_args.0.N RABRACKET commanddecl {
				   		if (!_.skip())
				 				$$ = std::move(_.make_command_template(std::move($2),
							        std::move($4), std::move($6)));
				     }
				;

commandname:  IDENTIFIER { $$ = std::move($1); }
			| IDENTIFIER ASSIGN { $$ = std::move($1); }

commanddecl: 
		   commandname parameters.0.N SEMICOLON      { if (!_.skip()) $$ = std::move(_.make_command(std::move($1), std::move($2))); }
		 | commandname parameters.0.N LBRACKET       { if (!_.skip()) $$ = std::move(_.make_command(std::move($1), std::move($2), true)); else _.enter_skip_scope(); }
		 
instancedecl: USING IDENTIFIER LABRACKET list.0.N RABRACKET SEMICOLON { if (!_.skip()) $$ = std::move(_.make_instance(std::move($2), std::move($4))); }
		 | USING IDENTIFIER LABRACKET list.0.N RABRACKET LBRACKET         { if (!_.skip()) $$ = std::move(_.make_instance(std::move($2), std::move($4), true)); }

parameters.0.N:   /* empty string */                    
				  | parameters.0.N parameter  { if (!_.skip()) { $1.append(std::move($2)); $$ = std::move($1); } }
				  | parameters.0.N LBRACES special_parameters.0.N RBRACES { 
							if (!_.skip()) { $1.append_expanded(std::move($3)); $$ = std::move($1);  }
						}
				  ;

special_parameters.0.N:     /* empty string */       
			| special_parameter       { 
				if (!_.skip())
				{
					command::list l; 
					l.emplace_back(std::move($1)); 
					$$ = std::move(l); 
				}
			}
			| special_parameters.0.N COMMA special_parameter 
			{ 
				if (!_.skip())
				{
					command::list list;
					if ($1.index() == 1) 
						list.emplace_back(std::move($1));
					else if ($1.index() == 2) 
						list = std::get<command::list>(std::move($1));

					list.emplace_back(std::move($3));
					$$ = std::move(list);
				}
			}
			;

special_parameter: IDENTIFIER  ASSIGN  parameter 
			{ 
				if (!_.skip())
				{
					neo::command::parameters::set_name($3, std::move($1)); 
					$$ = std::move($3);	
				}
			}
			;

parameter: STRING_LITERAL						      { if (!_.skip()) $$ = command::single(std::move($1)); }
			| IDENTIFIER                        { if (!_.skip()) $$ = command::single(std::move($1)); }
			| LSQBRACKET list.0.N RSQBRACKET    { if (!_.skip()) $$ = std::move($2); }
			;
			
list: parameter					  { if (!_.skip()) $$ = std::move($1); }
	| special_parameter			{ if (!_.skip()) $$ = std::move($1); }
    ;
	
list.0.N:	/* empty string */							
		|  list                         { 
			if (!_.skip())
			{
				command::list list; 
				list.emplace_back(std::move($1)); 
				$$ = std::move(list);
			}
		}
	  |  list.0.N COMMA list          { 
		  if (!_.skip()) {
				command::list list; 
				if ($1.index() == 1) 
					list.emplace_back(std::move($1));
				else if ($1.index() == 2) 
					list = std::get<command::list>(std::move($1));
				list.emplace_back(std::move($3));
				$$ = std::move(list);
		  }
	  }
	  ;


%%
/*============================================================================*/

namespace neo 
{

void parser_impl::error(location_type const& l,
												std::string const & e) 
{
  _.push_error(l, e.c_str());
}

void state_machine::parse(std::string_view src_name, std::shared_ptr<std::istream> const& ifile) 
{
	auto restore_file = current_file_;
	current_file_ = ifile;
	auto restore_source_name = source_name_;
	source_name_ = src_name;
	auto restore_loc = loc_;
	loc_ = location_type();
	loc_.source_name = &source_name_;
	void* restore_scanner = scanner;
	scanner = nullptr;
	start_region("");
	begin_scan();
	parser_impl parser(*this);
	parser.set_debug_level(flags_ & f_trace_parse);
	int res = parser.parse();
	end_scan();
	current_file_ = restore_file;
	source_name_ = restore_source_name;
	loc_ = restore_loc;
	scanner = restore_scanner;
}

}
