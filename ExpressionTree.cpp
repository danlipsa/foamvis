/**
 * @file   ExpressionTree.cpp
 * @author Dan R. Lipsa
 *
 * Member definitions  for nodes used  in an expression tree  built in
 * the parser
 * @todo cache an iterator to avoid looking up a name for every access of a 
 *       variable, array, function and so on.
 */
#include "Debug.h"
#include "DebugStream.h"
#include "ExpressionTree.h"
#include "ParsingData.h"

string ExpressionTree::ToParenthesisString ()
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
double ExpressionTreeNumber::Value (void)
{
    return m_value;
}

ExpressionTree* ExpressionTreeNumber::GetSimplifiedTree ()
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
ExpressionTreeVariable::ExpressionTreeVariable (
    const ParsingData& parsingData, const char* name) :
    
    ExpressionTree (parsingData),
    m_name (name),
    m_it (parsingData.GetVariableItEnd ())
{
}

double ExpressionTreeVariable::Value (void)
{
    if (m_it == m_parsingData.GetVariableItEnd ())
    {
        m_it = m_parsingData.GetVariableIt (m_name);
        RuntimeAssert (m_it != m_parsingData.GetVariableItEnd (), 
                       "Undeclared variable: ", m_name);
    }
    return m_parsingData.GetVariableValue (m_it);
}

ExpressionTree* ExpressionTreeVariable::GetSimplifiedTree ()
{
    if (! IsCoordinate () && m_parsingData.IsVariableSet (m_name))
	return new ExpressionTreeNumber (m_parsingData, Value ());
    else
	return new ExpressionTreeVariable (m_parsingData, m_name.c_str ());
}

string ExpressionTreeVariable::ToString ()
{
    ostringstream ostr;
    if (m_parsingData.IsVariableSet (m_name))
	ostr << Value ();
    else
	ostr << m_name;
    return ostr.str ();
}

bool ExpressionTreeVariable::IsCoordinate () const
{
    return m_name == "x" || m_name == "y" || m_name == "z";
}

// ExpressionTreeArrayElement
// ======================================================================
double ExpressionTreeArrayElement::Value (void)
{
    return m_parsingData.GetArrayValue (m_name, m_index);
}

ExpressionTree* ExpressionTreeArrayElement::GetSimplifiedTree ()
{
    return new ExpressionTreeNumber (m_parsingData, Value ());
}

string ExpressionTreeArrayElement::ToString ()
{
    ostringstream ostr;
    ostr << Value ();
    return ostr.str ();
}


// ExpressionTreeUnaryFunction
// ======================================================================
double ExpressionTreeUnaryFunction::Value (void)
{
    double value = m_param->Value ();
    ParsingData::UnaryFunction f = m_parsingData.GetUnaryFunction (m_name);
    return f (value);
}

ExpressionTree* ExpressionTreeUnaryFunction::GetSimplifiedTree ()
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

string ExpressionTreeUnaryFunction::ToString ()
{
    ostringstream ostr;
    ostr << m_name << "(" << m_param->ToString () << ")";
    return ostr.str ();
}

// ExpressionTreeBinaryFunction
// ======================================================================
double ExpressionTreeBinaryFunction::Value (void)
{
    double second = m_second->Value ();
    double first = m_first->Value ();
    ParsingData::BinaryFunction f = m_parsingData.GetBinaryFunction (m_name);
    return f (first, second);
}

ExpressionTree* ExpressionTreeBinaryFunction::GetSimplifiedTree ()
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


string ExpressionTreeBinaryFunction::ToString ()
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
double ExpressionTreeConditional::Value (void)
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

ExpressionTree* ExpressionTreeConditional::GetSimplifiedTree ()
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

string ExpressionTreeConditional::ToString ()
{
    ostringstream ostr;
    ostr << m_first->ToParenthesisString () << " ? "
	 << m_second->ToParenthesisString () << " : "
	 << m_third->ToParenthesisString ();
    return ostr.str ();
}
