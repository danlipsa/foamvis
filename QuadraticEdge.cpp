/**
 * @file   Edge.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the Edge class
 */

#include "QuadraticEdge.h"
#include "Vertex.h"

QuadraticEdge::QuadraticEdge (
    const boost::shared_ptr<Vertex>& begin,
    const boost::shared_ptr<Vertex>& end,
    const boost::shared_ptr<Vertex>& middle,
    const G3D::Vector3int16& endLocation, 
    size_t id,
    ElementStatus::Enum duplicateStatus) :

    Edge (begin, end, endLocation, id, duplicateStatus),
    m_middle (middle)
{
}

QuadraticEdge::QuadraticEdge (const QuadraticEdge& quadraticEdge) :
    Edge (quadraticEdge),
    m_middle (quadraticEdge.m_middle)
{
    
}


boost::shared_ptr<Edge> QuadraticEdge::Clone () const
{
    return boost::shared_ptr<Edge> (new QuadraticEdge(*this));
}

void QuadraticEdge::Display (Color::Enum defaultColor, float alpha) const
{
    Color::Enum color = GetColor (defaultColor);
    glColor (G3D::Color4 (Color::GetValue(color), alpha));
    const Vertex& b = *GetBegin ();
    const Vertex& e = *GetEnd ();
    const Vertex& m = *GetMiddle ();
    glBegin(GL_LINE_STRIP);
    glVertex (b);
    //glVertex (m);
    glVertex (e);
    glEnd ();
}

