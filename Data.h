#ifndef __DATA_H__
#define __DATA_H__

#include <vector>
#include <list>
using namespace std;

#include "Point.h"
#include "Edge.h"
#include "Face.h"
#include "Body.h"

class Data
{
public:
    ~Data();
private:
    vector<Point*> m_vertices;
    vector<Edge*> m_edges;
    vector<Face*> m_faces;
    list<Body*> m_bodies;
};

#endif
