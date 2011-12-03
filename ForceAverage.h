/**
 * @file   ForceAverage.h
 * @author Dan R. Lipsa
 * @date  8 June 2010
 *
 * Interface for the ForceAverage class
 */

#ifndef __FORCE_AVERAGE_H__
#define __FORCE_AVERAGE_H__

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
    void DisplayOneTimeStep (ViewNumber::Enum viewNumber) const;
    void Display (ViewNumber::Enum viewNumber,
		  bool adjustForAverageAroundMovementRotation = false) const;    
    virtual void AverageRotateAndDisplay (
	ViewNumber::Enum viewNumber, 
	StatisticsType::Enum displayType = StatisticsType::AVERAGE,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;
    void AverageRelease () {}
    virtual void AverageInit (ViewNumber::Enum viewNumber);

protected:
    virtual void addStep (ViewNumber::Enum viewNumber, size_t timeStep, 
			  size_t subStep);
    virtual void removeStep (ViewNumber::Enum viewNumber, size_t timeStep, 
			     size_t subStep);

private:
    void display (
	ViewNumber::Enum viewNumber, 
	const vector<Force>& forces, size_t timeWindow,
	bool adjustForAverageAroundMovementRotation = false) const;
    void displayForces (
	ViewNumber::Enum viewNumber, const Force& force, size_t count) const;
    void displayForce (ViewNumber::Enum viewNumber, QColor color,
		       const G3D::Vector3& center, 
		       const G3D::Vector3& force) const;

private:
    vector<Force> m_average;
};


#endif //__FORCE_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
