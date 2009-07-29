/**
 * @file   Edge.h
 * @author Dan R. Lipsa
 * 
 * Declaration of the Edge class
 */
#ifndef __EDGE_H__
#define __EDGE_H__

#include "Point.h"
using namespace std;

/**
 * An edge is an object that stores a begin and an end vertices (Point)
 */
class Edge
{
public:
    /**
     * Creates an Edge object
     * @param begin the first point of the endge
     * @param end the last point of the edge
     */
    Edge (Point* begin, Point* end):
	m_begin(begin), m_end(end) {}
    /**
     * @return the first vertex of the edge
     */
    const Point* GetBegin(void) const
    {return m_begin;}
    /**
     * Sets the first vertex of the edge
     * @param begin value stored in the first vertex of the edge
     */
    void SetBegin(Point* begin) {m_begin = begin;}
    /**
     * @return last vertex of the edge
     */
    const Point* GetEnd(void) const
    {return m_end;}
    /**
     * Sets the last vertex of the edge
     * @param end value stored in the last vertex of the edge
     */
    void SetEnd(Point* end) {m_end = end;}
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
private:
    /**
     * First vertex of the edge
     */
    Point* m_begin;
    /**
     * Last vertex of the edge
     */
    Point* m_end;
};


#endif

// Local Variables:
// mode: c++
// End:
