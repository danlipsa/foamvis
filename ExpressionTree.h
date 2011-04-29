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
class ExpressionTree
{
public:
    /**
     * Constructs an ExpressionTree with one node
     */
    ExpressionTree () : m_first (0), m_second (0) {}
    virtual ~ExpressionTree () {}
    /**
     * Constructs an ExpressionTree
     * @param left the left child of the current node
     * @param second the second child of the current node
     */
    ExpressionTree (ExpressionTree* first, ExpressionTree* second = 0,
		    ExpressionTree* third = 0)
        : m_first (first), m_second (second), m_third (third) {}
    /**
     * Deletes all nodes in an ExpressionTree
     * @param node tree to be deleted
     */
    static void Delete (ExpressionTree* node);
    /**
     * Calculates and returns the  value of the expression represented
     * by the tree
     * @return the value of the tree
     */
    virtual double Value (void) = 0;
protected:
    /**
     * First child of this node
     */
    ExpressionTree* m_first;
    /**
     * Second child of this node
     */
    ExpressionTree* m_second;
    ExpressionTree* m_third;
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
    ExpressionTreeNumber (double value) : m_value (value) {}
    /**
     * Value of the number
     * @return value of the number
     */
    virtual double Value (void);
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
    ExpressionTreeVariable (const string* name, ParsingData& parsingData)
        : m_name (name->c_str()), m_parsingData (parsingData) {}
    /**
     * Value of the variable
     * @return the value of the variable
     */
    virtual double Value (void);
private:
    /**
     * Variable name
     */
    const char* m_name;
    /**
     * Allows us to get variable values
     */
    ParsingData& m_parsingData;
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
        const string* name, ExpressionTree* param, ParsingData& parsingData)
        : ExpressionTree (param), m_name (name->c_str()), 
          m_parsingData (parsingData) {}
    /**
     * Value of the function applied to the parameter.
     * @return the value of the function applied to the parameter.
     */
    virtual double Value (void);
private:
    /**
     * Name of the function
     */
    const char* m_name;
    /**
     * Allows us to get the function associated with the function name.
     */
    ParsingData& m_parsingData;
};

/**
 * A binary function or binary operator expression tree node
 */
class ExpressionTreeBinaryFunction : public ExpressionTree
{
public:
    /**
     * Constructs a binary function tree node
     * @param name name of the function
     * @param first child of the tree node
     * @param second second child of the tree node
     * @param  parsingData allows  us to  get the  function associated
     * with the function name
     */
    ExpressionTreeBinaryFunction (
        const string* name, 
        ExpressionTree* first, ExpressionTree* second, ParsingData& parsingData)
        : ExpressionTree (first, second), m_name (name->c_str()),
          m_parsingData (parsingData) {}
    /**
     * Value of the function applied to the parameters
     * @return the value of the function applied to the parameters.
     */
    virtual double Value (void);
private:
    /**
     * Function name
     */
    const char* m_name;
    /**
     * Allows us to get the function associated with the function name.
     */
    ParsingData& m_parsingData;
};


/**
 * A conditional expression `first ? second : third'
 */
class ExpressionTreeConditional : public ExpressionTree
{
public:
    /**
     * Constructs a binary function tree node
     * @param name name of the function
     * @param first child of the tree node
     * @param second second child of the tree node
     * @param  parsingData allows  us to  get the  function associated
     * with the function name
     */
    ExpressionTreeConditional (
        ExpressionTree* first, ExpressionTree* second, ExpressionTree* third)
        : ExpressionTree (first, second, third)
    {
    }
    /**
     * Value of the contitional expression
     */
    virtual double Value (void);
};


#endif //__EXPRESSION_TREE_H__

// Local Variables:
// mode: c++
// End:
