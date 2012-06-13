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
#include "ForcesOneObject.h"
class WidgetGl;

class ForceAverage : public Average
{
public:
    ForceAverage (const WidgetGl& widgetGl) :
	Average (widgetGl)
    {
    }
    void DisplayOneTimeStep (ViewNumber::Enum viewNumber) const;
    void Display (ViewNumber::Enum viewNumber,
		  bool adjustForAverageAroundMovementRotation = false) const;    
    virtual void AverageRotateAndDisplay (
	ViewNumber::Enum viewNumber, 
	ComputationType::Enum displayType = ComputationType::AVERAGE,
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
    void displayForcesAllObjects (
	ViewNumber::Enum viewNumber, 
	const vector<ForcesOneObject>& forces, size_t timeWindow,
	bool adjustForAverageAroundMovementRotation = false) const;
    void displayForcesOneObject (
	ViewNumber::Enum viewNumber, const ForcesOneObject& force, 
	size_t count) const;
    void displayTorqueOneObject (
	ViewNumber::Enum viewNumber, const ForcesOneObject& force, 
	size_t count) const;
    void displayForcesTorqueOneObject (
	ViewNumber::Enum viewNumber, const ForcesOneObject& force, 
	size_t count) const;
    void displayForce (ViewNumber::Enum viewNumber, QColor color,
		       const G3D::Vector2& center, 
		       const G3D::Vector2& force) const;
    void displayTorque (
	ViewNumber::Enum viewNumber, QColor color,
	const G3D::Vector2& center, 
	float distance, float angleRadians, float torque) const;
    const ForcesOneObject getForceDifference (
	ViewNumber::Enum viewNumber, 
	const vector<ForcesOneObject>& forces) const;
    pair<G3D::Vector2, G3D::Vector2> computeTorque (
	G3D::Vector2 center, float distance, float angle, float torque) const;

private:
    // One set of forces for each object
    vector<ForcesOneObject> m_average;
};


#endif //__FORCE_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
