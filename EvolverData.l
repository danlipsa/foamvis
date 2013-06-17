/**
 * @file EvolverData.l
 * @author Dan R. Lipsa
 * @brief Lexical analyser for parsing a DMP file produced by Surface Evolver.
 * @ingroup parser
 */

/*
 * WARNING. If a character not  described in this file is matched,
 * it is simply  printed to stdout. So, an  important way to check
 * for  parsing  errors,  is  to  make  sure  you  have  no  extra
 * characters to stdout, even if there are no parsing errors.
 */

/** @cond */

%{
#include "ParsingData.h"
#include "EvolverData_yacc.h"
#include "Debug.h"
using EvolverData::parser;
#define YY_EXTRA_TYPE ParsingData*
%}

%option reentrant noyywrap nounput yylineno header-file="EvolverData_lex.h"
%option bison-bridge bison-locations
%option debug


ID    [a-zA-Z][a-zA-Z0-9_]*
D     [0-9]
H     [0-9A-Fa-f]
E     [Ee][+-]?{D}+
NEWLINE ("\n"|"\r\n")
SPACENONEWLINE ([[:space:]]{-}[{NEWLINE}])

%%

"/*area" {
    yylval->m_id = yyextra->CreateIdentifier (yytext + 2);
    yylloc->begin.line = yylineno;
    return parser::token::AREA;    
}


"/*actual" {
    yylval->m_id = yyextra->CreateIdentifier (yytext + 2);
    yylloc->begin.line = yylineno;
    return parser::token::ACTUAL;    
}

"*/" {
    yylval->m_id = yyextra->CreateIdentifier (yytext);
    yylloc->begin.line = yylineno;
    return parser::token::END_COMMENT;
}


"/*"             { /* eat comment */
    register int c;
    while (1)
    { while ( (c = yyinput(yyscanner)) != '*' && c != 0 )
	    ; /* eat up text of comment */
	if ( c == '*' )
	{ while ( (c = yyinput(yyscanner)) == '*' ) ;
	    if ( c == '/' ) break;    /* found the end */ 
	}
	if ( c == 0 && yywrap(yyscanner) )
            ThrowException ("Scanner: end-of-file in comment\n");
    }
		  }

"//".*       /* eat comments */

"\\"{NEWLINE} /* splice a line terminated in \ to the next line */


[+-]?{D}+ { /* decimal integers */
    yylval->m_int = yyextra->ReadInteger (yytext, 10);
    yylloc->begin.line = yylineno;
    return parser::token::INTEGER_VALUE;
	}

[+-]?0x{H}+ { /* hexadecimal integers */
    yylval->m_int = yyextra->ReadInteger (yytext, 16);
    yylloc->begin.line = yylineno;
    return parser::token::INTEGER_VALUE;
	  }
[+-]?[01]+[Bb]   { /* binary integers */
    yylval->m_int = yyextra->ReadInteger (yytext, 2);
    yylloc->begin.line = yylineno;
    return parser::token::INTEGER_VALUE;
}

[+-]?{D}+"."{D}*({E})?   |
[+-]?{D}*"."{D}+({E})?   |
[+-]?{D}+{E}   { /* reals */
    char *tail = yytext;
    errno = 0;
    yylval->m_real = strtod (yytext, &tail);
    yylloc->begin.line = yylineno;
    // report error only for overflow, not for underflow
    if (errno && (yylval->m_real == HUGE_VAL || yylval->m_real == -HUGE_VAL))
    {
	ThrowException (string("Scanner: overflow ") + yytext);
	errno = 0;
    }
    return parser::token::REAL_VALUE;
	}

{ID} { /*identifiers*/
    int id = ParsingDriver::GetKeywordId (yytext);
    yylloc->begin.line = yylineno;
    if (! yyextra->KeywordsIgnored () && id)
    {
	if (id == parser::token::READ)
	    // ignore READ and everything after it.
	    yyterminate();
	else
	{
	    yylval->m_id = yyextra->CreateIdentifier (yytext);
	    return id;
	}
    }
    else
    {
	const char* id = yyextra->CreateIdentifier (yytext);
	yylval->m_id = id;
	if (yyextra->IsAttribute (id))
	    return parser::token::ATTRIBUTE_ID;
	else if (yyextra->IsMethodOrQuantity (id))
	    return parser::token::METHOD_OR_QUANTITY_ID;
	else
	    return parser::token::IDENTIFIER;
    }
}

">="|"<=" {
    yylval->m_id = yyextra->CreateIdentifier (yytext);
    yylloc->begin.line = yylineno;
    return *yytext == '>' ? parser::token::GE : parser::token::LE;
}

"&&" {
    yylval->m_id = yyextra->CreateIdentifier (yytext);
    yylloc->begin.line = yylineno;
    return parser::token::AND;
}

"||" {
    yylval->m_id = yyextra->CreateIdentifier (yytext);
    yylloc->begin.line = yylineno;
    return parser::token::OR;
}

"^"|"**" {/*exponentiation*/
    yylval->m_id = yyextra->CreateIdentifier ("^");
    yylloc->begin.line = yylineno;
    return *yytext;
}


"+"|"-"|"*"|"/"|">"|"<"|"!"|"."  {/*operators*/
    yylval->m_id = yyextra->CreateIdentifier (yytext);
    yylloc->begin.line = yylineno;
    return *yytext;
}

":"|"="  {/*assignments*/
    yylval->m_id = yyextra->CreateIdentifier (yytext);
    yylloc->begin.line = yylineno;
    return *yytext;
}

"?"  {/*conditional*/
    yylloc->begin.line = yylineno;
    return *yytext;
}


"("  {/*open paranthesis: expressions*/
    yyextra->OpenParenthesis ();
    yylloc->begin.line = yylineno;
    return *yytext;
}

")"  {/*close paranthesis: expressions*/
    yyextra->CloseParenthesis ();
    yylloc->begin.line = yylineno;
    return *yytext;
}


"["|"]"  {/*brakets: size of arrays*/
    yylloc->begin.line = yylineno;
    return *yytext;
}

"{"|"}"  {/*curly brakets: elements of arrays*/
    yylloc->begin.line = yylineno;
    return *yytext;
}

","|"\""|";"      {/*element separator for arrays, quote, semicolon*/
    yylloc->begin.line = yylineno;
    return *yytext;
}


{SPACENONEWLINE}*{NEWLINE}{SPACENONEWLINE}* {
    yylloc->begin.line = yylineno;
    return '\n';
}

{SPACENONEWLINE}+ {
/* ignore  white spaces, for  some reason  this eats
 * the end  of the line too  so that is  why we need
 * the rule above */
    if (yyextra->IsSpaceSignificant ())
	return parser::token::SPACE;
}

%%

void ParsingDriver::ScanBegin ()
{
    EvolverDatalex_init (&m_scanner);
    EvolverDataset_extra (static_cast<ParsingData*>(this), m_scanner);
    EvolverDataset_debug(m_debugScanning, m_scanner);
    FILE* dataFile = fopen (m_file.c_str (), "r");
    if (! dataFile)
        ThrowException (string () + "Scanner: cannot open " + m_file);
    EvolverDataset_in (dataFile, m_scanner);
}

void ParsingDriver::ScanEnd ()
{
    fclose ( EvolverDataget_in(m_scanner));
    EvolverDatalex_destroy(m_scanner);
}
/** @endcond */


// Local Variables:
// mode: c++
// End:
