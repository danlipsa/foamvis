/**
 * @file   Edge.h
 * @author Dan R. Lipsa
 * 
 * Declaration of the Edge class
 */
#ifndef __EDGE_H__
#define __EDGE_H__
#include "Vertex.h"

class AttributesInfo;
class Face;
/**
 * An edge is an object that stores a begin and an end vertex
 */
class Edge : public Element
{
public:
    /**
     * Creates an Edge object
     * @param begin the first point of the endge
     * @param end the last point of the edge
     */
    Edge (Vertex* begin, Vertex* end):
        m_begin (begin), m_end (end) {}
    /**
     * @return the first vertex of the edge
     */
    Vertex* GetBegin(void) const
    {
        return m_begin;
    }
    /**
     * Sets the first vertex of the edge
     * @param begin value stored in the first vertex of the edge
     */
    void SetBegin(Vertex* begin) {m_begin = begin;}
    /**
     * @return last vertex of the edge
     */
    Vertex* GetEnd(void) const
    {
        return m_end;
    }
    /**
     * Sets the last vertex of the edge
     * @param end value stored in the last vertex of the edge
     */
    void SetEnd(Vertex* end) {m_end = end;}
    /**
     * Prints the two vertices of an edge in reverse order (end , begin)
     * @param ostr the stream where to write the edge
     */
    void ReversePrint (ostream& ostr);
    /**
     * Is this a physical edge (not a tesselation edge)?
     * @return true if this is a physical edge, false otherwise
     */
    bool IsPhysical () const {return (m_faces.size () == 3);}
    /**
     * Adds a face touched by this edge
     * @param face face touched by this edge
     */
    void AddFace (const Face* face) {m_faces.push_back (face);}

    /**
     * Specifies the default attributes for an Edge object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    static void StoreDefaultAttributes (AttributesInfo& info);
    /**
     * Prints an edge to the output stream
     * @param ostr where to write the edge
     * @param e edge to write
     */
    friend ostream& operator<< (ostream& ostr, const Edge& e);
private:
    /**
     * First vertex of the edge
     */
    Vertex* m_begin;
    /**
     * Last vertex of the edge
     */
    Vertex* m_end;
    vector<const Face*> m_faces;
    /**
     * Stores information about all vertex attributes
     */
    static AttributesInfo* m_infos;
};
/**
 * Pretty print an Edge*
 * @param ostr where to print
 * @param e what to print
 * @return where to print the next data
 */
inline ostream& operator<< (ostream& ostr, Edge* e)
{
    return ostr << *e;
}

#endif //__EDGE_H__

// Local Variables:
// mode: c++
// End:
