/**
 * @file   Base.cpp
 * @author Dan R. Lipsa
 * @date 11 Dec. 2012
 * 
 * Definitions for a class that stores settings and simulations
 */


#include "Base.h"
#include "Debug.h"
#include "Settings.h"
#include "Simulation.h"
#include "ViewSettings.h"

Base::Base () :
    m_simulationGroup (0)
{
}

ViewNumber::Enum Base::GetViewNumber () const
{
    return GetSettings ()->GetViewNumber ();
}

ViewSettings& Base::GetViewSettings (ViewNumber::Enum viewNumber) const
{
    return GetSettings ()->GetViewSettings (viewNumber);
}

size_t Base::GetTime (ViewNumber::Enum viewNumber) const
{
    return GetViewSettings (viewNumber).GetTime ();
}

size_t Base::GetViewCount () const
{
    return GetSettings ()->GetViewCount ();
}

const Simulation& Base::GetSimulation (ViewNumber::Enum viewNumber) const
{
    return GetSimulationGroup ().GetSimulation (*GetSettings (), viewNumber);
}

const Simulation& Base::GetSimulation (size_t index) const
{
    return GetSimulationGroup ().GetSimulation (index);
}



bool Base::IsVtkView (ViewNumber::Enum viewNumber) const
{
    __ENABLE_LOGGING__;
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const Simulation& simulation = GetSimulation (viewNumber);
    bool isVtkView = 
        simulation.Is3D () && vs.GetViewType () == ViewType::AVERAGE;
    __LOG__ (cdbg << isVtkView << endl;);
    return isVtkView;
}

bool Base::IsGlView (ViewNumber::Enum viewNumber) const
{
    return ! IsVtkView (viewNumber);
}


bool Base::IsHistogramShown (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = 
	GetViewSettings (ViewNumber::FromSizeT (viewNumber));
    return vs.IsHistogramShown ();
}


ViewCount::Enum Base::getViewCount (
    vector<ViewNumber::Enum>* mapping, IsViewType isView) const
{
    vector<ViewNumber::Enum> m;
    if (mapping == 0)
	mapping = &m;
    size_t viewCount = GetViewCount ();
    mapping->resize (viewCount);
    fill (mapping->begin (), mapping->end (), ViewNumber::COUNT);
    int count = 0;
    for (size_t i = 0; i < viewCount; ++i)
	if (CALL_MEMBER (*this, isView) (ViewNumber::FromSizeT (i)))
	{
	    (*mapping)[i] = ViewNumber::FromSizeT (count);
	    count++;
	}
    return ViewCount::FromSizeT (count);
}


ViewCount::Enum Base::GetVtkCount (vector<ViewNumber::Enum>* mapping) const
{
    return getViewCount (mapping, &Base::IsVtkView);
}

ViewCount::Enum Base::GetHistogramCount (
    vector<ViewNumber::Enum>* mapping) const
{
    return getViewCount (mapping, &Base::IsHistogramShown);
}

ViewCount::Enum Base::GetGlCount (vector<ViewNumber::Enum>* mapping) const
{
    return getViewCount (mapping, &Base::IsGlView);
}
