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
#include <NeoBaseDecl.h>
#include <location.h>
#include <ASTValue.h>
#include <ASTDocument.h>
#include <ASTCommand.h>
#include <ASTTextRegion.h>
#include <ASTVisitor.h>
#include <ASTBlock.h>
#include <ASTBlockRegion.h>
#include <ASTTemplate.h>
#include <ASTInstance.h>
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
  @$.begin.source_name = @$.end.source_name = &_.get_file_name();
}

%code
{
#include "context.h"
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
	INSTANCE   "instance"
	IMPORT	   "import"
	;

%token <std::string> REGION_ID TEXT_REGION_ID TEXT_CONTENTS IDENTIFIER STRING_LITERAL

// Types
%type <ASTDocumentPtr> script  named_regions.1.N
%type <ASTNodePtr> import named_region unnamed_region command instance
%type <ASTNodeList> commands.0.N commands.1.N top.commands.0.N top.commands.1.N
%type <ASTParameter> parameters.0.N 
%type <ASTValuePtr> special_parameter special_parameters.0.N  parameter list.0.N list
%type <std::vector<std::string>> template_args.0.N
%type <ASTTemplatePtr> templatedecl
%printer { yyoutput << $$; } <std::string>

%start script

%%
/*============================================================================*/
script:                                  { $$.reset(); }
		| named_regions.1.N                { $$ = $1; }
		| unnamed_region                   { $$ = _.MakeDocument($1); }
		| unnamed_region named_regions.1.N { $2->PushFront($1); $$ = $2; }
		;

named_regions.1.N: named_region             { $$ = _.MakeDocument($1); }
					| named_regions.1.N named_region { $1->PushBack($2); $$ = $1; }
					;

unnamed_region: top.commands.1.N          { $$ = _.MakeBlockRegion("", std::move($1)); }
				;

named_region: REGION_ID top.commands.0.N  { $$ = _.MakeBlockRegion($1, std::move($2)); }
				| TEXT_REGION_ID TEXT_CONTENTS  { $$ = _.MakeTextRegion($1, std::move($2)); }
				;

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

templatedecl: TEMPLATE LABRACKET template_args.0.N RABRACKET command {
								$$ = std::static_pointer_cast<ASTTemplate>(
									_.MakeTemplate($5->GetValue(), $3, 
										std::static_pointer_cast<ASTCommand>($5)));
							}
			   | TEMPLATE IDENTIFIER LABRACKET template_args.0.N RABRACKET command {									$$ = std::static_pointer_cast<ASTTemplate>(
									_.MakeTemplate($2, $4, 
										std::static_pointer_cast<ASTCommand>($6)));
							}
				;

top.commands.0.N:                         {  }
				| top.commands.1.N        { $$ = std::move($1); }
				;

import: IMPORT IDENTIFIER SEMICOLON		{ $$ = _.Import($2); }
		| IMPORT STRING_LITERAL SEMICOLON		{ $$ = _.Import($2); }
		;

instance: INSTANCE IDENTIFIER LABRACKET list.0.N RABRACKET SEMICOLON {
				$$ = _.MakeInstance($2, 
							std::static_pointer_cast<ASTList>($4));
			}
		| INSTANCE IDENTIFIER LABRACKET list.0.N RABRACKET LBRACKET commands.0.N RBRACKET {
				$$ = _.MakeInstance($2, 
						std::static_pointer_cast<ASTList>($4), std::move($7));
			}

top.commands.1.N:	import		{ if($1) $$.push_back($1); }
				|   commands.1.N		{
					$$ = std::move($1);
				}
				|	top.commands.1.N commands.1.N        { 
						$$ = std::move($1); 
						std::move(std::begin($2), std::end($2), std::back_inserter($$)); 
					}
				|  top.commands.1.N import	{ 
						$$ = std::move($1); 
						$$.push_back($2); 
				}
				;

commands.0.N:                         {  }
				| commands.1.N        { $$ = std::move($1); }
				;

commands.1.N:	command                 { if ($1) $$.push_back($1); }
				| templatedecl			{ $$.push_back($1); }
				| commands.1.N instance { $1.push_back($2); $$=std::move($1); }
				| commands.1.N command    { if ($2) $1.push_back($2); $$ = std::move($1); }
				| commands.1.N templatedecl  { $1.push_back($2); $$ = std::move($1); }
				;

command: SEMICOLON                          { $$.reset(); }
		 | IDENTIFIER parameters.0.N SEMICOLON   { $$ = _.MakeCommand($1, std::move($2)); }
		 | IDENTIFIER parameters.0.N LBRACKET commands.0.N RBRACKET
		 {
			 /* returns $4 if its not null with appropriate stuff */
			 $$ = _.MakeBlock($1, std::move($2), std::move($4));
		 }
		 ;

parameters.0.N:                           { }
				  | parameters.0.N parameter  { $1.Append(std::move($2)); $$ = std::move($1); }
				  | parameters.0.N LBRACES special_parameters.0.N RBRACES { 
							$1.AppendExpanded(std::move($3)); $$ = std::move($1); 
						}
				  ;

special_parameters.0.N:            {  }
			| special_parameter       { 
					ASTListPtr l = std::make_shared<ASTList>(); 
					l->values.emplace_back(std::move($1)); 
					$$ = std::move(l); 
				}
			| special_parameters.0.N COMMA special_parameter 
			{ 
				ASTListPtr list;
				if (static_cast<ASTList*>($1.get()) == nullptr) 
					list = std::make_shared<ASTList>();
				else
					list = std::static_pointer_cast<ASTList>($1);
				list->values.emplace_back(std::move($3));
				$$ = list;
			}
			;

special_parameter: IDENTIFIER  ASSIGN  parameter 
			{ 
				$3->SetName($1); 
				$$ = std::move($3);	
			}
			;

parameter: STRING_LITERAL						{ $$ = std::make_shared<ASTValue>(std::move($1)); }
			| IDENTIFIER                        { $$ = std::make_shared<ASTValue>(std::move($1)); }
			| LSQBRACKET list.0.N RSQBRACKET    { $$ = $2; }
			;
			
list: parameter					{ $$ = std::move($1); }
	| special_parameter			{ $$ = std::move($1); }
    ;
	
list.0.N:								{ }
		|  list                         { 
				ASTListPtr list;
				list = std::make_shared<ASTList>(); 
				list->values.emplace_back(std::move($1)); 
				$$ = list;
			}
	    |  list.0.N COMMA list          { 
			ASTListPtr list;
			if ($1.get() == nullptr)  
				list = std::make_shared<ASTList>();
			else
				list = std::move(std::static_pointer_cast<ASTList>($1));
			list->values.emplace_back(std::move($3));
			$$ = list;
		}
	    ;

%%
/*============================================================================*/

namespace neo {

void parser_impl::error(const location_type& l,
												  const std::string& e) {
  _.ParseError(l, e.c_str());
  L_THROW_CompilationFailed();
}

ASTDocumentPtr context::Parse() {
	BeginScan();
	parser_impl parser(*this);
	parser.set_debug_level(traceParsing);
	int res = parser.parse();
	EndScan();
	if (res)
		return ASTDocumentPtr();
	return document;
}

}
