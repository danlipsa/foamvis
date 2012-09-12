/**
 * @file   ExpressionTree.h
 * @author Dan R. Lipsa
 *
 * Nodes used in an expression tree built in the parser
 */
#ifndef __EXPRESSION_TREE_H__
#define __EXPRESSION_TREE_H__

class ParsingData;

/**
 * Base  class for  all  types of  nodes  in an  expression tree.   An
 * expression tree is a tree with operators (or functions) as internal
 * nodes and operands (numbers and variables) as leafs.
 */

class ExpressionTreeType
{
public:
    enum Enum
    {
	NUMBER,
	VARIABLE,
	ARRAY_ELEMENT,
	UNARY_FUNCTION,
	BINARY_FUNCTION,
	CONDITIONAL
    };
};

class ExpressionTree
{
public:
    /**
     * Constructs an ExpressionTree
     */
    ExpressionTree (ParsingData& parsingData)

        : m_parsingData (parsingData)
    {}
    virtual ~ExpressionTree () 
    {}

    ParsingData& GetParsingData ()
    {
	return m_parsingData;
    }
    /**
     * Calculates and returns the  value of the expression represented
     * by the tree
     * @return the value of the tree
     */
    virtual double Value () const = 0;
    virtual size_t Height () const
    {
	return 0;
    }
    virtual ExpressionTreeType::Enum GetType () const = 0;
    virtual ExpressionTree* GetSimplifiedTree () const = 0;
    virtual string ToString () const = 0;
    virtual bool HasConditional ()
    {
	return false;
    }

    string ToParenthesisString () const;
    bool IsProperBinaryFunction () const;

protected:
    ParsingData& m_parsingData;
};

/**
 * A number node  in an expression tree.
 */
class ExpressionTreeNumber : public ExpressionTree
{
public:
    /**
     * Constructs a number node
     */
    ExpressionTreeNumber (ParsingData& parsingData, double value) : 
	ExpressionTree (parsingData),
	m_value (value) 
    {}
    /**
     * Value of the number
     * @return value of the number
     */
    virtual double Value () const;
    virtual ExpressionTree* GetSimplifiedTree () const;
    virtual ExpressionTreeType::Enum GetType () const 
    {
	return ExpressionTreeType::NUMBER;
    }
    virtual string ToString () const;
private:
    /**
     * Value of the nubmber node
     */
    double m_value;
};

/**
 * A variable node in an expression tree
 */
class ExpressionTreeVariable : public ExpressionTree
{
public:
    /**
     * Constructs a variable node
     * @param name the name of the variable
     * @param parsingData data which allows us to get the value of a variable.
     */
    ExpressionTreeVariable (ParsingData& parsingData, const char* name);
    /**
     * Value of the variable
     * @return the value of the variable
     */
    virtual double Value () const;
    virtual ExpressionTree* GetSimplifiedTree () const;
    virtual ExpressionTreeType::Enum GetType () const 
    {
	return ExpressionTreeType::VARIABLE;
    }
    virtual string ToString () const;
private:
    /**
     * Variable name
     */
    string m_name;
};


/**
 * An array element node in an expression tree
 */
class ExpressionTreeArrayElement : public ExpressionTree
{
public:
    ExpressionTreeArrayElement (ParsingData& parsingData, 
				const char* name, const vector<size_t>& index) :

	ExpressionTree (parsingData),
        m_name (name),
	m_index (index)
    {}
    /**
     * Value of the variable
     * @return the value of the variable
     */
    virtual double Value () const;
    virtual ExpressionTree* GetSimplifiedTree () const;
    virtual ExpressionTreeType::Enum GetType () const 
    {
	return ExpressionTreeType::ARRAY_ELEMENT;
    }
    virtual string ToString () const;
private:
    /**
     * Variable name
     */
    string m_name;
    vector<size_t> m_index;
};





/**
 * An unary function or unary operator expression tree node.
 */
