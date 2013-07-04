/**
 * @file   VectorAverage.h
 * @author Dan R. Lipsa
 * @date  25 Jul 2011
 * @ingroup average
 * @brief Computes a pixel-based time-average of vector attributes
 */

#ifndef __VECTOR_AVERAGE_H__
#define __VECTOR_AVERAGE_H__

#include "ImageBasedAverage.h"
#include "PropertySetter.h"
#include "TensorAverage.h"

class ScalarAverage;
class VectorDisplay;
class AverageCacheT1KDEVelocity;

/**
 * @brief Computes a pixel-based time-average of vector attributes
 */
class VectorAverage : public TensorAverageTemplate<SetterVelocity>
{
public:
    VectorAverage (ViewNumber::Enum viewNumber, const WidgetGl& widgetGl);
    static void InitShaders ();
    void CacheData (boost::shared_ptr<AverageCacheT1KDEVelocity> averageCache) const;

private:
    vtkSmartPointer<vtkImageData> getData () const;    
};



#endif //__VECTOR_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
