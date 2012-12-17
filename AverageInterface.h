/**
 * @file   AverageInterface.h
 * @author Dan R. Lipsa
 * @date  9 June 2010
 *
 * Functions all Average classes have to implement.
 */

#ifndef __AVERAGE_INTERFACE_H__
#define __AVERAGE_INTERFACE_H__

#include "Enums.h"

/**
 * Interface for computing and displaying averages of datasets.
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
    size_t GetBodyAttribute () const;
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
