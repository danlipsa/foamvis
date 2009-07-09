#ifndef __EDGE_H__
#define __EDGE_H__

#include "Point.h"

class Edge
{
public:
    const Point* GetBegin(void) {return m_begin;}
    void SetBegin(Point* begin) {m_begin = begin;}
    const Point* GetEnd(void) {return m_end;}
    void SetEnd(Point* end) {m_end = end;}

private:
    Point* m_begin;
    Point* m_end;
};


#endif
