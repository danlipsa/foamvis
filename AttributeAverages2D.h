/**
 * @file   AttributeAverages2D.h
 * @author Dan R. Lipsa
 * @date 19 July 2012
 * @brief Average for several attributes in a 2D view.
 */
#ifndef __ATTRIBUTES_AVERAGE2D_H__
#define __ATTRIBUTES_AVERAGE2D_H__

#include "AttributeAverages.h"

class ForceAverage;
class ScalarAverage2D;
class T1KDE2D;
class TensorAverage;
class WidgetGl;
class VectorAverage;
class ViewSettings;

/**
 * @brief Average for several attributes in a 2D view.
 */
class AttributeAverages2D : public AttributeAverages
{
public:
    AttributeAverages2D (ViewNumber::Enum viewNumber, 
                         const WidgetGl& widgetGl);

    ScalarAverage2D& GetScalarAverage () const
    {
	return *boost::static_pointer_cast<ScalarAverage2D> (m_scalarAverage);
    }
    
    T1KDE2D& GetT1KDE () const
    {
	return *boost::static_pointer_cast<T1KDE2D> (m_t1KDE);
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
