/**
 * @file foam.lex
 * @author Dan R. Lipsa
 *
 * Description file for  the lexical analyser used to  read a DMP data
 * file produced by the Surface Evolver software
 */
%{
/*
 * WARNING. If a character not  described in this file is matched,
 * it is simply  printed to stdout. So, an  important way to check
 * for  parsing  errors,  is  to  make  sure  you  have  no  extra
 * characters to stdout, even if there are no parsing errors.
 */
#ifdef _MSC_VER
#include <io.h>
#define YY_NO_UNISTD_H
#endif //_MSC_VER

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <string>
#include "ParsingData.h"
#include "EvolverData_yacc.h"
using namespace std;
using EvolverData::parser;

%}

%option reentrant noyywrap nounput yylineno header-file="EvolverData_lex.h"
%option bison-bridge bison-locations
%option extra-type="ParsingData*"

ID    [a-zA-Z][a-zA-Z0-9_]*
D     [0-9]
H     [0-9A-Fa-f]
E     [Ee][+-]?{D}+

%%

"/*"   { /* eat comment */
    register int c;
    while (1)
    { while ( (c = yyinput(yyscanner)) != '*' && c != 0 )
	    ; /* eat up text of comment */
	if ( c == '*' )
	{ while ( (c = yyinput(yyscanner)) == '*' ) ;
	    if ( c == '/' ) break;    /* found the end */ 
	}
	if ( c == 0 && yywrap(yyscanner) )
	{
            yyextra->PrintError (string("Scanner: end-of-file in comment\n"));
            break;
        }
    }
}

"//".*""    /* eat comment */ ;

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
    if (errno)
    {
	yyextra->PrintError(string("Scanner: overflow ") + yytext);
	exit(13);
    }
    return parser::token::REAL_VALUE;
	}

{ID} { /*identifiers*/
    int id = yyextra->GetKeywordId (yytext);
    yylloc->begin.line = yylineno;
    if (id)
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
	yylval->m_id = yyextra->CreateIdentifier (yytext);
	return parser::token::IDENTIFIER;
    }
}

"+"|"-"|"*"|"/"|"^"  {/*operators*/
    yylval->m_id = yyextra->CreateIdentifier (yytext);
    yylloc->begin.line = yylineno;
    return *yytext;
}

":"|"="  {/*assignments*/
    yylval->m_id = yyextra->CreateIdentifier (yytext);
    yylloc->begin.line = yylineno;
    return *yytext;
}

"("|")"  {/*paranthesis: expressions*/
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

","      {/*element separator for arrays*/
    yylloc->begin.line = yylineno;
    return *yytext;
}

"\\\n"  /*ignore escaping of new lines*/

[[:space:]]+   /* ignore whitespace */


%%

void ParsingDriver::ScanBegin ()
{
    EvolverDatalex_init_extra (static_cast<ParsingData*>(this), &m_scanner);
    EvolverDataset_debug(m_debugScanning, m_scanner);
    FILE* dataFile = fopen (m_file.c_str (), "r");
    if (! dataFile)
        PrintError (string ("Scanner: cannot open ") + m_file);
    EvolverDataset_in (dataFile, m_scanner);
}

void ParsingDriver::ScanEnd ()
{
    fclose ( EvolverDataget_in(m_scanner));
    EvolverDatalex_destroy(m_scanner);
}



// Local Variables:
// mode: c++
// End:
