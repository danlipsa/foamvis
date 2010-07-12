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
	ContainerIterator first, ContainerIterator second, 
	VertexLessThanAlong lessThan);

    void operator() (AggregateOnContainer aggregate,
		     G3D::Vector3* v, Container& vertices)
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

template<typename Element>
void copyStandalone (const vector< boost::shared_ptr<Element> >& source,
		     vector< boost::shared_ptr<Element> >* destination)
{
    BOOST_FOREACH (boost::shared_ptr<Element> e, source)
    {
	if (e->IsStandalone ())
	    destination->push_back (e);
    }
}


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

boost::shared_ptr<Vertex> Foam::getVertexDuplicate (
    const Vertex& original, const G3D::Vector3int16& translation,
    VertexSet* vertexSet) const
{
    boost::shared_ptr<Vertex> searchDummy = boost::make_shared<Vertex>(
	GetOriginalDomain ().TorusTranslate (original, translation));
    VertexSet::iterator it = fuzzyFind 
	<VertexSet, VertexSet::iterator, VertexSet::key_type> (
	    *vertexSet, searchDummy);
    if (it != vertexSet->end ())
	return *it;
    boost::shared_ptr<Vertex> duplicate = createVertexDuplicate (
	original, translation);
    vertexSet->insert (duplicate);
    return duplicate;
}

boost::shared_ptr<Edge> Foam::getEdgeDuplicate (
    const Edge& original, const G3D::Vector3& newBegin,
    VertexSet* vertexSet, EdgeSet* edgeSet) const
{
    boost::shared_ptr<Edge> searchDummy = 
	boost::make_shared<Edge>(
	    boost::make_shared<Vertex> (newBegin), original.GetId ());
    EdgeSet::iterator it = 
	fuzzyFind <EdgeSet, EdgeSet::iterator, EdgeSet::key_type> (
	    *edgeSet, searchDummy);
    if (it != edgeSet->end ())
	return *it;
    boost::shared_ptr<Edge> duplicate = createEdgeDuplicate (
	original, newBegin, vertexSet);
    edgeSet->insert (duplicate);
    return duplicate;
}

boost::shared_ptr<Face> Foam::GetFaceDuplicate (
    const Face& original, const G3D::Vector3int16& translation,
    VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet) const
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
	    *faceSet, searchDummy);
    if (it != faceSet->end ())
	return *it;
    boost::shared_ptr<Face> duplicate = createFaceDuplicate (
	original, newBegin, vertexSet, edgeSet);
    faceSet->insert (duplicate);
    return duplicate;
}

boost::shared_ptr<Vertex> Foam::createVertexDuplicate (
    const Vertex& original, const G3D::Vector3int16& translation) const
{
    boost::shared_ptr<Vertex> duplicate = boost::make_shared<Vertex> (
	original);
    duplicate->SetDuplicateStatus (ElementStatus::DUPLICATE);
    torusTranslate (duplicate.get (), translation);
    return duplicate;
}

void Foam::torusTranslate (
    Vertex* vertex, const G3D::Vector3int16& translation) const
{
    *static_cast<G3D::Vector3*>(vertex) = 
	GetOriginalDomain ().TorusTranslate (*vertex, translation);
}


boost::shared_ptr<Edge> Foam::createEdgeDuplicate (
    const Edge& original, const G3D::Vector3& newBegin, 
    VertexSet* vertexSet) const
{
    G3D::Vector3int16 translation = GetOriginalDomain ().GetTranslation (
	*original.GetBegin (), newBegin);
    boost::shared_ptr<Vertex> beginDuplicate = getVertexDuplicate (
	*original.GetBegin (), translation, vertexSet);
    boost::shared_ptr<Vertex> endDuplicate = getVertexDuplicate (
	*original.GetEnd (), translation, vertexSet);
    boost::shared_ptr<Edge> duplicate = boost::make_shared<Edge> (original);
    duplicate->SetBegin (beginDuplicate);
    duplicate->SetEnd (endDuplicate);
    duplicate->SetDuplicateStatus (ElementStatus::DUPLICATE);
    return duplicate;
}

boost::shared_ptr<Face> Foam::createFaceDuplicate (
    const Face& original, const G3D::Vector3& newBegin,
    VertexSet* vertexSet, EdgeSet* edgeSet) const
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
	    getEdgeDuplicate (*oe->GetEdge (), edgeBegin, vertexSet, edgeSet);
	oe->SetEdge (edgeDuplicate);
	begin = *oe->GetEnd ();
    }
    return faceDuplicate;
}

void Foam::SetBody (size_t i, vector<int>& faces,
                    vector<NameSemanticValue*>& attributes)
{
    if (i >= m_bodies.size ())
        m_bodies.resize (i + 1);
    boost::shared_ptr<Body> body = boost::make_shared<Body> (
	faces, GetParsingData ().GetFaces (), i);
    if (&attributes != 0)
        body->StoreAttributes (attributes,
			       m_attributesInfo[DefineAttribute::BODY]);    
    m_bodies[i] = body;
}



void Foam::compact (void)
{
    ::compact (GetParsingData ().GetVertices ());
    ::compact (GetParsingData ().GetEdges ());
    ::compact (GetParsingData ().GetFaces ());
    ::compact (m_bodies);
}


void Foam::ReleaseParsingData ()
{
    m_parsingData.reset ();
}

void Foam::updatePartOf ()
{
    using boost::mem_fn;
    for_each (m_bodies.begin (), m_bodies.end (), 
	      boost::bind(&Body::UpdatePartOf, _1, _1));
    ParsingData::Edges edges = GetParsingData ().GetEdges ();
    for_each (edges.begin (), edges.end (), 
	      boost::bind (&Edge::UpdateEdgePartOf, _1, _1));
}

void Foam::calculateAABox ()
{
    using G3D::Vector3;
    Vector3 low, high;
    VertexSet vertexSet;
    GetVertexSet (&vertexSet);
    calculateAggregate <VertexSet, VertexSet::iterator>() (
	min_element, &low, vertexSet);
    calculateAggregate <VertexSet, VertexSet::iterator>()(
	max_element, &high, vertexSet);
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
	    *low, 
	    origin, first, sum, second,
	    origin + third, first + third, sum + third, second + third, 
	    *high
    }};
    vector<Vector3*> v(additionalVertices.size ());
    transform (additionalVertices.begin (), additionalVertices.end (),
	       v.begin (), Vector3Address);
    calculateAggregate<vector<Vector3*>, vector<Vector3*>::iterator>() (
	min_element, low, v);
    calculateAggregate<vector<Vector3*>, vector<Vector3*>::iterator>() (
	max_element, high, v);
}

void Foam::calculateBodiesCenters ()
{
    for_each (m_bodies.begin (), m_bodies.end (),
	      boost::bind(&Body::CalculateCenter, _1,
			  GetSpaceDimension (), IsQuadratic ()));
}

void Foam::calculateTorusClipped ()
{
    const OOBox& periods = GetOriginalDomain ();
    BOOST_FOREACH (boost::shared_ptr<Edge> e, GetParsingData ().GetEdges ())
    {
	if (e->IsClipped ())
	    e->CalculateTorusClipped (periods);
    }
}

void Foam::unwrap (VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet)
{
    BOOST_FOREACH (boost::shared_ptr<Vertex> v, GetParsingData ().GetVertices ())
    {
	vertexSet->insert (v);
    }
    BOOST_FOREACH (boost::shared_ptr<Edge> e, GetParsingData ().GetEdges ())
    {
	unwrap (e, vertexSet);
	edgeSet->insert (e);
    }
    BOOST_FOREACH (boost::shared_ptr<Face> f, GetParsingData ().GetFaces ())
    {
	unwrap (f, vertexSet, edgeSet);
	f->CalculateNormal ();
	faceSet->insert (f);
    }
    BOOST_FOREACH (boost::shared_ptr<Body> b, GetBodies ())
    {
	unwrap (b, vertexSet, edgeSet, faceSet);
    }
}

void Foam::unwrap (boost::shared_ptr<Edge> edge, VertexSet* vertexSet) const
{
    if (edge->GetEndTranslation () != Vector3int16Zero)
	edge->SetEnd (
	    getVertexDuplicate (
		*edge->GetEnd (), edge->GetEndTranslation (), vertexSet));
}

void Foam::unwrap (boost::shared_ptr<Face> face, 
		   VertexSet* vertexSet, EdgeSet* edgeSet) const
{
    Face::OrientedEdges& orientedEdges = face->GetOrientedEdges ();
    G3D::Vector3* begin = (*orientedEdges.begin())->GetBegin ().get ();
    BOOST_FOREACH (boost::shared_ptr<OrientedEdge> oe, orientedEdges)
    {
	boost::shared_ptr<Edge>  edge = oe->GetEdge ();
	G3D::Vector3 edgeBegin = 
	    (oe->IsReversed ()) ? edge->GetTranslatedBegin (*begin) : *begin;
	oe->SetEdge (getEdgeDuplicate (*edge, edgeBegin, vertexSet, edgeSet));
	begin = oe->GetEnd ().get ();
    }
}

void Foam::unwrap (
    boost::shared_ptr<Body> body,
    VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet) const
{
    ProcessBodyTorus(*this, body).Unwrap (vertexSet, edgeSet, faceSet);
}




void Foam::copyStandaloneElements ()
{
    copyStandalone (GetParsingData ().GetEdges (), &m_standaloneEdges);
    copyStandalone (GetParsingData ().GetFaces (), &m_standaloneFaces);
}

void Foam::PostProcess ()
{
    VertexSet vertexSet;
    EdgeSet edgeSet;
    FaceSet faceSet;
    compact ();
    updatePartOf ();
    copyStandaloneElements ();
    if (IsTorus ())
	unwrap (&vertexSet, &edgeSet, &faceSet);
    calculateAABox ();
    calculateBodiesCenters ();
    if (IsTorus ())
    {
	bodiesInsideOriginalDomain (&vertexSet, &edgeSet, &faceSet);
	calculateTorusClipped ();
    }
}

bool Foam::IsTorus () const
{
    return ! GetOriginalDomain ().IsZero ();
}


void Foam::AddAttributeInfo (
    DefineAttribute::Type type, const char* name,
    auto_ptr<AttributeCreator> creator)
{
    m_attributesInfo[type].AddAttributeInfo (name, creator);
    m_parsingData->AddAttribute (name);
}

void Foam::bodiesInsideOriginalDomain (
    VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet)
{
    for_each (m_bodies.begin (), m_bodies.end (),
	      boost::bind (&Foam::bodyInsideOriginalDomain, this,
			   _1, vertexSet, edgeSet, faceSet));
}

Foam::Bodies::iterator Foam::BodyInsideOriginalDomainStep (
    Foam::Bodies::iterator begin,
    VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet)
{
    cdbg << "BodyInsideOriginalDomainStep" << endl;
    Bodies::iterator it = begin;
    while (it != m_bodies.end () && 
	   bodyInsideOriginalDomain (*it, vertexSet, edgeSet, faceSet))
	++it;
    if (it == m_bodies.end ())
	return it;
    else
	return ++it;
}


bool Foam::bodyInsideOriginalDomain (
    const boost::shared_ptr<Body>& body,
    VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet)
{
    using G3D::Vector3int16;
    Vector3int16 centerLocation = 
	GetOriginalDomain ().GetLocation (body->GetCenter ());
    if (centerLocation == Vector3int16Zero)
	return true;
    Vector3int16 translation = Vector3int16Zero - centerLocation;
    bodyTranslate (body, translation, vertexSet, edgeSet, faceSet);
    return false;
}

void Foam::bodyTranslate (
    const boost::shared_ptr<Body>& body,const G3D::Vector3int16& translate,
    VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet)
{
    BOOST_FOREACH (boost::shared_ptr<OrientedFace> of, body->GetOrientedFaces ())
    {
	const Face& original = *of->GetFace ();
	boost::shared_ptr<Face> duplicate = GetFaceDuplicate (
	    original, translate, vertexSet, edgeSet, faceSet);
	of->SetFace (duplicate);
    }
    body->CalculateCenter (GetSpaceDimension (), IsQuadratic ());
}

void Foam::GetVertexSet (VertexSet* vertexSet) const
{
    const Bodies& bodies = GetBodies ();
    for_each (bodies.begin (), bodies.end (),
	      boost::bind (&Body::GetVertexSet, _1, vertexSet));
}

void Foam::GetEdgeSet (EdgeSet* edgeSet) const
{
    const Bodies& bodies = GetBodies ();
    for_each (bodies.begin (), bodies.end (),
	      boost::bind (&Body::GetEdgeSet, _1, edgeSet));
}

void Foam::GetFaceSet (FaceSet* faceSet) const
{
    const Bodies& bodies = GetBodies ();
    for_each (bodies.begin (), bodies.end (),
	      boost::bind (&Body::GetFaceSet, _1, faceSet));
}

const AttributesInfo& Foam::GetAttributesInfo (
    DefineAttribute::Type attributeType) const
{
    return m_attributesInfo[attributeType];
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

    ostr << "bodies:\n";
    ostream_iterator<boost::shared_ptr<Body> > bOutput (ostr, "\n");
    copy (d.m_bodies.begin (), d.m_bodies.end (), bOutput);
    return ostr;
}
