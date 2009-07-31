/**
 * Interface between flex and bison
 * @file   lexYacc.h
 * @author Dan R. Lipsa
 */
#ifndef _LEXYACC_H_
#define _LEXYACC_H_

#include <string>
#include <vector>
#include <map>
#include "Attribute.h"
using namespace std;

class ExpressionTree;
class AttributeCreator;

/**
 * Defines the datatypes for semantic values for terminals and non-terminals
 * in the gramar
 */
typedef union
{
    /**
     * Value for an iteger
     */
    int i;
    /**
     * Value for a floating point
     */
    float r;
    /**
     * Value for a list of integers
     */
    vector<int>* intList;
    /**
     * Value for a list of numbers (ints or reals)
     */
    vector<float>* numberList;
    /**
     * An identifier
     */
    std::string* id;
    /**
     * An expression tree
     */
    ExpressionTree* node;
    /**
     * Knows how to create an attribute
     */
    AttributeCreator* attributeCreator;
    enum Attribute::Type attributeType;
} SemanticValue;
/**
 * Defines the datatype for semantic values in bison to be yystype
 */
#define YYSTYPE SemanticValue

/**
 * Checks if an identifier is a keyword.
 * @param s identifier to be checked.
 * @return the keyword ID or 0 if `s' is not a keyword
 */
int KeywordId (const char* s);

const char* KeywordString (int id);


/**
 * Turns on or off flex debugging
 * @param debugging turns on debugging with 1 and turns off debugging with 0
 */
void FlexDebugging (int debugging);

/**
 * Turns on or off bison debugginig
 * @param debugging turns on debugging for 1, off for 0
 */
void BisonDebugging (int debugging);

/**
 * Standard bison function. Called when there is a 
 * parse error. I also call it for an error in the lexical analyzer.
 * @param error error message
 */
void foamerror (const char* error);

/**
 * Parser generated by bison from the description file foam.y
 * @return 0 for success, different than 0 otherwise
 */
int foamparse(void);

#endif

// Local Variables:
// mode: c++
// End:
