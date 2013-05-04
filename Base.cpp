/**
 * @file   Base.cpp
 * @author Dan R. Lipsa
 * @date 11 Dec. 2012
 * 
 * Definitions for a class that stores settings and simulations
 */


#include "Base.h"
#include "Body.h"
#include "Debug.h"
#include "Foam.h"
#include "Settings.h"
#include "Simulation.h"
#include "ViewSettings.h"


ViewNumber::Enum Base::GetViewNumber () const
{
    return GetSettings ().GetViewNumber ();
}

ViewSettings& Base::GetViewSettings (ViewNumber::Enum viewNumber) const
{
    return GetSettings ().GetViewSettings (viewNumber);
}

size_t Base::GetTime (ViewNumber::Enum viewNumber) const
{
    return GetViewSettings (viewNumber).GetTime ();
}

size_t Base::GetViewCount () const
{
    return GetSettings ().GetViewCount ();
}

const Simulation& Base::GetSimulation (ViewNumber::Enum viewNumber) const
{
    return GetSimulationGroup ().GetSimulation (GetSettings (), viewNumber);
}

const Simulation& Base::GetSimulation (size_t index) const
{
    return GetSimulationGroup ().GetSimulation (index);
}

const Foam& Base::GetFoam (size_t timeStep) const
{
    return GetSimulation ().GetFoam (timeStep);
}

const Foam& Base::GetFoam () const
{
    size_t currentTime = GetViewSettings ().GetTime ();
    return GetSimulation ().GetFoam (currentTime);
}

const Foam& Base::GetFoam (ViewNumber::Enum viewNumber) const
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    return GetSimulation (viewNumber).GetFoam (vs.GetTime ());
}

bool Base::IsVtkView (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const Simulation& simulation = GetSimulation (viewNumber);
    bool isVtkView = 
        simulation.Is3D () && 
        (vs.GetViewType () == ViewType::AVERAGE || 
         vs.GetViewType () == ViewType::T1_KDE);
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

float Base::GetBubbleDiameter (ViewNumber::Enum viewNumber) const
{    
    vector<ViewNumber::Enum> vn = 
	GetSettings ().GetTwoHalvesViewNumbers (viewNumber);
    float size = GetSimulation (vn[0]).GetBubbleDiameter ();
    for (size_t i = 1; i < vn.size (); ++i)
    {
	float s = GetSimulation (vn[i]).GetBubbleDiameter ();
	size = min (size, s);
    }
    return size;
}

void Base::CopyTransformFrom (ViewNumber::Enum viewNumber)
{
    if (GetViewNumber () == viewNumber)
        return;
    ViewSettings& vs = GetViewSettings ();
    const ViewSettings& fromVs = GetViewSettings (viewNumber);
    vs.SetRotation (fromVs.GetRotation ());
    vs.SetTranslation (fromVs.GetTranslation ());
    float bubbleDiameterInPixels = GetBubbleDiameter () / 
        vs.GetOnePixelInObjectSpace ();    
    float fromBubbleDiameterInPixels = GetBubbleDiameter (viewNumber) / 
        fromVs.GetOnePixelInObjectSpace ();
    vs.SetScaleRatio (
        vs.GetScaleRatio () * 
        fromBubbleDiameterInPixels / bubbleDiameterInPixels);
}


void Base::CopyForceRatioFrom (ViewNumber::Enum viewNumber)
{
    if (GetViewNumber () == viewNumber)
        return;
    float objectDiameter = GetFoam ().GetObjects ()[0]->GetBubbleDiameter ();
    float fromObjectDiameter = 
        GetFoam (viewNumber).GetObjects ()[0]->GetBubbleDiameter ();
    ViewSettings& vs = GetViewSettings ();
    const ViewSettings& fromVs = GetViewSettings (viewNumber);
    float bubbleDiameter = GetBubbleDiameter ();
    float fromBubbleDiameter = GetBubbleDiameter (viewNumber);
    float bubbleDiameterInPixels = bubbleDiameter / 
        vs.GetOnePixelInObjectSpace ();    
    float fromBubbleDiameterInPixels = fromBubbleDiameter / 
        fromVs.GetOnePixelInObjectSpace ();
    float ratio = objectDiameter / bubbleDiameter;
    float fromRatio = fromObjectDiameter / fromBubbleDiameter;
    float scaleRatioSameBubbleDiameter = 
        vs.GetScaleRatio () * fromBubbleDiameterInPixels / 
        bubbleDiameterInPixels / fromVs.GetScaleRatio ();
    float scaleRatio = vs.GetScaleRatio () / fromVs.GetScaleRatio ();
    vs.SetForceRatio (fromVs.GetForceRatio () * 
                      (fromRatio / ratio) *
                      (scaleRatioSameBubbleDiameter / scaleRatio));
}
