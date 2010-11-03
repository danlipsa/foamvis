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

void DisplayEdgeTube::operator() (
    GLUquadricObj* quadric,
    double edgeRadius, const G3D::Vector3& begin, const G3D::Vector3& end)
{
    G3D::Matrix3 rotation = edgeRotation (begin, end);
    G3D::CoordinateFrame frame (rotation, begin);
    gluQuadricOrientation (quadric, GLU_OUTSIDE);
    glPushMatrix ();
    {
	glMultMatrix (frame);
	gluCylinder (
	    quadric, edgeRadius, edgeRadius, (end - begin).length (),
	    GLWidget::QUADRIC_SLICES, GLWidget::QUADRIC_STACKS);
	gluQuadricOrientation (quadric, GLU_INSIDE);
	gluDisk (quadric, 0, edgeRadius, GLWidget::QUADRIC_SLICES,
		 GLWidget::QUADRIC_STACKS);
	glTranslatef (end.x, end.y, end.z);
    }
    glPopMatrix ();
}

void DisplayEdge::operator() (
    GLUquadricObj* quadric,
    double edgeRadius, const G3D::Vector3& begin, const G3D::Vector3& end)
{
    static_cast<void> (quadric);
    static_cast<void> (edgeRadius);
    glBegin(GL_LINES);
    glVertex(begin);
    glVertex(end);
    glEnd();
}

void DisplayArrowTube::operator () (
    GLUquadricObj* quadric,
    double baseRadius, double topRadius, double height,
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    G3D::Matrix3 rotation = edgeRotation (begin, end);
    G3D::CoordinateFrame frame (rotation, (begin + end)/2);
    gluQuadricOrientation (quadric, GLU_OUTSIDE);
    glPushMatrix ();
    {
	glMultMatrix (frame);
	gluCylinder (quadric, baseRadius, topRadius, height,
		     GLWidget::QUADRIC_SLICES, GLWidget::QUADRIC_STACKS);
	gluQuadricOrientation (quadric, GLU_INSIDE);
	gluDisk (quadric, 0, baseRadius, GLWidget::QUADRIC_SLICES,
		 GLWidget::QUADRIC_STACKS);
    }
    glPopMatrix ();
}

void DisplayArrow::operator () (
    GLUquadricObj*,
    double, double, double, const G3D::Vector3& begin, const G3D::Vector3& end)
{
    glPushAttrib (GL_LINE_BIT);
    glLineWidth (2.0);
    glBegin(GL_LINES);
    glVertex(begin);
    glVertex((begin + end) / 2);
    glEnd();
    glPopAttrib ();
}
