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
#include "Data.h"
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
	   size_t originalIndex, Data* data, ElementStatus::Name status) :
    ColoredElement (originalIndex, data, status)
{
    m_edges.resize (edgeIndexes.size ());
    transform (edgeIndexes.begin(), edgeIndexes.end(), m_edges.begin(), 
               indexToOrientedEdge(edges));
    if (m_data->IsTorus ())
    {
	G3D::Vector3* begin = (*m_edges.begin())->GetBegin ();
	BOOST_FOREACH (OrientedEdge* oe, m_edges)
	{
	    Edge* edge = oe->GetEdge ();
	    G3D::Vector3 edgeBegin = 
		(oe->IsReversed ()) ? edge->GetBegin (begin) : *begin;
	    oe->SetEdge (data->GetEdgeDuplicate (edge, edgeBegin));
	    begin = oe->GetEnd ();
	}
    }
}


Face::Face (Edge* edge, size_t originalIndex) :
    ColoredElement (originalIndex, 0, ElementStatus::ORIGINAL)
{
    m_edges.push_back (new OrientedEdge (edge, false));
}

Face::Face (const Face& original) :
    ColoredElement (original.GetOriginalIndex (), original.GetData (), 
		    ElementStatus::DUPLICATE)
{
    BOOST_FOREACH (OrientedEdge* oe, original.m_edges)
	m_edges.push_back (new OrientedEdge (*oe));
}

Face::~Face()
{
    using boost::bind;
    for_each(m_edges.begin(), m_edges.end(),
	     bind(DeletePointer<OrientedEdge>(), _1));
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

/*
    OrientedEdges::const_iterator it = find_if (
	m_edges.begin (), m_edges.end (), !bind (&OrientedEdge::IsZero, _1));
    RuntimeAssert (it != m_edges.end (), "Face with all edges 0");
    Vector3 one = (*it)->GetEdgeVector ();
    it++;
    it = find_if (it, m_edges.end (), !bind (&OrientedEdge::IsZero, _1));
    Vector3 two = (*it)->GetEdgeVector ();
*/
    Vector3 one = GetOrientedEdge (0)->GetEdgeVector ();
    Vector3 two = GetOrientedEdge (1)->GetEdgeVector ();
    return (one.cross (two).unit ());
}

Face* Face::CreateDuplicate (const G3D::Vector3& newBegin) const
{
    Face* faceDuplicate = new Face(*this);
    G3D::Vector3 begin = newBegin;
    BOOST_FOREACH (OrientedEdge* oe, faceDuplicate->m_edges)
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
    return *m_edges[0]->GetBegin () == *m_edges[m_edges.size () - 1]->GetEnd ();
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
    ostr << "Adjacent bodies" << "(" << f.m_adjacentBodies.size () << "): ";
    BOOST_FOREACH (Body* b, f.m_adjacentBodies)
	ostr << b->GetOriginalIndex () << " ";
    ostr << endl;
    ostr << "edges part of the face" << endl;
    ostream_iterator<OrientedEdge*> output (ostr, "\n");
    copy (f.m_edges.begin (), f.m_edges.end (), output);
    ostr << " Face attributes: ";
    return f.PrintAttributes (ostr);
}
