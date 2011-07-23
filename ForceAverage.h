/**
 * @file   ForceAverage.h
 * @author Dan R. Lipsa
 * @date  8 June 2010
 *
 * Interface for the ForceAverage class
 */

#ifndef __DISPLAY_FORCE_AVERAGE_H__
#define __DISPLAY_FORCE_AVERAGE_H__

#include "Average.h"
#include "Enums.h"
#include "Force.h"
class GLWidget;

class ForceAverage : public Average
{
public:
    ForceAverage (const GLWidget& glWidget) :
	Average (glWidget)
    {
    }
    void Display (ViewNumber::Enum viewNumber) const;
    void DisplayAverage (ViewNumber::Enum viewNumber) const;

protected:
    virtual void init (ViewNumber::Enum viewNumber);
    virtual void addStep (ViewNumber::Enum viewNumber, size_t timeStep);
    virtual void removeStep (ViewNumber::Enum viewNumber, size_t timeStep);

private:
    void display (
	ViewNumber::Enum viewNumber, 
	const vector<Force>& forces, size_t count) const;
    void displayForces (
	ViewNumber::Enum viewNumber, const Force& force, size_t count) const;
    void displayForce (
	QColor color,
	const G3D::Vector3& center, const G3D::Vector3& force) const;

private:
    vector<Force> m_average;
};


#endif //__DISPLAY_FORCE_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
