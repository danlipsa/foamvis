#ifndef _LEXYACC_H_
#define _LEXYACC_H_

/**
 * 
 */
typedef struct 
{
    long i;
    long double r;
} yystype;

/**
 * Standard bison define
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

#endif
