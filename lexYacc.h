#ifndef _LEXYACC_H_
#define _LEXYACC_H_

#include <vector>

/**
 * Defines the datatype for semantic values for terminals and non-terminals
 * in the gramar
 */
typedef union
{
    int i;
    float r;
    std::vector<int>* int_list;
} yystype;
/**
 * Defines the datatype for semantic values in bison to be yystype
 */
#define YYSTYPE yystype

/**
 * Checks if an identifier is a keyword.
 * @param s identifier to be checked.
 * @return the keyword ID or 0 if `s' is not a keyword
 */
int KeywordId (char* s);


/**
 * Turns on or off flex debugging
 * @param debugging turns on debugging with 1 and turns off debugging with 0
 */
void FlexDebugging (int debugging);

/**
 * Standard bison function. Called when there is a 
 * parse error or an error in the lexical analyzer.
 * @param error error message
 */
void yyerror (const char* error);

#endif

// Local Variables:
// mode: c++
// End:
