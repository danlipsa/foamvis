/**
 * @file   AttributesAverage2D.h
 * @author Dan R. Lipsa
 * @date 19 July 2012
 *
 * Per view average calculation
 */
#ifndef __ATTRIBUTES_AVERAGE2D_H__
#define __ATTRIBUTES_AVERAGE2D_H__

#include "AttributesAverage.h"

class ForceAverage;
class ScalarAverage;
class T1sKDE;
class TensorAverage;
class WidgetGl;
class VectorAverage;
class ViewSettings;

class AttributesAverage2D : public AttributesAverage
{
public:
    AttributesAverage2D (ViewNumber::Enum viewNumber, 
                         const WidgetGl& widgetGl);

    ScalarAverage& GetScalarAverage () const
    {
	return *boost::static_pointer_cast<ScalarAverage> (m_scalarAverage);
    }
    
    T1sKDE& GetT1sKDE () const
    {
	return *boost::static_pointer_cast<T1sKDE> (m_t1sKDE);
    }

    TensorAverage& GetDeformationAverage () const
    {
	return *boost::static_pointer_cast<TensorAverage> (
            m_deformationAverage);
    }
    VectorAverage& GetVelocityAverage () const
    {
	return *boost::static_pointer_cast<VectorAverage> (m_velocityAverage);
    }
};



#endif //__ATTRIBUTES_AVERAGE2D_H__

// Local Variables:
// mode: c++
// End: