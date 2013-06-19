/**
 * @file   RegularGridAverage.h
 * @author Dan R. Lipsa
 * @date  8 June 2010
 * @ingroup average
 * @brief Time-average for a 3D regular grid
 */

#ifndef __REGULAR_GRID_AVERAGE_H__
#define __REGULAR_GRID_AVERAGE_H__

#include "Average.h"
#include "Enums.h"
class Settings;
class SimulationGroup;

/**
 * @brief Time-average for a 3D regular grid
 */
class RegularGridAverage : public Average
{
public:
    typedef boost::function<double (double, double)> OpType;

public:
    RegularGridAverage (
        ViewNumber::Enum viewNumber, 
        boost::shared_ptr<Settings> settings, 
        boost::shared_ptr<const SimulationGroup> simulationGroup);
    bool IsInitialized () const
    {
        return m_sum != 0;
    }
    virtual void AverageInit ();
    virtual void AverageRotateAndDisplay (
	StatisticsType::Enum displayType = StatisticsType::AVERAGE,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;
    virtual void AverageRelease ();
    void ComputeAverage ();
    const vtkImageData& GetAverage () const
    {
        return *m_average;
    }
    size_t GetBodyAttribute () const
    {
        return m_bodyAttribute;
    }
    void SetBodyAttribute (size_t attribute)
    {
        m_bodyAttribute = attribute;
    }

protected:
    virtual void addStep (
	size_t timeStep, size_t subStep);
    virtual void removeStep (size_t timeStep, size_t subStep);
    virtual size_t getStepSize (size_t timeStep) const;

private:
    void opStep (size_t timeStep, size_t subStep, OpType f);

private:
    size_t m_bodyAttribute;
    vtkSmartPointer<vtkImageData> m_sum;
    vtkSmartPointer<vtkImageData> m_average;
};


#endif //__REGULAR_GRID_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
