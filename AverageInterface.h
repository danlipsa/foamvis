/**
 * @file   AverageInterface.h
 * @author Dan R. Lipsa
 * @date  9 June 2010
 *
 * Functions all Average classes have to implement.
 */

#ifndef __AVERAGE_INTERFACE_H__
#define __AVERAGE_INTERFACE_H__

struct AverageInterface
{
    virtual void AverageInit (ViewNumber::Enum viewNumber) = 0;
    virtual void AverageSetTimeWindow (size_t timeSteps) = 0;
    virtual void AverageStep (ViewNumber::Enum viewNumber, int timeStep) = 0;
    virtual void AverageRotateAndDisplay (
	ViewNumber::Enum viewNumber, StatisticsType::Enum displayType,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const = 0;
    virtual void AverageRelease () = 0;
};

#endif //__AVERAGE_INTERFACE_H__

// Local Variables:
// mode: c++
// End:
