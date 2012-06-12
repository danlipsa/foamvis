/**
 * @file   VectorAverage.h
 * @author Dan R. Lipsa
 * @date  25 Jul 2011
 *
 * Interface for the VectorAverage class
 */

#ifndef __VECTOR_AVERAGE_H__
#define __VECTOR_AVERAGE_H__

#include "ImageBasedAverage.h"
#include "PropertySetter.h"
#include "TensorAverage.h"

class ScalarAverage;
class VectorDisplay;

class VectorAverage : public TensorAverageTemplate<SetterVelocity>
{
public:
    VectorAverage (const GLWidget& widgetGl,
		   FramebufferObjects& scalarAverageFbos);

    static void InitShaders ();
};



#endif //__VECTOR_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
