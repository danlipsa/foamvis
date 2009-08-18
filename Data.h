/**
 * @file Data.h
 * @author Dan R. Lipsa
 *
 * Declaration of the Data class
 */
#ifndef __DATA_H__
#define __DATA_H__

#include <vector>
#include <map>
#include <iostream>
#include "Vertex.h"
#include "Edge.h"
#include "Face.h"
#include "Body.h"
#include "AttributeCreator.h"
#include "AttributeInfo.h"
#include "NameSemanticValue.h"
using namespace std;

class ParsingData;
/**
 * Stores information  read from  a DMP file  produced by  the Surface
 * Evolver software.
 */
class Data
{
public:
    Data ();
    /**
     * Destroys a Data object
     */
    ~Data ();
    /**
     * Gets a Point object from the Data object
     * @param i index where the Point object is stored
     * @return a pointer to the Point object
     */
    Vertex* GetPoint (int i) {return m_vertices[i];}
    const vector<Vertex*>& GetPoints () {return m_vertices;}
    /**
     * Stores a Point object a certain index in the Data object
     * @param i where to store the Point object
     * @param x coordinate X of the Point object
     * @param y coordinate Y of the Point object
     * @param z coordinate Z of the Point object
     */
    void SetPoint (unsigned int i, float x, float y, float z,
                   vector<NameSemanticValue*>& list);
    /**
     * Stores an Edge object in the Data object at a certain index
     * @param i index where to store the Edge object
     * @param begin index of the first Point that determines the edge
     * @param end index of the last Point that determines the edge
     */
    void SetEdge (unsigned int i, unsigned int begin, unsigned int end,
                  vector<NameSemanticValue*>& list);
    const vector<Edge*>& GetEdges () {return m_edges;}
    /**
     * Stores a Face object in the Data object 
     * 
     * @param i index where to store the Face object
     * @param edges  vector of  edges that form  the face. An  edge is
     *        specified using an index of the edge that should already
     *        be stored in the Data  object. If the index is negative,
     *        the edge part of the  Face is in reversed order than the
     *        Edge that is stored in the Data object.
     */
    void SetFace (unsigned int i, const vector<int>& edges,
                  vector<NameSemanticValue*>& list);
    const vector<Face*>& GetFaces () {return m_faces;}

    /**
     * Stores a Body object in the Data object
     * @param i index where to store the Body object

     * @param  faces vector of  faces that  form the  body. A  face is
     *         specified  using  an  index  of the  face  that  should
     *         already be *  stored in the Data object.   If the index
     *         is negative, the face * that  is part of the Body is in
     *         reverse order  than the  Face that *  is stored  in the
     *         Data object.
     */
    void SetBody (unsigned int i, const vector<int>& faces,
                  vector<NameSemanticValue*>& list);
    const vector<Body*>& GetBodies () {return m_bodies;}
    /**
     * Stores an element of the 4x4 view matrix.
     * @param i index where to store the elment
     * @param f the value to be stored
     */
    void SetViewMatrixElement (unsigned int i, float f)
    {m_viewMatrix[i] = f;}
    const float* GetViewMatrix () {return m_viewMatrix;}
    /**
     * Make the parsing data accessible
     * @return reference to the ParsingData object.
     */
    ParsingData& GetParsingData () {return *m_parsingData;}
    void Compact ();
    void AddAttributeInfo (
        AttributeType type, const char* name, AttributeCreator* creator)
    {
        m_attributesInfo[type].AddAttributeInfo (name, creator);
    }

    /**
     * Pretty print for the Data object
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
    vector<AttributesInfo> m_attributesInfo;
    ParsingData* m_parsingData;
};

/**
 * Declaration of the global Data object
 */
extern Data data;

#endif

// Local Variables:
// mode: c++
// End:
