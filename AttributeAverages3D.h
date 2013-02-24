/**
 * @file   AttributeAverages3D.h
 * @author Dan R. Lipsa
 * @date 7 February 2013
 *
 * Per view average calculation
 */
#ifndef __ATTRIBUTES_AVERAGE3D_H__
#define __ATTRIBUTES_AVERAGE3D_H__


#include "AttributeAverages.h"

class ForceAverage;
class RegularGridAverage;
class Settings;
class SimulationGroup;
class ViewSettings;

class AttributeAverages3D : public AttributeAverages
{
public:
    AttributeAverages3D (
        ViewNumber::Enum viewNumber,
        boost::shared_ptr<Settings> settings, 
        boost::shared_ptr<const SimulationGroup> simulationGroup);
    
    const RegularGridAverage& GetScalarAverage () const
    {
	return *boost::static_pointer_cast<RegularGridAverage> (
            m_scalarAverage);
    }
    boost::shared_ptr<RegularGridAverage> GetScalarAveragePtr () const
    {
        return boost::static_pointer_cast<RegularGridAverage> (
            m_scalarAverage);
    }
    const  RegularGridAverage& GetVelocityAverage () const
    {
	return *boost::static_pointer_cast<RegularGridAverage> (
            m_velocityAverage);
    }    
    const RegularGridAverage& GetDeformationAverage () const
    {
	return *boost::static_pointer_cast<RegularGridAverage> (
            m_deformationAverage);
    }
};



#endif //__ATTRIBUTES_AVERAGE3D_H__

// Local Variables:
// mode: c++
// End:
