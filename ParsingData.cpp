/**
 * @file   ParsingData.cpp
 * @author Dan R. Lipsa
 *
 * Definition of the ParsingData class
 */

#include "Debug.h"
#include "DebugStream.h"
#include "Edge.h"
#include "Face.h"
#include "ParsingData.h"
#include "QuadraticEdge.h"
#include "Vertex.h"

// Private Classes
// ======================================================================

/**
 * Pretty prints a variable. Used by a for_each algorithm.
 */
struct printVariable : public unary_function<pair<const char*, double>, void>
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
    void operator() (pair<const char*, double> nameValue)
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
 * Throws  an  exception  because  we  should  not  have
 * assignments in constant expressions.
 * @return it throws an exception before returning.
 */
static double assignmentFunction (double, double)
{
    throw logic_error ("Assignment operation in constant expression");
}

/**
 * Deletes an identifier
 * @param  pair  this  is  how  an identifier  is  stored  in  the
 * ParsingData object. We delete the string* part.
 */
inline void deleteIdentifier (pair<const char*, string*> pair)
{
    delete pair.second;
}


ostream& operator<< (ostream& ostr, ParsingData& pd)
{
    ostr << "Variables: " << endl;
    for_each (pd.m_variables.begin (), pd.m_variables.end (),
              printVariable (ostr));
    return ostr;
}

const char* ParsingData::IMPLEMENTED_METHODS[] = 
{
    "edge_area",
    "vertex_scalar_integral",
    "facet_general_integral"
};


// Methods
// ======================================================================
ParsingData::ParsingData ()
{
    BinaryFunctionInformation BINARY_FUNCTION_INFORMATION[] = 
    {
	{"+", plus<double> ()},
	{"-", minus<double> ()},
	{"*", multiplies<double> ()},
	{"/", divides<double> ()},
	{"^", powf},
	{"=", assignmentFunction},
	{"atan2", atan2f},
	{">", greater<double> ()},
	{">=", greater_equal<double> ()},
	{"<", less<double> ()},
	{"<=", less_equal<double> ()}
    };
    UnaryFunctionInformation UNARY_FUNCTION_INFORMATION[] =
    {
	{"-", negate<double> ()},
	{"sqrt", sqrtf}
    };

    BOOST_FOREACH (BinaryFunctionInformation bfi, BINARY_FUNCTION_INFORMATION)
	m_binaryFunctions[bfi.m_name] = bfi.m_function;
    BOOST_FOREACH (UnaryFunctionInformation ufi, UNARY_FUNCTION_INFORMATION)
	m_unaryFunctions[ufi.m_name] = ufi.m_function;

    m_previousTime = clock ();

    BOOST_FOREACH (const char* method, IMPLEMENTED_METHODS)
	AddMethodOrQuantity (method);
}

ParsingData::~ParsingData ()
{
    for_each(m_identifiers.begin (), m_identifiers.end (), deleteIdentifier);
}

double ParsingData::GetVariableValue (const char* id) 
{
    Variables::iterator it = m_variables.find (id);
    RuntimeAssert (it != m_variables.end (), "Undeclared variable: ", id);
    return it->second;
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



string* ParsingData::CreateIdentifier(char* id)
{
    Identifiers::iterator it = m_identifiers.find (id);
    if (it == m_identifiers.end ())
    {
        string* stringId = new string(id);
        // do not store id, as it comes from the parser and it will go away.
        m_identifiers[stringId->c_str ()] = stringId;
        return stringId;
    }
    else
        return it->second;
}

void ParsingData::PrintTimeCheckpoint (string& description)
{
    clock_t time = clock ();
	cdbg << description << ": " 
		<< static_cast<double>(time - m_previousTime) / CLOCKS_PER_SEC
		<< " sec" << endl;
	m_previousTime = time;
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
	edge = boost::make_shared<QuadraticEdge> (
	    GetVertex(begin), GetVertex(end), GetVertex (middle), 
	    endTranslation, i);
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
