/**
 * @file   Point.h
 * @author Dan R. Lipsa
 * 
 * Declaration of the Point class
 */

#ifndef __POINT_H__
#define __POINT_H__
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
     * Creates a Point object
     * @param x the X coordinate
     * @param y the Y coordinate
     * @param z the Z coordinate
     */
    Vertex(float x, float y, float z):
	m_x(x), m_y(y), m_z(z) {}
    /**
     * Gets the X coordinate of the Point
     * @return the X coordinate
     */
    float GetX (void) const {return m_x;}
    /**
     * Sets the X coordinate of the Point
     * @param x the value for the X coordinate
     */
    void SetX (float x) {m_x = x;}
    /**
     * Gets the Y coordinate of the Point
     * @return the Y coordinate
     */
    float GetY (void) const {return m_y;}
    /**
     * Sets the Y coordinate of the Point
     * @param y the value for the Y coordinate
     */
    void SetY (float y) {m_y = y;}
    /**
     * Gets the Z coordinate of the Point
     * @return the Z coordinate of the Point
     */
    float GetZ (void) const {return m_z;}
    /**
     * Sets the Z coordinate of the Point
     * @param z the value for the Z coordinate
     */
    void SetZ (float z) {m_z = z;}
    /**
     * Pretty print for a Point object
     * @param ostr output stream to print the Point to
     * @param p Point object to be printed
     * @return output stream used to print the object to
     */
    static void SetDefaultAttributes (AttributesInfo& info);

    friend ostream& operator<< (ostream& ostr, Vertex& p);

private:
    /**
     * the X coordinate of the Point
     */
    float m_x;
    /**
     * the Y coordinate of the Point
     */
    float m_y;
    /**
     * the Y coordinate of the Point
     */
    float m_z;

    static AttributesInfo* m_infos;
};

#endif

// Local Variables:
// mode: c++
// End:
