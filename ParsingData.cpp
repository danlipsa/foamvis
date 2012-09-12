/**
 * @file   ParsingData.cpp
 * @author Dan R. Lipsa
 *
 * Definition of the ParsingData class
 */

#include "Attribute.h"
#include "Debug.h"
#include "DebugStream.h"
#include "Edge.h"
#include "ExpressionTree.h"
#include "Face.h"
#include "Foam.h"
#include "ParsingData.h"
#include "QuadraticEdge.h"
#include "Utils.h"
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

double doubleAbs (double value)
{
    return abs (value);
}

// Static fields
// ======================================================================

const char* ParsingData::IMPLEMENTED_METHODS[] = 
{
    // 0-dimensional
    "vertex_scalar_integral",

    // 1-dimensional
    "edge_area",
    "edge_length", "edge_tension",
    "edge_general_integral",

    // 2-dimensional
    "facet_general_integral"
};

boost::array<string, 11> _operators = {{
	"+", "-", "*", "/", "^", "=", ">", ">=", "<", "<=", "&&"
    }};
const set<string> ParsingData::OPERATORS (
    _operators.begin (), _operators.end ());


// Methods
// ======================================================================
ParsingData::ParsingData (
    bool useOriginal, 
    const DmpObjectInfo& dmpObjectInfo,
    const vector<ForcesOneObjectNames>& forcesNames) :

    m_spaceSignificant (false),
    m_parenthesisCount (0),
    m_newLineSignificant (false),
    m_useOriginal (useOriginal),
    m_dmpObjectInfo (dmpObjectInfo),
    m_keywordsIgnored (false)
{
    m_forcesNames.resize (forcesNames.size ());
    copy (forcesNames.begin (), forcesNames.end (), m_forcesNames.begin ());
    BinaryFunctionInformation BINARY_FUNCTION_INFORMATION[] = 
    {
	{"+", plus<double> ()},
	{"-", minus<double> ()},
	{"*", multiplies<double> ()},
	{"/", divides<double> ()},
	{"^", powf},
	{"=", minus<double> ()}, // left = right is the same as left - right = 0
	{"atan2", atan2f},
	// results in a boolean
	{">", greater<double> ()},
	{">=", greater_equal<double> ()},
	{"<", less<double> ()},
	{"<=", less_equal<double> ()},
	{"&&", logical_and<double> ()}
    };
    UnaryFunctionInformation UNARY_FUNCTION_INFORMATION[] =
    {
	{"-", negate<double> ()},
	{"sqrt", sqrtf},
	{"cos", cosf},
	{"sin", sinf},
	{"asin", asinf},
        {"acos", acosf},
	{"abs", doubleAbs}
    };

    BOOST_FOREACH (BinaryFunctionInformation bfi, BINARY_FUNCTION_INFORMATION)
	m_binaryFunctions[bfi.m_name] = bfi.m_function;
    BOOST_FOREACH (UnaryFunctionInformation ufi, UNARY_FUNCTION_INFORMATION)
	m_unaryFunctions[ufi.m_name] = ufi.m_function;

    BOOST_FOREACH (const char* method, IMPLEMENTED_METHODS)
	AddMethodOrQuantity (method);
}

pair<bool, float> ParsingData::GetVariableValue (const char* name) const
{
    Variables::const_iterator it = m_variables.find (name);
    if (it == m_variables.end ())
        return pair<bool, float> (false, 0);
    else 
        return pair<bool, float> (true, it->second);
}

pair<bool, ParsingData::ArrayId> ParsingData::GetArrayId (
    const char* name) const
{
    Arrays::const_iterator it = m_arrays.find (name);
    return pair<bool, ArrayId> (it != m_arrays.end (), it);
}

pair<bool, float> ParsingData::GetArrayValue (const char* name, 
                                              const vector<size_t>& index) const
{
    pair<bool, ArrayId> p = GetArrayId (name);
    if (p.first)
        return pair<bool, float> (true, GetArrayValue (p.second, index));
    else
        return pair<bool, float> (false, 0);
}

float ParsingData::GetArrayValue (ArrayId it, const vector<size_t>& index) const
{
    return it->second->Get (index);
}


void ParsingData::SetArray (const char* id, AttributeArrayAttribute* array)
{
    m_arrays[id] = boost::shared_ptr<AttributeArrayAttribute> (array);
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
    resizeAllowIndex (&m_vertices, i);
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
    resizeAllowIndex (&m_edges, i);
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
    resizeAllowIndex (&m_faces, i);
    boost::shared_ptr<Face> face = boost::make_shared<Face> (edges, m_edges, i);
    if (&attributes != 0)
        face->StoreAttributes (attributes, attributesInfo);
    m_faces[i] = face;
}

void ParsingData::SetConstraint (size_t i, ExpressionTree* function)
{
    resizeAllowIndex (&m_constraints, i);
    m_constraints[i].reset (function);
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

bool ParsingData::GetT1s (const char* arrayName, const char* countName, 
                          vector<G3D::Vector3>* t1s) const
{
    pair<bool, ArrayId> setId = GetArrayId (arrayName);
    if (! setId.first)
        return false;
    ArrayId id = setId.second;
    pair<bool, float> c = GetVariableValue (countName);
    if (! c.first)
        return false;
    size_t count = c.second;
    t1s->resize (count);
    vector<size_t> index (2);
    for (size_t i = 0; i < count; ++i)
    {
        index[1] = i;
        index[0] = 0;
        float x = GetArrayValue (id, index);
        index[0] = 1;
        float y = GetArrayValue (id, index);
        float z = Foam::Z_COORDINATE_2D;
        if (DATA_PROPERTIES.Is3D ())
        {
            index[0] = 2;
            z = GetArrayValue (id, index);
        }
        (*t1s)[i] = G3D::Vector3 (x, y, z);
    }
    return true;
}
