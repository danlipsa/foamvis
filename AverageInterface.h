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

class AverageInterface
{
public:
    AverageInterface () :
	m_viewNumber (ViewNumber::COUNT)
    {
    }
    virtual void AverageInit (ViewNumber::Enum viewNumber)
    {
	m_viewNumber = viewNumber;
    }
    ViewNumber::Enum GetViewNumber () const
    {
	return m_viewNumber;
    }
    void AverageInitStep (ViewNumber::Enum viewNumber)
    {
	AverageInit (viewNumber);
	AverageStep (1);
    }
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
