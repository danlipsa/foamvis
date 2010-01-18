/**
 * @file   Vertex.h
 * @author Dan R. Lipsa
 * 
 * Vertex represented in a DMP file. Is part of edges, faces and bodies.
 */
#ifndef __VERTEX_H__
#define __VERTEX_H__

#include "Point.h"
#include "Element.h"

class AttributesInfo;
class Edge;
/**
 * Vertex represented in a DMP file. Is part of edges, faces and bodies.
 */
class Vertex : public Point, public Element
{
public:
    /**
     * Creates a Vertex object
     * @param x the X coordinate
     * @param y the Y coordinate
     * @param z the Z coordinate
     */
    Vertex(float x, float y, float z):
        Point(x, y, z) {}
    bool IsPhysical () const {return (m_edges.size () == 4);}
    void AddEdge (Edge* edge) {m_edges.push_back (edge);}
    /**
     * Specifies the default attributes for the Vertex object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    static void StoreDefaultAttributes (AttributesInfo& info);
    /**
     * Pretty print for a Vertex object
     * @param ostr output stream to print the Vertex to
     * @param p Vertex object to be printed
     * @return output stream used to print the object to
     */
    friend std::ostream& operator<< (std::ostream& ostr, const Vertex& v);
private:
    std::vector<Edge*> m_edges;
    /**
     * Stores information about all vertex attributes
     */
    static AttributesInfo* m_infos;
};
/**
 * Pretty prints a Vertex* by calling the operator<< for a Vertex.
 * @param ostr where to print
 * @param pv what to print
 * @return where to print
 */
inline std::ostream& operator<< (std::ostream& ostr, const Vertex* pv)
{
    return ostr << *pv;
}

#endif //__VERTEX_H__

// Local Variables:
// mode: c++
// End:
