#ifndef __POINT_H__
#define __POINT_H__
#include <iostream>
class Point
{
public:
    Point(float x, float y, float z) {m_x = x; m_y = y; m_z = z;}

    float GetX (void) {return m_x;}
    void SetX (float x) {m_x = x;}
    float GetY(void) {return m_y;}
    void SetY(float y) {m_y = y;}
    float GetZ(void) {return m_z;}
    void SetZ(float z) {m_z = z;}
    friend std::ostream& operator<< (std::ostream& ostr, Point& p)
    {return ostr << "Point: " << p.m_x << ", " << p.m_y << ", " 
		 << p.m_z;}

private:
    float m_x, m_y, m_z;
};

#endif

// Local Variables:
// mode: c++
// End:
