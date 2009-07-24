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
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <string>
#include "lexYacc.h"
#include "Data.h"
#include "ParsingData.h"
#include "y.tab.h"

/**
 * Converts a  string in a given  base to an integer.  Prints an error
 * and exits if the conversion fails.
 * @param str string to be converted to an integer
 * @param base the base that the string uses to represent the integer
 * @return the converted integer
 */
static long readInteger (char* str, int base);
using namespace std;
%}

%option noyywrap
%option nounput
%option yylineno

ID    [a-zA-Z][a-zA-Z0-9_]*
D     [0-9]
H     [0-9A-Fa-f]
E     [Ee][+-]?{D}+

%%

"/*"   { /* eat comment */
    register int c;
    while (1)
    { while ( (c = yyinput()) != '*' && c != 0 )
	    ; /* eat up text of comment */
	if ( c == '*' )
	{ while ( (c = yyinput()) == '*' ) ;
	    if ( c == '/' ) break;    /* found the end */ 
	}
	if ( c == 0 && yywrap() )
	{yyerror ("lexical error, end-of-file in comment\n");break;}
    }
}

"//".*""    /* eat comment */ ;

[+-]?{D}+ { /* decimal integers */
    yylval.i = readInteger (yytext, 10);
    yylloc.first_line = yylineno;
    return INTEGER_VALUE;
	}

[+-]?0x{H}+ { /*hex integers*/
    yylval.i = readInteger (yytext, 16);
    yylloc.first_line = yylineno;
    return INTEGER_VALUE;
	  }
[+-]?[01]+[Bb]   { /*binary integers*/
    yylval.i = readInteger (yytext, 2);
    yylloc.first_line = yylineno;
    return INTEGER_VALUE;
}

[+-]?{D}+"."{D}*({E})?   |
[+-]?{D}*"."{D}+({E})?   |
[+-]?{D}+{E}   { /* reals */
    char *tail = yytext;
    errno = 0;
    yylval.r = strtod (yytext, &tail);
    yylloc.first_line = yylineno;
    if (errno)
    {
	ostringstream message;
	message << "lexical error, overflow " << yytext << ends;
	yyerror(message.str().c_str());
	exit(13);
    }
    return REAL_VALUE;
	}

{ID} { /*identifiers*/
    int id = KeywordId (yytext);
    yylloc.first_line = yylineno;
    if (id)
    {
	if (id == READ)
	    // ignore everything after READ
	    yyterminate();
	return id;
    }
    else
    {
	yylval.id = data.GetParsingData ().CreateIdentifier (yytext);
	return IDENTIFIER;
    }
}

"+"|"-"|"*"|"/"|"^"  {/*operators*/
    yylval.id = data.GetParsingData ().CreateIdentifier (yytext);
    yylloc.first_line = yylineno;
    return *yytext;
}

":"|"="  {/*assignments*/
    yylval.id = data.GetParsingData ().CreateIdentifier (yytext);
    yylloc.first_line = yylineno;
    return *yytext;
}

"("|")"  {/*paranthesis for expressions*/
    yylloc.first_line = yylineno;
    return *yytext;
}

"["|"]"  {/*brakets the size of arrays*/
    yylloc.first_line = yylineno;
    return *yytext;
}

"{"|"}"  {/*curly brakets elements of arrays*/
    yylloc.first_line = yylineno;
    return *yytext;
}

","      {/*element separator for arrays*/
    yylloc.first_line = yylineno;
    return *yytext;
}

"\\\n"  /*ignore escaping of new lines*/

[[:space:]]+   /* ignore whitespace */


%%

static long readInteger (char* str, int base)
{
    char *tail = str;
    errno = 0;
    long i = strtol (str, &tail, base);
    if (errno)
    {
	ostringstream ostr;
	ostr << "lexical error, long overflow " << str;
	yyerror (ostr.str().c_str());
	exit(13);
    }
    return i;
}

void FlexDebugging (int debugging)
{
    yy_flex_debug = debugging;
}

// Local Variables:
// mode: c++
// End:
