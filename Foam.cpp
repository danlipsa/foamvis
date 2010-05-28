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
#include "ElementUtils.h"
#include "OrientedEdge.h"
#include "ParsingData.h"
#include "ProcessBodyTorus.h"
#include "Vertex.h"

// Private Classes
// ======================================================================

struct EdgeSearchDummy
{
    EdgeSearchDummy (const G3D::Vector3& position, size_t edgeOriginalIndex) : 
	m_vertex (position), m_edge (&m_vertex, edgeOriginalIndex) {}
    Vertex m_vertex;
    Edge m_edge;
};

struct FaceSearchDummy
{
    FaceSearchDummy (const G3D::Vector3& position, size_t faceOriginalIndex) : 
	m_vertex (position), m_edge (&m_vertex, 0),
	m_face (&m_edge, faceOriginalIndex) {}
    Vertex m_vertex;
    Edge m_edge;
    Face m_face;
};

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
	  typename ContainerIterator, typename ContainerKeyType>
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
void compact (vector<E*>& v)
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
    m_spaceDimension (3)
{
    fill (m_viewMatrix.begin (), m_viewMatrix.end (), 0);
    Vertex::StoreDefaultAttributes (&m_attributesInfo[DefineAttribute::VERTEX]);
    Edge::StoreDefaultAttributes (&m_attributesInfo[DefineAttribute::EDGE]);
    Face::StoreDefaultAttributes (&m_attributesInfo[DefineAttribute::FACE]);
    Body::StoreDefaultAttributes (&m_attributesInfo[DefineAttribute::BODY]);
}

Foam::~Foam ()
{
    for_each(m_bodies.begin (), m_bodies.end (), bl::delete_ptr());
    for_each(m_faces.begin (), m_faces.end (), bl::delete_ptr ());
    for_each(m_edges.begin (), m_edges.end (), bl::delete_ptr ());
    for_each(m_vertices.begin (), m_vertices.end (), bl::delete_ptr ());
    delete m_parsingData;
}

Vertex* Foam::GetVertexDuplicate (
    Vertex* original, const G3D::Vector3int16& translation)
{
    Vertex searchDummy (
	GetPeriods ().TorusTranslate (*original, translation));
    VertexSet::iterator it = fuzzyFind 
	<VertexSet, VertexSet::iterator, VertexSet::key_type> (
	    m_vertexSet, &searchDummy);
    if (it != m_vertexSet.end ())
	return *it;
    Vertex* duplicate = CreateVertexDuplicate (original, translation);
    m_vertexSet.insert (duplicate);
    m_vertices.push_back (duplicate);
    return duplicate;
}

Edge* Foam::GetEdgeDuplicate (
    Edge* original, const G3D::Vector3& newBegin)
{
    EdgeSearchDummy searchDummy (newBegin, original->GetId ());
    EdgeSet::iterator it = 
	fuzzyFind <EdgeSet, EdgeSet::iterator, EdgeSet::key_type> (
	    m_edgeSet, &searchDummy.m_edge);
    if (it != m_edgeSet.end ())
	return *it;
    Edge* duplicate = CreateEdgeDuplicate (original, newBegin);
    m_edgeSet.insert (duplicate);
    m_edges.push_back (duplicate);
    return duplicate;
}

Face* Foam::GetFaceDuplicate (
    const Face& original, const G3D::Vector3int16& translation)
{
    const G3D::Vector3* begin = original.GetOrientedEdge (0)->GetBegin ();
    const G3D::Vector3& newBegin = 
	GetPeriods ().TorusTranslate (*begin, translation);
    FaceSearchDummy searchDummy (newBegin, original.GetId ());
    FaceSet::iterator it = m_faceSet.find (&searchDummy.m_face);
    if (it != m_faceSet.end ())
	return *it;
    Face* duplicate = CreateFaceDuplicate (original, newBegin);
    m_faceSet.insert (duplicate);
    m_faces.push_back (duplicate);
    return duplicate;
}

Vertex* Foam::CreateVertexDuplicate (
    Vertex* original, const G3D::Vector3int16& translation)
{
    original->SetStatus (ElementStatus::DUPLICATE_MADE);
    Vertex* duplicate = new Vertex (*original);
    duplicate->SetStatus (ElementStatus::DUPLICATE);
    TorusTranslate (duplicate, translation);
    return duplicate;
}

void Foam::TorusTranslate (
    Vertex* vertex, const G3D::Vector3int16& translation) const
{
    *static_cast<G3D::Vector3*>(vertex) = 
	GetPeriods ().TorusTranslate (*vertex, translation);
}


Edge* Foam::CreateEdgeDuplicate (Edge* original, const G3D::Vector3& newBegin)
{
    original->SetStatus (ElementStatus::DUPLICATE_MADE);
    G3D::Vector3int16 translation = GetPeriods ().GetTranslation (
	*original->GetBegin (), newBegin);
    Vertex* beginDuplicate = GetVertexDuplicate (
	original->GetBegin (), translation);
    Vertex* endDuplicate = GetVertexDuplicate (
	original->GetEnd (), translation);
    Edge* duplicate = new Edge (*original);
    duplicate->SetBegin (beginDuplicate);
    duplicate->SetEnd (endDuplicate);
    duplicate->SetStatus (ElementStatus::DUPLICATE);
    return duplicate;
}

Face* Foam::CreateFaceDuplicate (
    const Face& original, const G3D::Vector3& newBegin)
{
    Face* faceDuplicate = new Face (original);
    faceDuplicate->SetStatus (ElementStatus::DUPLICATE);
    G3D::Vector3 begin = newBegin;
    BOOST_FOREACH (OrientedEdge* oe, faceDuplicate->GetOrientedEdges ())
    {
	G3D::Vector3 edgeBegin;
	if (oe->IsReversed ())
	    edgeBegin = oe->GetEdge ()->GetTranslatedBegin (begin);
	else
	    edgeBegin = begin;
	Edge* edgeDuplicate = GetEdgeDuplicate (oe->GetEdge (), edgeBegin);
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
    Vertex* vertex = new Vertex (x, y ,z, i);
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
    Edge* edge = new Edge (GetVertex(begin), GetVertex(end), endTranslation, i);
    if (&list != 0)
        edge->StoreAttributes (list, m_attributesInfo[DefineAttribute::EDGE]);
    m_edges[i] = edge;
}

void Foam::SetFace (size_t i,  vector<int>& edges,
                    vector<NameSemanticValue*>& list)
{
    if (i >= m_faces.size ())
        m_faces.resize (i + 1);
    Face* face = new Face (edges, m_edges, i);
    if (&list != 0)
        face->StoreAttributes (list, m_attributesInfo[DefineAttribute::FACE]);
    m_faces[i] = face;
}

void Foam::SetBody (size_t i,  vector<int>& faces,
                    vector<NameSemanticValue*>& list)
{
    if (i >= m_bodies.size ())
        m_bodies.resize (i + 1);
    Body* body = new Body (faces, m_faces, i);
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
    delete m_parsingData;
    m_parsingData = 0;
}

void Foam::UpdatePartOf ()
{
    using boost::mem_fn;
    for_each (m_bodies.begin (), m_bodies.end (), 
	      mem_fn(&Body::UpdatePartOf));
    for_each (m_edges.begin (), m_edges.end (), 
	      mem_fn (&Edge::UpdateEdgePartOf));
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
    Vector3 first = m_periods[0];
    Vector3 second = m_periods[1];
    Vector3 sum = first + second;
    Vector3 third = m_periods[2];
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
	      mem_fun(&Body::CacheEdgesVertices));
}

void Foam::CalculateBodiesCenters ()
{
    for_each (m_bodies.begin (), m_bodies.end (),
	      mem_fun(&Body::CalculateCenter));
}

void Foam::CalculateTorusClipped ()
{
    const OOBox& periods = GetPeriods ();
    BOOST_FOREACH (Edge* e, m_edges)
    {
	if (e->IsClipped ())
	    e->CalculateTorusClipped (periods);
    }
}

void Foam::Unwrap ()
{
    BOOST_FOREACH (Vertex* v, m_vertices)
    {
	m_vertexSet.insert (v);
    }
    BOOST_FOREACH (Edge* e, m_edges)
    {
	e->Unwrap (this);
	m_edgeSet.insert (e);
    }
    BOOST_FOREACH (Face* f, m_faces)
    {
	f->Unwrap (this);
	f->CalculateNormal ();
	m_faceSet.insert (f);
    }
    BOOST_FOREACH (Body* b, m_bodies)
    {
	b->Unwrap (this);
    }
}

ostream& Foam::PrintFaceInformation (ostream& ostr) const
{
    BOOST_FOREACH (Body* b, m_bodies)
	b->PrintFaceInformation (ostr);
    return ostr;
}

void Foam::PostProcess ()
{
    Compact ();
    UpdatePartOf ();
    //PrintFaceInformation (cdbg);
    if (IsTorus ())
	Unwrap ();
    if (GetSpaceDimension () == 2)
    {
	for_each (m_vertices.begin (), m_vertices.end (),
		  boost::bind (&Vertex::SetPhysical, _1, true));
	for_each (m_edges.begin (), m_edges.end (),
		  boost::bind (&Edge::SetPhysical, _1, true));
    }
    CalculateAABox ();
    CacheEdgesVerticesInBodies ();
    if (! IsTorus () || GetSpaceDimension () == 2)
	CalculateBodiesCenters ();
    if (IsTorus ())
	CalculateTorusClipped ();
}

bool Foam::IsTorus () const
{
    return ! (GetPeriod (0).isZero () && GetPeriod (1).isZero () && 
	      GetPeriod (2).isZero ());
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
	ostr << d.m_periods;
    }

    ostr << "vertices:\n";
    ostream_iterator<Vertex*> vOutput (ostr, "\n");
    copy (d.m_vertices.begin (), d.m_vertices.end (), vOutput);
    
    ostr << "edges:\n";
    ostream_iterator<Edge*> eOutput (ostr, "\n");
    copy (d.m_edges.begin (), d.m_edges.end (), eOutput);

    ostr << "faces:\n";
    ostream_iterator<Face*> fOutput (ostr, "\n");
    copy (d.m_faces.begin (), d.m_faces.end (), fOutput);

    ostr << "bodies:\n";
    ostream_iterator<Body*> bOutput (ostr, "\n");
    copy (d.m_bodies.begin (), d.m_bodies.end (), bOutput);

    Vertex::PrintDomains (ostr, d.m_vertices);
    return ostr;
}
