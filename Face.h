#ifndef __FACE_H__
#define __FACE_H__

#include <vector>
#include "OrientedEdge.h"

class Face
{
public:
    Face(std::vector<int>& edge_indexes, std::vector<Edge*>& edges);
    ~Face();

private:
    /**
     * Edges that are part of this face
     */
    std::vector<OrientedEdge*> m_edges;
};


#endif

// Local Variables:
// mode: c++
// End:
