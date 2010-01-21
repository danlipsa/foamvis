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
 * Functor that adds a face as being touched by an edge
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
     * Functor that adds specifies that a face is touched by an edge
     * @param orientedEdge the edge that touches the face
     */
    void operator () (OrientedEdge* orientedEdge)
    {
	orientedEdge->AddFace (m_face);
    }
private:
    /**
     * Face that is touched by edges
     */
    Face* m_face;
};

/**
 * Move elements in a vector toward the begining of the vector so that you 
 * eliminate holes.
 * @param v vector of elements
 */
template <class E>
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
 * For both vertices of this edge, add the edge as being touched by the vertices
 */
void updateEdgeForVertices (Edge* edge)
{
    if (edge->IsPhysical ())
    {
	edge->GetBegin ()->AddEdge (edge);
	edge->GetEnd ()->AddEdge (edge);
    }
}


ostream& operator<< (ostream& ostr, Data& d)
{
    ostr << "Data:" << endl;
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
    Vertex* vertex = new Vertex (x, y ,z);
    if (&list != 0)
        vertex->StoreAttributes (
            list, m_attributesInfo[DefineAttribute::VERTEX]);
    m_vertices[i] = vertex;
}

void Data::SetEdge (unsigned int i, unsigned int begin, unsigned int end,
                    vector<NameSemanticValue*>& list) 
{
    if (i >= m_edges.size ())
        m_edges.resize (i + 1); 
    Edge* edge = new Edge (GetVertex(begin), GetVertex(end));
    if (&list != 0)
        edge->StoreAttributes (list, m_attributesInfo[DefineAttribute::EDGE]);
    m_edges[i] = edge;
}

void Data::SetFace (unsigned int i, const vector<int>& edges,
                    vector<NameSemanticValue*>& list)
{
    if (i >= m_faces.size ())
        m_faces.resize (i + 1);
    Face* face = new Face (edges, m_edges);
    if (&list != 0)
        face->StoreAttributes (list, m_attributesInfo[DefineAttribute::FACE]);
    m_faces[i] = face;
}

void Data::SetBody (unsigned int i, const vector<int>& faces,
                    vector<NameSemanticValue*>& list)
{
    if (i >= m_bodies.size ())
        m_bodies.resize (i + 1);
    Body* body = new Body (faces, m_faces);
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
}

void Data::ReleaseParsingData ()
{
    delete m_parsingData;
}

void Data::CalculatePhysical ()
{
    for_each (m_faces.begin (), m_faces.end (), updateFaceForEdges);
    for_each (m_edges.begin (), m_edges.end (), updateEdgeForVertices);
}
