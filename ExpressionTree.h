#ifndef __EXPRESSION_TREE_H__
#define __EXPRESSION_TREE_H__

#include <string>
#include "lexYacc.h"
#include "ParsingData.h"
using namespace std;

class ExpressionTree
{
public:
    ExpressionTree () : m_left (0), m_right (0) {}
    ExpressionTree (ExpressionTree* left, ExpressionTree* right)
	: m_left (left), m_right (right) {}
    static void Delete (ExpressionTree* node);
    virtual float Value (void) = 0;
protected:
    ExpressionTree* m_left;
    ExpressionTree* m_right;
};


class ExpressionTreeNumber : public ExpressionTree
{
public:
    ExpressionTreeNumber (float value) : m_value (value) {}
    virtual float Value (void);
private:
    float m_value;
};

class ExpressionTreeVariable : public ExpressionTree
{
public:
    ExpressionTreeVariable (string* name, ParsingData& parsingData)
	: m_name (name->c_str()), m_parsingData (parsingData) {}
    virtual float Value (void);
private:
    const char* m_name;
    ParsingData& m_parsingData;
};

class ExpressionTreeUnaryFunction : public ExpressionTree
{
public:
    ExpressionTreeUnaryFunction (
	string* name, ExpressionTree* param, ParsingData& parsingData)
	: ExpressionTree (param, 0), m_name (name->c_str()), 
	  m_parsingData (parsingData) {}
    virtual float Value (void);
private:
    const char* m_name;
    ParsingData& m_parsingData;
};


class ExpressionTreeBinaryFunction : public ExpressionTree
{
public:
    ExpressionTreeBinaryFunction (
	string* name, 
	ExpressionTree* left, ExpressionTree* right, ParsingData& parsingData)
	: ExpressionTree (left, right), m_name (name->c_str()),
	  m_parsingData (parsingData) {}
    virtual float Value (void);
private:
    const char* m_name;
    ParsingData& m_parsingData;
};


#endif

// Local Variables:
// mode: c++
// End:
