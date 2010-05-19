/**
 * @file   Face.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Face class
 */
#include "Attribute.h"
#include "AttributeCreator.h"
#include "AttributeInfo.h"
#include "Body.h"
#include "Foam.h"
#include "Debug.h"
#include "Edge.h"
#include "ElementUtils.h"
#include "Face.h"
#include "OrientedEdge.h"
#include "ParsingDriver.h"
#include "Vertex.h"

// Private Classes
// ======================================================================

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


// Static Fields
// ======================================================================

AttributesInfo* Face::m_infos;


// Methods
// ======================================================================

Face::Face(vector<int>& edgeIndexes, vector<Edge*>& edges, 
	   size_t id, Foam* data, ElementStatus::Name status) :
    ColoredElement (id, data, status)
{
    Body* nullBody = 0;
    fill (m_adjacentBodies.begin (), m_adjacentBodies.end (), nullBody);
    m_orientedEdges.resize (edgeIndexes.size ());
    transform (edgeIndexes.begin(), edgeIndexes.end(), m_orientedEdges.begin(), 
               indexToOrientedEdge(edges));
    if (m_data->IsTorus ())
    {
	G3D::Vector3* begin = (*m_orientedEdges.begin())->GetBegin ();
	BOOST_FOREACH (OrientedEdge* oe, m_orientedEdges)
	{
	    Edge* edge = oe->GetEdge ();
	    G3D::Vector3 edgeBegin = 
		(oe->IsReversed ()) ? edge->GetBegin (begin) : *begin;
	    oe->SetEdge (data->GetEdgeDuplicate (edge, edgeBegin));
	    begin = oe->GetEnd ();
	}
    }
}


Face::Face (Edge* edge, size_t id) :
    ColoredElement (id, 0, ElementStatus::ORIGINAL)
{
    Body* nullBody = 0;
    fill (m_adjacentBodies.begin (), m_adjacentBodies.end (), nullBody);
    m_orientedEdges.push_back (new OrientedEdge (edge, false));
}

Face::Face (const Face& original) :
    ColoredElement (original.GetId (), original.GetFoam (), 
		    ElementStatus::DUPLICATE)
{
    Body* nullBody = 0;
    fill (m_adjacentBodies.begin (), m_adjacentBodies.end (), nullBody);
    BOOST_FOREACH (OrientedEdge* oe, original.m_orientedEdges)
	m_orientedEdges.push_back (new OrientedEdge (*oe));
}

Face::~Face()
{
    for_each(m_orientedEdges.begin(), m_orientedEdges.end(), bl::delete_ptr ());
}

void Face::UpdateEdgeAdjacency ()
{
    using boost::bind;
    OrientedEdges& orientedEdges = GetOrientedEdges ();
    for (size_t i = 0; i < orientedEdges.size (); i++)
    {
	OrientedEdge* oe = orientedEdges[i];
	oe->AddAdjacentFace (this, i);
    }
}

void Face::ClearEdgeAdjacency ()
{
    using boost::bind;
    OrientedEdges& orientedEdges = GetOrientedEdges ();
    for_each (orientedEdges.begin (), orientedEdges.end (),
	      bind (&OrientedEdge::ClearAdjacentFaces, _1));
}

size_t Face::GetNextValidIndex (size_t index) const
{
    RuntimeAssert (
	index < m_orientedEdges.size (), "Edge index ", index, 
	" greater than the number of edges ", m_orientedEdges.size ());
    if (index == (m_orientedEdges.size () - 1))
	return 0;
    else
	return index + 1;
}

size_t Face::GetPreviousValidIndex (size_t index) const
{
    RuntimeAssert (
	index < m_orientedEdges.size (), "Edge index ", index,
	" greater than the number of edges ", m_orientedEdges.size ());
    if (index == 0)
	return m_orientedEdges.size () - 1;
    else
	return index - 1;
}

bool Face::operator== (const Face& face) const
{
    return GetId () == face.GetId () &&
	*GetOrientedEdge (0)->GetBegin () == 
	*face.GetOrientedEdge (0)->GetBegin ();
}

G3D::Vector3 Face::GetNormal () const
{
    using boost::bind; using G3D::Vector3;
    Vector3 one = GetOrientedEdge (0)->GetEdgeVector ();
    Vector3 two = GetOrientedEdge (1)->GetEdgeVector ();
    return (one.cross (two).unit ());
}

Face* Face::CreateDuplicate (const G3D::Vector3& newBegin) const
{
    Face* faceDuplicate = new Face(*this);
    G3D::Vector3 begin = newBegin;
    BOOST_FOREACH (OrientedEdge* oe, faceDuplicate->m_orientedEdges)
    {
	G3D::Vector3 edgeBegin;
	if (oe->IsReversed ())
	    edgeBegin = oe->GetEdge ()->GetBegin (&begin);
	else
	    edgeBegin = begin;
	Edge* edgeDuplicate = m_data->GetEdgeDuplicate (
	    oe->GetEdge (), edgeBegin);
	oe->SetEdge (edgeDuplicate);
	begin = *oe->GetEnd ();
    }
    return faceDuplicate;
}

bool Face::IsClosed () const
{
    return 
	*m_orientedEdges[0]->GetBegin () == 
	*m_orientedEdges[m_orientedEdges.size () - 1]->GetEnd ();
}

bool Face::IsAdjacent (size_t bodyOriginalIndex)
{
    BOOST_FOREACH (Body* b, m_adjacentBodies)
	if (b->GetId () == bodyOriginalIndex)
	    return true;
    return false;
}


bool Face::HasWrap () const
{
    BOOST_FOREACH (OrientedEdge* oe, m_orientedEdges)
	if (oe->GetEdge ()->GetEndTranslation () != G3D::Vector3int16 (0, 0, 0))
	    return true;
    return false;
}

Edge* Face::GetEdge (size_t i) const
{
    return GetOrientedEdge (i)->GetEdge ();
}


// Static and Friends Methods
// ======================================================================

void Face::StoreDefaultAttributes (AttributesInfo* infos)
{
    using EvolverData::parser;
    m_infos = infos;
    ColoredElement::StoreDefaultAttributes (infos);
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL),
        new IntegerAttributeCreator());
}

ostream& operator<< (ostream& ostr, const Face& f)
{
    ostr << "face " << f.GetId () << ":\n";
    ostr << "edges:\n";
    ostream_iterator<OrientedEdge*> output (ostr, "\n");
    copy (f.m_orientedEdges.begin (), f.m_orientedEdges.end (), output);
    ostr << "Face attributes: ";
    f.PrintAttributes (ostr);
    ostr << "Adjacent bodies" << "(" << f.m_adjacentBodies.size () << "): ";
    BOOST_FOREACH (Body* b, f.m_adjacentBodies)
	ostr << b->GetId () << " ";
    ostr << endl;
    return ostr;
}
