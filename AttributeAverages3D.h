/**
 * @file   AttributeAverages3D.h
 * @author Dan R. Lipsa
 * @date 7 February 2013
 * @brief Computes the average for several attributes in a 3D
 * view. Casts the computed averages to the proper 3D types.
 * @ingroup average
 */
#ifndef __ATTRIBUTES_AVERAGE3D_H__
#define __ATTRIBUTES_AVERAGE3D_H__


#include "AttributeAverages.h"

class ForceAverage;
class RegularGridAverage;
class Settings;
class SimulationGroup;
class ViewSettings;

/**
 * @brief Computes the average for several attributes in a 3D
 * view. Casts the computed averages to the proper 3D types.
 */
class AttributeAverages3D : public AttributeAverages
{
public:
    AttributeAverages3D (
        ViewNumber::Enum viewNumber,
        boost::shared_ptr<Settings> settings, 
        boost::shared_ptr<const SimulationGroup> simulationGroup);
    
    void ComputeAverage ();

    boost::shared_ptr<RegularGridAverage> GetScalarAverage ()
    {
        return boost::static_pointer_cast<RegularGridAverage> (
            m_scalarAverage);
    }
    boost::shared_ptr<RegularGridAverage> GetT1KDE ()
    {
	return boost::static_pointer_cast<RegularGridAverage> (
            m_t1KDE);
    }
    boost::shared_ptr<RegularGridAverage> GetBodyOrOtherScalarAverage ();
    boost::shared_ptr<RegularGridAverage> GetVelocityAverage ()
    {
	return boost::static_pointer_cast<RegularGridAverage> (
            m_velocityAverage);
    }
    boost::shared_ptr<RegularGridAverage> GetDeformationAverage ()
    {
	return boost::static_pointer_cast<RegularGridAverage> (
            m_deformationAverage);
    }
};



#endif //__ATTRIBUTES_AVERAGE3D_H__

// Local Variables:
// mode: c++
// End:
