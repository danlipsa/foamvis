/**
 * @file Data.h
 * @author Dan R. Lipsa
 *
 * Declaration of the Data class
 */
#ifndef __DATA_H__
#define __DATA_H__

#include "DefineAttributeType.h"
#include "Body.h"
#include "Edge.h"
#include "Face.h"
#include "AttributeInfo.h"

class ParsingData;
/**
 * Stores information  read from  a DMP file  produced by  the Surface
 * Evolver software.
 */
class Data
{
public:
    /**
     * Member function which returns one of the corners of the AABox for a
     * data object
     */
    typedef const G3D::Vector3& (Data::*Corner) () const;
    typedef set<Vertex*, Vertex::LessThan> VertexSet;
    typedef set<Edge*, Edge::LessThan> EdgeSet;
    typedef boost::unordered_set<Face*, Face::Hash> FaceSet;
    /**
     * Iterator over the vertices in this Data object
     */
    typedef vector<Vertex*> Vertices;
    /**
     * Functor applied to a collection of vertices
     */
    typedef Vertices::iterator (*AggregateOnVertices)(
	Vertices::iterator first, Vertices::iterator last, 
	Vertex::LessThanAlong lessThan);

    class LessThanAlong
    {
    public:
	/**
	 * Constructor
	 * @param axis along which axis to compare
	 * @param corner which corner of the AABox to compare
	 */
	LessThanAlong (G3D::Vector3::Axis axis, Corner corner) : 
	    m_axis (axis), m_corner(corner) {}
	/**
	 * Functor that compares two data objects
	 * @param first first data object
	 * @param second second data object
	 */
	bool operator() (const Data* first, const Data* second)
	{
	    return operator() (*first, *second);
	}

	bool operator() (const Data& first, const Data& second)
	{
	    return 
		(first.*m_corner) ()[m_axis] < (second.*m_corner) ()[m_axis];
	}
	
    private:
	/**
	 * Along which axis to compare
	 */
	G3D::Vector3::Axis m_axis;
	/**
	 * What corner of the AABox to compare
	 */
	Corner m_corner;
    };
public:
    /**
     * Constructs a Data object.
     */
    Data ();
    /**
     * Destroys a Data object
     */
    ~Data ();
    /**
     * Gets a Vertex from the Data object
     * @param i index where the Vertex object is stored
     * @return a pointer to the Vertex object
     */
    Vertex* GetVertex (int i) 
    {
	return m_vertices[i];
    }
    Vertex* GetVertexDuplicate (
	const Vertex& original, const G3D::Vector3int16& domainIncrement);
    /**
     * Gets the vector of vertices
     * @return the vector of vertices
     */
    vector<Vertex*>& GetVertices () 
    {
	return m_vertices;
    }
    /**
     * Stores a Vertex object a certain index in the Data object
     * @param i where to store the Vertex object
     * @param x coordinate X of the Vertex object
     * @param y coordinate Y of the Vertex object
     * @param z coordinate Z of the Vertex object
     * @param list the list of attributes for the vertex
     */
    void SetVertex (size_t i, float x, float y, float z,
		    vector<NameSemanticValue*>& list);
    /**
     * Returns all edges from this Data
     * @return a vector of Edge pointers
     */
    vector<Edge*>& GetEdges () 
    {
	return m_edges;
    }
    Edge* GetEdgeDuplicate (
	const Edge& original, const G3D::Vector3& edgeBegin);
    /**
     * Stores an Edge object in the Data object at a certain index
     * @param i index where to store the Edge object
     * @param begin index of the first Point that determines the edge
     * @param end index of the last Point that determines the edge
     * @param list the list of attributes for this edge
     */
    void SetEdge (size_t i, size_t begin, size_t end,
		  G3D::Vector3int16& domainIncrement,
                  vector<NameSemanticValue*>& list);
    /**
     * Gets all faces from this Data
     */
    vector<Face*>& GetFaces () 
    {
	return m_faces;
    }
    const vector<Face*>& GetFaces () const
    {
	return m_faces;
    }
    Face* GetFaceDuplicate (
	const Face& original, const G3D::Vector3& newBegin);

    Face* GetFace (size_t i)
    {
	return m_faces[i];
    }
    /**
     * Stores a Face object in the Data object 
     * 
     * @param i index where to store the Face object
     * @param edges  vector of  edges that form  the face. An  edge is
     *        specified using an index of the edge that should already
     *        be stored in the Data  object. If the index is negative,
     *        the edge part of the  Face is in reversed order than the
     *        Edge that is stored in the Data object.
     * @param list the list of attributes for the face
     */
    void SetFace (size_t i,  vector<int>& edges,
                  vector<NameSemanticValue*>& list);
    /**
     * Gets ith body
     * @param i index of the body to be returned
     * @return the body
     */
    Body* GetBody (size_t i);
    /**
     * Gets all bodies from the Data
     * @return a vector of Body pointers
     */
    vector<Body*>& GetBodies () 
    {
	return m_bodies;
    }
    /**
     * Stores a Body object in the Data object
     * @param i index where to store the Body object
     * @param  faces vector of  faces that  form the  body. A  face is
     *         specified  using  an  index  of the  face  that  should
     *         already be *  stored in the Data object.   If the index
     *         is negative, the face * that  is part of the Body is in
     *         reverse order  than the  Face that *  is stored  in the
     *         Data object.
     * @param list the list of attributes
     */
    void SetBody (size_t i,  vector<int>& faces,
                  vector<NameSemanticValue*>& list);
    /**
     * Stores an element of the 4x4 view matrix.
     * @param i index where to store the elment
     * @param f the value to be stored
     */
    void SetViewMatrixElement (size_t i, float f)
    {
	m_viewMatrix[i] = f;
    }
    /**
     * Gets the view matrix
     * @return the 4x4 view matrix
     */
    const boost::array<float,16>& GetViewMatrix () const 
    {
	return m_viewMatrix;
    }
    /**
     * Make the parsing data accessible
     * @return reference to the ParsingData object.
     */
    ParsingData& GetParsingData () 
    {
	return *m_parsingData;
    }
    /**
     * Deletes the parsing data
     */
    void ReleaseParsingData ();
    /**
     * The vectors of vertices, edges, faces and bodies may have holes.
     * This function compacts the elements in those vectors so that it
     * eliminates the holes.
     */
    void Compact ();
    /**
     * Stores information about an attribute.
     * @param type the type of attribute (@see DefineAttribute)
     * @param name the name of the attribute
     * @param creator knows to create the attribute (@see AttributeCreator)
     */
    void AddAttributeInfo (
        DefineAttribute::Type type, const char* name, AttributeCreator* creator)
    {
        m_attributesInfo[type].AddAttributeInfo (name, creator);
    }
    /**
     * Calculate the physical (not tesselated) edges and vertices
     */
    void CalculatePhysical ();
    /**
     * Calculate the bounding box for all vertices in this Data
     */
    void CalculateAABox ();
    /**
     * Cache edges and vertices for all bodies stored in the Data object
     */
    void CacheEdgesVerticesInBodies ();
    /**
     * Calculate centers for all bodies.
     */
    void CalculateBodiesCenters ();
    /**
     * Gets a AABox of this Data object
     * @return an AABox of this Data object
     */
    const G3D::AABox& GetAABox () const
    {
	return m_AABox;
    }
    /**
     * Gets the low point of the AABox of this Data object
     */
    const G3D::Vector3& GetAABoxLow () const
    {
	return m_AABox.low ();
    }
    /**
     * Gets the high point of the AABox of this Data object
     */
    const G3D::Vector3& GetAABoxHigh () const
    {
	return m_AABox.high ();
    }
    /**
     * Compares the low element of two data objects on the X,Y or Z axes
     * @return  true if  the  first  object is  less  than the  second
     * object, false otherwise.
     */
    void PostProcess ();
    void PrintDomains (ostream& ostr) const
    {
	Vertex::PrintDomains(ostr, m_vertices);
    }

    /**
     * Insert into the original index - body map
     * @param body to insert
     */
    void InsertOriginalIndexBodyMap (Body* body);
    /**
     * Gets the original index - body map
     */
    map<size_t, Body*>& GetOriginalIndexBodyMap ()
    {
	return m_originalIndexBodyMap;
    }

    void SetPeriod (size_t i,  G3D::Vector3 v) 
    {
	m_periods[i] = v;
    }
    const G3D::Vector3& GetPeriod (size_t i) const
    {
	return m_periods[i];
    }
    const G3D::Vector3* GetPeriods () const 
    {
	return m_periods;
    }
    bool IsTorus () const;
    bool HasEdge (Edge* edge) const 
    {
	return m_edgeSet.find (edge) != m_edgeSet.end ();
    }
    G3D::Vector3int16 GetDomainIncrement (
	const G3D::Vector3& original, const G3D::Vector3& duplicate) const;
    void SetSpaceDimension (size_t spaceDimension) 
    {
	m_spaceDimension = spaceDimension;
    }
    size_t GetSpaceDimension () const 
    {
	return m_spaceDimension;
    }
    void calculateAABoxForTorus (G3D::Vector3* low, G3D::Vector3* high);

public:
    /**
     * Pretty print the Data object
     */
    friend ostream& operator<< (ostream& ostr, Data& d);

private:
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

private:
    ostream& PrintFacesWithIntersection (ostream& ostr) const;

private:
    /**
     * A vector of points
     */
    Vertices m_vertices;
    VertexSet m_vertexSet;
    /**
     * A vector of edges
     */
    vector<Edge*> m_edges;
    EdgeSet m_edgeSet;
    /**
     * A vector of faces
     */
    vector<Face*> m_faces;
    FaceSet m_faceSet;
    /**
     * A vector of bodies.
     */
    vector<Body*> m_bodies;
    /**
     * View matrix for displaying vertices, edges, faces and bodies.
     */
    boost::array<float,16> m_viewMatrix;
    G3D::Vector3 m_periods[3];
    /**
     * Vector of maps between the name of an attribute and information about it.
     * The indexes in the vector are for vertices, edges, faces, ...
     */
    vector<AttributesInfo> m_attributesInfo;
    /**
     * Data used in parsing the DMP file.
     */
    ParsingData* m_parsingData;
    /**
     * The axially aligned bounding box for all vertices.
     */
    G3D::AABox m_AABox;
    /**
     * Map between the original index and the body pointer
     */
    map<size_t, Body*> m_originalIndexBodyMap;
    size_t m_spaceDimension;
};

/**
 * Pretty prints a Data*
 * @param ostr where to print
 * @param d what to print
 * @return where to print something else
 */
inline ostream& operator<< (ostream& ostr, Data* d)
{
    return ostr << *d;
}

#endif //__DATA_H__

// Local Variables:
// mode: c++
// End:
