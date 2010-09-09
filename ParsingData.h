/**
 * @file   ParsingData.h
 * @author Dan R. Lipsa
 *
 * Declaration of ParsingData
 */
#ifndef __PARSING_DATA_H__
#define __PARSING_DATA_H__

#include "Comparisons.h"
#include "ParsingDriver.h"

class AttributesInfo;

/**
 * Stores data used during  the parsing such as identifiers, variables
 * and functions.
 */
class ParsingData : public ParsingDriver
{
public:
    /**
     * How are variables stored
     */
    typedef map<const char*, double, LessThanNoCase> Variables;
    /**
     * A unary function
     */
    //typedef double (*UnaryFunction)(double);
    typedef boost::function<double (double)> UnaryFunction;
    /**
     * A binary function
     */
    typedef boost::function<double (double, double)> BinaryFunction;
    /**
     * How are unary functions stored.
     */
    typedef map<const char*, UnaryFunction, 
		LessThanNoCase> UnaryFunctions;
    /**
     * How are binary functions stored
     */
    typedef map<const char*, BinaryFunction, 
                     LessThanNoCase> BinaryFunctions;
    /**
     * How are identifiers stored
     */
    typedef map<const char*, string*, LessThanNoCase> Identifiers;
    typedef vector< boost::shared_ptr<Vertex> > Vertices;
    typedef vector< boost::shared_ptr<Edge> > Edges;
    typedef vector< boost::shared_ptr<Face> > Faces;

public:
    /**
     * Constructs a ParsingData object
     */
    ParsingData ();
    /**
     * Destructs this object
     */
    ~ParsingData ();
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
    /**
     * Retrieves a variable value
     * @param name variable name
     * @return variable value
     */
    double GetVariableValue (const char* name);
    /**
     * Stores a string from the lexer for later use in the parser
     * @param id string from the lexer
     * @return a string pointer which is stored in ParsingData object
     */
    string* CreateIdentifier(const char* id);
    /**
     * Returns the unary function with the name supplied by the parameter
     * @param name name of the function to be retrieved
     * @return a unary function
     */
    UnaryFunction GetUnaryFunction (const char* name);
    /**
     * Returns the binary function with the name supplied by the parameter
     * @param name name of the function to be retrieved
     * @return a binary function
     */
    BinaryFunction GetBinaryFunction (const char* name);
    /**
     * Used for  profiling. Prints to  the debug stream  a description
     * and the time since the last checkpoint.  
     * @param description what should be printed together with the time
     */
    bool IsAttribute (const char* s)
    {
	return m_attributes.find (s) != m_attributes.end ();
    }
    void AddAttribute (const char* s)
    {
	m_attributes.insert (s);
    }
    bool IsMethodOrQuantity (const char* s)
    {
	bool result = m_methodOrQuantity.find (s) != m_methodOrQuantity.end ();
	return result;
    }
    void AddMethodOrQuantity (const char* s)
    {
	m_methodOrQuantity.insert (s);
    }

    /**
     * Gets the vector of vertices
     * @return the vector of vertices
     */
    const Vertices& GetVertices () const
    {
	return m_vertices;
    }
    Vertices& GetVertices ()
    {
	return m_vertices;
    }
    /**
     * Stores a Vertex object a certain index in the Foam object
     * @param i where to store the Vertex object
     * @param x coordinate X of the Vertex object
     * @param y coordinate Y of the Vertex object
     * @param z coordinate Z of the Vertex object
     * @param attributes the list of attributes for the vertex
     */
    void SetVertex (size_t i, double x, double y, double z,
		    vector<NameSemanticValue*>& attributes,
		    const AttributesInfo& attributesInfo);
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
    /**
     * Stores an Edge object in the Foam object at a certain index
     * @param i index where to store the Edge object
     * @param begin index of the first Point that determines the edge
     * @param end index of the last Point that determines the edge
     * @param attributes the list of attributes for this edge
     */
    void SetEdge (size_t i, size_t begin, size_t end, size_t middle,
		  G3D::Vector3int16& endTranslation,
                  vector<NameSemanticValue*>& attributes,
		  const AttributesInfo& attributesInfo,
		  bool isQuadratic);
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
     * Stores a Face object in the Foam object 
     * 
     * @param i index where to store the Face object
     * @param edges  vector of  edges that form  the face. An  edge is
     *        specified using an index of the edge that should already
     *        be stored in the Foam  object. If the index is negative,
     *        the edge part of the  Face is in reversed order than the
     *        Edge that is stored in the Foam object.
     * @param attributes the list of attributes for the face
     */
    void SetFace (size_t i,  vector<int>& edges,
                  vector<NameSemanticValue*>& attributes,
		  const AttributesInfo& attributesInfo);
    void SetSpaceSignificant (bool spaceSignificant)
    {
	m_spaceSignificant = spaceSignificant;
    }
    bool IsSpaceSignificant () const
    {
	return m_spaceSignificant && m_parenthesisCount == 0;
    }
    void OpenParenthesis ()
    {
	++m_parenthesisCount;
    }
    void CloseParenthesis ()
    {
	--m_parenthesisCount;
    }

public:
    /**
     * Pretty prints the ParsingData object
     * @param ostr output stream where to print the object
     * @param pd object to be printed
     */    
    friend ostream& operator<< (ostream& ostr, ParsingData& pd);

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
    set<const char*, LessThanNoCase> m_attributes;
    set<const char*, LessThanNoCase> m_methodOrQuantity;
    bool m_spaceSignificant;
    size_t m_parenthesisCount;

private:
    static const char* IMPLEMENTED_METHODS[];
};

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
