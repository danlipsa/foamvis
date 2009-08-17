/**
 * @file   Point.h
 * @author Dan R. Lipsa
 * 
 * Declaration of the Vertex class
 */

#ifndef __VERTEX_H__
#define __VERTEX_H__
#include <iostream>
#include "Element.h"
using namespace std;

class AttributesInfo;
/**
 * Represents a point the the 3D Euclidian space
 */
class Vertex : public Element
{
public:
    /**
     * Creates a Vertex object
     * @param x the X coordinate
     * @param y the Y coordinate
     * @param z the Z coordinate
     */
    Vertex(float x, float y, float z):
        m_x(x), m_y(y), m_z(z) {}
    /**
     * Gets the X coordinate of the Vertex
     * @return the X coordinate
     */
    float GetX (void) const {return m_x;}
    /**
     * Sets the X coordinate of the Vertex
     * @param x the value for the X coordinate
     */
    void SetX (float x) {m_x = x;}
    /**
     * Gets the Y coordinate of the Vertex
     * @return the Y coordinate
     */
    float GetY (void) const {return m_y;}
    /**
     * Sets the Y coordinate of the Vertex
     * @param y the value for the Y coordinate
     */
    void SetY (float y) {m_y = y;}
    /**
     * Gets the Z coordinate of the Vertex
     * @return the Z coordinate of the Vertex
     */
    float GetZ (void) const {return m_z;}
    /**
     * Sets the Z coordinate of the Vertex
     * @param z the value for the Z coordinate
     */
    void SetZ (float z) {m_z = z;}
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
    friend ostream& operator<< (ostream& ostr, Vertex& v);
private:
    /**
     * the X coordinate of the Vertex
     */
    float m_x;
    /**
     * the Y coordinate of the Vertex
     */
    float m_y;
    /**
     * the Y coordinate of the Vertex
     */
    float m_z;
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
inline ostream& operator<< (ostream& ostr, Vertex* pv)
{
    return ostr << *pv;
}

#endif

// Local Variables:
// mode: c++
// End:
