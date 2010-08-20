/**
 * @file FoamAlongTime.cpp
 * @author Dan R. Lipsa
 *
 * Method implementation for a list of Foam objects
 */

#include "Body.h"
#include "Debug.h"
#include "DebugStream.h"
#include "Foam.h"
#include "FoamAlongTime.h"
#include "Utils.h"

// Private Functions
// ======================================================================
inline void calculateBodyWraps (BodiesAlongTime::BodyMap::value_type& v,
				const FoamAlongTime& foamAlongTime)
{
    v.second->CalculateBodyWraps (foamAlongTime);
}

inline bool isInside (double value, double min, double max)
{
    return value >= min && value <= max;
}

inline bool isIntersection (double firstMin, double firstMax,
			    double secondMin, double secondMax)
{
    if ((firstMax - firstMin) > (secondMax - secondMin))
    {
	swap (firstMin, secondMin);
	swap (firstMax, secondMax);
    }
    // first interval less than or equal with the second interval
    return 
	isInside (firstMin, secondMin, secondMax) || 
	isInside (firstMax, secondMin, secondMax);
}


// Members
// ======================================================================
FoamAlongTime::FoamAlongTime () :
    m_maxCountPerBinIndividual (BodyProperty::COUNT)
{
}

void FoamAlongTime::calculate (
    Aggregate aggregate, FoamLessThanAlong::Corner corner, G3D::Vector3& v)
{
    using G3D::Vector3;
    Foams::iterator it;
    it = aggregate (m_foams.begin (), m_foams.end (), 
		    FoamLessThanAlong(Vector3::X_AXIS, corner));
    v.x = ((*it).get()->*corner) ().x;

    it = aggregate (m_foams.begin (), m_foams.end (), 
	    FoamLessThanAlong(Vector3::Y_AXIS, corner));    
    v.y = ((*it).get()->*corner) ().y;

    it = aggregate (m_foams.begin (), m_foams.end (), 
	    FoamLessThanAlong(Vector3::Z_AXIS, corner));
    v.z = ((*it).get()->*corner) ().z;
}

void FoamAlongTime::CalculateAABox ()
{
    using G3D::Vector3;
    Vector3 low, high;
    calculate (min_element, &Foam::GetAABoxLow, low);
    calculate (max_element, &Foam::GetAABoxHigh, high);
    m_AABox.set (low, high);
}

void FoamAlongTime::calculateBodyWraps ()
{
    if (m_foams.size () > 1)
    {
	BodiesAlongTime::BodyMap bodyMap = GetBodiesAlongTime ().GetBodyMap ();
	for_each (bodyMap.begin (), bodyMap.end (),
		  boost::bind(::calculateBodyWraps, _1, *this));
    }
}

void FoamAlongTime::PostProcess ()
{
    CalculateAABox ();
    CacheBodiesAlongTime ();
    calculateBodyWraps ();
    GetBodiesAlongTime ().CalculateRange (*this);
    GetBodiesAlongTime ().CalculateHistogram (*this);
    GetBodiesAlongTime ().CalculateMaxCountPerBin ();
    calculateRange ();
    calculateHistogram ();
    calculateMaxCountPerBin ();
}

void FoamAlongTime::CacheBodiesAlongTime ()
{
    Foam::Bodies& bodies = m_foams[0]->GetBodies ();
    for_each (bodies.begin (), bodies.end (), 
	      boost::bind (&BodiesAlongTime::Allocate,
			   &m_bodiesAlongTime, _1, m_foams.size ()));
    for (size_t timeStep = 0; timeStep < m_foams.size (); timeStep++)
    {
	Foam::Bodies& bodies = m_foams[timeStep]->GetBodies ();
	BOOST_FOREACH (boost::shared_ptr<Body>  body, bodies)
	    m_bodiesAlongTime.Cache (body, timeStep);
    }
    for_each (bodies.begin (), bodies.end (),
	      boost::bind (&BodiesAlongTime::Resize, &m_bodiesAlongTime, _1));
}

float FoamAlongTime::GetBodyProperty (
    BodyProperty::Enum property,
    size_t bodyId, size_t timeStep) const
{
    const BodyAlongTime& bat = GetBodiesAlongTime ().GetBodyAlongTime (bodyId);
    if (property >= BodyProperty::VELOCITY_BEGIN &&
	property < BodyProperty::VELOCITY_END)
    {
	if (timeStep == GetBodiesAlongTime ().GetBodyAlongTime (bodyId).
	    GetBodies ().size () - 1)
	    --timeStep;
	StripIterator it (bat, *this, timeStep);
	RuntimeAssert (it.HasNext (),
		       "Cannot find velocity. First point not available");
	StripIterator::Point p = it.Next ();
	RuntimeAssert (it.HasNext (), 
		       "Cannot find velocity. Second point not available");
	StripIterator::Point next = it.Next ();
	return StripIterator::GetPropertyValue (property, next, p);
    }
    else
	return bat.GetBody (timeStep)->GetPropertyValue (property);
}


