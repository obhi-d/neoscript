%option reentrant
%option noyywrap
%option prefix="neo_"
%option never-interactive
%{
#include <cstdint>
#include <ParserContext.h>
#include <parse.neo.hpp>
#define YY_NO_UNISTD_H
#define YY_EXTRA_TYPE lumiere::ParserContext*
#define YY_INPUT(buf,result,max_size)       \
	result = yyextra->Read(buf, max_size);	\
	L_ASSERT(result <= max_size);			\
	L_ASSERT(buf[result] == 0);				\

#define yyterminate() return lumiere::neo_parser_impl::make_END(b.Loc())
#define YY_USER_ACTION                     b.Loc().Columns((lumiere::uint32)yyleng);
void * neo_alloc (size_t bytes, void * yyscanner);
void * neo_realloc (void * ptr, size_t bytes, void * yyscanner);
void   neo_free (void * ptr, void * yyscanner);
%}

%x STATE_REGION STATE_REGION_TEXT STATE_REGION_CODE STATE_TEXT STATE_STR STATE_CONTENT STATE_COMMENT STATE_SINGLE_LINE_COMMENT

REG     [a-zA-Z#_$:\-\.]+
REGND   [a-zA-Z#_$\-\.]+
BLANK   [ \t]
%%


<STATE_TEXT>{

\\\[ 		 	b.PutContent('[');
\\\{ 		 	b.PutContent('{');
^\{\{        {
	BEGIN(STATE_REGION); 
	lumiere::String content = b.GetContent(); 
	b.StartContent(); 
	return lumiere::neo_parser_impl::make_TEXT_CONTENTS(content, b.Loc()); 
}

[^\\\[\n\{]+       b.PutContent(yytext);
\\[^\[\{]+			 b.PutContent(yytext);

<<EOF>>  {
	BEGIN(INITIAL); 
	return lumiere::neo_parser_impl::make_TEXT_CONTENTS(b.GetContent(), b.Loc()); 
}

}


<STATE_STR>{
\"     			{
	BEGIN(STATE_CONTENT);  
	return lumiere::neo_parser_impl::make_STRING_LITERAL(b.GetContent(), b.Loc());
}
\\[0-7]{1,3}  {  
	int result;  
	std::sscanf( yytext + 1, "%o", &result);  
	if ( result > 0xff )	  
		b.PutContent(result); 
}

\\[0-9]+     			b.ParseError(lumiere::ParserContext::BAD_ESCAPE_SEQ);
\\n  					b.PutContent('\n');
\\t  					b.PutContent('\t');
\\r  					b.PutContent('\r');
\\b  					b.PutContent('\b');
\\f  					b.PutContent('\f');
\\(.|\n)  		b.PutContent(yytext[1]);
[^\\\n\"]+   	b.PutContent(yytext);
}

<INITIAL,STATE_CONTENT>{
^\{\{          BEGIN(STATE_REGION); b.StartContent();
"/*"           BEGIN(STATE_COMMENT);
"//"           BEGIN(STATE_SINGLE_LINE_COMMENT);
"{"            return lumiere::neo_parser_impl::make_LBRACKET(b.Loc());
"}"            return lumiere::neo_parser_impl::make_RBRACKET(b.Loc());
"template"	   return lumiere::neo_parser_impl::make_TEMPLATE(b.Loc());
"instance"	   return lumiere::neo_parser_impl::make_INSTANCE(b.Loc());
"import"	   return lumiere::neo_parser_impl::make_IMPORT(b.Loc());
"<"			   return lumiere::neo_parser_impl::make_LABRACKET(b.Loc());
">"			   return lumiere::neo_parser_impl::make_RABRACKET(b.Loc());
"["            return lumiere::neo_parser_impl::make_LSQBRACKET(b.Loc());
"]"            return lumiere::neo_parser_impl::make_RSQBRACKET(b.Loc());
","            return lumiere::neo_parser_impl::make_COMMA(b.Loc());
";"            return lumiere::neo_parser_impl::make_SEMICOLON(b.Loc());
"("            return lumiere::neo_parser_impl::make_LBRACES(b.Loc());
")"            return lumiere::neo_parser_impl::make_RBRACES(b.Loc());
"="            return lumiere::neo_parser_impl::make_ASSIGN(b.Loc());
\"             BEGIN(STATE_STR);  b.StartContent();
[\n]+  		  	b.Loc().Lines((lumiere::uint32)yyleng); b.Loc().Step();
{BLANK}+   	   b.Loc().Step();
[^ \<\>\"\{\}\[\],;()\t\n\r=]+    return lumiere::neo_parser_impl::make_IDENTIFIER(yytext, b.Loc());

}

<STATE_REGION>{
{BLANK}+   	b.Loc().Step();
{REGND}		b.PutContent(yytext);
":"			{ 				
				if (b.GetContent() == "code")
					BEGIN(STATE_REGION_CODE); 
				else 
					BEGIN(STATE_REGION_TEXT);
				b.PutContent(':');
			}
}

<STATE_REGION_TEXT,STATE_REGION_CODE>{

{REG} {
	b.PutContent(yytext);
}

{BLANK}+  b.Loc().Step();

\}\}  {
	lumiere::String value = b.GetContent();
	b.StartContent();
	if (YY_START == STATE_REGION_CODE) {
		BEGIN(STATE_CONTENT);
		return lumiere::neo_parser_impl::make_REGION_ID(value, b.Loc());
	} else {
		BEGIN(STATE_TEXT);
		return lumiere::neo_parser_impl::make_TEXT_REGION_ID(value, b.Loc());
	}
}

}


<STATE_SINGLE_LINE_COMMENT>{
[^\n]+		// eat comment in chunks
\n		      b.Loc().Lines((lumiere::uint32)1); BEGIN(STATE_CONTENT);
}

<STATE_COMMENT>{
"*/" 			BEGIN(STATE_CONTENT);
[^*\n]+		// eat comment in chunks
\*          // eat it up
\n		      b.Loc().Lines((lumiere::uint32)1);
}

<STATE_TEXT,STATE_STR>\n      {
  // no error, just increase the line number
  // @todo
  b.Loc().Lines((lumiere::uint32)yyleng);
  b.Loc().Step();
  b.PutContent('\n');
}

%%

namespace lumiere {

void ParserContext::BeginScan() {
  neo_lex_init_extra(this, &scanner);
  neo_set_debug(traceScanning, scanner);
}

void ParserContext::EndScan() {
  neo_lex_destroy(scanner);
}

}
