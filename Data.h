/**
 * @file Data.h
 * @author Dan R. Lipsa
 *
 * Declaration of the Data class
 */
#ifndef __DATA_H__
#define __DATA_H__

#include "Body.h"
#include "AttributeInfo.h"

class ParsingData;
/**
 * Stores information  read from  a DMP file  produced by  the Surface
 * Evolver software.
 */
class Data
{
public:
    typedef vector<Vertex*>::iterator IteratorVertices;
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
    Vertex* GetVertex (int i) {return m_vertices[i];}
    /**
     * Stores a Vertex object a certain index in the Data object
     * @param i where to store the Vertex object
     * @param x coordinate X of the Vertex object
     * @param y coordinate Y of the Vertex object
     * @param z coordinate Z of the Vertex object
     * @param list the list of attributes for the vertex
     */
    void SetVertex (unsigned int i, float x, float y, float z,
                   vector<NameSemanticValue*>& list);
    /**
     * Returns all edges from this Data
     * @return a vector of Edge pointers
     */
    const vector<Edge*>& GetEdges () {return m_edges;}
    /**
     * Stores an Edge object in the Data object at a certain index
     * @param i index where to store the Edge object
     * @param begin index of the first Point that determines the edge
     * @param end index of the last Point that determines the edge
     * @param list the list of attributes for this edge
     */
    void SetEdge (unsigned int i, unsigned int begin, unsigned int end,
                  vector<NameSemanticValue*>& list);
    /**
     * Gets all faces from this Data
     * @return a vector of Face pointers
     */
    const vector<Face*>& GetFaces () {return m_faces;}
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
    void SetFace (unsigned int i, const vector<int>& edges,
                  vector<NameSemanticValue*>& list);
    /**
     * Gets all bodies from the Data
     * @return a vector of Body pointers
     */
    const vector<Body*>& GetBodies () {return m_bodies;}
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
    void SetBody (unsigned int i, const vector<int>& faces,
                  vector<NameSemanticValue*>& list);
    /**
     * Stores an element of the 4x4 view matrix.
     * @param i index where to store the elment
     * @param f the value to be stored
     */
    void SetViewMatrixElement (unsigned int i, float f)
    {m_viewMatrix[i] = f;}
    /**
     * Gets the view matrix
     * @return the 4x4 view matrix
     */
    const float* GetViewMatrix () {return m_viewMatrix;}
    /**
     * Make the parsing data accessible
     * @return reference to the ParsingData object.
     */
    ParsingData& GetParsingData () {return *m_parsingData;}
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
    void Calculate (
	IteratorVertices (*f)(
	    IteratorVertices first,
	    IteratorVertices last,
	    bool (*LessThan)(Point* p1, Point* p2)),
	Point& p);
    void CalculateMinMax ()
    {
	Calculate (min_element, m_min);
	Calculate (max_element, m_max);
    }

    /**
     * Pretty print the Data object
     */
    friend ostream& operator<< (ostream& ostr, Data& d);
private:

    /**
     * A vector of points
     */
    vector<Vertex*> m_vertices;
    /**
     * A vector of edges
     */
    vector<Edge*> m_edges;
    /**
     * A vector of faces
     */
    vector<Face*> m_faces;
    /**
     * A vector of bodies.
     */
    vector<Body*> m_bodies;
    /**
     * View matrix for displaying vertices, edges, faces and bodies.
     */
    float m_viewMatrix[16];
    /**
     * Vector of maps between the name of an attribute and information about it.
     * The indexes in the vector are for vertices, edges, faces, ...
     */
    vector<AttributesInfo> m_attributesInfo;
    /**
     * Data used in parsing the DMP file.
     */
    ParsingData* m_parsingData;
    Point m_min;
    Point m_max;
};

#endif //__DATA_H__

// Local Variables:
// mode: c++
// End:
