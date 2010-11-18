/**
 * @file   DisplayEdgeFunctors.h
 * @author Dan R. Lipsa
 * @date  25 Oct. 2010
 *
 * Implementation for functors that display an edge
 */

#include "DisplayEdgeFunctors.h"

G3D::Matrix3 edgeRotation (const G3D::Vector3& begin, const G3D::Vector3& end)
{
    using G3D::Vector3;
    Vector3 newZ = end - begin;
    if (newZ.isZero ())
	return G3D::Matrix3::identity ();
    newZ = newZ.unit ();
    Vector3 newX, newY;
    newZ.getTangents (newX, newY);
    G3D::Matrix3 rotation;
    rotation.setColumn (0, newX);
    rotation.setColumn (1, newY);
    rotation.setColumn (2, newZ);
    return rotation;
}

void DisplayEdge::operator() (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    glBegin(GL_LINES);
    glVertex(begin);
    glVertex(end);
    glEnd();
}

void DisplayEdgeCylinder::operator() (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    G3D::Matrix3 rotation = edgeRotation (begin, end);
    G3D::CoordinateFrame frame (rotation, begin);
    gluQuadricOrientation (m_quadric, GLU_OUTSIDE);
    glPushMatrix ();
    {
	glMultMatrix (frame);
	gluCylinder (
	    m_quadric, m_edgeRadius, m_edgeRadius, (end - begin).length (),
	    GLWidget::QUADRIC_SLICES, GLWidget::QUADRIC_STACKS);
	/*
	gluQuadricOrientation (m_quadric, GLU_INSIDE);	
	gluDisk (m_quadric, 0, m_edgeRadius, GLWidget::QUADRIC_SLICES,
		 GLWidget::QUADRIC_STACKS);
	glTranslatef (end.x, end.y, end.z);
	*/
    }
    glPopMatrix ();
}

void DisplayArrow::operator () (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    glPushAttrib (GL_LINE_BIT);
    glLineWidth (2.0);
    glBegin(GL_LINES);
    glVertex(begin);
    glVertex((begin + end) / 2);
    glEnd();
    glPopAttrib ();
}

void DisplayArrowTube::operator () (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    G3D::Vector3 translation;
    translation = ((m_position == BASE_MIDDLE) ?
		   (begin + end) / 2 :
		   (end - (end - begin).direction () * m_height));
    G3D::Matrix3 rotation = edgeRotation (begin, end);
    G3D::CoordinateFrame objectToWorld (rotation, translation);
    glPushMatrix ();
    {
	glMultMatrix (objectToWorld);
	gluCylinder (m_quadric, m_baseRadius, m_topRadius, m_height,
		     GLWidget::QUADRIC_SLICES, GLWidget::QUADRIC_STACKS);
	gluQuadricOrientation (m_quadric, GLU_INSIDE);
	gluDisk (m_quadric, 0, m_baseRadius, GLWidget::QUADRIC_SLICES,
		 GLWidget::QUADRIC_STACKS);
    }
    gluQuadricOrientation (m_quadric, GLU_OUTSIDE);
    glPopMatrix ();
}

void DisplayEdgeTorusClipped::operator () (
    const boost::shared_ptr<Edge>& edge) const
{
    const OOBox& periods = m_glWidget.GetCurrentFoam ().GetOriginalDomain ();
    if (edge->IsClipped ())
    {
	Color::Enum color = edge->GetColor (Color::BLACK);
	glColor (Color::GetValue(color));
	glBegin(GL_LINES);
	for (size_t i = 0; i < edge->GetTorusClippedSize (periods); i++)
	{
	    glVertex(edge->GetTorusClippedBegin (i));
	    glVertex (edge->GetTorusClippedEnd (i));
	}
	glEnd ();
    }
}

void DisplayOrientedEdge::operator () (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    DisplayEdge displayEdge (m_quadric, m_topRadius);
    DisplayArrow displayArrow (
	m_quadric, m_baseRadius, m_topRadius, m_height, m_position);
    displayEdge (begin, end);
    displayArrow (begin, end);
}


void DisplayOrientedEdgeTube::operator () (
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    DisplayEdgeCylinder displayEdge (m_quadric, m_topRadius);
    DisplayArrowTube displayArrow (
	m_quadric, m_baseRadius, m_topRadius, m_height, m_position);
    displayEdge (begin, end);
    displayArrow (begin, end);
}




void DisplaySameEdges::operator() (const boost::shared_ptr<Face>& f)
{
    glBegin (GL_POLYGON);
    const vector<boost::shared_ptr<OrientedEdge> >& v =
	f->GetOrientedEdges ();
    for_each (v.begin (), v.end (), DisplayAllButLastVertices ());
    if (! f->IsClosed ())
	glVertex (*v[v.size () - 1]->GetEnd ());
    glEnd ();
}


void DisplaySameTriangles::operator() (const boost::shared_ptr<Face>& f)
{
    const vector<boost::shared_ptr<OrientedEdge> >& orientedEdges = 
	f->GetOrientedEdges ();
    glBegin (GL_TRIANGLES);
    if (f->IsTriangle ())
	for_each (orientedEdges.begin (), orientedEdges.end (),
		  DisplayBeginVertex());
    else
    {
	DisplayTriangle dt (f->GetCenter ());
	for_each (orientedEdges.begin (), orientedEdges.end (),
		  boost::bind (dt, _1));
	if (! f->IsClosed ())
	    dt(*orientedEdges[orientedEdges.size () - 1]->GetEnd (),
	       *orientedEdges[0]->GetBegin ());
    }
    glEnd ();
}
