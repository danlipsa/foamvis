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
    virtual void Init (ViewNumber::Enum viewNumber) = 0;
    virtual void SetTimeWindow (size_t timeSteps) = 0;
    virtual void Step (ViewNumber::Enum viewNumber, int timeStep) = 0;
    virtual void RotateAndDisplay (
	ViewNumber::Enum viewNumber, StatisticsType::Enum displayType,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const = 0;
    virtual void Release () = 0;
};

#endif //__AVERAGE_INTERFACE_H__

// Local Variables:
// mode: c++
// End:
