/**
 * @file   Edge.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the Edge class
 */

#include "OOBox.h"
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
    glVertex (m);
    glVertex (e);
    glEnd ();
}


boost::shared_ptr<Edge> QuadraticEdge::createDuplicate (
    const OOBox& periods,
    const G3D::Vector3& newBegin, VertexSet* vertexSet) const
{
    G3D::Vector3int16 translation = periods.GetTranslation (
	*GetBegin (), newBegin);
    boost::shared_ptr<QuadraticEdge> duplicate = 
	boost::static_pointer_cast<QuadraticEdge> (
	    Edge::createDuplicate (periods, newBegin, vertexSet));
    boost::shared_ptr<Vertex> middleDuplicate = GetMiddle ()->GetDuplicate (
	periods, translation, vertexSet);
    duplicate->SetMiddle (middleDuplicate);
    return duplicate;
}