bool FoamAlongTime::ExistsBodyProperty (
    BodyProperty::Enum property,
    size_t bodyId, size_t timeStep) const
{
    if (property >= BodyProperty::VELOCITY_BEGIN &&
	property < BodyProperty::VELOCITY_END)
    {
	if (timeStep == 0 && 
	    GetBodiesAlongTime ().GetBodyAlongTime (bodyId).
	    GetBodies ().size () == 1)
	    return false;
	else
	    return true;
    }
    return GetBody (bodyId, timeStep).ExistsPropertyValue (property);
}


void FoamAlongTime::calculateHistogram ()
{
    for (size_t timeStep = 0; timeStep < GetTimeSteps (); ++timeStep)
	calculateHistogram (timeStep);
}

void FoamAlongTime::calculateHistogram (size_t timeStep)
{
    boost::shared_ptr<const Foam> foam = GetFoam (timeStep);
    BOOST_FOREACH (boost::shared_ptr<const Body> body, foam->GetBodies ())
    {
	size_t bodyId = body->GetId ();
	m_foamsStatistics[timeStep].HistogramStep (
	    *this, bodyId, timeStep, GetBodiesAlongTime ());
    }
}

void FoamAlongTime::calculateRange ()
{
    for (size_t timeStep = 0; timeStep < GetTimeSteps (); ++timeStep)
	calculateRange (timeStep);
}

void FoamAlongTime::calculateRange (size_t timeStep)
{
    boost::shared_ptr<const Foam> foam = GetFoam (timeStep);
    BOOST_FOREACH (boost::shared_ptr<const Body> body, foam->GetBodies ())
    {
	size_t bodyId = body->GetId ();
	for (size_t i = BodyProperty::ENUM_BEGIN; i < BodyProperty::COUNT; ++i)
	{
	    BodyProperty::Enum bodyProperty = 
		BodyProperty::FromSizeT(i);
	    if (ExistsBodyProperty (bodyProperty, bodyId, timeStep))
		m_foamsStatistics[timeStep].RangeStep (
		    bodyProperty,
		    GetBodyProperty (bodyProperty, bodyId, timeStep));
	}
    }
}

const Body& FoamAlongTime::GetBody (size_t bodyId, size_t timeStep) const
{
    const BodyAlongTime& bat = GetBodiesAlongTime ().GetBodyAlongTime (bodyId);
    return *bat.GetBody (timeStep);
}

string FoamAlongTime::ToString () const
{
    ostringstream ostr;
    ostr << "FoamAlongTime: " << endl;
    ostr << m_AABox << endl;
    ostream_iterator< boost::shared_ptr<const Foam> > output (ostr, "\n");
    copy (m_foams.begin (), m_foams.end (), output);
    ostr << m_bodiesAlongTime;
    return ostr.str ();
}

void FoamAlongTime::SetTimeSteps (size_t timeSteps)
{
    m_foams.resize (timeSteps);
    m_foamsStatistics.resize (timeSteps);
}


void FoamAlongTime::GetTimeStepSelection (
    BodyProperty::Enum bodyProperty,
    const vector<QwtDoubleInterval>& valueIntervals,
    vector<bool>* timeStepSelection) const
{
    timeStepSelection->resize (GetTimeSteps ());
    fill (timeStepSelection->begin (), timeStepSelection->end (), false);
    BOOST_FOREACH (QwtDoubleInterval valueInterval, valueIntervals)
	GetTimeStepSelection (bodyProperty, valueInterval, timeStepSelection);
}

void FoamAlongTime::GetTimeStepSelection (
    BodyProperty::Enum bodyProperty,
    const QwtDoubleInterval& valueInterval,
    vector<bool>* timeStepSelection) const
{
    const size_t INVALID = numeric_limits<size_t> ().max ();
    size_t beginRange = INVALID;
    for (size_t timeStep = 0; timeStep < GetTimeSteps (); ++timeStep)
	if (isIntersection (
		valueInterval.minValue (), valueInterval.maxValue (),
		GetMin (bodyProperty, timeStep), 
		GetMax (bodyProperty, timeStep)))
	{
	    if (beginRange == INVALID)
		beginRange = timeStep;
	}
	else
	{
	    if (beginRange != INVALID)
	    {
		fill (timeStepSelection->begin () + beginRange, 
		      timeStepSelection->begin () + timeStep, true);
		beginRange = INVALID;				  
	    }
	}
    if (beginRange != INVALID)
	fill (timeStepSelection->begin () + beginRange, 
	      timeStepSelection->begin () + GetTimeSteps (), true);
}

void FoamAlongTime::calculateMaxCountPerBin ()
{
    for (size_t timeStep = 0; timeStep < GetTimeSteps (); ++timeStep)
	m_foamsStatistics[timeStep].CalculateMaxCountPerBin ();


    for (size_t bodyProperty = BodyProperty::ENUM_BEGIN;
	 bodyProperty < BodyProperty::COUNT; ++bodyProperty)
    {
	size_t maxCount = 0;
	for (size_t timeStep = 0; timeStep < GetTimeSteps (); ++timeStep)
	{
	    maxCount = max (
		maxCount, 
		m_foamsStatistics[timeStep].GetMaxCountPerBin (bodyProperty));
	}
	m_maxCountPerBinIndividual[bodyProperty] = maxCount;
    }
}
