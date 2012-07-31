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
    void AverageInitStep ()
    {
	AverageInit ();
	AverageStep (1);
    }
    size_t GetBodyAttribute () const;
    virtual void AverageSetTimeWindow (size_t timeSteps) = 0;
    virtual void AverageStep (int timeDifference) = 0;
    virtual void AverageRotateAndDisplay (	
	ComputationType::Enum displayType = ComputationType::AVERAGE,
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
