/**
 * @file   Base.h
 * @author Dan R. Lipsa
 * @date 11 Dec 2012
 * 
 * 
 */

#ifndef __BASE_H__
#define __BASE_H__

#include "Enums.h"
class ViewSettings;
class Settings;
class SimulationGroup;

class Base
{
public:
    Base ();
    boost::shared_ptr<Settings> GetSettings () const
    {
	return m_settings;
    }
    void SetSettings (boost::shared_ptr<Settings> s)
    {
        m_settings = s;
    }

    ViewNumber::Enum GetViewNumber () const;
    ViewSettings& GetViewSettings (ViewNumber::Enum viewNumber) const;
    size_t GetTime (ViewNumber::Enum viewNumber) const;
    size_t GetViewCount () const;

    ViewSettings& GetViewSettings () const
    {
	return GetViewSettings (GetViewNumber ());
    }
    size_t GetTime () const
    {
	return GetTime (GetViewNumber ());
    }
    const SimulationGroup& GetSimulationGroup () const
    {
	return *m_simulationGroup;
    }
    void SetSimulationGroup (const SimulationGroup* sg)
    {
        m_simulationGroup = sg;
    }

private:
    boost::shared_ptr<Settings> m_settings;
    const SimulationGroup* m_simulationGroup;
};


#endif //__BASE_H__

// Local Variables:
// mode: c++
// End:
