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
    Delete (node->m_first);Delete (node->m_second);
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
    float value = m_first->Value ();
    ParsingData::UnaryFunction f = m_parsingData.GetUnaryFunction (m_name);
    return f (value);
}

float ExpressionTreeBinaryFunction::Value (void)
{
    float second = m_second->Value ();
    float first = m_first->Value ();
    ParsingData::BinaryFunction f = m_parsingData.GetBinaryFunction (m_name);
    return f (first, second);
}

float ExpressionTreeConditional::Value (void)
{
    float first = m_first->Value ();
    return first ? m_second->Value () : m_third->Value ();
}

