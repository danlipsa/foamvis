/**
 * @file   DisplayEdgeFunctors.h
 * @author Dan R. Lipsa
 * @date  4 Nov. 2010
 *
 * Implementation for functors that display a face
 */


#include "DisplayFaceFunctors.h"

void DisplayFaceWithNormal::display (
    const boost::shared_ptr<OrientedFace>& of)
{
    using G3D::Vector3;
    Vector3 normal = of->GetNormal ();
/*
    glColor (Qt::black);
    G3D::Vector3 first = *of->GetOrientedEdge (0).GetBegin ();
    DisplayOrientedEdge displayOrientedEdge;
    displayOrientedEdge(first, first + normal);
*/


    glNormal (normal);
    glColor (Color::GetValue(of->GetColor ()));

    // specify the vertices
    (DisplaySameEdges (m_glWidget)) (of);
}
