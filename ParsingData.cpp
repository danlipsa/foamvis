/**
 * @file   ParsingData.cpp
 * @author Dan R. Lipsa
 *
 * Definition of the ParsingData class
 */

#include "Debug.h"
#include "DebugStream.h"
#include "Edge.h"
#include "ExpressionTree.h"
#include "Face.h"
#include "ParsingData.h"
#include "QuadraticEdge.h"
#include "Vertex.h"

// Private Classes
// ======================================================================

/**
 * Pretty prints a variable. Used by a for_each algorithm.
 */
struct printVariable
{
    /**
     * Constructs a printVariable object
     * @param ostr stream where the variable is printed
     */
    printVariable (ostream& ostr) : m_ostr(ostr) {}
    /**
     * Pretty prints a variable
     * @param nameValue a name-value pair
     */
    void operator () (pair<string, double> nameValue)
    {
        m_ostr << nameValue.first << ": " << nameValue.second << endl;
    }

private:
    /**
     * Stream where the variable will be printed.
     */
    ostream& m_ostr;
};


// Private functions
// ======================================================================


/**
 * Deletes an identifier
 * @param  pair  this  is  how  an identifier  is  stored  in  the
 * ParsingData object. We delete the string* part.
 */
inline void deleteIdentifier (pair<const char*, string*> pair)
{
    delete pair.second;
}


const char* ParsingData::IMPLEMENTED_METHODS[] = 
{
    "edge_area",
    "vertex_scalar_integral",
    "facet_general_integral"
};


// Methods
// ======================================================================
ParsingData::ParsingData () :
    m_spaceSignificant (false),
    m_parenthesisCount (0),
    m_newLineSignificant (false)
{
    BinaryFunctionInformation BINARY_FUNCTION_INFORMATION[] = 
    {
	{"+", plus<double> ()},
	{"-", minus<double> ()},
	{"*", multiplies<double> ()},
	{"/", divides<double> ()},
	{"^", powf},
	{"=", minus<double> ()}, // an equation is the same a a function = 0
	{"atan2", atan2f},
	{">", greater<double> ()},
	{">=", greater_equal<double> ()},
	{"<", less<double> ()},
	{"<=", less_equal<double> ()},
	{"&&", logical_and<bool> ()}
    };
    UnaryFunctionInformation UNARY_FUNCTION_INFORMATION[] =
    {
	{"-", negate<double> ()},
	{"sqrt", sqrtf},
	{"cos", cosf},
	{"sin", sinf}
    };

    BOOST_FOREACH (BinaryFunctionInformation bfi, BINARY_FUNCTION_INFORMATION)
	m_binaryFunctions[bfi.m_name] = bfi.m_function;
    BOOST_FOREACH (UnaryFunctionInformation ufi, UNARY_FUNCTION_INFORMATION)
	m_unaryFunctions[ufi.m_name] = ufi.m_function;

    BOOST_FOREACH (const char* method, IMPLEMENTED_METHODS)
	AddMethodOrQuantity (method);
}

ParsingData::~ParsingData ()
{
    for_each (m_constraints.begin (), m_constraints.end (), 
	      bl::delete_ptr ());
}

double ParsingData::GetVariableValue (const char* id) 
{
    Variables::iterator it = m_variables.find (id);
    RuntimeAssert (it != m_variables.end (), "Undeclared variable: ", id);
    return it->second;
}

bool ParsingData::IsVariableSet (const char* id)
{
    Variables::iterator it = m_variables.find (id);
    return it != m_variables.end ();
}

ParsingData::UnaryFunction ParsingData::GetUnaryFunction (const char* name)
{
    UnaryFunctions::iterator it = m_unaryFunctions.find (name);
    RuntimeAssert (it != m_unaryFunctions.end (),
		   "Invalid unary function name: ", name);
    return it->second;
}

ParsingData::BinaryFunction ParsingData::GetBinaryFunction (const char* name)
{
    BinaryFunctions::iterator it = m_binaryFunctions.find (name);
    RuntimeAssert (it != m_binaryFunctions.end (),
		   "Invalid binary function name: ", name);
    return it->second;
}



const char* ParsingData::CreateIdentifier(const char* id)
{
    pair<Identifiers::iterator, bool> p = m_identifiers.insert (id);
    return p.first->c_str ();
}

void ParsingData::SetVertex (size_t i, double x, double y, double z,
			     vector<NameSemanticValue*>& attributes,
			     const AttributesInfo& attributesInfo) 
{
    if (i >= m_vertices.size ())
        m_vertices.resize (i + 1);
    boost::shared_ptr<Vertex> vertex = boost::make_shared<Vertex> (x, y ,z, i);
    if (&attributes != 0)
        vertex->StoreAttributes (attributes, attributesInfo);
    m_vertices[i] = vertex;
}

void ParsingData::SetEdge (size_t i,
			   size_t begin, size_t end, size_t middle,
			   G3D::Vector3int16& endTranslation,
			   vector<NameSemanticValue*>& attributes,
			   const AttributesInfo& attributesInfo,
			   bool isQuadratic)
{
    if (i >= m_edges.size ())
        m_edges.resize (i + 1);
    boost::shared_ptr<Edge> edge;
    if (isQuadratic)
	edge.reset (new QuadraticEdge (
			GetVertex(begin), GetVertex(end), GetVertex (middle), 
			endTranslation, i));
    else
	edge = boost::make_shared<Edge> (
	    GetVertex(begin), GetVertex(end), endTranslation, i);
    if (&attributes != 0)
        edge->StoreAttributes (attributes, attributesInfo);
    m_edges[i] = edge;
}

void ParsingData::SetFace (size_t i,  vector<int>& edges,
			   vector<NameSemanticValue*>& attributes,
			   const AttributesInfo& attributesInfo)
{
    if (i >= m_faces.size ())
        m_faces.resize (i + 1);
    boost::shared_ptr<Face> face = boost::make_shared<Face> (edges, m_edges, i);
    if (&attributes != 0)
        face->StoreAttributes (attributes, attributesInfo);
    m_faces[i] = face;
}

void ParsingData::SetConstraint (size_t i, ExpressionTree* constraint)
{
    if (i >= m_constraints.size ())
	m_constraints.resize (i+1);
    m_constraints[i] = constraint;
}

string ParsingData::ToString () const
{
    ostringstream ostr;
    ostr << "Variables: " << endl;
    for_each (m_variables.begin (), m_variables.end (),
              printVariable (ostr));
    return ostr.str ();
}

void ParsingData::UnsetVariable (const char* name)
{
    Variables::iterator it = m_variables.find (name);
    if (it != m_variables.end ())
	m_variables.erase (it);
}
