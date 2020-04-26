%option reentrant
%option noyywrap
%option prefix="neo_"
%option never-interactive
%{
#include <cstdint>
#include <string>
#include <string_view>
#include <neo_context.hpp>
#include <parse.neo.hpp>
#define YY_NO_UNISTD_H
#define YY_EXTRA_TYPE neo::context*
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

\\\[ 		 	_.put('[');
\\\{ 		 	_.put('{');
^\{\{        {
	BEGIN(STATE_REGION); 
	std::string content = _.get(); 
	_.start(); 
	return neo::parser_impl::make_TEXT_CONTENTS(content, _.loc()); 
}

[^\\\[\n\{]+       _.put(yytext);
\\[^\[\{]+			 _.put(yytext);

<<EOF>>  {
	BEGIN(INITIAL); 
	return neo::parser_impl::make_TEXT_CONTENTS(_.get(), _.loc()); 
}

}


<STATE_STR>{
\"     			{
	BEGIN(STATE_CONTENT);  
	return neo::parser_impl::make_STRING_LITERAL(_.get(), _.loc());
}
\\[0-7]{1,3}  {  
	int result;  
	std::sscanf( yytext + 1, "%o", &result);  
	if ( result > 0xff )	  
		_.put(result); 
}

\\[0-9]+     	_.push_error(_.loc(), "bad escape sequence");
\\n  					_.put('\n');
\\t  					_.put('\t');
\\r  					_.put('\r');
\\b  					_.put('\b');
\\f  					_.put('\f');
\\(.|\n)  		_.put(yytext[1]);
[^\\\n\"]+   	_.put(yytext);
}

<INITIAL,STATE_CONTENT>{
^\{\{                              BEGIN(STATE_REGION); _.start();
"/*"                               BEGIN(STATE_COMMENT);
"//"                               BEGIN(STATE_SINGLE_LINE_COMMENT);
"{"                                return neo::parser_impl::make_LBRACKET(_.loc());
"}"                                return neo::parser_impl::make_RBRACKET(_.loc());
"template"	                       return neo::parser_impl::make_TEMPLATE(_.loc());
"using"	                           return neo::parser_impl::make_USING(_.loc());
"import"	                       return neo::parser_impl::make_IMPORT(_.loc());
"<"			                       return neo::parser_impl::make_LABRACKET(_.loc());
">"			                       return neo::parser_impl::make_RABRACKET(_.loc());
"["                                return neo::parser_impl::make_LSQBRACKET(_.loc());
"]"                                return neo::parser_impl::make_RSQBRACKET(_.loc());
","                                return neo::parser_impl::make_COMMA(_.loc());
";"                                return neo::parser_impl::make_SEMICOLON(_.loc());
"("                                return neo::parser_impl::make_LBRACES(_.loc());
")"                                return neo::parser_impl::make_RBRACES(_.loc());
"="                                return neo::parser_impl::make_ASSIGN(_.loc());
\"                                 BEGIN(STATE_STR);  _.start();
[\n]+  		  	                     _.loc().lines((std::uint32_t)yyleng); _.loc().step();
{BLANK}+   	                       _.loc().step();
[^ \<\>\"\{\}\[\],;()\t\n\r=]+     return neo::parser_impl::make_IDENTIFIER(yytext, _.loc());

}

<STATE_REGION>{
{BLANK}+   	_.loc().step();
{REGND}		  _.put(yytext);
":"			{ 				
				if (_.get() == "code")
					BEGIN(STATE_REGION_CODE); 
				else 
					BEGIN(STATE_REGION_TEXT);
				_.put(':');
			}
}

<STATE_REGION_TEXT,STATE_REGION_CODE>{

{REG} {
	_.put(yytext);
}

{BLANK}+  _.loc().step();

\}\}  {
	std::string value = _.get();
	_.start();
	if (YY_START == STATE_REGION_CODE) {
		BEGIN(STATE_CONTENT);
		return neo::parser_impl::make_REGION_ID(value, _.loc());
	} else {
		BEGIN(STATE_TEXT);
		return neo::parser_impl::make_TEXT_REGION_ID(value, _.loc());
	}
}

}


<STATE_SINGLE_LINE_COMMENT>{
[^\n]+		// eat comment in chunks
\n		      _.loc().lines((std::uint32_t)1); BEGIN(STATE_CONTENT);
}

<STATE_COMMENT>{
"*/" 			BEGIN(STATE_CONTENT);
[^*\n]+		// eat comment in chunks
\*          // eat it up
\n		      _.loc().lines((std::uint32_t)1);
}

<STATE_TEXT,STATE_STR>\n      {
  // no error, just increase the line number
  // @todo
  _.loc().lines((std::uint32_t)yyleng);
  _.loc().step();
  _.put('\n');
}

%%

namespace neo 
{

void context::begin_scan() 
{
  neo_lex_init_extra(this, &scanner);
  neo_set_debug(flags_ & f_trace_scan, scanner);
}

void context::end_scan() 
{
  neo_lex_destroy(scanner);
}

}
