/**
 * @file   AverageInterface.h
 * @author Dan R. Lipsa
 * @date  9 June 2010
 * @brief Interface for computing a time-average of a simulation attribute.
 * @ingroup average
 *
 * @defgroup average Average
 * Computes a time-average of simulation attributes
 */

#ifndef __AVERAGE_INTERFACE_H__
#define __AVERAGE_INTERFACE_H__

#include "Enums.h"

/**
 * @brief Interface for computing a time-average of a simulation attribute.
 */
class AverageInterface
{
public:
    AverageInterface (ViewNumber::Enum viewNumber) :
	m_viewNumber (viewNumber)
    {
    }
    virtual void AverageInit () = 0;
    ViewNumber::Enum GetViewNumber () const
    {
	return m_viewNumber;
    }
    void AverageInitStep (size_t timeWindow)
    {
	AverageInit ();
	AverageStep (1, timeWindow);
    }
    virtual void AverageStep (int timeDifference, size_t timeWindow) = 0;
    virtual void AverageRotateAndDisplay (	
	StatisticsType::Enum displayType = StatisticsType::AVERAGE,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const = 0;
    virtual void AverageRelease () = 0;

private:
    ViewNumber::Enum m_viewNumber;
};

#endif //__AVERAGE_INTERFACE_H__

// Local Variables:
// mode: c++
// End:
