#ifndef __ORIENTED_EDGE_H__
#define __ORIENTED_EDGE_H__

#include "Edge.h"

class OrientedEdge
{
public:
    OrientedEdge(Edge* edge, bool reversed)
    {m_edge = edge; m_reversed = reversed;}
private:
    Edge* m_edge;
    bool m_reversed;
};


#endif

// Local Variables:
// mode: c++
// End:
