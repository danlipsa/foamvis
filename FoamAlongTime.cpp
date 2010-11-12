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


// Members
// ======================================================================
FoamAlongTime::FoamAlongTime () :
    m_maxCountPerBinIndividual (BodyProperty::COUNT)
{
}

void FoamAlongTime::calculateAggregate (
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
    G3D::Vector3 low, high;
    calculateAggregate (min_element, &Foam::GetBoundingBoxLow, low);
    calculateAggregate (max_element, &Foam::GetBoundingBoxHigh, high);
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
    cdbg << "Calculating overall statistics..." << endl;
    GetBodiesAlongTime ().CalculateOverallRange (*this);
    GetBodiesAlongTime ().CalculateOverallHistogram (*this);
    GetBodiesAlongTime ().CalculateMaxCountPerBin ();

    cdbg << "Calculating per time step statistics..." << endl;
    calculatePerTimeStepRanges ();
    calculatePerTimeStepHistograms ();
    calculatePerTimeStepMaxCountPerBin ();
    // adjusting pressure
    // calculatePerTimeStepMedians();
    // recalculate overall stuff for pressure and per step stuff for pressure
    // GetBodiesAlongTime ().RecalculateOverallRangePressure ();
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

double FoamAlongTime::GetBodyProperty (
    BodyProperty::Enum property,
    size_t bodyId, size_t timeStep) const
{
    double value = getBodyPropertyNoAdjustment (property, bodyId, timeStep);
    if (property == BodyProperty::PRESSURE)
	value -= m_foamsStatistics[property].GetMedian (property);
    return value;
}



double FoamAlongTime::getBodyPropertyNoAdjustment (
    BodyProperty::Enum property,
    size_t bodyId, size_t timeStep) const
{
    const BodyAlongTime& bat = GetBodiesAlongTime ().GetBodyAlongTime (bodyId);
    double value;
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
	value = StripIterator::GetPropertyValue (property, next, p);
    }
    else
	value = bat.GetBody (timeStep)->GetPropertyValue (property);
    RuntimeAssert (
	value >= GetMin (property) && value <= GetMax (property),
	"Value outside range of permited values for bodyId: ", bodyId, 
	" timeStep: ", timeStep);
    return value;
}


size_t FoamAlongTime::GetDimension () const
{
    return GetFoam (0)->GetDimension ();
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


void FoamAlongTime::calculatePerTimeStepHistograms ()
{
    for (size_t timeStep = 0; timeStep < GetTimeSteps (); ++timeStep)
	calculatePerTimeStepHistogram (timeStep);
}

void FoamAlongTime::calculatePerTimeStepHistogram (size_t timeStep)
{
    boost::shared_ptr<const Foam> foam = GetFoam (timeStep);
    BOOST_FOREACH (boost::shared_ptr<const Body> body, foam->GetBodies ())
    {
	size_t bodyId = body->GetId ();
	m_foamsStatistics[timeStep].HistogramStep (
	    *this, bodyId, timeStep, GetBodiesAlongTime ());
    }
}

void FoamAlongTime::calculatePerTimeStepMedians ()
{
    for (size_t timeStep = 0; timeStep < GetTimeSteps (); ++timeStep)
	m_foamsStatistics[timeStep].ApproximateMedian ();
}



void FoamAlongTime::calculatePerTimeStepRanges ()
{
    for (size_t timeStep = 0; timeStep < GetTimeSteps (); ++timeStep)
	calculatePerTimeStepRange (timeStep);
}

void FoamAlongTime::calculatePerTimeStepRange (size_t timeStep)
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
    m_foamsStatistics[timeStep].NormalizeEmptyRange ();
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
	if (valueInterval.intersects (GetRange (bodyProperty, timeStep)))
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

void FoamAlongTime::calculatePerTimeStepMaxCountPerBin ()
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

bool FoamAlongTime::IsQuadratic () const
{
    return m_foams[0]->IsQuadratic ();
}
