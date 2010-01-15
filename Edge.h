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
        m_begin(begin), m_end(end) {}
    /**
     * @return the first vertex of the edge
     */
    const Vertex* GetBegin(void) const
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
    const Vertex* GetEnd(void) const
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
    void ReversePrint (std::ostream& ostr);
    /**
     * Calculate the average point of this element
     */
    virtual void CalculateAverage ();
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
    friend std::ostream& operator<< (std::ostream& ostr, Edge& e);
private:
    /**
     * First vertex of the edge
     */
    Vertex* m_begin;
    /**
     * Last vertex of the edge
     */
    Vertex* m_end;
};
/**
 * Pretty print an Edge*
 * @param ostr where to print
 * @param e what to print
 * @return where to print the next data
 */
inline std::ostream& operator<< (std::ostream& ostr, Edge* e)
{
    return ostr << *e;
}

#endif //__EDGE_H__

// Local Variables:
// mode: c++
// End:
