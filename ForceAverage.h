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
#include "ForceOneObject.h"
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
    virtual void AverageRotateAndDisplay (
	StatisticsType::Enum displayType = StatisticsType::AVERAGE,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;
    void AverageRelease () {}
    virtual void AverageInit ();
    const vector<ForceOneObject>& GetForces (size_t timeStep) const;
    const ForceOneObject& GetForceOneObject (size_t i) const
    {
        return m_average[i];
    }

protected:
    virtual void addStep (size_t timeStep, size_t subStep);
    virtual void removeStep (size_t timeStep, size_t subStep);

private:
    void displayForcesAllObjects (
	const vector<ForceOneObject>& forces, size_t timeWindow,
	bool isAverageAroundRotationShown = false) const;
    void displayForceOneObject (
	const ForceOneObject& force, size_t count) const;
    void displayTorqueOneObject (const ForceOneObject& force, 
				 size_t count) const;
    void displayForcesTorqueOneObject (const ForceOneObject& force, 
				       size_t count) const;
    void displayForce (QColor color, const G3D::Vector2& center, 
		       const G3D::Vector2& force) const;
    void displayTorque (
	QColor color, const G3D::Vector2& center, 
	float distance, float angleRadians, float torque) const;
    ForceOneObject getForceDifference (
	const vector<ForceOneObject>& forces) const;
    pair<G3D::Vector2, G3D::Vector2> calculateTorque (
	G3D::Vector2 center, float distance, float angle, float torque) const;

private:
    // One set of forces for each object
    vector<ForceOneObject> m_average;
};


#endif //__FORCE_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
