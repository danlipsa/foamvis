/**
 * @file   ParsingData.h
 * @author Dan R. Lipsa
 *
 * Declaration of ParsingData
 */
#ifndef __PARSING_DATA_H__
#define __PARSING_DATA_H__

#include "ObjectPosition.h"
#include "Comparisons.h"
#include "DataProperties.h"
#include "ForceOneObject.h"
#include "ParsingDriver.h"

class AttributesInfo;
class ExpressionTree;
class AttributeArrayAttribute;

/**
 * Stores data used during  the parsing such as identifiers, variables
 * and functions.
 */
class ParsingData : public ParsingDriver
{
public:
    /**
     * Variable type
     */
    typedef map<string, double, LessThanNoCase> Variables;
    /**
     * Note this iterator is not invalidated by inserting or deleting elements
     * in the map.
     */
    typedef Variables::const_iterator VariableIt;


    typedef map<string, 
		boost::shared_ptr<AttributeArrayAttribute>, 
		LessThanNoCase> Arrays;
    typedef Arrays::const_iterator ArrayIt;


    /**
     * Unary function type
     */
    typedef boost::function<double (double)> UnaryFunction;
    /**
     * Binary function type
     */
    typedef boost::function<double (double, double)> BinaryFunction;
    /**
     * Unary functions type
     */
    typedef map<string, UnaryFunction, LessThanNoCase> UnaryFunctions;
    typedef UnaryFunctions::const_iterator UnaryFunctionIt;

    /**
     * Binary functions type
     */
    typedef map<string, BinaryFunction, LessThanNoCase> BinaryFunctions;
    typedef BinaryFunctions::const_iterator BinaryFunctionIt;
    /**
     * Identifiers type
     */
    typedef set<string, LessThanNoCase> Identifiers;
    typedef vector< boost::shared_ptr<Vertex> > Vertices;
    typedef vector< boost::shared_ptr<Edge> > Edges;
    typedef vector< boost::shared_ptr<Face> > Faces;
    typedef vector< boost::shared_ptr<ExpressionTree> > Constraints;

public:
    /**
     * Constructs a ParsingData object
     */
    ParsingData (bool useOriginal, 
		 const DmpObjectInfo& dmpObjectInfo,
		 const vector<ForceNamesOneObject>& forcesNames);

    void AddAttribute (const char* s)
    {
	m_attributes.insert (s);
    }

    void CloseParenthesis ()
    {
	--m_parenthesisCount;
    }
    /**
     * Stores a string from the lexer for later use in the parser
     * @param id string from the lexer
     * @return a string pointer which is stored in ParsingData object
     */
    const char* CreateIdentifier(const char* id);


    /**
     * Gets all faces from this Foam
     */
    Faces& GetFaces () 
    {
	return m_faces;
    }
    const Faces& GetFaces () const
    {
	return m_faces;
    }

    boost::shared_ptr<Face>  GetFace (size_t i)
    {
	return m_faces[i];
    }
    
    /**
     * @{
     * @name Variables
     */
    VariableIt GetVariableIt (const char* name) const;
    VariableIt GetVariableIt (const string& name) const
    {
        return GetVariableIt (name.c_str ());
    }
    VariableIt GetVariableItEnd () const
    {
        return m_variables.end ();
    }
    double GetVariableValue (const char* name) const;
    double GetVariableValue (VariableIt it) const;
    double GetVariableValue (const string& name) const
    {
	return GetVariableValue (name.c_str ());
    }
    bool IsVariableSet (const char* name) const;
    bool IsVariableSet (const string& name) const
    {
	return IsVariableSet (name.c_str ());
    }
    // @}



    /**
     * @{
     * @name Arrays
     */
    ArrayIt GetArrayIt (const char* name) const;
    ArrayIt GetArrayIt (const string& name) const
    {
        return GetArrayIt (name.c_str ());
    }
    ArrayIt GetArrayItEnd () const
    {
        return m_arrays.end ();
    }
    double GetArrayValue (const char* name, const vector<size_t>& index) const;
    double GetArrayValue (const string& name, const vector<size_t>& index) const
    {
        return GetArrayValue (name.c_str (), index);
    }
    double GetArrayValue (ArrayIt, const vector<size_t>& index) const;
    // @}

    /**
     * @{
     * @name UnaryFunction
     */
    UnaryFunction GetUnaryFunction (const char* name) const;
    UnaryFunction GetUnaryFunction (const string& name) const
    {
	return GetUnaryFunction (name.c_str ());
    }
    UnaryFunctionIt GetUnaryFunctionIt (const char* name) const;
    UnaryFunctionIt GetUnaryFunctionIt (const string& name) const
    {
        return GetUnaryFunctionIt (name.c_str ());
    }    
    UnaryFunctionIt GetUnaryFunctionItEnd () const
    {
        return m_unaryFunctions.end ();
    }
    UnaryFunction GetUnaryFunction (UnaryFunctionIt it) const;
    //@

    /**
     * @{
     * @name BinaryFunction
     */
    BinaryFunction GetBinaryFunction (const char* name) const;
    BinaryFunction GetBinaryFunction (const string& name) const
    {
	return GetBinaryFunction (name.c_str ());
    }
    BinaryFunction GetBinaryFunction (BinaryFunctionIt it) const;

    BinaryFunctionIt GetBinaryFunctionIt (const char* name) const;
    BinaryFunctionIt GetBinaryFunctionIt (const string& name) const
    {
        return GetBinaryFunctionIt (name.c_str ());
    }
    BinaryFunctionIt GetBinaryFunctionItEnd () const
    {
        return m_binaryFunctions.end ();
    }
    bool IsOperator (const char* name) const
    {
	return OPERATORS.find (name) != OPERATORS.end ();
    }
    bool IsOperator (const string& name) const
    {
	return IsOperator (name.c_str ());
    }
    // @}

    
    /**
     * Gets the vector of vertices
     * @return the vector of vertices
     */
    const Vertices& GetVertices () const
    {
	return m_vertices;
    }
    /**
     * Gets a Vertex from the Foam object
     * @param i index where the Vertex object is stored
     * @return a pointer to the Vertex object
     */
    boost::shared_ptr<Vertex> GetVertex (int i) 
    {
	return m_vertices[i];
    }
    /**
     * Returns all edges from this Foam
     * @return a vector of Edge pointers
     */
    Edges& GetEdges () 
    {
	return m_edges;
    }
    Vertices& GetVertices ()
    {
	return m_vertices;
    }
    bool IsAttribute (const char* s)
    {
	return m_attributes.find (s) != m_attributes.end ();
    }
    bool IsSpaceSignificant () const
    {
	return m_spaceSignificant && m_parenthesisCount == 0;
    }
    bool IsNewLineSignificant () const
    {
	return m_newLineSignificant;
    }
    void AddMethodOrQuantity (const char* s)
    {
	m_methodOrQuantity.insert (s);
    }
    bool IsMethodOrQuantity (const char* s)
    {
	bool result = m_methodOrQuantity.find (s) != m_methodOrQuantity.end ();
	return result;
    }
    void OpenParenthesis ()
    {
	++m_parenthesisCount;
    }
    /**
     * Stores a Vertex object a certain index in the Foam object
     */
    void SetVertex (size_t i, double x, double y, double z,
		    vector<NameSemanticValue*>& attributes,
		    const AttributesInfo& attributesInfo);
    /**
     * Stores a variable name and value in the ParsingData object
     * @param id  variable name. It  is allocated by the  lexer using
     *         CreateIdentifier and it is dealocated by the ParsingData object
     * @param value variable value
     */
    void SetVariable (const char* id, double value) 
    {
	m_variables[id] = value;
    }
    void SetArray (const char* id, AttributeArrayAttribute* array);
    void SetVariable (const string& id, double value)
    {
	SetVariable (id.c_str (), value);
    }

    /**
     * Stores an Edge object in the Foam object at a certain index
     */
    void SetEdge (size_t i, size_t begin, size_t end, size_t middle,
		  G3D::Vector3int16& endTranslation,
                  vector<NameSemanticValue*>& attributes,
		  const AttributesInfo& attributesInfo,
		  bool isQuadratic);
    /**
     * Stores a Face object in the Foam object 
     */
    void SetFace (size_t i,  vector<int>& edges,
                  vector<NameSemanticValue*>& attributes,
		  const AttributesInfo& attributesInfo);

    void SetConstraint (size_t i, ExpressionTree* constraint);
    boost::shared_ptr<ExpressionTree> GetConstraint (size_t i) const
    {
	return m_constraints[i];
    }

    bool OriginalUsed () const
    {
	return m_useOriginal;
    }

    const DmpObjectInfo& GetDmpObjectInfo () const
    {
	return m_dmpObjectInfo;
    }
    const vector<ForceNamesOneObject>& GetForcesNames () const
    {
	return m_forceNames;
    }

    /**
     * Gets the T1s stored in the DMP between this step and the
     * previous one. Returns false if the arrayName and countNames
     * variables are not set, true otherwise.
     */
    bool GetT1 (
        const char* arrayName, const char* countName, 
        vector<T1>* t1s, bool is2D) const;

    /**
     * @{
     * @name Scanner instructions
     */
    void SetSpaceSignificant (bool spaceSignificant)
    {
	m_spaceSignificant = spaceSignificant;
    }
    void SetNewLineSignificant (bool newLineSignificant)
    {
	m_newLineSignificant = newLineSignificant;
    }    
    bool KeywordsIgnored () const
    {
        return m_keywordsIgnored;
    }
    void SetKeywordsIgnored (bool ignored)
    {
        m_keywordsIgnored = ignored;
    }
    // @}

public:
    string ToString () const;

private:
    struct BinaryFunctionInformation
    {
	const char* m_name;
	BinaryFunction m_function;
    };
    struct UnaryFunctionInformation
    {
	const char* m_name;
	UnaryFunction m_function;
    };




private:
    /**
     * A vector of points
     */
    Vertices m_vertices;
    /**
     * A vector of edges
     */
    Edges m_edges;
    /**
     * A vector of faces
     */
    Faces m_faces;
    /**
     * Stores  variables  read   from  the  datafile  (declared  using
     * PARAMETER keyword in the Evolver DMP file)
     */
    Variables m_variables;
    Arrays m_arrays;
    /**
     * Unary functions
     */
    UnaryFunctions m_unaryFunctions;
    /**
     * Binary functions
     */
    BinaryFunctions m_binaryFunctions;
    /**
     * Identifiers
     */
    Identifiers m_identifiers;
    set<string, LessThanNoCase> m_attributes;
    set<string, LessThanNoCase> m_methodOrQuantity;
    Constraints m_constraints;
    bool m_spaceSignificant;
    size_t m_parenthesisCount;
    bool m_newLineSignificant;
    bool m_useOriginal;
    DmpObjectInfo m_dmpObjectInfo;
    vector<ForceNamesOneObject> m_forceNames;
    bool m_keywordsIgnored;

private:
    static const char* IMPLEMENTED_METHODS[];
    static const set<string> OPERATORS;
};

/**
 * Pretty prints the ParsingData object
 * @param ostr output stream where to print the object
 * @param d object to be printed
 */    
inline ostream& operator<< (ostream& ostr, const ParsingData& d)
{
    return ostr << d.ToString ();
}



/**
 * Stores semantic values
 */
typedef EvolverData::parser::semantic_type YYSTYPE;
/**
 * Stores location in the parsed file
 */
typedef EvolverData::parser::location_type YYLTYPE;
/**
 * Prototype for lexing function
 */
# define YY_DECL int \
    EvolverDatalex (YYSTYPE* yylval_param, YYLTYPE* yylloc_param, \
           void* yyscanner)
/**
 * The lexing function
 * @param yylval_param where to store the value read by the lexer
 * @param yylloc_param where to store the location in the file
 * @param yyscanner opaque object for the scanner
 * @return 0 for success, different then 0 otherwise
 */
YY_DECL;

#endif //__PARSING_DATA_H__

// Local Variables:
// mode: c++
// End:
