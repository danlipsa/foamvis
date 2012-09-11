/**
 * @file   ExpressionTree.cpp
 * @author Dan R. Lipsa
 *
 * Member definitions  for nodes used  in an expression tree  built in
 * the parser
 */
#include "DebugStream.h"
#include "ExpressionTree.h"
#include "ParsingData.h"

string ExpressionTree::ToParenthesisString () const
{
    ExpressionTreeType::Enum type = GetType ();
    if (Height () == 0 || 
	type == ExpressionTreeType::UNARY_FUNCTION ||
	IsProperBinaryFunction ())
	return ToString ();
    else
	return "(" + ToString () + ")";
}

bool ExpressionTree::IsProperBinaryFunction () const
{
    if (GetType () == ExpressionTreeType::BINARY_FUNCTION)
    {
	const ExpressionTreeBinaryFunction* bf = 
	    static_cast<const ExpressionTreeBinaryFunction*> (this);
	return ! bf->IsOperator ();
    }
    else
	return false;
}


// ExpressionTreeNumber
// ======================================================================
double ExpressionTreeNumber::Value (void) const
{
    return m_value;
}

ExpressionTree* ExpressionTreeNumber::GetSimplifiedTree () const
{
    return new ExpressionTreeNumber (m_parsingData, m_value);
}

string ExpressionTreeNumber::ToString () const
{
    ostringstream ostr;
    ostr << m_value;
    return ostr.str ();
}

// ExpressionTreeVariable
// ======================================================================
double ExpressionTreeVariable::Value (void) const
{
    return m_parsingData.GetVariableValue (m_name).second;
}

ExpressionTree* ExpressionTreeVariable::GetSimplifiedTree () const
{
    if (m_parsingData.GetVariableValue (m_name).first)
	return new ExpressionTreeNumber (m_parsingData, Value ());
    else
	return new ExpressionTreeVariable (m_parsingData, m_name.c_str ());
}

string ExpressionTreeVariable::ToString () const
{
    ostringstream ostr;
    if (m_parsingData.GetVariableValue (m_name).first)
	ostr << Value ();
    else
	ostr << m_name;
    return ostr.str ();
}

// ExpressionTreeArrayElement
// ======================================================================
double ExpressionTreeArrayElement::Value (void) const
{
    return m_parsingData.GetArrayValue (m_name, m_index).second;
}

ExpressionTree* ExpressionTreeArrayElement::GetSimplifiedTree () const
{
    return new ExpressionTreeNumber (m_parsingData, Value ());
}

string ExpressionTreeArrayElement::ToString () const
{
    ostringstream ostr;
    ostr << Value ();
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

ExpressionTree* ExpressionTreeUnaryFunction::GetSimplifiedTree () const
{
    ExpressionTree* simplifiedParam = m_param->GetSimplifiedTree ();
    if (simplifiedParam->GetType () == ExpressionTreeType::NUMBER)
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

string ExpressionTreeUnaryFunction::ToString () const
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

ExpressionTree* ExpressionTreeBinaryFunction::GetSimplifiedTree () const
{
    ExpressionTree* simplifiedFirst = m_first->GetSimplifiedTree ();
    ExpressionTree* simplifiedSecond = m_second->GetSimplifiedTree ();
    if (simplifiedFirst->GetType () == ExpressionTreeType::NUMBER && 
	simplifiedSecond->GetType () == ExpressionTreeType::NUMBER)
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

bool ExpressionTreeBinaryFunction::IsOperator () const
{
    return m_parsingData.IsOperator (m_name);
}


string ExpressionTreeBinaryFunction::ToString () const
{
    ostringstream ostr;
    if (IsOperator ())
    {
	ostr << m_first->ToParenthesisString () << " " << m_name << " "
	     << m_second->ToParenthesisString ();
    }
    else
    {
	ostr << m_name << "(" 
	     << m_first->ToString () << ", " << m_second->ToString () << ")";
    }
    return ostr.str ();
}


// ExpressionTreeConditional
// ======================================================================
double ExpressionTreeConditional::Value (void) const
{
    double first = m_first->Value ();
    if (first)
    {
	//cdbg << "left ";
	return m_second->Value ();
    }
    else
    {
	//cdbg << "right ";
	return m_third->Value ();
    }
}

ExpressionTree* ExpressionTreeConditional::GetSimplifiedTree () const
{
    ExpressionTree* simplifiedFirst = m_first->GetSimplifiedTree ();
    ExpressionTree* simplifiedSecond = m_second->GetSimplifiedTree ();
    ExpressionTree* simplifiedThird = m_third->GetSimplifiedTree ();
    if (simplifiedFirst->GetType () == ExpressionTreeType::NUMBER)
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

string ExpressionTreeConditional::ToString () const
{
    ostringstream ostr;
    ostr << m_first->ToParenthesisString () << " ? "
	 << m_second->ToParenthesisString () << " : "
	 << m_third->ToParenthesisString ();
    return ostr.str ();
}
