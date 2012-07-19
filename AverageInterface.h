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
    virtual void AverageInit (ViewNumber::Enum viewNumber) = 0;
    void AverageInitStep (ViewNumber::Enum viewNumber)
    {
	AverageInit (viewNumber);
	AverageStep (viewNumber, 1);
    }
    virtual void AverageSetTimeWindow (size_t timeSteps) = 0;
    virtual void AverageStep (ViewNumber::Enum viewNumber, 
			      int timeDifference) = 0;
    virtual void AverageRotateAndDisplay (
	ViewNumber::Enum viewNumber, 
	ComputationType::Enum displayType = ComputationType::AVERAGE,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const = 0;
    virtual void AverageRelease () = 0;
};

#endif //__AVERAGE_INTERFACE_H__

// Local Variables:
// mode: c++
// End:
