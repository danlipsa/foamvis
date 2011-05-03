/**
 * @file   ExpressionTree.cpp
 * @author Dan R. Lipsa
 *
 * Member definitions  for nodes used  in an expression tree  built in
 * the parser
 */
#include "ExpressionTree.h"
#include "ParsingData.h"

// ExpressionTreeNumber
// ======================================================================
double ExpressionTreeNumber::Value (void) const
{
    return m_value;
}

ExpressionTree* ExpressionTreeNumber::Simplify () const
{
    return new ExpressionTreeNumber (m_parsingData, m_value);
}

string ExpressionTreeNumber::ToString ()
{
    ostringstream ostr;
    ostr << m_value;
    return ostr.str ();
}

// ExpressionTreeVariable
// ======================================================================
double ExpressionTreeVariable::Value (void) const
{
    return m_parsingData.GetVariableValue (m_name);
}

ExpressionTree* ExpressionTreeVariable::Simplify () const
{
    if (m_parsingData.IsVariableSet (m_name))
	return new ExpressionTreeNumber (m_parsingData, Value ());
    else
	return new ExpressionTreeVariable (m_parsingData, m_name.c_str ());
}

string ExpressionTreeVariable::ToString ()
{
    ostringstream ostr;
    ostr << m_name;
    if (m_parsingData.IsVariableSet (m_name))
    {
	ostr << "=" << Value ();
    }
    return ostr.str ();
}


// ExpressionTreeUnaryFunction
// ======================================================================
double ExpressionTreeUnaryFunction::Value (void) const
{
    double value = m_param->Value ();
    ParsingData::UnaryFunction f = m_parsingData.GetUnaryFunction (m_name);
    return f (value);
}

ExpressionTree* ExpressionTreeUnaryFunction::Simplify () const
{
    ExpressionTree* simplifiedParam = m_param->Simplify ();
    if (simplifiedParam->IsNumber ())
    {
	ParsingData::UnaryFunction f = m_parsingData.GetUnaryFunction (m_name);
	double value = f(simplifiedParam->Value ());
	delete simplifiedParam;
	return new ExpressionTreeNumber (m_parsingData, value);
    }
    else
	return new ExpressionTreeUnaryFunction (
	    m_parsingData, m_name.c_str (), simplifiedParam);
}

string ExpressionTreeUnaryFunction::ToString ()
{
    ostringstream ostr;
    ostr << m_name << "(" << m_param->ToString () << ")";
    return ostr.str ();
}

// ExpressionTreeBinaryFunction
// ======================================================================
double ExpressionTreeBinaryFunction::Value (void) const
{
    double second = m_second->Value ();
    double first = m_first->Value ();
    ParsingData::BinaryFunction f = m_parsingData.GetBinaryFunction (m_name);
    return f (first, second);
}

ExpressionTree* ExpressionTreeBinaryFunction::Simplify () const
{
    ExpressionTree* simplifiedFirst = m_first->Simplify ();
    ExpressionTree* simplifiedSecond = m_second->Simplify ();
    if (simplifiedFirst->IsNumber () && simplifiedSecond->IsNumber ())
    {
	ParsingData::BinaryFunction f = m_parsingData.GetBinaryFunction (m_name);
	double firstValue = simplifiedFirst->Value ();
	delete simplifiedFirst;
	double secondValue = simplifiedSecond->Value ();
	delete simplifiedSecond;
	return new ExpressionTreeNumber (
	    m_parsingData, f(firstValue, secondValue));
    }
    else
	return new ExpressionTreeBinaryFunction (
	    m_parsingData, m_name.c_str (), simplifiedFirst, simplifiedSecond);
}

string ExpressionTreeBinaryFunction::ToString ()
{
    ostringstream ostr;
    ostr << m_name << "(" 
	 << m_first->ToString () << ", " << m_second->ToString () << ")";
    return ostr.str ();
}


// ExpressionTreeConditional
// ======================================================================
double ExpressionTreeConditional::Value (void) const
{
    double first = m_first->Value ();
    return first ? m_second->Value () : m_third->Value ();
}

ExpressionTree* ExpressionTreeConditional::Simplify () const
{
    ExpressionTree* simplifiedFirst = m_first->Simplify ();
    ExpressionTree* simplifiedSecond = m_second->Simplify ();
    ExpressionTree* simplifiedThird = m_third->Simplify ();
    if (simplifiedFirst->IsNumber ())
    {
	double firstValue = simplifiedFirst->Value ();
	delete simplifiedFirst;
	if (firstValue)
	{
	    delete simplifiedThird;
	    return simplifiedSecond;
	}
	else
	{
	    delete simplifiedSecond;
	    return simplifiedThird;
	}
    }
    else
    {
	return new ExpressionTreeConditional (
	    m_parsingData, simplifiedFirst, simplifiedSecond, simplifiedThird);
    }
}

string ExpressionTreeConditional::ToString ()
{
    ostringstream ostr;
    ostr << "?:(" 
	 << m_first->ToString () << ", "
	 << m_second->ToString () << ", "
	 << m_third->ToString () << ")";
    return ostr.str ();
}
