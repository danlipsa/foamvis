/**
 * @file   DisplayForces.h
 * @author Dan R. Lipsa
 * @date  8 June 2010
 *
 * Interface for the DisplayForces class
 */

#ifndef __DISPLAY_FORCES_H__
#define __DISPLAY_FORCES_H__

#include "Enums.h"
class GLWidget;
class Force;

class DisplayForces
{
public:
    DisplayForces (const GLWidget& glWidget) :
	m_glWidget (glWidget)
    {
    }
    void Display (ViewNumber::Enum viewNumber) const;

private:
    void displayForces (
	ViewNumber::Enum viewNumber, const Force& force) const;
    void displayForce (
	QColor color,
	const G3D::Vector3& center, const G3D::Vector3& force) const;

private:
    const GLWidget& m_glWidget;
};


#endif //__DISPLAY_FORCES_H__

// Local Variables:
// mode: c++
// End:
