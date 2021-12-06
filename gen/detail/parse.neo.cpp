// A Bison parser, made by GNU Bison 3.7.4.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2020 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.


// Take the name prefix into account.
#define yylex   neo_lex



#include "parse.neo.hpp"


// Unqualified %code blocks.

#include "neo_state_machine.hpp"
#define SCANNER_PARAM _.scanner
YY_DECL;



#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if NEO_DEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !NEO_DEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !NEO_DEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

namespace neo {

  /// Build a parser object.
  parser_impl::parser_impl (state_machine& __yyarg)
#if NEO_DEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      _ (__yyarg)
  {}

  parser_impl::~parser_impl ()
  {}

  parser_impl::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------------.
  | symbol kinds.  |
  `---------------*/



  // by_state.
  parser_impl::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  parser_impl::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  parser_impl::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  parser_impl::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  parser_impl::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  parser_impl::symbol_kind_type
  parser_impl::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  parser_impl::stack_symbol_type::stack_symbol_type ()
  {}

  parser_impl::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_commanddecl: // commanddecl
        value.YY_MOVE_OR_COPY< command > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_special_parameter: // special_parameter
      case symbol_kind::S_parameter: // parameter
      case symbol_kind::S_any_parameter: // any_parameter
      case symbol_kind::S_35_list_0_N: // list.0.N
        value.YY_MOVE_OR_COPY< command::param_t > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_31_parameters_0_N: // parameters.0.N
        value.YY_MOVE_OR_COPY< command::parameters > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_instancedecl: // instancedecl
        value.YY_MOVE_OR_COPY< command_instance > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_templatedecl: // templatedecl
        value.YY_MOVE_OR_COPY< command_template > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_STRING_LITERAL: // STRING_LITERAL
        value.YY_MOVE_OR_COPY< neo::flex_string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_TEXT_CONTENTS: // TEXT_CONTENTS
        value.YY_MOVE_OR_COPY< neo::text_content > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_REGION_ID: // REGION_ID
      case symbol_kind::S_TEXT_REGION_ID: // TEXT_REGION_ID
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_commandname: // commandname
        value.YY_MOVE_OR_COPY< std::string_view > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_26_template_args_0_N: // template_args.0.N
        value.YY_MOVE_OR_COPY< std::vector<std::string_view> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  parser_impl::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_commanddecl: // commanddecl
        value.move< command > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_special_parameter: // special_parameter
      case symbol_kind::S_parameter: // parameter
      case symbol_kind::S_any_parameter: // any_parameter
      case symbol_kind::S_35_list_0_N: // list.0.N
        value.move< command::param_t > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_31_parameters_0_N: // parameters.0.N
        value.move< command::parameters > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_instancedecl: // instancedecl
        value.move< command_instance > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_templatedecl: // templatedecl
        value.move< command_template > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_STRING_LITERAL: // STRING_LITERAL
        value.move< neo::flex_string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_TEXT_CONTENTS: // TEXT_CONTENTS
        value.move< neo::text_content > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_REGION_ID: // REGION_ID
      case symbol_kind::S_TEXT_REGION_ID: // TEXT_REGION_ID
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_commandname: // commandname
        value.move< std::string_view > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_26_template_args_0_N: // template_args.0.N
        value.move< std::vector<std::string_view> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  parser_impl::stack_symbol_type&
  parser_impl::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_commanddecl: // commanddecl
        value.copy< command > (that.value);
        break;

      case symbol_kind::S_special_parameter: // special_parameter
      case symbol_kind::S_parameter: // parameter
      case symbol_kind::S_any_parameter: // any_parameter
      case symbol_kind::S_35_list_0_N: // list.0.N
        value.copy< command::param_t > (that.value);
        break;

      case symbol_kind::S_31_parameters_0_N: // parameters.0.N
        value.copy< command::parameters > (that.value);
        break;

      case symbol_kind::S_instancedecl: // instancedecl
        value.copy< command_instance > (that.value);
        break;

      case symbol_kind::S_templatedecl: // templatedecl
        value.copy< command_template > (that.value);
        break;

      case symbol_kind::S_STRING_LITERAL: // STRING_LITERAL
        value.copy< neo::flex_string > (that.value);
        break;

      case symbol_kind::S_TEXT_CONTENTS: // TEXT_CONTENTS
        value.copy< neo::text_content > (that.value);
        break;

      case symbol_kind::S_REGION_ID: // REGION_ID
      case symbol_kind::S_TEXT_REGION_ID: // TEXT_REGION_ID
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_commandname: // commandname
        value.copy< std::string_view > (that.value);
        break;

      case symbol_kind::S_26_template_args_0_N: // template_args.0.N
        value.copy< std::vector<std::string_view> > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

  parser_impl::stack_symbol_type&
  parser_impl::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_commanddecl: // commanddecl
        value.move< command > (that.value);
        break;

      case symbol_kind::S_special_parameter: // special_parameter
      case symbol_kind::S_parameter: // parameter
      case symbol_kind::S_any_parameter: // any_parameter
      case symbol_kind::S_35_list_0_N: // list.0.N
        value.move< command::param_t > (that.value);
        break;

      case symbol_kind::S_31_parameters_0_N: // parameters.0.N
        value.move< command::parameters > (that.value);
        break;

      case symbol_kind::S_instancedecl: // instancedecl
        value.move< command_instance > (that.value);
        break;

      case symbol_kind::S_templatedecl: // templatedecl
        value.move< command_template > (that.value);
        break;

      case symbol_kind::S_STRING_LITERAL: // STRING_LITERAL
        value.move< neo::flex_string > (that.value);
        break;

      case symbol_kind::S_TEXT_CONTENTS: // TEXT_CONTENTS
        value.move< neo::text_content > (that.value);
        break;

      case symbol_kind::S_REGION_ID: // REGION_ID
      case symbol_kind::S_TEXT_REGION_ID: // TEXT_REGION_ID
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_commandname: // commandname
        value.move< std::string_view > (that.value);
        break;

      case symbol_kind::S_26_template_args_0_N: // template_args.0.N
        value.move< std::vector<std::string_view> > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  parser_impl::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if NEO_DEBUG
  template <typename Base>
  void
  parser_impl::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
        switch (yykind)
    {
      case symbol_kind::S_REGION_ID: // REGION_ID
                 { yyoutput << yysym.value.template as < std::string_view > (); }
        break;

      case symbol_kind::S_TEXT_REGION_ID: // TEXT_REGION_ID
                 { yyoutput << yysym.value.template as < std::string_view > (); }
        break;

      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
                 { yyoutput << yysym.value.template as < std::string_view > (); }
        break;

      case symbol_kind::S_commandname: // commandname
                 { yyoutput << yysym.value.template as < std::string_view > (); }
        break;

      default:
        break;
    }
        yyo << ')';
      }
  }
#endif

  void
  parser_impl::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  parser_impl::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  parser_impl::yypop_ (int n)
  {
    yystack_.pop (n);
  }

#if NEO_DEBUG
  std::ostream&
  parser_impl::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  parser_impl::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  parser_impl::debug_level_type
  parser_impl::debug_level () const
  {
    return yydebug_;
  }

  void
  parser_impl::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // NEO_DEBUG

  parser_impl::state_type
  parser_impl::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  parser_impl::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  parser_impl::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  parser_impl::operator() ()
  {
    return parse ();
  }

  int
  parser_impl::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    // User initialization code.
{
  yyla.location.source_name = _.get_file_name();
}



    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            symbol_type yylookahead (yylex (_, SCANNER_PARAM));
            yyla.move (yylookahead);
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case symbol_kind::S_commanddecl: // commanddecl
        yylhs.value.emplace< command > ();
        break;

      case symbol_kind::S_special_parameter: // special_parameter
      case symbol_kind::S_parameter: // parameter
      case symbol_kind::S_any_parameter: // any_parameter
      case symbol_kind::S_35_list_0_N: // list.0.N
        yylhs.value.emplace< command::param_t > ();
        break;

      case symbol_kind::S_31_parameters_0_N: // parameters.0.N
        yylhs.value.emplace< command::parameters > ();
        break;

      case symbol_kind::S_instancedecl: // instancedecl
        yylhs.value.emplace< command_instance > ();
        break;

      case symbol_kind::S_templatedecl: // templatedecl
        yylhs.value.emplace< command_template > ();
        break;

      case symbol_kind::S_STRING_LITERAL: // STRING_LITERAL
        yylhs.value.emplace< neo::flex_string > ();
        break;

      case symbol_kind::S_TEXT_CONTENTS: // TEXT_CONTENTS
        yylhs.value.emplace< neo::text_content > ();
        break;

      case symbol_kind::S_REGION_ID: // REGION_ID
      case symbol_kind::S_TEXT_REGION_ID: // TEXT_REGION_ID
      case symbol_kind::S_IDENTIFIER: // IDENTIFIER
      case symbol_kind::S_commandname: // commandname
        yylhs.value.emplace< std::string_view > ();
        break;

      case symbol_kind::S_26_template_args_0_N: // template_args.0.N
        yylhs.value.emplace< std::vector<std::string_view> > ();
        break;

      default:
        break;
    }


      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 4: // statement: ";"
                                                         {                                                                      }
    break;

  case 5: // statement: commanddecl
                                                         { if (!_.skip()) { if(!_.consume(std::move(yystack_[0].value.as < command > ()))) { YYACCEPT; }   }     }
    break;

  case 6: // statement: templatedecl
                                                         { if (!_.skip()) _.add_template(std::move(yystack_[0].value.as < command_template > ()));                        }
    break;

  case 7: // statement: instancedecl
                                                         { if (!_.skip()) { if(!_.consume(std::move(yystack_[0].value.as < command_instance > ()))) { YYACCEPT; }   }     }
    break;

  case 8: // statement: "}"
                                                         { if (!_.skip()) { if(!_.end_block()) YYACCEPT; } else _.exit_skip_scope();          }
    break;

  case 9: // statement: REGION_ID
                                                         { _.start_region(std::move(yystack_[0].value.as < std::string_view > ()));                                       }
    break;

  case 10: // statement: TEXT_REGION_ID TEXT_CONTENTS
                                                         { _.start_region(std::move(yystack_[1].value.as < std::string_view > ()), std::move(yystack_[0].value.as < neo::text_content > ()));                        }
    break;

  case 11: // statement: "import" STRING_LITERAL ";"
                                                         { 
													auto const& ss = yystack_[1].value.as < neo::flex_string > ();
													if (ss.index() == 0) 
														_.import_script(std::get<std::string_view>(ss)); 
													else 
														_.import_script(std::get<fixed_string>(ss).as_view()); 
												 }
    break;

  case 13: // template_args.0.N: IDENTIFIER
                                                        { 
						if (!_.skip())
						{
							std::vector<std::string_view> args; 
							args.push_back(yystack_[0].value.as < std::string_view > ()); 
							yylhs.value.as < std::vector<std::string_view> > () = std::move(args); 
						}
					}
    break;

  case 14: // template_args.0.N: template_args.0.N "," IDENTIFIER
                                                                              { 
						if (!_.skip())
						{
							yylhs.value.as < std::vector<std::string_view> > () = std::move(yystack_[2].value.as < std::vector<std::string_view> > ());
							yylhs.value.as < std::vector<std::string_view> > ().push_back(yystack_[0].value.as < std::string_view > ()); 
						}
					}
    break;

  case 15: // templatedecl: "template" "<" template_args.0.N ">" commanddecl
                                                                         {
						if (!_.skip())
								yylhs.value.as < command_template > () = std::move(_.make_command_template(
											std::move(yystack_[2].value.as < std::vector<std::string_view> > ()), std::move(yystack_[0].value.as < command > ())));
						 }
    break;

  case 16: // templatedecl: "template" IDENTIFIER "<" template_args.0.N ">" commanddecl
                                                                                                   {
				   		if (!_.skip())
				 				yylhs.value.as < command_template > () = std::move(_.make_command_template(std::move(yystack_[4].value.as < std::string_view > ()),
							        std::move(yystack_[2].value.as < std::vector<std::string_view> > ()), std::move(yystack_[0].value.as < command > ())));
				     }
    break;

  case 17: // commandname: IDENTIFIER
                         { yylhs.value.as < std::string_view > () = std::move(yystack_[0].value.as < std::string_view > ()); }
    break;

  case 18: // commandname: IDENTIFIER "="
                                            { yylhs.value.as < std::string_view > () = std::move(yystack_[1].value.as < std::string_view > ()); }
    break;

  case 19: // commandname: IDENTIFIER ":"
                                           { yylhs.value.as < std::string_view > () = std::move(yystack_[1].value.as < std::string_view > ()); }
    break;

  case 20: // commanddecl: commandname parameters.0.N ";"
                                                             { if (!_.skip()) yylhs.value.as < command > () = std::move(_.make_command(std::move(yystack_[2].value.as < std::string_view > ()), std::move(yystack_[1].value.as < command::parameters > ()))); }
    break;

  case 21: // commanddecl: commandname parameters.0.N "{"
                                                             { if (!_.skip()) yylhs.value.as < command > () = std::move(_.make_command(std::move(yystack_[2].value.as < std::string_view > ()), std::move(yystack_[1].value.as < command::parameters > ()), true)); else _.enter_skip_scope(); }
    break;

  case 22: // instancedecl: "using" IDENTIFIER "<" list.0.N ">" ";"
                                                                      { if (!_.skip()) yylhs.value.as < command_instance > () = std::move(_.make_instance(std::move(yystack_[4].value.as < std::string_view > ()), std::move(yystack_[2].value.as < command::param_t > ()))); }
    break;

  case 23: // instancedecl: "using" IDENTIFIER "<" list.0.N ">" "{"
                                                                                  { if (!_.skip()) yylhs.value.as < command_instance > () = std::move(_.make_instance(std::move(yystack_[4].value.as < std::string_view > ()), std::move(yystack_[2].value.as < command::param_t > ()), true)); }
    break;

  case 25: // parameters.0.N: parameter
                                                { if (!_.skip()) yylhs.value.as < command::parameters > ().append(std::move(yystack_[0].value.as < command::param_t > ())); }
    break;

  case 26: // parameters.0.N: "(" list.0.N ")"
                                                             { 
							if (!_.skip()) { yylhs.value.as < command::parameters > ().append_expanded(std::move(yystack_[1].value.as < command::param_t > ())); }
						}
    break;

  case 27: // parameters.0.N: parameters.0.N parameter
                                                              { if (!_.skip()) { yystack_[1].value.as < command::parameters > ().append(std::move(yystack_[0].value.as < command::param_t > ())); yylhs.value.as < command::parameters > () = std::move(yystack_[1].value.as < command::parameters > ()); } }
    break;

  case 28: // parameters.0.N: parameters.0.N "(" list.0.N ")"
                                                                            { if (!_.skip()) { yystack_[3].value.as < command::parameters > ().append_expanded(std::move(yystack_[1].value.as < command::param_t > ())); yylhs.value.as < command::parameters > () = std::move(yystack_[3].value.as < command::parameters > ()); } }
    break;

  case 29: // special_parameter: IDENTIFIER "=" parameter
                        { 
				if (!_.skip())
				{
					neo::command::parameters::set_name(yystack_[0].value.as < command::param_t > (), std::move(yystack_[2].value.as < std::string_view > ())); 
					yylhs.value.as < command::param_t > () = std::move(yystack_[0].value.as < command::param_t > ());	
				}
			}
    break;

  case 30: // parameter: STRING_LITERAL
                                                                        { 
													if (!_.skip()) 
													{
														auto& ss = yystack_[0].value.as < neo::flex_string > ();	
														if (ss.index() == 0) 
															yylhs.value.as < command::param_t > () = command::single(std::get<std::string_view>(ss));  
														else 
															yylhs.value.as < command::param_t > () = std::move(command::esq_string(std::move(std::get<fixed_string>(ss))));  
													}
														
												}
    break;

  case 31: // parameter: IDENTIFIER
                                                            { if (!_.skip()) yylhs.value.as < command::param_t > () = command::single(std::move(yystack_[0].value.as < std::string_view > ())); }
    break;

  case 32: // parameter: "[" list.0.N "]"
                                                            { if (!_.skip()) yylhs.value.as < command::param_t > () = std::move(yystack_[1].value.as < command::param_t > ()); }
    break;

  case 33: // any_parameter: parameter
                                                                  { if (!_.skip()) yylhs.value.as < command::param_t > () = std::move(yystack_[0].value.as < command::param_t > ()); }
    break;

  case 34: // any_parameter: special_parameter
                                                { if (!_.skip()) yylhs.value.as < command::param_t > () = std::move(yystack_[0].value.as < command::param_t > ()); }
    break;

  case 36: // list.0.N: any_parameter
                                                         { 
			if (!_.skip())
			{
				command::list list; 
				list.emplace_back(std::move(yystack_[0].value.as < command::param_t > ())); 
				yylhs.value.as < command::param_t > () = std::move(list);
			}
		}
    break;

  case 37: // list.0.N: list.0.N "," any_parameter
                                                   { 
		  if (!_.skip()) {
				command::list list; 
				if (yystack_[2].value.as < command::param_t > ().index() == 1) 
					list.emplace_back(std::move(yystack_[2].value.as < command::param_t > ()));
				else if (yystack_[2].value.as < command::param_t > ().index() == 2) 
					list = std::get<command::list>(std::move(yystack_[2].value.as < command::param_t > ()));
				list.emplace_back(std::move(yystack_[0].value.as < command::param_t > ()));
				yylhs.value.as < command::param_t > () = std::move(list);
		  }
	  }
    break;



            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  parser_impl::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  parser_impl::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr;
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              else
                goto append;

            append:
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }

  std::string
  parser_impl::symbol_name (symbol_kind_type yysymbol)
  {
    return yytnamerr_ (yytname_[yysymbol]);
  }



  // parser_impl::context.
  parser_impl::context::context (const parser_impl& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  parser_impl::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;

    int yyn = yypact_[+yyparser_.yystack_[0].state];
    if (!yy_pact_value_is_default_ (yyn))
      {
        /* Start YYX at -YYN if negative to avoid negative indexes in
           YYCHECK.  In other words, skip the first -YYN actions for
           this state because they are default actions.  */
        int yyxbegin = yyn < 0 ? -yyn : 0;
        // Stay within bounds of both yycheck and yytname.
        int yychecklim = yylast_ - yyn + 1;
        int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
          if (yycheck_[yyx + yyn] == yyx && yyx != symbol_kind::S_YYerror
              && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
            {
              if (!yyarg)
                ++yycount;
              else if (yycount == yyargn)
                return 0;
              else
                yyarg[yycount++] = YY_CAST (symbol_kind_type, yyx);
            }
      }

    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }



  int
  parser_impl::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.
    */

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  // Generate an error message.
  std::string
  parser_impl::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum { YYARGS_MAX = 5 };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char parser_impl::yypact_ninf_ = -23;

  const signed char parser_impl::yytable_ninf_ = -1;

  const signed char
  parser_impl::yypact_[] =
  {
       7,   -23,   -23,    25,    14,    17,   -23,    32,     0,    44,
       7,   -23,     9,   -23,   -23,    33,    47,    49,    53,   -23,
     -23,   -23,   -23,   -23,    11,    11,   -23,   -23,    -2,   -23,
     -23,    31,    33,    11,   -23,    45,   -23,   -23,   -23,    29,
      -5,   -23,   -23,    11,   -23,    39,    41,    38,    40,    22,
     -23,    11,   -23,    -4,   -23,   -23,    39,    48,   -23,   -23,
     -23,   -23,   -23,   -23
  };

  const signed char
  parser_impl::yydefact_[] =
  {
       0,     4,     8,     0,     0,     0,     9,     0,    17,     0,
       2,     6,    24,     5,     7,    12,     0,     0,     0,    10,
      18,    19,     1,     3,    35,    35,    31,    30,     0,    25,
      13,     0,    12,    35,    11,    31,    34,    33,    36,     0,
       0,    20,    21,    35,    27,     0,     0,     0,     0,     0,
      32,     0,    26,     0,    15,    14,     0,     0,    29,    37,
      28,    16,    22,    23
  };

  const signed char
  parser_impl::yypgoto_[] =
  {
     -23,    50,   -23,    30,   -23,   -23,   -10,   -23,   -23,   -23,
     -12,    12,   -22
  };

  const signed char
  parser_impl::yydefgoto_[] =
  {
      -1,     9,    10,    31,    11,    12,    13,    14,    28,    36,
      37,    38,    39
  };

  const signed char
  parser_impl::yytable_[] =
  {
      29,    41,    42,    40,    24,    51,    51,    52,    60,    43,
       1,    48,     2,    20,    21,    24,    44,    24,    26,    27,
      25,    53,     3,     4,     5,     6,     7,     8,    24,    26,
      27,    35,    27,    15,    17,    54,    50,    58,    18,    51,
      45,    46,    26,    27,    22,    16,    61,    56,    46,    57,
      51,    62,    63,    30,    19,    32,    34,    33,    49,     8,
      23,    55,    47,    59
  };

  const signed char
  parser_impl::yycheck_[] =
  {
      12,     3,     4,    25,     6,    10,    10,    12,    12,    11,
       3,    33,     5,    13,    14,     6,    28,     6,    20,    21,
      11,    43,    15,    16,    17,    18,    19,    20,     6,    20,
      21,    20,    21,     8,    20,    45,     7,    49,    21,    10,
       9,    10,    20,    21,     0,    20,    56,     9,    10,     9,
      10,     3,     4,    20,    22,     8,     3,     8,    13,    20,
      10,    20,    32,    51
  };

  const signed char
  parser_impl::yystos_[] =
  {
       0,     3,     5,    15,    16,    17,    18,    19,    20,    24,
      25,    27,    28,    29,    30,     8,    20,    20,    21,    22,
      13,    14,     0,    24,     6,    11,    20,    21,    31,    33,
      20,    26,     8,     8,     3,    20,    32,    33,    34,    35,
      35,     3,     4,    11,    33,     9,    10,    26,    35,    13,
       7,    10,    12,    35,    29,    20,     9,     9,    33,    34,
      12,    29,     3,     4
  };

  const signed char
  parser_impl::yyr1_[] =
  {
       0,    23,    24,    24,    25,    25,    25,    25,    25,    25,
      25,    25,    26,    26,    26,    27,    27,    28,    28,    28,
      29,    29,    30,    30,    31,    31,    31,    31,    31,    32,
      33,    33,    33,    34,    34,    35,    35,    35
  };

  const signed char
  parser_impl::yyr2_[] =
  {
       0,     2,     1,     2,     1,     1,     1,     1,     1,     1,
       2,     3,     0,     1,     3,     5,     6,     1,     2,     2,
       3,     3,     6,     6,     0,     1,     3,     2,     4,     3,
       1,     1,     3,     1,     1,     0,     1,     3
  };


#if NEO_DEBUG || 1
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const parser_impl::yytname_[] =
  {
  "\"end of file\"", "error", "\"invalid token\"", "\";\"", "\"{\"",
  "\"}\"", "\"[\"", "\"]\"", "\"<\"", "\">\"", "\",\"", "\"(\"", "\")\"",
  "\"=\"", "\":\"", "\"template\"", "\"using\"", "\"import\"", "REGION_ID",
  "TEXT_REGION_ID", "IDENTIFIER", "STRING_LITERAL", "TEXT_CONTENTS",
  "$accept", "script", "statement", "template_args.0.N", "templatedecl",
  "commandname", "commanddecl", "instancedecl", "parameters.0.N",
  "special_parameter", "parameter", "any_parameter", "list.0.N", YY_NULLPTR
  };
#endif


#if NEO_DEBUG
  const unsigned char
  parser_impl::yyrline_[] =
  {
       0,    83,    83,    84,    87,    88,    89,    90,    91,    92,
      93,    94,   103,   104,   112,   121,   126,   133,   134,   135,
     138,   139,   141,   142,   144,   145,   146,   149,   150,   178,
     188,   199,   200,   203,   204,   207,   208,   216
  };

  void
  parser_impl::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  parser_impl::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // NEO_DEBUG


} // neo


/*============================================================================*/

namespace neo 
{

void parser_impl::error(location_type const& l,
												std::string const & e)
{
  _.push_error(l, e.c_str());
}

void state_machine::parse(std::string_view src_name, std::string_view content) noexcept
{
	auto restore_file = current_file_;
	auto restore_source_name = source_name_;
	auto restore_pos = pos_;
	auto restore_pos_commit = pos_commit_;
	auto restore_len_reading = len_reading_;

	pos_ = 0;
	pos_commit_ = 0;
	len_reading_ = 0;
	current_file_ = content;
	source_name_ = src_name;
	auto restore_loc = loc_;
	loc_ = location_type();
	loc_.source_name = source_name_;
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
	pos_ = restore_pos;
	pos_commit_ = restore_pos_commit;
	len_reading_ = restore_len_reading;

	loc_ = restore_loc;
	scanner = restore_scanner;
}

}
