/**
 * @file   Data.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Data object
 */
#include "Data.h"
#include "ElementUtils.h"
#include "ParsingData.h"

/**
 * Prints a 4x4 matrix element. Used in for_each algorithm.
 */
class printMatrixElement : public unary_function<float, void>
{
public:
    /**
     * Constructs the object
     * @param ostr stream where to print the matrix element
     */
    printMatrixElement(ostream& ostr) : m_ostr(ostr), m_index (0) {}
    /**
     * Prints the matrix element to the output stream
     * @param f element to be printed.
     */
    void operator() (float f)
    {
        if (m_index != 0)
        {
            if ((m_index % 4) == 0)
                m_ostr << endl;
            else
                m_ostr << ", ";
        }
        m_ostr << f;
        m_index++;
    }
private:
    /**
     * Output stream where to print the matrix element
     */
    ostream& m_ostr;
    /**
     * Index of the element currently printed.
     */
    int m_index;
};

/**
 * Move elements in a vector toward the begining of the vector so that we 
 * eliminate holes.
 * @param v vector of elements
 */
template <typename E>
void compact (vector<E*>& v)
{
    unsigned int step = 0;
    for (unsigned int i = 0; i < v.size (); i++)
    {
        if (v[i] == 0)
            step++;
        else if (step != 0)
            v[i - step] = v[i];
    }
    unsigned int resize = v.size () - step;
    v.resize (resize);
}

/**
 * For all the edges in the face, add the face as being adjacent to the edge
 */
void updateFaceForEdges (Face* face)
{
     vector<OrientedEdge*>& orientedEdges = face->GetOrientedEdges ();
    for_each (orientedEdges.begin (), orientedEdges.end (),
	      bind2nd (mem_fun (&OrientedEdge::AddAdjacentFace), face));
}
/**
 * For both  vertices of this edge,  add the edge as  being adjacent to
 * the vertices
 */
void updateEdgeForVertices (Edge* edge)
{
    edge->GetBegin ()->AddAdjacentEdge (edge);
    edge->GetEnd ()->AddAdjacentEdge (edge);
}


ostream& operator<< (ostream& ostr, Data& d)
{
    ostr << "Data:" << endl;
    ostr << d.m_AABox << endl;
    PrintElements<Vertex*> (ostr, d.m_vertices, "vertices", true);
    PrintElements<Edge*> (ostr, d.m_edges, "edges", true);
    PrintElements<Face*> (ostr, d.m_faces, "faces", true);
    PrintElements<Body*> (ostr, d.m_bodies, "bodies", true);
    ostr << "view matrix:" << endl;
    for_each (d.m_viewMatrix.begin (), 
              d.m_viewMatrix.end (), 
              printMatrixElement (ostr));
    ostr << endl;
    Vertex::PrintDomains (ostr, d.m_vertices);
    d.PrintFacesWithIntersection (ostr);
    return ostr;
}

Data::Data () : 
    m_attributesInfo(DefineAttribute::COUNT),
    m_parsingData (new ParsingData ()),
    m_spaceDimension (3)
{
    using namespace boost::lambda;
    for_each (m_viewMatrix.begin (), m_viewMatrix.end (), _1 = 0);
    Vertex::StoreDefaultAttributes (m_attributesInfo[DefineAttribute::VERTEX]);
    Edge::StoreDefaultAttributes (m_attributesInfo[DefineAttribute::EDGE]);
    Face::StoreDefaultAttributes (m_attributesInfo[DefineAttribute::FACE]);
    Body::StoreDefaultAttributes (m_attributesInfo[DefineAttribute::BODY]);
}

Data::~Data ()
{
    using namespace boost::lambda;
    for_each(m_bodies.begin (), m_bodies.end (), bind (delete_ptr (), _1));
    for_each(m_faces.begin (), m_faces.end (), bind (delete_ptr (), _1));
    for_each(m_edges.begin (), m_edges.end (), bind (delete_ptr (), _1));
    for_each(m_vertices.begin (), m_vertices.end (), bind (delete_ptr (), _1));
    delete m_parsingData;
}

void Data::SetVertex (unsigned int i, float x, float y, float z,
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
    Vertex searchDummy (&original, this);
    searchDummy.AdjustPosition (domainIncrement);
    set<Vertex*, Vertex::LessThan>::iterator it = 
	m_vertexSet.find (&searchDummy);
    if (it != m_vertexSet.end ())
	return *it;
    Vertex* duplicate = new Vertex (original);
    duplicate->SetDuplicate (true);
    duplicate->AdjustPosition (domainIncrement);
    m_vertexSet.insert (duplicate);
    m_vertices.push_back (duplicate);
    return duplicate;
}

Edge* Data::GetEdgeDuplicate (Edge& original, G3D::Vector3& newBegin)
{
    using namespace G3D;
    Vertex beginDummy (&newBegin, this);
    Edge searchDummy (&beginDummy, original.GetOriginalIndex ());
    set<Edge*, Edge::LessThan>::iterator it = m_edgeSet.find (&searchDummy);
    if (it != m_edgeSet.end ())
	return *it;
    G3D::Vector3int16 domainIncrement = GetDomainIncrement (
	*original.GetBegin (), newBegin);
    Vertex* beginDuplicate = GetVertexDuplicate (*original.GetBegin (),
						 domainIncrement);
    Vertex* endDuplicate = GetVertexDuplicate (*original.GetEnd (),
					       domainIncrement);
    Edge* duplicate = new Edge (original);
    duplicate->SetDuplicate (true);
    duplicate->SetBegin (beginDuplicate);
    duplicate->SetEnd (endDuplicate);
    m_edgeSet.insert (duplicate);
    m_edges.push_back (duplicate);
    return duplicate;
}

void Data::SetEdge (unsigned int i, unsigned int begin, unsigned int end,
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

void Data::SetFace (unsigned int i,  vector<int>& edges,
                    vector<NameSemanticValue*>& list)
{
    if (i >= m_faces.size ())
        m_faces.resize (i + 1);
    Face* face = new Face (edges, m_edges, i, this);
    if (&list != 0)
        face->StoreAttributes (list, m_attributesInfo[DefineAttribute::FACE]);
    m_faces[i] = face;
}

void Data::SetBody (unsigned int i,  vector<int>& faces,
                    vector<NameSemanticValue*>& list)
{
    if (i >= m_bodies.size ())
        m_bodies.resize (i + 1);
    Body* body = new Body (faces, m_faces, i, this);
    if (&list != 0)
        body->StoreAttributes (list, m_attributesInfo[DefineAttribute::BODY]);    
    m_bodies[i] = body;
}



void Data::Compact (void)
{
    compact (m_vertices);
    compact (m_edges);
    compact (m_faces);
    compact (m_bodies);
    for_each (m_bodies.begin (), m_bodies.end (),
	      bind1st(mem_fun(&Data::InsertOriginalIndexBodyMap), this));
}

Body* Data::GetBody (unsigned int i)
{
    map<unsigned int, Body*>::iterator it = 
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

void updateBodyForFaces (Body* body)
{
     vector<OrientedFace*>& of = body->GetOrientedFaces ();
    for_each (of.begin (), of.end (),
	      bind2nd(mem_fun(&OrientedFace::AddAdjacentBody), body));
}

void Data::CalculatePhysical ()
{
    for_each (m_bodies.begin (), m_bodies.end (), updateBodyForFaces);
    for_each (m_faces.begin (), m_faces.end (), updateFaceForEdges);
    for_each (m_edges.begin (), m_edges.end (), updateEdgeForVertices);
}


void Data::Calculate (AggregateOnVertices aggregate, G3D::Vector3& v)
{
    using namespace G3D;
    IteratorVertices it;
    it = aggregate (m_vertices.begin (), m_vertices.end (), 
	    Vertex::LessThanAlong(Vector3::X_AXIS));;
    v.x = (*it)->x;
    it = aggregate (m_vertices.begin (), m_vertices.end (), 
	    Vertex::LessThanAlong(Vector3::Y_AXIS));
    v.y = (*it)->y;
    it = aggregate (m_vertices.begin (), m_vertices.end (), 
	    Vertex::LessThanAlong(Vector3::Z_AXIS));
    v.z = (*it)->z;
}


void Data::CalculateAABox ()
{
    Vector3 low, high;
    Calculate (min_element, low);
    Calculate (max_element, high);
    m_AABox.set(low, high);
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
    //CalculateBodiesCenters ();
}

unsigned int countIntersections (OrientedEdge* e)
{
     Vector3int16& domainIncrement = e->GetEdge ()->GetEndDomainIncrement ();
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
	vector<unsigned int> intersections(v.size ());
	transform (
	    v.begin (), v.end (), intersections.begin (), countIntersections);
	unsigned int totalIntersections = accumulate (
	    intersections.begin (), intersections.end (), 0);
	m_ostr << f->GetOriginalIndex () << " has " 
	       << totalIntersections << " intersections" << endl;
    }
private:
    ostream& m_ostr;
};


ostream& Data::PrintFacesWithIntersection (ostream& ostr)
{
    ostr << "Face intersections:" << endl;
    for_each(m_faces.begin (), m_faces.end (), printFaceIfIntersection (ostr));
    return ostr;
}

G3D::Vector3int16 Data::GetDomainIncrement (
    const G3D::Vector3& original, const G3D::Vector3& duplicate) const
{
    using namespace G3D;
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
	toOrthonormal.setRow (2, Vector3 (0, 0, 0));
    }
    else
    {
	Matrix3 toPeriods (
	    GetPeriod (0).x, GetPeriod (1).x, GetPeriod (2).x,
	    GetPeriod (0).y, GetPeriod (1).y, GetPeriod (2).y,
	    GetPeriod (0).z, GetPeriod (1).z, GetPeriod (2).z);
	toOrthonormal = toPeriods.inverse ();
    }
    Vector3 o = toOrthonormal * original;
    Vector3 d = toOrthonormal * duplicate;
    Vector3int16 originalDomain (floorf (o.x), floorf(o.y), floorf(o.z));
    Vector3int16 duplicateDomain (floorf (d.x), floorf(d.y), floorf(o.z));
    return duplicateDomain - originalDomain;
}