class ExpressionTreeUnaryFunction : public ExpressionTree
{
public:
    /**
     * Constructs an unary function tree node
     * @param name name of the unary function
     * @param param expression tree parameter to the function
     * @param parsingData  allows us  to get the  function associated
     * with the function name
     */
    ExpressionTreeUnaryFunction (
        ParsingData& parsingData, const char* name, ExpressionTree* param)
        : ExpressionTree (parsingData), 
	  m_name (name), m_param (param)
    {
    }
    /**
     * Value of the function applied to the parameter.
     * @return the value of the function applied to the parameter.
     */
    virtual double Value () const;
    virtual size_t Height () const
    {
	return 1 + m_param->Height ();
    }
    virtual ExpressionTree* GetSimplifiedTree () const;
    virtual ExpressionTreeType::Enum GetType () const 
    {
	return ExpressionTreeType::UNARY_FUNCTION;
    }
    virtual string ToString () const;
    virtual bool HasConditional ()
    {
	return m_param->HasConditional ();
    }

private:
    /**
     * Name of the function
     */
    string m_name;
    boost::shared_ptr<ExpressionTree> m_param;
};

/**
 * A binary function or binary operator expression tree node
 */
class ExpressionTreeBinaryFunction : public ExpressionTree
{
public:
    /**
     * Constructs a binary function tree node
     * @param  parsingData allows  us to  get the  function associated
     * with the function name
     * @param name name of the function
     * @param first child of the tree node
     * @param second second child of the tree node
     */
    ExpressionTreeBinaryFunction (
	ParsingData& parsingData,
        const char* name, 
        ExpressionTree* first, ExpressionTree* second)
        : ExpressionTree (parsingData), m_name (name),
	  m_first (first), m_second (second)
    {
    }
    /**
     * Value of the function applied to the parameters
     * @return the value of the function applied to the parameters.
     */
    virtual double Value () const;
    virtual size_t Height () const
    {
	return 1 + max (m_first->Height (), m_second->Height ());
    }
    virtual ExpressionTree* GetSimplifiedTree () const;
    virtual ExpressionTreeType::Enum GetType () const 
    {
	return ExpressionTreeType::BINARY_FUNCTION;
    }
    virtual string ToString () const;
    virtual bool HasConditional ()
    {
	return m_first->HasConditional () || m_second->HasConditional ();
    }
    bool IsOperator () const;
private:
    /**
     * Function name
     */
    string m_name;
    boost::shared_ptr<ExpressionTree> m_first;
    boost::shared_ptr<ExpressionTree> m_second;
};


/**
 * A conditional expression `first ? second : third'
 */
class ExpressionTreeConditional : public ExpressionTree
{
public:
    /**
     * Constructs a binary function tree node
     */
    ExpressionTreeConditional (
	ParsingData& parsingData,
        ExpressionTree* first, ExpressionTree* second, ExpressionTree* third)
        : ExpressionTree (parsingData),
	  m_first (first), m_second (second), m_third (third)
    {
    }
    /**
     * Value of the contitional expression
     */
    virtual double Value () const;
    virtual size_t Height () const
    {
	return 1 + max (max (m_first->Height (), m_second->Height ()), 
		 m_third->Height ());
    }
    virtual ExpressionTree* GetSimplifiedTree () const;
    virtual ExpressionTreeType::Enum GetType () const 
    {
	return ExpressionTreeType::CONDITIONAL;
    }
    virtual string ToString () const;
    virtual bool HasConditional ()
    {
	return true;
    }
private:
    boost::shared_ptr<ExpressionTree> m_first;
    boost::shared_ptr<ExpressionTree> m_second;
    boost::shared_ptr<ExpressionTree> m_third;
};


inline ostream& operator<< (ostream& ostr, const ExpressionTree& t)
{
    return ostr << t.ToString ();
}


#endif //__EXPRESSION_TREE_H__

// Local Variables:
// mode: c++
// End:
