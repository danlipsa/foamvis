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
class Settings;
class SimulationGroup;
class Simulation;
class Foam;

/**
 * Average along time for forces acting on objects interacting with foam.
 */
class ForceAverage : public Average
{
public:
    ForceAverage (ViewNumber::Enum viewNumber, const Settings& settings, 
		  const SimulationGroup& simulationGroup) :
	Average (viewNumber, settings, simulationGroup)
    {
    }
    void DisplayOneTimeStep () const;
    void Display (bool adjustForAverageAroundMovementRotation = false) const;    
    virtual void AverageRotateAndDisplay (
	ComputationType::Enum displayType = ComputationType::AVERAGE,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;
    void AverageRelease () {}
    virtual void AverageInit ();
    const vector<ForcesOneObject>& GetForces (size_t timeStep) const;


protected:
    virtual void addStep (size_t timeStep, size_t subStep);
    virtual void removeStep (size_t timeStep, size_t subStep);

private:
    void displayForcesAllObjects (
	const vector<ForcesOneObject>& forces, size_t timeWindow,
	bool adjustForAverageAroundMovementRotation = false) const;
    void displayForcesOneObject (
	const ForcesOneObject& force, size_t count) const;
    void displayTorqueOneObject (const ForcesOneObject& force, 
				 size_t count) const;
    void displayForcesTorqueOneObject (const ForcesOneObject& force, 
				       size_t count) const;
    void displayForce (QColor color, const G3D::Vector2& center, 
		       const G3D::Vector2& force) const;
    void displayTorque (
	QColor color, const G3D::Vector2& center, 
	float distance, float angleRadians, float torque) const;
    const ForcesOneObject getForceDifference (
	const vector<ForcesOneObject>& forces) const;
    pair<G3D::Vector2, G3D::Vector2> calculateTorque (
	G3D::Vector2 center, float distance, float angle, float torque) const;

private:
    // One set of forces for each object
    vector<ForcesOneObject> m_average;
};


#endif //__FORCE_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
