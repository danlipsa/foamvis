#ifndef __FACE_H__
#define __FACE_H__

#include <list>
using namespace std;

#include "OrientedEdge.h"

class Face
{
public:

private:
    /**
     * Edges that are part of this face
     */
    list<OrientedEdge*> m_edges;
};


#endif
