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
#include "Utils.h"
#include "Face.h"
#include "OrientedEdge.h"
#include "OrientedFace.h"
#include "ParsingDriver.h"
#include "Vertex.h"

// Private Classes
// ======================================================================

/**
 * Unary function that  creates an oriented edge from  an index into a
 * vector of Edge objects.
 */
class indexToOrientedEdge : 
    public unary_function<int, boost::shared_ptr<OrientedEdge> >
{
public:
    /**
     * Constructs this function object
     * @param edges vector of Edge objects
     */
    indexToOrientedEdge(
	const vector<boost::shared_ptr<Edge> >& edges): m_edges(edges)
    {
    }
    /**
     * Creates an oriented edge from an 1-based index (signed integer).
     * @param i index into the  vector of edges. A negative sign means
     *        that the edege  apears in the face in  the reverse order
     *        than it appears in the vector of edges.
     * @return an OrientedEdge which is  like an Edge and a boolean that
     *        specifies the order of the edge.
     */
    boost::shared_ptr<OrientedEdge> operator() (int i)
    {
        bool reversed = false;
        if (i < 0)
        {
            i = -i;
            reversed = true;
        }
        i--;
        return boost::make_shared<OrientedEdge> (m_edges[i], reversed);
    }
private:
    /**
     * Vector of edges
     */
    const vector<boost::shared_ptr<Edge> >& m_edges;
};


// Methods
// ======================================================================
Face::Face (const boost::shared_ptr<Edge>& edge, size_t id) :
    ColoredElement (id, ElementStatus::ORIGINAL)
{
    m_orientedEdges.push_back (boost::make_shared<OrientedEdge> (edge, false));
}

Face::Face (const Face& original) :
    ColoredElement (original),
    m_bodiesPartOf (original.m_bodiesPartOf),
    m_normal (original.m_normal)
{
    BOOST_FOREACH (boost::shared_ptr<OrientedEdge> oe, original.m_orientedEdges)
	m_orientedEdges.push_back (boost::make_shared<OrientedEdge> (*oe));
}

Face::Face (const vector<int>& edgeIndexes,
	    const vector<boost::shared_ptr<Edge> >& edges, 
	    size_t id, ElementStatus::Enum duplicateStatus) :
    ColoredElement (id, duplicateStatus)
{
    m_bodiesPartOf.reserve (2);
    m_orientedEdges.resize (edgeIndexes.size ());
    transform (edgeIndexes.begin(), edgeIndexes.end(), m_orientedEdges.begin(), 
               indexToOrientedEdge(edges));
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

bool Face::operator== (const Face& other) const
{
    return GetId () == other.GetId () &&
	*GetOrientedEdge (0)->GetBegin () == 
	*other.GetOrientedEdge (0)->GetBegin ();
}

bool Face::fuzzyEq (const Face& other) const
{
    return GetId () == other.GetId () &&
	GetOrientedEdge (0)->GetBegin ()->fuzzyEq (
	    *other.GetOrientedEdge (0)->GetBegin ());
}

bool Face::operator< (const Face& other) const
{
    return GetId () < other.GetId () ||
	(GetId () == other.GetId () &&
	 *GetOrientedEdge (0)->GetBegin () < 
	 *other.GetOrientedEdge (0)->GetBegin ());
}

void Face::CalculateNormal ()
{
    using G3D::Vector3; using G3D::Plane;
    boost::shared_ptr<OrientedEdge> one = GetOrientedEdge (0);
    boost::shared_ptr<OrientedEdge> two = GetOrientedEdge (1);
    G3D::Plane plane (*one->GetBegin (), *two->GetBegin (), *two->GetEnd ());
    m_normal = plane.normal ();
}

bool Face::IsClosed () const
{
    return 
	*m_orientedEdges[0]->GetBegin () == 
	*m_orientedEdges[m_orientedEdges.size () - 1]->GetEnd ();
}

bool Face::HasWrap () const
{
    BOOST_FOREACH (boost::shared_ptr<OrientedEdge> oe, m_orientedEdges)
	if (oe->GetEdge ()->GetEndTranslation () != Vector3int16Zero)
	    return true;
    return false;
}

boost::shared_ptr<Edge>  Face::GetEdge (size_t i) const
{
    return GetOrientedEdge (i)->GetEdge ();
}

const BodyIndex& Face::GetBodyPartOf (bool faceReversed) const
{
    size_t index = faceReversed ^ m_bodiesPartOf[0].IsOrientedFaceReversed ();
    return m_bodiesPartOf[index];
}

void Face::PrintBodyPartOfInformation (ostream& ostr) const
{
    size_t bodyPartOfSize = GetBodyPartOfSize ();
    ostr << "Face " << GetStringId () << " part of " 
	 << bodyPartOfSize << " bodies: ";
    for (size_t j = 0; j < bodyPartOfSize; j++)
    {
	const BodyIndex& bi = GetBodyPartOf (j);
	ostr << setw (3) << bi.GetBody ()->GetId ()
	     << " at index " << bi.GetOrientedFaceIndex () << " ";
    }
    ostr << endl;
}

string Face::ToString () const
{
    ostringstream ostr;
    ostr << "Face " << GetStringId () << " "
	 << GetDuplicateStatus () << ":\n";
    ostr << "edges:\n";
    ostream_iterator< boost::shared_ptr<OrientedEdge> > output (ostr, "\n");
    copy (m_orientedEdges.begin (), m_orientedEdges.end (), output);
    ostr << "Face attributes: ";
    PrintAttributes (ostr);
    ostr << "Adjacent bodies" << "(" << m_bodiesPartOf.size () << "): ";
    BOOST_FOREACH (BodyIndex bi, m_bodiesPartOf)
	ostr << "(" << bi.GetBody ()->GetId () 
	     << ", " << bi.GetOrientedFaceIndex ()<< ") ";
    ostr << endl;
    return ostr.str ();
}

void Face::GetVertexSet (VertexSet* vertexSet) const
{
    const OrientedEdges& orientedEdges = GetOrientedEdges ();
    for_each (orientedEdges.begin (), orientedEdges.end (),
	      boost::bind (&OrientedEdge::GetVertexSet, _1, vertexSet));
}

void Face::GetEdgeSet (EdgeSet* edgeSet) const
{
    const OrientedEdges& orientedEdges = GetOrientedEdges ();
    BOOST_FOREACH (boost::shared_ptr<OrientedEdge> oe, orientedEdges)
	edgeSet->insert (oe->GetEdge ());
}

boost::shared_ptr<Face> Face::GetDuplicate (
    const OOBox& periods, const G3D::Vector3int16& translation,
    VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet) const
{
    const G3D::Vector3* begin = 
	GetOrientedEdge (0)->GetBegin ().get ();
    const G3D::Vector3& newBegin = 
	periods.TorusTranslate (*begin, translation);
    boost::shared_ptr<Face> searchDummy =
	boost::make_shared<Face> (
	    boost::make_shared<Edge> (
		boost::make_shared<Vertex> (newBegin), 0), GetId ());
    FaceSet::iterator it = 
	fuzzyFind <FaceSet, FaceSet::iterator, FaceSet::key_type> (
	    *faceSet, searchDummy);
    if (it != faceSet->end ())
	return *it;
    boost::shared_ptr<Face> duplicate = this->createDuplicate (
	periods, newBegin, vertexSet, edgeSet);
    faceSet->insert (duplicate);
    return duplicate;
}

boost::shared_ptr<Face> Face::createDuplicate (
    const OOBox& periods, const G3D::Vector3& newBegin,
    VertexSet* vertexSet, EdgeSet* edgeSet) const
{
    boost::shared_ptr<Face> faceDuplicate = boost::make_shared<Face> (*this);
    faceDuplicate->SetDuplicateStatus (ElementStatus::DUPLICATE);
    G3D::Vector3 begin = newBegin;
    BOOST_FOREACH (boost::shared_ptr<OrientedEdge> oe,
		   faceDuplicate->GetOrientedEdges ())
    {
	G3D::Vector3 edgeBegin;
	if (oe->IsReversed ())
	    edgeBegin = oe->GetEdge ()->GetTranslatedBegin (begin);
	else
	    edgeBegin = begin;
	boost::shared_ptr<Edge> edgeDuplicate = 
	    oe->GetEdge ()->GetDuplicate (
		periods, edgeBegin, vertexSet, edgeSet);
	oe->SetEdge (edgeDuplicate);
	begin = *oe->GetEnd ();
    }
    return faceDuplicate;
}
