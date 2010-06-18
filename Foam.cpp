/**
 * @file   Foam.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Foam object
 */
#include "AttributeInfo.h"
#include "AttributeCreator.h"
#include "Body.h"
#include "DebugStream.h"
#include "Foam.h"
#include "Edge.h"
#include "Face.h"
#include "Utils.h"
#include "OrientedEdge.h"
#include "OrientedFace.h"
#include "ParsingData.h"
#include "ProcessBodyTorus.h"
#include "Vertex.h"

// Private Classes
// ======================================================================

template<typename Container, typename ContainerIterator>
struct calculateAggregate
{
    typedef ContainerIterator (*AggregateOnContainer) (
	ContainerIterator first, 
	ContainerIterator second, 
	VertexLessThanAlong lessThan);
    void operator() (AggregateOnContainer aggregate,
		     G3D::Vector3* v,
		     Container& vertices)
    {
	using G3D::Vector3;
	ContainerIterator it;
	it = aggregate (vertices.begin (), vertices.end (), 
			VertexLessThanAlong(Vector3::X_AXIS));;
	v->x = (*it)->x;
	it = aggregate (vertices.begin (), vertices.end (), 
			VertexLessThanAlong(Vector3::Y_AXIS));
	v->y = (*it)->y;
	it = aggregate (vertices.begin (), vertices.end (), 
			VertexLessThanAlong(Vector3::Z_AXIS));
	v->z = (*it)->z;
    }
};


// Private Functions
// ======================================================================


template <typename Container, 
	  typename ContainerIterator,
	  typename ContainerKeyType>
ContainerIterator fuzzyFind (const Container& s, const ContainerKeyType& x)
{
    ContainerIterator it = s.lower_bound (x);
    if (it != s.end () && (*it)->fuzzyEq (*x))
	return it;
    if (it != s.begin ())
    {
	--it;
	if ((*(it))->fuzzyEq (*x))
	    return it;
    }
    return s.end ();
}


G3D::Vector3* Vector3Address (G3D::Vector3& v)
{
    return &v;
}

/**
 * Move elements in a vector toward the begining of the vector so that we 
 * eliminate holes.
 * @param v vector of elements
 */
template <typename E>
void compact (vector< boost::shared_ptr<E> >& v)
{
    size_t step = 0;
    for (size_t i = 0; i < v.size (); i++)
    {
        if (v[i] == 0)
            step++;
        else if (step != 0)
            v[i - step] = v[i];
    }
    size_t resize = v.size () - step;
    v.resize (resize);
}

// Methods
// ======================================================================

Foam::Foam () : 
    m_parsingData (new ParsingData ()),
    m_spaceDimension (3),
    m_quadratic (false)
{
    fill (m_viewMatrix.begin (), m_viewMatrix.end (), 0);
    Vertex::StoreDefaultAttributes (&m_attributesInfo[DefineAttribute::VERTEX]);
    Edge::StoreDefaultAttributes (&m_attributesInfo[DefineAttribute::EDGE]);
    Face::StoreDefaultAttributes (&m_attributesInfo[DefineAttribute::FACE]);
    Body::StoreDefaultAttributes (&m_attributesInfo[DefineAttribute::BODY]);
}

boost::shared_ptr<Vertex> Foam::GetVertexDuplicate (
    const Vertex& original, const G3D::Vector3int16& translation)
{
    boost::shared_ptr<Vertex> searchDummy = boost::make_shared<Vertex>(
	GetOriginalDomain ().TorusTranslate (original, translation));
    VertexSet::iterator it = fuzzyFind 
	<VertexSet, VertexSet::iterator, VertexSet::key_type> (
	    m_vertexSet, searchDummy);
    if (it != m_vertexSet.end ())
	return *it;
    boost::shared_ptr<Vertex> duplicate = CreateVertexDuplicate (
	original, translation);
    m_vertexSet.insert (duplicate);
    m_vertices.push_back (duplicate);
    return duplicate;
}

boost::shared_ptr<Edge> Foam::GetEdgeDuplicate (
    const Edge& original, const G3D::Vector3& newBegin)
{
    boost::shared_ptr<Edge> searchDummy = 
	boost::make_shared<Edge>(
	    boost::make_shared<Vertex> (newBegin), original.GetId ());
    EdgeSet::iterator it = 
	fuzzyFind <EdgeSet, EdgeSet::iterator, EdgeSet::key_type> (
	    m_edgeSet, searchDummy);
    if (it != m_edgeSet.end ())
	return *it;
    boost::shared_ptr<Edge> duplicate = CreateEdgeDuplicate (original, newBegin);
    m_edgeSet.insert (duplicate);
    m_edges.push_back (duplicate);
    return duplicate;
}

boost::shared_ptr<Face> Foam::GetFaceDuplicate (
    const Face& original, const G3D::Vector3int16& translation)
{
    const G3D::Vector3* begin = 
	original.GetOrientedEdge (0)->GetBegin ().get ();
    const G3D::Vector3& newBegin = 
	GetOriginalDomain ().TorusTranslate (*begin, translation);
    boost::shared_ptr<Face> searchDummy =
	boost::make_shared<Face> (
	    boost::make_shared<Edge> (
		boost::make_shared<Vertex> (newBegin), 0), original.GetId ());
    FaceSet::iterator it = 
	fuzzyFind <FaceSet, FaceSet::iterator, FaceSet::key_type> (
	    m_faceSet, searchDummy);
    if (it != m_faceSet.end ())
	return *it;
    boost::shared_ptr<Face> duplicate = CreateFaceDuplicate (original, newBegin);
    m_faceSet.insert (duplicate);
    m_faces.push_back (duplicate);
    return duplicate;
}

boost::shared_ptr<Vertex> Foam::CreateVertexDuplicate (
    const Vertex& original, const G3D::Vector3int16& translation)
{
    boost::shared_ptr<Vertex> duplicate = boost::make_shared<Vertex> (
	original);
    duplicate->SetDuplicateStatus (ElementStatus::DUPLICATE);
    TorusTranslate (duplicate.get (), translation);
    return duplicate;
}

void Foam::TorusTranslate (
    Vertex* vertex, const G3D::Vector3int16& translation) const
{
    *static_cast<G3D::Vector3*>(vertex) = 
	GetOriginalDomain ().TorusTranslate (*vertex, translation);
}


boost::shared_ptr<Edge> Foam::CreateEdgeDuplicate (
    const Edge& original, const G3D::Vector3& newBegin)
{
    G3D::Vector3int16 translation = GetOriginalDomain ().GetTranslation (
	*original.GetBegin (), newBegin);
    boost::shared_ptr<Vertex> beginDuplicate = GetVertexDuplicate (
	*original.GetBegin (), translation);
    boost::shared_ptr<Vertex> endDuplicate = GetVertexDuplicate (
	*original.GetEnd (), translation);
    boost::shared_ptr<Edge> duplicate = boost::make_shared<Edge> (original);
    duplicate->SetBegin (beginDuplicate);
    duplicate->SetEnd (endDuplicate);
    duplicate->SetDuplicateStatus (ElementStatus::DUPLICATE);
    return duplicate;
}

boost::shared_ptr<Face> Foam::CreateFaceDuplicate (
    const Face& original, const G3D::Vector3& newBegin)
{
    boost::shared_ptr<Face> faceDuplicate = boost::make_shared<Face> (original);
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
	    GetEdgeDuplicate (*oe->GetEdge (), edgeBegin);
	oe->SetEdge (edgeDuplicate);
	begin = *oe->GetEnd ();
    }
    return faceDuplicate;
}



void Foam::SetVertex (size_t i, float x, float y, float z,
                     vector<NameSemanticValue*>& list) 
{
    if (i >= m_vertices.size ())
        m_vertices.resize (i + 1);
    boost::shared_ptr<Vertex> vertex = boost::make_shared<Vertex> (x, y ,z, i);
    if (&list != 0)
        vertex->StoreAttributes (
            list, m_attributesInfo[DefineAttribute::VERTEX]);
    m_vertices[i] = vertex;
}


void Foam::SetEdge (size_t i, size_t begin, size_t end,
		    G3D::Vector3int16& endTranslation,
                    vector<NameSemanticValue*>& list) 
{
    if (i >= m_edges.size ())
        m_edges.resize (i + 1); 
    boost::shared_ptr<Edge> edge = boost::make_shared<Edge> (
	GetVertex(begin), GetVertex(end), endTranslation, i);
    if (&list != 0)
        edge->StoreAttributes (list, m_attributesInfo[DefineAttribute::EDGE]);
    m_edges[i] = edge;
}

void Foam::SetFace (size_t i,  vector<int>& edges,
                    vector<NameSemanticValue*>& list)
{
    if (i >= m_faces.size ())
        m_faces.resize (i + 1);
    boost::shared_ptr<Face> face = boost::make_shared<Face> (edges, m_edges, i);
    if (&list != 0)
        face->StoreAttributes (list, m_attributesInfo[DefineAttribute::FACE]);
    m_faces[i] = face;
}

void Foam::SetBody (size_t i, vector<int>& faces,
                    vector<NameSemanticValue*>& list)
{
    if (i >= m_bodies.size ())
        m_bodies.resize (i + 1);
    boost::shared_ptr<Body> body = boost::make_shared<Body> (faces, m_faces, i);
    if (&list != 0)
        body->StoreAttributes (list,
			       m_attributesInfo[DefineAttribute::BODY]);    
    m_bodies[i] = body;
}



void Foam::Compact (void)
{
    compact (m_vertices);
    compact (m_edges);
    compact (m_faces);
    compact (m_bodies);
}


void Foam::ReleaseParsingData ()
{
    m_parsingData.reset ();
}

void Foam::UpdatePartOf ()
{
    using boost::mem_fn;
    for_each (m_bodies.begin (), m_bodies.end (), 
	      boost::bind(&Body::UpdatePartOf, _1, _1));
    for_each (m_edges.begin (), m_edges.end (), 
	      boost::bind (&Edge::UpdateEdgePartOf, _1, _1));
}

void Foam::CalculateAABox ()
{
    using G3D::Vector3;
    Vector3 low, high;
    calculateAggregate <Vertices, Vertices::iterator>() (
	min_element, &low, m_vertices);
    calculateAggregate <Vertices, Vertices::iterator>()(
	max_element, &high, m_vertices);
    if (IsTorus ())
	calculateAABoxForTorus (&low, &high);
    m_AABox.set(low, high);
}

void Foam::calculateAABoxForTorus (G3D::Vector3* low, G3D::Vector3* high)
{
    using boost::array;
    using G3D::Vector3;
    Vector3 origin = Vector3::zero ();
    Vector3 first = m_originalDomain[0];
    Vector3 second = m_originalDomain[1];
    Vector3 sum = first + second;
    Vector3 third = m_originalDomain[2];
    array<Vector3, 10> additionalVertices = 
    {{
	    *low, origin, first, sum, second,
	    origin + third, first + third, sum + third, second + third, *high
    }};
    vector<Vector3*> v(additionalVertices.size ());
    transform (additionalVertices.begin (), additionalVertices.end (),
	       v.begin (), Vector3Address);
    calculateAggregate<vector<Vector3*>, vector<Vector3*>::iterator>() (
	min_element, low, v);
    calculateAggregate<vector<Vector3*>, vector<Vector3*>::iterator>() (
	max_element, high, v);
}

void Foam::CacheEdgesVerticesInBodies ()
{
    for_each (m_bodies.begin (), m_bodies.end (), 
	      boost::bind(&Body::CacheEdgesVertices, _1, 
			  GetSpaceDimension (), IsQuadratic ()));
}

void Foam::CalculateBodiesCenters ()
{
    for_each (m_bodies.begin (), m_bodies.end (),
	      boost::mem_fn(&Body::CalculateCenter));
}

void Foam::CalculateTorusClipped ()
{
    const OOBox& periods = GetOriginalDomain ();
    BOOST_FOREACH (boost::shared_ptr<Edge> e, m_edges)
    {
	if (e->IsClipped ())
	    e->CalculateTorusClipped (periods);
    }
}

void Foam::Unwrap ()
{
    BOOST_FOREACH (boost::shared_ptr<Vertex> v, m_vertices)
    {
	m_vertexSet.insert (v);
    }
    BOOST_FOREACH (boost::shared_ptr<Edge> e, m_edges)
    {
	e->Unwrap (this);
	m_edgeSet.insert (e);
    }
    BOOST_FOREACH (boost::shared_ptr<Face> f, m_faces)
    {
	f->Unwrap (this);
	f->CalculateNormal ();
	m_faceSet.insert (f);
    }
    BOOST_FOREACH (boost::shared_ptr<Body> b, m_bodies)
    {
	b->Unwrap (this);
    }
}

void Foam::PrintFaceInformation (ostream& ostr) const
{
    BOOST_FOREACH (boost::shared_ptr<Face> f, m_faces)
	f->PrintBodyPartOfInformation (ostr);
}

void Foam::PrintEdgeInformation (ostream& ostr) const
{
    BOOST_FOREACH (boost::shared_ptr<Edge> e, m_edges)
	e->PrintFacePartOfInformation (ostr);
}


void Foam::PostProcess ()
{
    Compact ();
    UpdatePartOf ();
    if (IsTorus ())
	Unwrap ();
    CalculateAABox ();
    CacheEdgesVerticesInBodies ();
    CalculateBodiesCenters ();
    if (IsTorus ())
    {
	BodiesInsideOriginalDomain ();
	CalculateTorusClipped ();
    }
}

bool Foam::IsTorus () const
{
    return ! GetOriginalDomain ().IsZero ();
}

void Foam::PrintDomains (ostream& ostr) const
{
    Vertex::PrintDomains(ostr, m_vertices);
}


void Foam::AddAttributeInfo (
    DefineAttribute::Type type, const char* name,
    auto_ptr<AttributeCreator> creator)
{
    m_attributesInfo[type].AddAttributeInfo (name, creator);
    m_parsingData->AddAttribute (name);
}

void Foam::BodiesInsideOriginalDomain ()
{
    for_each (m_bodies.begin (), m_bodies.end (),
	      boost::bind (&Foam::BodyInsideOriginalDomain, this, _1));
}

Foam::Bodies::iterator Foam::BodyInsideOriginalDomainStep (
    Foam::Bodies::iterator begin)
{
    cdbg << "BodyInsideOriginalDomainStep" << endl;
    Bodies::iterator it = begin;
    while (it != m_bodies.end () && BodyInsideOriginalDomain (*it))
	++it;
    if (it == m_bodies.end ())
	return it;
    else
	return ++it;
}


bool Foam::BodyInsideOriginalDomain (const boost::shared_ptr<Body>& body)
{
    using G3D::Vector3int16;
    Vector3int16 centerLocation = 
	GetOriginalDomain ().GetLocation (body->GetCenter ());
    if (centerLocation == Vector3int16Zero)
	return true;
    Vector3int16 translation = Vector3int16Zero - centerLocation;
    BodyTranslate (body, translation);
    return false;
}

void Foam::BodyTranslate (const boost::shared_ptr<Body>& body,
			  const G3D::Vector3int16& translate)
{
    BOOST_FOREACH (boost::shared_ptr<OrientedFace> of, body->GetOrientedFaces ())
    {
	const Face& original = *of->GetFace ();
	boost::shared_ptr<Face> duplicate = GetFaceDuplicate (
	    original, translate);
	of->SetFace (duplicate);
    }
    body->CacheEdgesVertices (GetSpaceDimension (), IsQuadratic ());
    body->CalculateCenter ();
}


// Static and Friends Methods
// ======================================================================
ostream& operator<< (ostream& ostr, const Foam& d)
{
    ostr << "Foam:\n";
    ostr << "AABox:\n";
    ostr << d.m_AABox << endl;
    {
	ostr << "view matrix:\n";
	ostream_iterator<float> o (ostr, " ");
	copy (d.m_viewMatrix.begin (), d.m_viewMatrix.end (), o);
	ostr << endl;
    }
    if (d.IsTorus ())
    {
	ostr << "torus periods:\n";
	ostr << d.m_originalDomain;
    }

    ostr << "vertices:\n";
    ostream_iterator< boost::shared_ptr<Vertex> > vOutput (ostr, "\n");
    copy (d.m_vertices.begin (), d.m_vertices.end (), vOutput);
    
    ostr << "edges:\n";
    ostream_iterator<boost::shared_ptr<Edge> > eOutput (ostr, "\n");
    copy (d.m_edges.begin (), d.m_edges.end (), eOutput);

    ostr << "faces:\n";
    ostream_iterator<boost::shared_ptr<Face> > fOutput (ostr, "\n");
    copy (d.m_faces.begin (), d.m_faces.end (), fOutput);

    ostr << "bodies:\n";
    ostream_iterator<boost::shared_ptr<Body> > bOutput (ostr, "\n");
    copy (d.m_bodies.begin (), d.m_bodies.end (), bOutput);
    return ostr;
}
