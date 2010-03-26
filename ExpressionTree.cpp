/**
 * @file   ExpressionTree.cpp
 * @author Dan R. Lipsa
 *
 * Member definitions  for nodes used  in an expression tree  built in
 * the parser
 */
#include "ExpressionTree.h"
#include "ParsingData.h"

void ExpressionTree::Delete (ExpressionTree* node)
{
    if (node == 0)
        return;
    Delete (node->m_left);Delete (node->m_right);
    delete node;
}

float ExpressionTreeNumber::Value (void)
{
    return m_value;
}


float ExpressionTreeVariable::Value (void)
{
    return m_parsingData.GetVariableValue (m_name);
}

float ExpressionTreeUnaryFunction::Value (void)
{
    float value = m_left->Value ();
    float (*f)(float) = m_parsingData.GetUnaryFunction (m_name);
    return f (value);
}

float ExpressionTreeBinaryFunction::Value (void)
{
    float right = m_right->Value ();
    float left = m_left->Value ();
    float (*f)(float, float) = m_parsingData.GetBinaryFunction (m_name);
    return f (left, right);
}
