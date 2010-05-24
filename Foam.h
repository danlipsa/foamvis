/**
 * @file Foam.h
 * @author Dan R. Lipsa
 *
 * Declaration of the Foam class
 */
#ifndef __FOAM_H__
#define __FOAM_H__

#include "AttributeInfo.h"
#include "Comparisons.h"
#include "Enums.h"
#include "Hashes.h"
#include "OOBox.h"

class AttributeCreator;
class Body;
class Edge;
class Face;
class NameSemanticValue;
class ParsingData;
/**
 * Stores information  read from  a DMP file  produced by  the Surface
 * Evolver software.
 */
class Foam
{
public:
    /**
     * Member function which returns one of the corners of the AABox for a
     * data object
     */
    typedef set<Vertex*, VertexLessThan> VertexSet;
    typedef set<Edge*, EdgeLessThan> EdgeSet;
    typedef boost::unordered_set<Face*, FaceHash> FaceSet;
    /**
     * Iterator over the vertices in this Foam object
     */
    typedef vector<Vertex*> Vertices;
    typedef vector<Edge*> Edges;
    typedef vector<Face*> Faces;
    typedef vector<Body*> Bodies;
    /**
     * Functor applied to a collection of vertices
     */
    typedef Vertices::iterator (*AggregateOnVertices)(
	Vertices::iterator first, Vertices::iterator last, 
	VertexLessThanAlong lessThan);

public:
    /**
     * Constructs a Foam object.
     */
    Foam (size_t timeStep);
    /**
     * Destroys a Foam object
     */
    ~Foam ();
    /**
     * Gets a Vertex from the Foam object
     * @param i index where the Vertex object is stored
     * @return a pointer to the Vertex object
     */
    Vertex* GetVertex (int i) 
    {
	return m_vertices[i];
    }
    Vertex* GetVertexDuplicate (
	Vertex* original, const G3D::Vector3int16& translation);
    Edge* GetEdgeDuplicate (
	Edge* original, const G3D::Vector3& edgeBegin);
    Face* GetFaceDuplicate (
	const Face& original, const G3D::Vector3& translation);
    Vertex* CreateVertexDuplicate (
	Vertex* original, const G3D::Vector3int16& translation);
    Edge* CreateEdgeDuplicate (Edge* original, const G3D::Vector3& newBegin);
    Face* CreateFaceDuplicate (
	const Face& original, const G3D::Vector3& newBegin);
    /**
     * Gets the vector of vertices
     * @return the vector of vertices
     */
    Vertices& GetVertices () 
    {
	return m_vertices;
    }
    /**
     * Stores a Vertex object a certain index in the Foam object
     * @param i where to store the Vertex object
     * @param x coordinate X of the Vertex object
     * @param y coordinate Y of the Vertex object
     * @param z coordinate Z of the Vertex object
     * @param list the list of attributes for the vertex
     */
    void SetVertex (size_t i, float x, float y, float z,
		    vector<NameSemanticValue*>& list);
    /**
     * Returns all edges from this Foam
     * @return a vector of Edge pointers
     */
    Edges& GetEdges () 
    {
	return m_edges;
    }
    /**
     * Stores an Edge object in the Foam object at a certain index
     * @param i index where to store the Edge object
     * @param begin index of the first Point that determines the edge
     * @param end index of the last Point that determines the edge
     * @param list the list of attributes for this edge
     */
    void SetEdge (size_t i, size_t begin, size_t end,
		  G3D::Vector3int16& endTranslation,
                  vector<NameSemanticValue*>& list);
    /**
     * Gets all faces from this Foam
     */
    Faces& GetFaces () 
    {
	return m_faces;
    }
    const Faces& GetFaces () const
    {
	return m_faces;
    }

    Face* GetFace (size_t i)
    {
	return m_faces[i];
    }
    /**
     * Stores a Face object in the Foam object 
     * 
     * @param i index where to store the Face object
     * @param edges  vector of  edges that form  the face. An  edge is
     *        specified using an index of the edge that should already
     *        be stored in the Foam  object. If the index is negative,
     *        the edge part of the  Face is in reversed order than the
     *        Edge that is stored in the Foam object.
     * @param list the list of attributes for the face
     */
    void SetFace (size_t i,  vector<int>& edges,
                  vector<NameSemanticValue*>& list);
    /**
     * Gets ith body
     * @param i index of the body to be returned
     * @return the body
     */
    Body* GetBody (size_t i) const
    {
	return m_bodies[i];
    }
    /**
     * Gets all bodies from the Foam
     * @return a vector of Body pointers
     */
    Bodies& GetBodies () 
    {
	return m_bodies;
    }
    /**
     * Stores a Body object in the Foam object
     * @param i index where to store the Body object
     * @param  faces vector of  faces that  form the  body. A  face is
     *         specified  using  an  index  of the  face  that  should
     *         already be *  stored in the Foam object.   If the index
     *         is negative, the face * that  is part of the Body is in
     *         reverse order  than the  Face that *  is stored  in the
     *         Foam object.
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
        DefineAttribute::Type type, const char* name,
	AttributeCreator* creator);
    /**
     * Calculate faces part of a body, edges part of a face, ...
     */
    void UpdatePartOf ();
    /**
     * Calculate the bounding box for all vertices in this Foam
     */
    void CalculateAABox ();
    /**
     * Cache edges and vertices for all bodies stored in the Foam object
     */
    void CacheEdgesVerticesInBodies ();
    /**
     * Calculate centers for all bodies.
     */
    void CalculateBodiesCenters ();
    void CalculateTorusClipped ();
    /**
     * Gets a AABox of this Foam object
     * @return an AABox of this Foam object
     */
    const G3D::AABox& GetAABox () const
    {
	return m_AABox;
    }
    /**
     * Gets the low point of the AABox of this Foam object
     */
    const G3D::Vector3& GetAABoxLow () const
    {
	return m_AABox.low ();
    }
    /**
     * Gets the high point of the AABox of this Foam object
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
    void PrintDomains (ostream& ostr) const;

    const G3D::Vector3& GetPeriod (size_t i) const
    {
	return m_periods[i];
    }
    const OOBox& GetPeriods () const 
    {
	return m_periods;
    }
    void SetPeriods (const G3D::Vector3& x, const G3D::Vector3& y,
		     const G3D::Vector3& z)
    {
	m_periods.Set (x, y, z);
    }

    bool IsTorus () const;
    bool HasEdge (Edge* edge) const 
    {
	return m_edgeSet.find (edge) != m_edgeSet.end ();
    }
    
    void SetSpaceDimension (size_t spaceDimension) 
    {
	m_spaceDimension = spaceDimension;
    }
    size_t GetSpaceDimension () const 
    {
	return m_spaceDimension;
    }
    void calculateAABoxForTorus (G3D::Vector3* low, G3D::Vector3* high);
    size_t GetTimeStep () const
    {
	return m_timeStep;
    }
    void Unwrap ();
    void TorusTranslate (
	Vertex* vertex, const G3D::Vector3int16& domainIncrement) const;

public:
    /**
     * Pretty print the Foam object
     */
    friend ostream& operator<< (ostream& ostr, Foam& d);

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
    Edges m_edges;
    EdgeSet m_edgeSet;
    /**
     * A vector of faces
     */
    Faces m_faces;
    FaceSet m_faceSet;
    /**
     * A vector of bodies.
     */
    Bodies m_bodies;
    /**
     * View matrix for displaying vertices, edges, faces and bodies.
     */
    boost::array<float, 16> m_viewMatrix;
    OOBox m_periods;
    /**
     * Vector of maps between the name of an attribute and information about it.
     * The indexes in the vector are for vertices, edges, faces, ...
     */
    boost::array<AttributesInfo, DefineAttribute::COUNT> m_attributesInfo;
    /**
     * Foam used in parsing the DMP file.
     */
    ParsingData* m_parsingData;
    /**
     * The axially aligned bounding box for all vertices.
     */
    G3D::AABox m_AABox;
    size_t m_spaceDimension;
    const size_t m_timeStep;
};

/**
 * Pretty prints a Foam*
 * @param ostr where to print
 * @param d what to print
 * @return where to print something else
 */
inline ostream& operator<< (ostream& ostr, Foam* d)
{
    return ostr << *d;
}

#endif //__FOAM_H__

// Local Variables:
// mode: c++
// End:
