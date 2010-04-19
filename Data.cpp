/**
 * @file   Data.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Data object
 */
#include "AttributeInfo.h"
#include "Body.h"
#include "Data.h"
#include "Edge.h"
#include "Face.h"
#include "ElementUtils.h"
#include "OrientedEdge.h"
#include "ParsingData.h"
#include "Vertex.h"

struct EdgeSearchDummy
{
    EdgeSearchDummy (const G3D::Vector3* position, Data* data,
		     size_t edgeOriginalIndex) : 
	m_vertex (position, data), m_edge (&m_vertex, edgeOriginalIndex) {}
    Vertex m_vertex;
    Edge m_edge;
};

struct FaceSearchDummy
{
    FaceSearchDummy (const G3D::Vector3* position, Data* data,
		     size_t faceOriginalIndex) : 
	m_vertex (position, data), m_edge (&m_vertex, 0),
	m_face (&m_edge, faceOriginalIndex) {}
    Vertex m_vertex;
    Edge m_edge;
    Face m_face;
};

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

ostream& operator<< (ostream& ostr, Data& d)
{
    ostr << "Data:\n";
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
	ostream_iterator<G3D::Vector3> o (ostr, "\n");
	copy (d.m_periods.begin (), d.m_periods.end (), o);
	ostr << endl;
    }

    ostr << "vertices:\n";
    ostream_iterator<Vertex*> vOutput (ostr, "\n");
    copy (d.m_vertices.begin (), d.m_vertices.end (), vOutput);
    
    ostr << "edges:\n";
    ostream_iterator<Edge*> eOutput (ostr, "\n");
    copy (d.m_edges.begin (), d.m_edges.end (), eOutput);

    ostr << "faces:";
    ostream_iterator<Face*> fOutput (ostr, "\n");
    copy (d.m_faces.begin (), d.m_faces.end (), fOutput);

    ostr << "bodies:\n";
    ostream_iterator<Body*> bOutput (ostr, "\n");
    copy (d.m_bodies.begin (), d.m_bodies.end (), bOutput);

    Vertex::PrintDomains (ostr, d.m_vertices);
    d.PrintFacesWithIntersection (ostr);
    return ostr;
}

Data::Data () : 
    m_parsingData (new ParsingData ()),
    m_spaceDimension (3)
{
    fill (m_viewMatrix.begin (), m_viewMatrix.end (), 0);
    Vertex::StoreDefaultAttributes (&m_attributesInfo[DefineAttribute::VERTEX]);
    Edge::StoreDefaultAttributes (&m_attributesInfo[DefineAttribute::EDGE]);
    Face::StoreDefaultAttributes (&m_attributesInfo[DefineAttribute::FACE]);
    Body::StoreDefaultAttributes (&m_attributesInfo[DefineAttribute::BODY]);
}

Data::~Data ()
{
    using boost::bind;
    for_each(m_bodies.begin (), m_bodies.end (),
	     bind ( DeletePointer<Body>(), _1));
    for_each(m_faces.begin (), m_faces.end (),
	     bind (DeletePointer<Face> (), _1));
    for_each(m_edges.begin (), m_edges.end (),
	     bind (DeletePointer<Edge> (), _1));
    for_each(m_vertices.begin (), m_vertices.end (),
	     bind (DeletePointer<Vertex> (), _1));
    delete m_parsingData;
}

void Data::SetVertex (size_t i, float x, float y, float z,
                     vector<NameSemanticValue*>& list) 
{
    if (i >= m_vertices.size ())
        m_vertices.resize (i + 1);
    Vertex* vertex = new Vertex (x, y ,z, i, this);
    if (&list != 0)
        vertex->StoreAttributes (
            list, m_attributesInfo[DefineAttribute::VERTEX]);
    m_vertices[i] = vertex;
    m_vertexSet.insert (vertex);
}

Vertex* Data::GetVertexDuplicate (
    const Vertex& original, const G3D::Vector3int16& domainIncrement)
{
    Vertex searchDummy (&original, this, domainIncrement);
    VertexSet::iterator it = m_vertexSet.find (&searchDummy);
    if (it != m_vertexSet.end ())
	return *it;
    Vertex* duplicate = original.CreateDuplicate (domainIncrement);
    m_vertexSet.insert (duplicate);
    m_vertices.push_back (duplicate);
    return duplicate;
}



Edge* Data::GetEdgeDuplicate (
    const Edge& original, const G3D::Vector3& newBegin)
{
    EdgeSearchDummy searchDummy (&newBegin, this, original.GetOriginalIndex ());
    EdgeSet::iterator it = m_edgeSet.find (&searchDummy.m_edge);
    if (it != m_edgeSet.end ())
	return *it;
    Edge* duplicate = original.CreateDuplicate (newBegin);
    m_edgeSet.insert (duplicate);
    m_edges.push_back (duplicate);
    return duplicate;
}

Face* Data::GetFaceDuplicate (
    const Face& original, const G3D::Vector3& newBegin)
{
    FaceSearchDummy searchDummy (&newBegin, this, original.GetOriginalIndex ());
    FaceSet::iterator it = m_faceSet.find (&searchDummy.m_face);
    if (it != m_faceSet.end ())
	return *it;
    Face* duplicate = original.CreateDuplicate (newBegin);
    m_faceSet.insert (duplicate);
    m_faces.push_back (duplicate);
    /*
    cdbg << "Original face:" << endl << original << endl
	 << "Duplicate face:" << endl << *duplicate << endl;
    */
    return duplicate;
}


void Data::SetEdge (size_t i, size_t begin, size_t end,
		    G3D::Vector3int16& domainIncrement,
                    vector<NameSemanticValue*>& list) 
{
    if (i >= m_edges.size ())
        m_edges.resize (i + 1); 
    Edge* edge = new Edge (
	GetVertex(begin), GetVertex(end), domainIncrement, i, this);
    if (&list != 0)
        edge->StoreAttributes (list, m_attributesInfo[DefineAttribute::EDGE]);
    m_edges[i] = edge;
    m_edgeSet.insert (edge);
}

void Data::SetFace (size_t i,  vector<int>& edges,
                    vector<NameSemanticValue*>& list)
{
    if (i >= m_faces.size ())
        m_faces.resize (i + 1);
    Face* face = new Face (edges, m_edges, i, this);
    if (&list != 0)
        face->StoreAttributes (list, m_attributesInfo[DefineAttribute::FACE]);
    m_faces[i] = face;
    m_faceSet.insert (face);
}

void Data::SetBody (size_t i,  vector<int>& faces,
                    vector<NameSemanticValue*>& list)
{
    if (i >= m_bodies.size ())
        m_bodies.resize (i + 1);
    Body* body = new Body (faces, m_faces, i, this);
    if (&list != 0)
        body->StoreAttributes (list,
			       m_attributesInfo[DefineAttribute::BODY]);    
    m_bodies[i] = body;
}



void Data::Compact (void)
{
    using boost::bind;
    compact (m_vertices);
    compact (m_edges);
    compact (m_faces);
    compact (m_bodies);
    for_each (m_bodies.begin (), m_bodies.end (),
	      bind(&Data::InsertOriginalIndexBodyMap, this, _1));
}

Body* Data::GetBody (size_t i)
{
    map<size_t, Body*>::iterator it = 
	m_originalIndexBodyMap.find (i);
    if (it == m_originalIndexBodyMap.end ())
	return 0;
    else
	return it->second;
}

void Data::InsertOriginalIndexBodyMap (Body* body)
{
    m_originalIndexBodyMap[body->GetOriginalIndex ()] = body;
}

void Data::ReleaseParsingData ()
{
    delete m_parsingData;
    m_parsingData = 0;
}

void Data::CalculatePhysical ()
{
    using boost::bind;
    for_each (m_bodies.begin (), m_bodies.end (), 
	      bind (&Body::UpdateFacesAdjacency, _1));
    for_each (m_faces.begin (), m_faces.end (), 
	      bind (&Face::UpdateEdgesAdjacency, _1));
    for_each (m_edges.begin (), m_edges.end (), 
	      bind (&Edge::UpdateVerticesAdjacency, _1));
}

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


void Data::CalculateAABox ()
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

G3D::Vector3* Vector3Address (G3D::Vector3& v)
{
    return &v;
}

void Data::calculateAABoxForTorus (G3D::Vector3* low, G3D::Vector3* high)
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

void Data::CacheEdgesVerticesInBodies ()
{
    for_each (m_bodies.begin (), m_bodies.end (), 
	      mem_fun(&Body::CacheEdgesVertices));
}

void Data::CalculateBodiesCenters ()
{
    for_each (m_bodies.begin (), m_bodies.end (),
	      mem_fun(&Body::CalculateCenter));
}

void Data::PostProcess ()
{
    Compact ();
    CalculatePhysical ();
    CalculateAABox ();
    CacheEdgesVerticesInBodies ();
    if (! IsTorus () && GetSpaceDimension () == 3)
	CalculateBodiesCenters ();
}

size_t countIntersections (OrientedEdge* e)
{
    const G3D::Vector3int16& domainIncrement = 
	e->GetEdge ()->GetEndDomainIncrement ();
    return ((domainIncrement.x != 0) + 
	    (domainIncrement.y != 0) + (domainIncrement.z != 0));
}

class printFaceIfIntersection
{
public:
    printFaceIfIntersection (ostream& ostr) : m_ostr(ostr) {}
    void operator () (Face* f)
    {
	vector<OrientedEdge*>::iterator it;
	vector<OrientedEdge*>& v = f->GetOrientedEdges ();
	vector<size_t> intersections(v.size ());
	transform (
	    v.begin (), v.end (), intersections.begin (), countIntersections);
	size_t totalIntersections = accumulate (
	    intersections.begin (), intersections.end (), 0);
	m_ostr << f->GetOriginalIndex () << " has " 
	       << totalIntersections << " intersections" << endl;
    }
private:
    ostream& m_ostr;
};


ostream& Data::PrintFacesWithIntersection (ostream& ostr) const
{
    ostr << "Face intersections:" << endl;
    for_each(m_faces.begin (), m_faces.end (), printFaceIfIntersection (ostr));
    return ostr;
}

G3D::Vector3int16 Data::GetDomainIncrement (
    const G3D::Vector3& original, const G3D::Vector3& duplicate) const
{
    using G3D::Matrix3;
    using G3D::Matrix2;
    using G3D::Vector3;
    using G3D::Vector3int16;
    Matrix3 toOrthonormal;
    if (GetSpaceDimension () == 2)
    {
	Matrix2 toPeriods (GetPeriod (0).x, GetPeriod (1).x,
			   GetPeriod (0).y, GetPeriod (1).y);
	// inverse does not work in G3D 7.01
	const Matrix2& toOrthonormal2d = inverse (toPeriods);
	const float* v = toOrthonormal2d[0];
	toOrthonormal.setRow (0, Vector3 (v[0], v[1], 0));
	v = toOrthonormal2d[1];
	toOrthonormal.setRow (1, Vector3 (v[0], v[1], 0));
	toOrthonormal.setRow (2, Vector3::zero ());
    }
    else
    {
	Matrix3 toPeriods;
	for (int i = 0; i < 3; i++)
	    toPeriods.setColumn (i, GetPeriod (i));
	toOrthonormal = toPeriods.inverse ();
    }
    Vector3 o = toOrthonormal * original;
    Vector3 d = toOrthonormal * duplicate;
    Vector3int16 originalDomain (floorf (o.x), floorf(o.y), floorf(o.z));
    Vector3int16 duplicateDomain (floorf (d.x), floorf(d.y), floorf(d.z));
    return duplicateDomain - originalDomain;
}

bool Data::IsTorus () const
{
    return 
	! (GetPeriod (0).isZero () && 
	   GetPeriod (1).isZero () && 
	   GetPeriod (2).isZero ());
}

void Data::PrintDomains (ostream& ostr) const
{
    Vertex::PrintDomains(ostr, m_vertices);
}

void Data::AddAttributeInfo (
    DefineAttribute::Type type, const char* name, AttributeCreator* creator)
{
    m_attributesInfo[type].AddAttributeInfo (name, creator);
    m_parsingData->AddAttribute (name);
}
