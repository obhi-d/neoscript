%option reentrant
%option noyywrap
%option prefix="neo_"
%option never-interactive

%{
#include <cstdint>
#include <string>
#include <string_view>
#include <neo_state_machine.hpp>
#include "parse.neo.hpp"
#define YY_NO_UNISTD_H
#define YY_EXTRA_TYPE neo::state_machine*
#define YY_INPUT(buf,result,max_size)       \
	result = yyextra->read(buf, max_size);	\
	assert(result <= max_size);			    \
	assert(buf[result] == 0);				\

#define yyterminate() return neo::parser_impl::make_END(_.loc())
#define YY_USER_ACTION                     _.loc().columns((std::uint32_t)yyleng);
void * neo_alloc (std::size_t bytes, void * yyscanner);
void * neo_realloc (void * ptr, std::size_t bytes, void * yyscanner);
void   neo_free (void * ptr, void * yyscanner);
%}

%x STATE_REGION STATE_REGION_TEXT STATE_REGION_CODE STATE_TEXT STATE_STR STATE_CONTENT STATE_COMMENT STATE_SINGLE_LINE_COMMENT

REG     [a-zA-Z#_$:\-\.]+
REGND   [a-zA-Z#_$\-\.]+
BLANK   [ \t]
%%


<STATE_TEXT>{

\\\{ 		 _.push_content(); _.skip_len(1); _.put(1); 
^\{\{        {
	BEGIN(STATE_REGION); 

	_.push_content(); 
	_.skip_len(2);
	return neo::parser_impl::make_TEXT_CONTENTS(std::move(_.retrieve_content()), _.loc()); 
}

<<EOF>>  {
	BEGIN(INITIAL); 
	  _.push_content(); 
	return neo::parser_impl::make_TEXT_CONTENTS(std::move(_.retrieve_content()), _.loc()); 
}

.		_.put(1);

}


<STATE_STR>{
\"     			{
	auto tok = _.make_token();
	auto& s  = _.manage_esq_string();
	_.skip_len(1);
	BEGIN(STATE_CONTENT);  
	if (s.empty())
		return neo::parser_impl::make_STRING_LITERAL(tok, _.loc());
	else
	{
		s += tok;
		auto fs = neo::fixed_string(s);
		s.clear();
		return neo::parser_impl::make_STRING_LITERAL(std::move(fs), _.loc());
	}
}
\\\"	{
	_.append_to_esqstr(_.make_token());
	_.skip_len(1);
	_.put(1);
}
.		{ 
		_.put(1);
}
}

<INITIAL,STATE_CONTENT>{
^\{\{                              BEGIN(STATE_REGION); _.skip_len(2);
"/*"                               BEGIN(STATE_COMMENT); _.skip_len(2);
"//"                               BEGIN(STATE_SINGLE_LINE_COMMENT); _.skip_len(2);
"{"                                _.skip_len(1); return neo::parser_impl::make_LBRACKET(_.loc());
"}"                                _.skip_len(1); return neo::parser_impl::make_RBRACKET(_.loc());
"template"	                       _.skip_len(8); return neo::parser_impl::make_TEMPLATE(_.loc());
"using"	                           _.skip_len(5); return neo::parser_impl::make_USING(_.loc());
"import"	                         _.skip_len(6); return neo::parser_impl::make_IMPORT(_.loc());
"<"			                           _.skip_len(1); return neo::parser_impl::make_LABRACKET(_.loc());
">"			                           _.skip_len(1); return neo::parser_impl::make_RABRACKET(_.loc());
"["                                _.skip_len(1); return neo::parser_impl::make_LSQBRACKET(_.loc());
"]"                                _.skip_len(1); return neo::parser_impl::make_RSQBRACKET(_.loc());
","                                _.skip_len(1); return neo::parser_impl::make_COMMA(_.loc());
";"                                _.skip_len(1); return neo::parser_impl::make_SEMICOLON(_.loc());
"("                                _.skip_len(1); return neo::parser_impl::make_LBRACES(_.loc());
")"                                _.skip_len(1); return neo::parser_impl::make_RBRACES(_.loc());
"="                                _.skip_len(1); return neo::parser_impl::make_ASSIGN(_.loc());
":"                                _.skip_len(1); return neo::parser_impl::make_COLON(_.loc());
"\""                               _.skip_len(1); BEGIN(STATE_STR);  
[\n]+  		  	                     _.skip_len(yyleng); _.loc().lines((std::uint32_t)yyleng); 
{BLANK}+   	                       _.skip_len(yyleng); 
[^ \<\>\"\{\}\[\],;()\t\n\r=\:]+   _.put(yyleng); return neo::parser_impl::make_IDENTIFIER(_.make_token(), _.loc());

}

<STATE_REGION>{
{BLANK}+   	_.skip_len(yyleng); 
{REGND}		_.put(yyleng);
{BLANK}*":"	{ 				
				auto id = _.make_token();
				_.skip_len(yyleng);
				if (id == "code")
				{
					BEGIN(STATE_REGION_CODE); 
				}
				else 
				{
					BEGIN(STATE_REGION_TEXT);
					_.set_current_reg_id(id);
			    }
			}
}

<STATE_REGION_TEXT,STATE_REGION_CODE>{

\}\}  {
	auto value = _.make_token();
	_.skip_len(yyleng);
	if (YY_START == STATE_REGION_CODE) {
		BEGIN(STATE_CONTENT);
		return neo::parser_impl::make_REGION_ID(value, _.loc());
	} else {
		BEGIN(STATE_TEXT);
		return neo::parser_impl::make_TEXT_REGION_ID(value, _.loc());
	}
}

.	_.put(1);

}


<STATE_SINGLE_LINE_COMMENT>{
[^\n]+		_.skip_len(yyleng); 
\n		    _.skip_len(1); _.loc().lines((std::uint32_t)1); BEGIN(STATE_CONTENT);
}

<STATE_COMMENT>{
"*/" 		_.skip_len(2);BEGIN(STATE_CONTENT);
[^*\n]+		_.skip_len(yyleng); 
\*          _.skip_len(1); 
\n		    _.skip_len(1); _.loc().lines((std::uint32_t)1);
}

<STATE_TEXT,STATE_STR>\n      {
  // no error, just increase the line number
  // @todo
  _.loc().lines((std::uint32_t)yyleng);
  _.put(1);
}

%%

namespace neo 
{

void state_machine::begin_scan() noexcept
{
  neo_lex_init_extra(this, &scanner);
  neo_set_debug(flags_ & f_trace_scan, scanner);
}

void state_machine::end_scan() noexcept
{
  neo_lex_destroy(scanner);
}

}
