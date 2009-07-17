/**
 * @file   OrientedEdge.h
 * @author Dan R. Lipsa
 *
 * Declaration of the OrientedEdge object
 */
#ifndef __ORIENTED_EDGE_H__
#define __ORIENTED_EDGE_H__

#include <iostream>
#include "Edge.h"
using namespace std;

/**
 * An oriented  edge is  an edge  that can have  its vertices  read in
 * direct and reverse order
 */
class OrientedEdge
{
public:
    /**
     * Constructs an oriented edge
     * @param edge information about the two vertices in the oriented edge
     * 
     * @param  reversed specifies  if  the two  vertices  are read  in
     *        reverse order or not.
     */
    OrientedEdge (Edge* edge, bool reversed): 
	m_edge (edge), m_reversed (reversed) {}
    /**
     * Pretty prints an Edge
     * @param ostr output stream where to print the edge
     * @param oe the edge to by printed.
     */
    friend ostream& operator<< (ostream& ostr, OrientedEdge& oe);

private:
    /**
     * Stores information about the two vertices in this oriented edge
     */
    Edge* m_edge;
    /**
     * Specifies if the  two vertices should be read  in reverse order
     * or in direct order.
     */
    bool m_reversed;
};


#endif

// Local Variables:
// mode: c++
// End:
