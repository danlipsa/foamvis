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
#include "FoamParameters.h"
#include "Debug.h"
#include "DebugStream.h"
#include "Edge.h"
#include "Utils.h"
#include "Face.h"
#include "OrientedEdge.h"
#include "OrientedFace.h"
#include "OpenGLUtils.h"
#include "ParsingDriver.h"
#include "QuadraticEdge.h"
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
	const vector<boost::shared_ptr<Edge> >& edges): 
	m_edges(edges)
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
    Element (id, ElementStatus::ORIGINAL)
{
    m_orientedEdges.push_back (boost::make_shared<OrientedEdge> (edge, false));
}

Face::Face (const Face& original) :
    Element (original),
    m_adjacentBodies (original.m_adjacentBodies),
    m_normal (original.m_normal),
    m_center (original.m_center)
{
    BOOST_FOREACH (boost::shared_ptr<OrientedEdge> oe, original.m_orientedEdges)
	m_orientedEdges.push_back (boost::make_shared<OrientedEdge> (*oe));
}

Face::Face (const vector<int>& edgeIndexes,
	    const vector<boost::shared_ptr<Edge> >& edges, 
	    size_t id, ElementStatus::Enum duplicateStatus) :
    Element (id, duplicateStatus)
{
    m_adjacentBodies.reserve (2);
    m_orientedEdges.resize (edgeIndexes.size ());
    transform (edgeIndexes.begin(), edgeIndexes.end(), m_orientedEdges.begin(), 
               indexToOrientedEdge(edges));
    CalculateCentroidAndArea ();
}

Face::Face (const vector<boost::shared_ptr<Edge> >& edges, size_t id) :
    Element (id, ElementStatus::ORIGINAL)
{
    m_adjacentBodies.reserve (2);
    m_orientedEdges.resize (edges.size ());
    for (size_t i = 0; i < edges.size (); ++i)
	m_orientedEdges[i].reset (new OrientedEdge (edges[i], false));
    CalculateCentroidAndArea ();
}

void Face::CalculateCentroidAndArea ()
{
    G3D::Matrix3 rotation;
    boost::array<G3D::Vector3, 3> a;
    calculateAxes (&a[0], &a[1], &a[2]);
    for (size_t i = 0; i < a.size (); ++i)
	rotation.setColumn (i, a[i]);
    G3D::Matrix3 inverseRotation = rotation.inverse ();

    vector<double> x, y;
    G3D::Vector3 translation = GetOrientedEdge (0).GetBeginVector ();
    BOOST_FOREACH (boost::shared_ptr<OrientedEdge> oe, m_orientedEdges)
    {
	size_t n = oe->GetPointCount ();
	for (size_t i = 0; i < (n - 1); ++i)
	{
	    G3D::Vector3 v = inverseRotation * (oe->GetPoint (i) - translation);
	    x.push_back (v.x);
	    y.push_back (v.y);
	}
    }
    double center[2];
    double area;
    polyCentroid2D (&x[0], &y[0], x.size (), &center[0], &center[1], &area);
    m_area = abs (area);
    m_center = rotation * G3D::Vector3 (center[0], center[1], 0) + translation;
}

double Face::getMaxEdgeLength ()
{
    double max = 0;
    BOOST_FOREACH (boost::shared_ptr<OrientedEdge> oe, m_orientedEdges)
    {
	G3D::Vector3 prev = oe->GetPoint (0);
	for (size_t i = 1; i < oe->GetPointCount (); ++i)
	{
	    G3D::Vector3 p = oe->GetPoint (i);
	    double length = (p - prev).length ();
	    if (length > max)
		max = length;
	    prev = p;
	}
    }
    return max;
}

void Face::CalculatePerimeter ()
{
    m_perimeter = 0;
    BOOST_FOREACH (boost::shared_ptr<OrientedEdge> oe, GetOrientedEdges ())
	m_perimeter += oe->GetLength ();
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
	GetOrientedEdge (0).GetBegin () == other.GetOrientedEdge (0).GetBegin ();
}

bool Face::fuzzyEq (const Face& other) const
{
    return GetId () == other.GetId () &&
	IsFuzzyZero (
	    GetOrientedEdge (0).GetBeginVector () -
	    other.GetOrientedEdge (0).GetBeginVector ());
}

bool Face::operator< (const Face& other) const
{
    return 
	GetId () < other.GetId () ||

	(GetId () == other.GetId () &&
	 GetOrientedEdge (0).GetBegin () < 
	 other.GetOrientedEdge (0).GetBegin ());
}

void Face::SetNormal ()
{
    G3D::Vector3 x, y, z;
    calculateAxes (&x, &y, &z);
    m_normal = z;
}

void Face::calculateAxes (
    G3D::Vector3* x, G3D::Vector3* y, G3D::Vector3* z) const
{
    const OrientedEdge& one = GetOrientedEdge (0);
    *x = (one.GetEndVector () - one.GetBeginVector ()).unit ();
    *z = GetPlane ().normal ();
    *y = z->cross (*x);
}

size_t Face::largestEdgeIndex () const
{
    size_t size = GetOrientedEdges ().size ();
    size_t largestIndex = 0;
    double largestLength = GetOrientedEdge (largestIndex).GetLength ();
    for (size_t i = 1; i < size; ++i)
    {
	double edgeLength = GetOrientedEdge (i).GetLength ();
	if (edgeLength > largestLength)
	{
	    largestIndex = i;
	    largestLength = edgeLength;
	}
    }
    return largestIndex;
}

// @todo: deal with ctrctndumps_725v_0.1480_0.2400_9.0000_rupt_v1/
// dump_0.1480_0.2400_9.0000_0171.dmp, face 300, which is a bubble 
// with two round edges.
G3D::Plane Face::GetPlane () const
{
    // faces at corners for ctrctn have 2 edges before edges on the constraint
    // are added.
    size_t size = GetOrientedEdges ().size ();
    size_t oneIndex = (size == 2) ? 0 : largestEdgeIndex ();
    const OrientedEdge& one = GetOrientedEdge (oneIndex);    
    size_t twoIndex = (oneIndex + 1) % size;
    const OrientedEdge& two = GetOrientedEdge (twoIndex);
    if (size == 2 &&
	IsFuzzyZero (one.GetBeginVector () - two.GetEndVector ()))
    {
	if (one.GetEdge ()->GetType () == Edge::QUADRATIC_EDGE)
	{
	    QuadraticEdge& e = static_cast<QuadraticEdge&> (*one.GetEdge ());
	    return G3D::Plane (e.GetBeginVector (), e.GetMiddleVector (),
			       e.GetEndVector ());
	}
	else
	    ThrowException ("Face ", this->GetId (), " has only two edges.");
    }
    return G3D::Plane (one.GetBeginVector (), two.GetBeginVector (), 
		       two.GetEndVector ());
}

bool Face::IsClosed () const
{
    return 
	m_orientedEdges[0]->GetBegin () == 
	m_orientedEdges[m_orientedEdges.size () - 1]->GetEnd ();
}


const AdjacentBody& Face::GetAdjacentBody (bool faceReversed) const
{
    RuntimeAssert (! IsStandalone (), "GetAdjacentBody for standalone face: ",
		   GetId ());
    size_t index = faceReversed ^ m_adjacentBodies[0].IsOrientedFaceReversed ();
    return m_adjacentBodies[index];
}

void Face::PrintAdjacentBodyInformation (ostream& ostr) const
{
    size_t bodyPartOfSize = GetAdjacentBodySize ();
    ostr << "Face " << GetStringId () << " part of " 
	 << bodyPartOfSize << " bodies: ";
    for (size_t j = 0; j < bodyPartOfSize; j++)
    {
	const AdjacentBody& bi = GetAdjacentBody (j);
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
    ostr << "center: " << m_center << endl;
    ostr << "edges:\n";
    ostream_iterator< boost::shared_ptr<OrientedEdge> > output (ostr, "\n");
    copy (m_orientedEdges.begin (), m_orientedEdges.end (), output);
    if (HasAttributes ())
    {
	ostr << "Face attributes: ";
	PrintAttributes (ostr);
    }
    ostr << "Adjacent bodies" << "(" << m_adjacentBodies.size () << "): ";
    BOOST_FOREACH (AdjacentBody bi, m_adjacentBodies)
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
    const Vertex& begin = GetOrientedEdge (0).GetBegin ();
    const G3D::Vector3& newBegin = 
	periods.TorusTranslate (begin.GetVector (), translation);
    boost::shared_ptr<Face> searchDummy =
	boost::make_shared<Face> (
	    boost::make_shared<Edge> (
		boost::make_shared<Vertex> (
		    newBegin, begin.GetId ()), 0), GetId ());
    FaceSet::iterator it = faceSet->find (searchDummy);
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
	begin = oe->GetEndVector ();
    }
    faceDuplicate->CalculateCentroidAndArea ();
    return faceDuplicate;
}

void Face::UpdateAdjacentFaceStandalone (boost::shared_ptr<Face> face)
{
    if (IsStandalone ())
    {
	/**
	 * Standalone faces need a place to store an OrientedFace
	 * for the list of faces part of each edge because AdjacentOrientedFace
	 * stores a weak_ptr to the OrientedFace
	 */
	m_orientedFace.reset (new OrientedFace (face, false));
	for (size_t i = 0; i < size (); i++)
	{
	    boost::shared_ptr<OrientedEdge> oe = GetOrientedEdgePtr (i);
	    oe->AddAdjacentFace (m_orientedFace, i);
	}
    }
}

void Face::AddEdge (boost::shared_ptr<Edge> edge)
{
    boost::shared_ptr<OrientedEdge> oe (new OrientedEdge (edge, false));
    m_orientedEdges.push_back (oe);
}

QColor Face::GetColor (const QColor& defaultColor) const
{
    if (HasAttribute (FaceAttributeIndex::COLOR))
	return Color::GetValue (
	    GetAttribute<ColorAttribute, ColorAttribute::value_type> (
		EdgeAttributeIndex::COLOR));
    else
	return defaultColor;
}

size_t Face::GetEdgesPerFace (const FoamParameters& foamParameters) const
{
    size_t count = 0;
    BOOST_FOREACH (boost::shared_ptr<OrientedEdge> oe, GetOrientedEdges ())
    {
	if (oe->GetBegin ().IsPhysical (foamParameters))
	    ++count;
    }
    return count;
}
