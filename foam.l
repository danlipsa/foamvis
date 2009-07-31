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
#include <string>
#include "lexYacc.h"
#include "Data.h"
#include "ParsingData.h"
#include "foam_yacc.h"

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
	{foamerror ("lexical error, end-of-file in comment\n");break;}
    }
}

"//".*""    /* eat comment */ ;

[+-]?{D}+ { /* decimal integers */
    foamlval.i = readInteger (yytext, 10);
    foamlloc.first_line = yylineno;
    return INTEGER_VALUE;
	}

[+-]?0x{H}+ { /*hex integers*/
    foamlval.i = readInteger (yytext, 16);
    foamlloc.first_line = yylineno;
    return INTEGER_VALUE;
	  }
[+-]?[01]+[Bb]   { /*binary integers*/
    foamlval.i = readInteger (yytext, 2);
    foamlloc.first_line = yylineno;
    return INTEGER_VALUE;
}

[+-]?{D}+"."{D}*({E})?   |
[+-]?{D}*"."{D}+({E})?   |
[+-]?{D}+{E}   { /* reals */
    char *tail = yytext;
    errno = 0;
    foamlval.r = strtod (yytext, &tail);
    foamlloc.first_line = yylineno;
    if (errno)
    {
	foamerror((string("lexical error, overflow ") + yytext).c_str());
	exit(13);
    }
    return REAL_VALUE;
	}

{ID} { /*identifiers*/
    int id = KeywordId (yytext);
    foamlloc.first_line = yylineno;
    if (id)
    {
	if (id == READ)
	    // ignore READ and everything after it.
	    yyterminate();
	return id;
    }
    else
    {
	foamlval.id = data.GetParsingData ().CreateIdentifier (yytext);
	return IDENTIFIER;
    }
}

"+"|"-"|"*"|"/"|"^"  {/*operators*/
    foamlval.id = data.GetParsingData ().CreateIdentifier (yytext);
    foamlloc.first_line = yylineno;
    return *yytext;
}

":"|"="  {/*assignments*/
    foamlval.id = data.GetParsingData ().CreateIdentifier (yytext);
    foamlloc.first_line = yylineno;
    return *yytext;
}

"("|")"  {/*paranthesis for expressions*/
    foamlloc.first_line = yylineno;
    return *yytext;
}

"["|"]"  {/*brakets the size of arrays*/
    foamlloc.first_line = yylineno;
    return *yytext;
}

"{"|"}"  {/*curly brakets elements of arrays*/
    foamlloc.first_line = yylineno;
    return *yytext;
}

","      {/*element separator for arrays*/
    foamlloc.first_line = yylineno;
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
	foamerror ((string("lexical error, long overflow ") + str).c_str());
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
