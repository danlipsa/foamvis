/**
 * @file   DisplayBodyFunctors.h
 * @author Dan R. Lipsa
 * @date  26 Oct. 2010
 *
 * Implementation for functors to display a body
 */

#include "DisplayBodyFunctors.h"

DisplayBodyCenter::DisplayBodyCenter (
    const GLWidget& widget, const BodySelector& bodySelector):
    DisplayBodyBase (widget, TexCoordSetter(widget), 
		     BodyProperty::NONE, bodySelector)
{}


void DisplayBodyCenter::display (boost::shared_ptr<Body> b, FocusContext fc)
{
    if (fc == FOCUS)
    {
	glBegin(GL_POINTS);
	G3D::Vector3 v = b->GetCenter ();
	glVertex(v);
	glEnd ();
    }
}
