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
    Delete (node->m_first);
    Delete (node->m_second);
    delete node;
}

double ExpressionTreeNumber::Value (void)
{
    return m_value;
}


double ExpressionTreeVariable::Value (void)
{
    return m_parsingData.GetVariableValue (m_name);
}

double ExpressionTreeUnaryFunction::Value (void)
{
    double value = m_first->Value ();
    ParsingData::UnaryFunction f = m_parsingData.GetUnaryFunction (m_name);
    return f (value);
}

double ExpressionTreeBinaryFunction::Value (void)
{
    double second = m_second->Value ();
    double first = m_first->Value ();
    ParsingData::BinaryFunction f = m_parsingData.GetBinaryFunction (m_name);
    return f (first, second);
}

double ExpressionTreeConditional::Value (void)
{
    double first = m_first->Value ();
    return first ? m_second->Value () : m_third->Value ();
}

