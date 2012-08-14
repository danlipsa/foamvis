/**
 * @file   RegularGridAverage.h
 * @author Dan R. Lipsa
 * @date  8 June 2010
 *
 * Interface for the RegularGridAverage class
 */

#ifndef __REGULAR_GRID_AVERAGE_H__
#define __REGULAR_GRID_AVERAGE_H__

#include "Average.h"
#include "Enums.h"
class Settings;
class SimulationGroup;

/**
 * Average along time for a 3D regular grid
 */
class RegularGridAverage : public Average
{
public:
    typedef boost::function<double (double, double)> OpType;

public:
    RegularGridAverage (ViewNumber::Enum viewNumber, 
			const Settings& settings, 
			const SimulationGroup& simulationGroup);
    virtual void AverageInit ();
    virtual void AverageRotateAndDisplay (
	StatisticsType::Enum displayType = StatisticsType::AVERAGE,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;
    virtual void AverageRelease ();
    vtkSmartPointer<vtkImageData> GetAverage ();
    G3D::Vector3 GetTranslation (size_t timeStep) const;
    G3D::Vector3 GetTranslation () const;

protected:
    virtual void addStep (
	size_t timeStep, size_t subStep);
    virtual void removeStep (size_t timeStep, size_t subStep);

private:
    void opStep (size_t timeStep, OpType f);
    void computeAverage ();

private:
    vtkSmartPointer<vtkImageData> m_sum;
    vtkSmartPointer<vtkImageData> m_average;
};


#endif //__REGULAR_GRID_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
