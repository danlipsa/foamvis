#ifndef __POINT_H__
#define __POINT_H__

class Point
{
public:
    double GetX (void) {return m_x;}
    void SetX (double x) {m_x = x;}
    double GetY(void) {return m_y;}
    void SetY(double y) {m_y = y;}
    double GetZ(void) {return m_z;}
    void SetZ(double z) {m_z = z;}


private:
    double m_x, m_y, m_z;
};

#endif
