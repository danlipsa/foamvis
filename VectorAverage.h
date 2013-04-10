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

class ScalarAverage2D;
class VectorDisplay;
class AverageCache;

class VectorAverage : public TensorAverageTemplate<SetterVelocity>
{
public:
    VectorAverage (ViewNumber::Enum viewNumber, const WidgetGl& widgetGl);
    static void InitShaders ();
    void CacheData (boost::shared_ptr<AverageCache> averageCache) const;

private:
    vtkSmartPointer<vtkImageData> getData () const;    
};



#endif //__VECTOR_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
