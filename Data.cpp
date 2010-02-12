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
 * Functor that adds a face adjacent to an edge
 */
class updateFaces 
{
public:
    /**
     * Constructor
     * @param face this face is touched by edges
     */
    updateFaces (Face* face) : m_face (face) 
    {}
    /**
     * Functor that adds a face adjacent to an edge
     * @param orientedEdge the edge that touches the face
     */
    void operator () (OrientedEdge* orientedEdge)
    {
	orientedEdge->AddAdjacentFace (m_face);
    }
private:
    /**
     * Face adjacent to edges
     */
    Face* m_face;
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
 * For all the edges in the face, add the face as being touched by the edges
 */
void updateFaceForEdges (Face* face)
{
    const vector<OrientedEdge*>& orientedEdges = face->GetOrientedEdges ();
    for_each (orientedEdges.begin (), orientedEdges.end (), updateFaces (face));
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
    for_each (d.m_viewMatrix, 
              d.m_viewMatrix + 
              sizeof(d.m_viewMatrix)/sizeof(d.m_viewMatrix[0]), 
              printMatrixElement (ostr));
    ostr << endl;
    d.PrintDomains (ostr);
    d.PrintFacesWithIntersection (ostr);
    return ostr;
}

Data::Data () : 
    m_attributesInfo(DefineAttribute::COUNT),
    m_parsingData (new ParsingData ())
{
	Vertex::StoreDefaultAttributes (m_attributesInfo[DefineAttribute::VERTEX]);
	Edge::StoreDefaultAttributes (m_attributesInfo[DefineAttribute::EDGE]);
	Face::StoreDefaultAttributes (m_attributesInfo[DefineAttribute::FACE]);
	Body::StoreDefaultAttributes (m_attributesInfo[DefineAttribute::BODY]);
}

Data::~Data ()
{
    for_each(m_bodies.begin (), m_bodies.end (), DeleteElementPtr<Body>);
    for_each(m_faces.begin (), m_faces.end (), DeleteElementPtr<Face>);
    for_each(m_edges.begin (), m_edges.end (), DeleteElementPtr<Edge>);
    for_each(m_vertices.begin (), m_vertices.end (), DeleteElementPtr<Vertex>);
    delete m_parsingData;
}

void Data::SetVertex (unsigned int i, float x, float y, float z,
                     vector<NameSemanticValue*>& list) 
{
    if (i >= m_vertices.size ())
        m_vertices.resize (i + 1);
    Vertex* vertex = new Vertex (i, x, y ,z);
    if (&list != 0)
        vertex->StoreAttributes (
            list, m_attributesInfo[DefineAttribute::VERTEX]);
    m_vertices[i] = vertex;
}

void Data::SetEdge (unsigned int i, unsigned int begin, unsigned int end,
		    G3D::Vector3int16& domainIncrement,
                    vector<NameSemanticValue*>& list) 
{
    if (i >= m_edges.size ())
        m_edges.resize (i + 1); 
    Edge* edge = new Edge (
	i, GetVertex(begin), GetVertex(end), domainIncrement);
    if (&list != 0)
        edge->StoreAttributes (list, m_attributesInfo[DefineAttribute::EDGE]);
    m_edges[i] = edge;
}

void Data::SetFace (unsigned int i, const vector<int>& edges,
                    vector<NameSemanticValue*>& list)
{
    if (i >= m_faces.size ())
        m_faces.resize (i + 1);
    Face* face = new Face (i, edges, m_edges);
    if (&list != 0)
        face->StoreAttributes (list, m_attributesInfo[DefineAttribute::FACE]);
    m_faces[i] = face;
}

void Data::SetBody (unsigned int i, const vector<int>& faces,
                    vector<NameSemanticValue*>& list)
{
    if (i >= m_bodies.size ())
        m_bodies.resize (i + 1);
    Body* body = new Body (i, faces, m_faces);
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

const Body* Data::GetBody (unsigned int i)
{
    map<unsigned int, const Body*>::iterator it = 
	m_originalIndexBodyMap.find (i);
    if (it == m_originalIndexBodyMap.end ())
	return 0;
    else
	return it->second;
}

void Data::InsertOriginalIndexBodyMap (const Body* body)
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
    for_each (m_faces.begin (), m_faces.end (), updateFaceForEdges);
    for_each (m_edges.begin (), m_edges.end (), updateEdgeForVertices);
}


void Data::Calculate (AggregateOnVertices aggregate, G3D::Vector3& v)
{
    using namespace G3D;
    IteratorVertices it;
    it = aggregate (m_vertices.begin (), m_vertices.end (), 
	    Vertex::LessThan(Vector3::X_AXIS));;
    v.x = (*it)->x;
    it = aggregate (m_vertices.begin (), m_vertices.end (), 
	    Vertex::LessThan(Vector3::Y_AXIS));
    v.y = (*it)->y;
    it = aggregate (m_vertices.begin (), m_vertices.end (), 
	    Vertex::LessThan(Vector3::Z_AXIS));
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
    CalculateBodiesCenters ();
    CalculateVertexDomains ();
}

void Data::CalculateVertexDomains ()
{
    //Vertex* v = m_edges[0]->GetBegin ();
    Vertex* v = m_vertices[0];
    v->SetDomain (Vector3int16 (0,0,0));
    Vertex::CalculateDomains (v);
}

struct lessThanVector3int16
{
bool operator () (const Vector3int16& first, const Vector3int16& second)
{
    return 
	first.x < second.x ||
	(first.x == second.x && first.y < second.y) ||
	(first.x == second.x && first.y == second.y && first.z < second.z);
}
};


class storeByDomain
{
public:
    storeByDomain (map< G3D::Vector3int16, list<const Vertex*>,
		   lessThanVector3int16 >& 
		   domainVerticesMap) : m_domainVerticesMap (domainVerticesMap)
    {}
    void operator() (Vertex* v)
    {
	m_domainVerticesMap[v->GetDomain ()].push_back (v);
    }
private:
    map< G3D::Vector3int16, list<const Vertex*>, 
	 lessThanVector3int16 >& m_domainVerticesMap;
};

class printVertexIndex
{
public:
    printVertexIndex (ostream& ostr) : m_ostr(ostr) {}
    void operator() (const Vertex* v)
    {
	m_ostr << (v->GetOriginalIndex () + 1) << " ";
    }
private:
    ostream& m_ostr;
};

class printDomainVertices
{
public:
    printDomainVertices (ostream& ostr) : m_ostr(ostr) {}

    void operator() (pair<const G3D::Vector3int16, list<const Vertex*> >& pair)
    {
	m_ostr << "Domain: " << pair.first
	     << " Vertices: ";
	for_each (pair.second.begin (), pair.second.end (), 
		  printVertexIndex (m_ostr));
	m_ostr << endl;
    }
private:
    ostream& m_ostr;
};

ostream& Data::PrintDomains (ostream& ostr)
{
    map < G3D::Vector3int16, list<const Vertex*>,lessThanVector3int16 > 
	domainVerticesMap;
    for_each (m_vertices.begin (), m_vertices.end (),
	      storeByDomain (domainVerticesMap));
    for_each (domainVerticesMap.begin (), domainVerticesMap.end (),
	      printDomainVertices (ostr));
    return ostr;
}

unsigned int countIntersections (OrientedEdge* e)
{
    const Vector3int16& domainIncrement = e->GetEdge ()->GetDomainIncrement ();
    return ((domainIncrement.x != 0) + 
	    (domainIncrement.y != 0) + (domainIncrement.z != 0));
}

class printFaceIfIntersection
{
public:
    printFaceIfIntersection (ostream& ostr) : m_ostr(ostr) {}
    void operator () (Face* f)
    {
	vector<const OrientedEdge*>::iterator it;
	const vector<OrientedEdge*>& v = f->GetOrientedEdges ();
	vector<unsigned int> intersections(v.size ());
	transform (
	    v.begin (), v.end (), intersections.begin (), countIntersections);
	unsigned int totalIntersections = accumulate (
	    intersections.begin (), intersections.end (), 0);
	m_ostr << (f->GetOriginalIndex () + 1) << " has " 
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
