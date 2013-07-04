/**
 * @file   ForceAverage.h
 * @author Dan R. Lipsa
 * @date  8 June 2010
 * @brief Time-average for forces acting on objects interacting with foam.
 * @ingroup average
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
class WidgetGl;

/**
 * @brief Time-average for forces acting on objects interacting with foam.
 */
class ForceAverage : public Average
{
public:
    ForceAverage (ViewNumber::Enum viewNumber,
                  boost::shared_ptr<Settings> settings, 
		  boost::shared_ptr<const SimulationGroup> simulationGroup,
                  boost::shared_ptr<DerivedData>* dd) :
	Average (viewNumber, settings, simulationGroup, dd)
    {
    }
    void DisplayOneTimeStep (WidgetGl* widgetGl) const;
    virtual void AverageRotateAndDisplay (
	StatisticsType::Enum displayType = StatisticsType::AVERAGE,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;
    void AverageRelease () {}
    virtual void AverageInit ();
    const vector<ForceOneObject>& GetForces (size_t timeStep) const;
    const vector<ForceOneObject>& GetForces () const;
    ForceOneObject GetAverageOneObject (size_t i) const;

protected:
    virtual void addStep (size_t timeStep, size_t subStep);
    virtual void removeStep (size_t timeStep, size_t subStep);

private:
    void displayForceAllObjects (WidgetGl* widgetGl,
	const vector<ForceOneObject>& forces, size_t timeWindow,
	bool isAverageAroundRotationShown = false) const;
    void displayForceOneObject (
        WidgetGl* widgetGl,
        const ForceOneObject& force) const;
    void displayTorqueOneObject (
        WidgetGl* widgetGl,
        const ForceOneObject& force) const;
    void displayForceTorqueOneObject (WidgetGl* widgetGl, 
                                      const ForceOneObject& force) const;
    void displayForce (
        WidgetGl* widgetGl,
        QColor color, const G3D::Vector3& center, 
        const G3D::Vector3& force) const;
    void displayTorque (
        WidgetGl* widgetGl,
	QColor color, const G3D::Vector3& center, 
	float distance, float angleRadians, float torque) const;
    ForceOneObject getForceDifference (
	const vector<ForceOneObject>& forces) const;
    pair<G3D::Vector3, G3D::Vector3> calculateTorque (
	G3D::Vector3 center, float distance, float angle, float torque) const;

private:
    // One set of forces for each object
    vector<ForceOneObject> m_average;
};


#endif //__FORCE_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
