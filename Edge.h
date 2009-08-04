/**
 * @file   Edge.h
 * @author Dan R. Lipsa
 * 
 * Declaration of the Edge class
 */
#ifndef __EDGE_H__
#define __EDGE_H__

#include "Vertex.h"
using namespace std;
class AttributesInfo;

/**
 * An edge is an object that stores a begin and an end vertices (Point)
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
    {return m_begin;}
    /**
     * Sets the first vertex of the edge
     * @param begin value stored in the first vertex of the edge
     */
    void SetBegin(Vertex* begin) {m_begin = begin;}
    /**
     * @return last vertex of the edge
     */
    const Vertex* GetEnd(void) const
    {return m_end;}
    /**
     * Sets the last vertex of the edge
     * @param end value stored in the last vertex of the edge
     */
    void SetEnd(Vertex* end) {m_end = end;}
    /**
     * Prints an edge to the output stream
     * @param ostr where to write the edge
     * @param e edge to write
     */
    friend ostream& operator<< (ostream& ostr, Edge& e);
    /**
     * Prints the two vertices of an edge in reverse order (end , begin)
     * @param ostr the stream where to write the edge
     */
    void ReversePrint (ostream& ostr);
    static void SetDefaultAttributes (AttributesInfo& info);
private:
    /**
     * First vertex of the edge
     */
    Vertex* m_begin;
    /**
     * Last vertex of the edge
     */
    Vertex* m_end;
    static AttributesInfo* m_infos;
};


#endif

// Local Variables:
// mode: c++
// End:
