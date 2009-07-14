#ifndef __DATA_H__
#define __DATA_H__

#include <vector>
#include <iostream>

#include "Point.h"
#include "Edge.h"
#include "Face.h"
#include "Body.h"

class Data
{
public:
    ~Data ();
    Point* GetPoint (int i) {return m_vertices[i];}    
    void SetPoint (unsigned int i, float x, float y, float z);
    void SetEdge (unsigned int i, unsigned int begin, unsigned int end);
    void SetFace (unsigned int i, const std::vector<int>& edges);
    void SetBody (unsigned int i, const std::vector<int>& faces);

    friend std::ostream& operator<< (std::ostream& ostr, Data& d);
private:
    std::vector<Point*> m_vertices;
    std::vector<Edge*> m_edges;
    std::vector<Face*> m_faces;
    std::vector<Body*> m_bodies;
};
extern Data data;

#endif

// Local Variables:
// mode: c++
// End:
