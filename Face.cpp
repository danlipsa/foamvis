/**
 * @file   Face.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Face class
 */
#include "Face.h"
#include "AttributeInfo.h"
#include "ParsingDriver.h"
#include "Debug.h"
#include "Body.h"
#include "Data.h"

/**
 * Unary function that  creates an oriented edge from  an index into a
 * vector of Edge objects.
 */
class indexToOrientedEdge : public unary_function<int, OrientedEdge*>
{
public:
    /**
     * Constructs this function object
     * @param edges vector of Edge objects
     */
    indexToOrientedEdge(vector<Edge*>& edges): m_edges(edges) {}
    /**
     * Creates an oriented edge from an 1-based index (signed integer).
     * @param i index into the  vector of edges. A negative sign means
     *        that the edege  apears in the face in  the reverse order
     *        than it appears in the vector of edges.
     * @return an OrientedEdge which is  like an Edge and a boolean that
     *        specifies the order of the edge.
     */
    OrientedEdge* operator() (int i)
    {
        bool reversed = false;
        if (i < 0)
        {
            i = -i;
            reversed = true;
        }
        i--;
        return new OrientedEdge(m_edges[i], reversed);
    }
private:
    /**
     * Vector of edges
     */
    vector<Edge*>& m_edges;
};

class printIndex
{
public:
    printIndex (ostream& ostr) : m_ostr (ostr) {}
    void operator () (Body* body)
    {
	m_ostr << body->GetOriginalIndex () << " ";
    }
private:
    ostream& m_ostr;
};


ostream& operator<< (ostream& ostr, const Face& f)
{
    ostr << "Adjacent bodies" << "(" << f.m_adjacentBodies.size () << "): ";
    for_each (f.m_adjacentBodies.begin (), f.m_adjacentBodies.end (),
	      printIndex (ostr));
    ostr << endl;
    PrintElements<OrientedEdge*> (ostr, f.m_edges, 
				  "edges part of the face", true);
    ostr << " Face attributes: ";
    return f.PrintAttributes (ostr, *Face::m_infos);
}


AttributesInfo* Face::m_infos;

Face::Face(vector<int>& edgeIndexes, vector<Edge*>& edges, 
	   unsigned int originalIndex, Data* data, bool duplicate) :
    Element (originalIndex, data, duplicate)
{
    m_edges.resize (edgeIndexes.size ());
    transform (edgeIndexes.begin(), edgeIndexes.end(), m_edges.begin(), 
               indexToOrientedEdge(edges));
    if (m_data->IsTorus ())
    {
	G3D::Vector3* begin = (*m_edges.begin())->GetBegin ();
	vector<OrientedEdge*>::iterator edgeIt;
	for (edgeIt = m_edges.begin (); edgeIt < m_edges.end (); edgeIt++)
	{
	    G3D::Vector3 edgeBegin;
	    if ((*edgeIt)->IsReversed ())
		edgeBegin = (*edgeIt)->GetEdge ()->GetBegin (begin);
	    else
		edgeBegin = *begin;
	    (*edgeIt)->SetEdge (
		data->GetEdgeDuplicate (*(*edgeIt)->GetEdge (), edgeBegin));
	    begin = (*edgeIt)->GetEnd ();
	}
    }
}

Face::~Face()
{
    using boost::bind;
    for_each(m_edges.begin(), m_edges.end(),
	     bind(DeletePointer<OrientedEdge>(), _1));
}

void Face::ReversePrint (ostream& ostr) const
{
    ReversePrintElements<OrientedEdge*> (
	ostr, m_edges, "edges part of the face", true);
    ostr << " Face attributes: ";
    PrintAttributes (ostr, *Face::m_infos);
}

void Face::StoreDefaultAttributes (AttributesInfo* infos)
{
    using EvolverData::parser;
    m_infos = infos;
    const char* colorString = 
        ParsingDriver::GetKeywordString(parser::token::COLOR);
    // load the color attribute and nothing else
    infos->Load (colorString);

    infos->AddAttributeInfo (colorString, new ColorAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL),
        new IntegerAttributeCreator());
}

Color::Name Face::GetColor () const
{
    if (m_attributes != 0)
	return dynamic_cast<ColorAttribute*>(
	    (*m_attributes)[COLOR_INDEX].get ())->GetColor ();
    else
	return static_cast<Color::Name>((GetOriginalIndex ()+1) % Color::COUNT);
}

void Face::UpdateEdgesAdjacency ()
{
    using boost::bind;
    vector<OrientedEdge*>& orientedEdges = GetOrientedEdges ();
    for_each (orientedEdges.begin (), orientedEdges.end (),
	      bind (&OrientedEdge::AddAdjacentFace, _1, this));
}

void Face::ClearEdgesAdjacency ()
{
    using boost::bind;
    vector<OrientedEdge*>& orientedEdges = GetOrientedEdges ();
    for_each (orientedEdges.begin (), orientedEdges.end (),
	      bind (&OrientedEdge::ClearAdjacentFaces, _1));
}

size_t Face::GetNextValidIndex (size_t index) const
{
    RuntimeAssert (index < m_edges.size (),
		   "Edge index ", index, 
		   " greater than the number of edges ", m_edges.size ());
    if (index == (m_edges.size () - 1))
	return 0;
    else
	return index + 1;
}

size_t Face::GetPreviousValidIndex (size_t index) const
{
    RuntimeAssert (index < m_edges.size (), "Edge index ", index,
		   " greater than the number of edges ", m_edges.size ());
    if (index == 0)
	return m_edges.size () - 1;
    else
	return index - 1;
}

bool Face::operator== (const Face& face) const
{
    return GetOriginalIndex () == face.GetOriginalIndex () &&
	*GetOrientedEdge (0)->GetBegin () == 
	*face.GetOrientedEdge (0)->GetBegin ();
}

G3D::Vector3 Face::GetNormal () const
{
    using boost::bind;
    using G3D::Vector3;
    OrientedEdges::const_iterator it = find_if (
	m_edges.begin (), m_edges.end (), !bind (&OrientedEdge::IsZero, _1));
    RuntimeAssert (it != m_edges.end (), "Face with all edges 0");
    Vector3 one = (*it)->GetEdgeVector ();
    it = find_if (
	it, m_edges.end (), !bind (&OrientedEdge::IsZero, _1));
    RuntimeAssert (it != m_edges.end (), "Face with only one edge != 0");
    Vector3 two = (*it)->GetEdgeVector ();
    return (one.cross (two).unit ());
}
