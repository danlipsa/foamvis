/**
 * @file   Base.h
 * @author Dan R. Lipsa
 * @date 11 Dec 2012
 * @ingroup model
 * @brief %Simulation data, derived data, and program status.
 * 
 *
 * @defgroup model Model
 * Data and program status
 */

#ifndef __BASE_H__
#define __BASE_H__

#include "Enums.h"
class Foam;
class Settings;
class Simulation;
class SimulationGroup;
class ViewSettings;
class DerivedData;
class AverageCacheT1KDEVelocity;
class ObjectPositions;

/**
 * @brief %Simulation data, derived data, and program status.
 */
class Base
{
public:
    typedef bool (Base::*IsViewType) (
	ViewNumber::Enum viewNumber) const;
    typedef ViewCount::Enum (Base::*GetViewCountType) (
	vector<ViewNumber::Enum>* mapping) const;


public:
    Base ();
    Base (boost::shared_ptr<Settings> settings, 
          boost::shared_ptr<const SimulationGroup> simulationGroup,
          boost::shared_ptr<DerivedData>* dd);
    const Settings& GetSettings () const
    {
	return *m_settings;
    }
    boost::shared_ptr<Settings> GetSettingsPtr () const
    {
	return m_settings;
    }
    void SetSettings (boost::shared_ptr<Settings> s)
    {
        m_settings = s;
    }

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
    const boost::shared_ptr<const SimulationGroup> GetSimulationGroupPtr () const
    {
        return m_simulationGroup;
    }
    void SetSimulationGroup (boost::shared_ptr<const SimulationGroup> sg)
    {
        m_simulationGroup = sg;
    }
    DerivedData& GetDerivedData (ViewNumber::Enum viewNumber) const
    {
        return *m_derivedData[viewNumber];
    }    
    DerivedData& GetDerivedData () const
    {
        return *m_derivedData[GetViewNumber ()];
    }
    boost::shared_ptr<DerivedData>* GetDerivedDataAllPtr () const
    {
        return m_derivedData;
    }
    boost::shared_ptr<AverageCacheT1KDEVelocity> GetAverageCache (
        ViewNumber::Enum viewNumber) const;
    boost::shared_ptr<ObjectPositions> GetObjectPositions (
        ViewNumber::Enum viewNumber) const;
    void SetDerivedData (boost::shared_ptr<DerivedData>* op)
    {
        m_derivedData = op;
    }
    const Simulation& GetSimulation (ViewNumber::Enum viewNumber) const;
    const Simulation& GetSimulation (size_t index) const;
    const Simulation& GetSimulation () const
    {
        return GetSimulation (GetViewNumber ());
    }
    // current viewNumber, different time steps
    const Foam& GetFoam (size_t timeStep) const;
    // current viewNumber, current time step
    const Foam& GetFoam () const;
    // different view number, current time step
    const Foam& GetFoam (ViewNumber::Enum viewNumber) const;


    /**
     * @{
     * @name Gl, Vtk, and Histogram views
     */
    bool IsVtkView (ViewNumber::Enum viewNumber) const;
    bool IsGlView (ViewNumber::Enum viewNumber) const;
    bool IsGlView () const
    {
        return IsGlView (GetViewNumber ());
    }
    
    bool IsHistogramShown (ViewNumber::Enum viewNumber) const;
    ViewCount::Enum GetVtkCount (vector<ViewNumber::Enum>* mapping = 0) const;
    ViewCount::Enum GetGlCount (vector<ViewNumber::Enum>* mapping = 0) const;
    ViewCount::Enum GetHistogramCount (
	vector<ViewNumber::Enum>* mapping = 0) const;
    // @}

    float GetBubbleDiameter (ViewNumber::Enum viewNumber) const;
    float GetBubbleDiameter () const
    {
        return GetBubbleDiameter (GetViewNumber ());
    }

    void CopyTransformFrom (ViewNumber::Enum viewNumber);
    void CopyForceRatioFrom (ViewNumber::Enum viewNumber);

    QwtDoubleInterval GetInterval (AverageType::Enum averageType, 
                                   ViewNumber::Enum viewNumber) const;
    QwtDoubleInterval GetInterval (AverageType::Enum averageType) const
    {
        return GetInterval (averageType, GetViewNumber ());
    }
    QwtDoubleInterval GetIntervalVelocityMagnitude (
	ViewNumber::Enum viewNumber) const;

    QwtDoubleInterval GetIntervalCount (ViewNumber::Enum viewNumber) const;
    QwtDoubleInterval GetIntervalCount () const
    {
        return GetIntervalCount (GetViewNumber ());
    }

    QwtDoubleInterval GetIntervalT1KDE (ViewNumber::Enum viewNumber) const;
    QwtDoubleInterval GetIntervalT1KDE () const
    {
	return GetIntervalT1KDE (GetViewNumber ());
    }


    virtual ViewNumber::Enum GetViewNumber () const;



private:
    ViewCount::Enum getViewCount (
	vector<ViewNumber::Enum>* mapping, IsViewType isView) const;


private:
    boost::shared_ptr<Settings> m_settings;
    boost::shared_ptr<const SimulationGroup> m_simulationGroup;
    boost::shared_ptr<DerivedData>* m_derivedData;
};


#endif //__BASE_H__

// Local Variables:
// mode: c++
// End:
