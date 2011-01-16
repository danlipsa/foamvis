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

// version 1669 reverts to by hand calculation of statistics

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
    m_histogram (
	BodyProperty::PROPERTY_END, HistogramStatistics (HISTOGRAM_INTERVALS))
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
/**
 * @todo Adjust pressures so that the median of each time step is 0.
 */
void FoamAlongTime::Preprocess ()
{
    cdbg << "Preprocess data ..." << endl;
    CalculateAABox ();
    CacheBodiesAlongTime ();
    calculateBodyWraps ();
    calculateVelocity ();
    calculateStatistics ();
}

void FoamAlongTime::calculateStatistics ()
{
    vector<MinMaxStatistics> minMaxStat(BodyProperty::PROPERTY_END);
    for (size_t i = BodyProperty::PROPERTY_BEGIN; 
	 i < BodyProperty::PROPERTY_END; ++i)
    {
	// statistics for all time-steps
	BodyProperty::Enum property = BodyProperty::FromSizeT (i);
	forAllBodiesAccumulate (&minMaxStat[property], property);
	m_histogram[property] (acc::min (minMaxStat[property]));
	m_histogram[property] (acc::max (minMaxStat[property]));
	forAllBodiesAccumulate (&m_histogram[property], property);


	// statistics per time-step
	double min = acc::min(m_histogram[property]);
	double max = acc::max(m_histogram[property]);
	QtConcurrent::blockingMap (
	    m_foams.begin (), m_foams.end (),
	    boost::bind (&Foam::CalculateHistogramStatistics, _1,
			 BodyProperty::FromSizeT (i), min, max));
    }
}

template <typename Accumulator>
void FoamAlongTime::forAllBodiesAccumulate (
    Accumulator* acc, BodyProperty::Enum property)
{
    QtConcurrent::blockingMap (
	m_foams.begin (), m_foams.end (),
	boost::bind (&Foam::Accumulate<Accumulator>, _1, acc, property));
}


void FoamAlongTime::calculateVelocityBody (
    pair< size_t, boost::shared_ptr<BodyAlongTime> > p)
{
    const BodyAlongTime& bat = *p.second;
    StripIterator stripIt = bat.GetStripIterator (*this);
    stripIt.ForEachSegment (boost::bind (&FoamAlongTime::storeVelocity,
					 this, _1, _2, _3, _4));    
}

void FoamAlongTime::calculateVelocity ()
{
    BodiesAlongTime::BodyMap& map = GetBodiesAlongTime ().GetBodyMap ();
    QtConcurrent::blockingMap (
	map.begin (), map.end (), 
	boost::bind (&FoamAlongTime::calculateVelocityBody, this, _1));
}

void FoamAlongTime::storeVelocity (
    const StripIterator::Point& beforeBegin,
    const StripIterator::Point& begin,
    const StripIterator::Point& end,
    const StripIterator::Point& afterEnd)
{
    (void)beforeBegin;
    (void)afterEnd;
    G3D::Vector3 velocity = end.m_point - begin.m_point;
    begin.m_body->SetVelocity (velocity);
    if (end.m_location == StripIterator::END)
	end.m_body->SetVelocity (velocity);
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

double FoamAlongTime::GetBodyPropertyValue (
    BodyProperty::Enum property,
    size_t bodyId, size_t timeStep) const
{
    return GetBody (bodyId, timeStep).GetPropertyValue (property);
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
}


void FoamAlongTime::GetTimeStepSelection (
    BodyProperty::Enum property,
    const vector<QwtDoubleInterval>& valueIntervals,
    vector<bool>* timeStepSelection) const
{
    timeStepSelection->resize (GetTimeSteps ());
    fill (timeStepSelection->begin (), timeStepSelection->end (), false);
    BOOST_FOREACH (QwtDoubleInterval valueInterval, valueIntervals)
	GetTimeStepSelection (property, valueInterval, timeStepSelection);
}

void FoamAlongTime::GetTimeStepSelection (
    BodyProperty::Enum property,
    const QwtDoubleInterval& valueInterval,
    vector<bool>* timeStepSelection) const
{
    const size_t INVALID = numeric_limits<size_t> ().max ();
    size_t beginRange = INVALID;
    for (size_t timeStep = 0; timeStep < GetTimeSteps (); ++timeStep)
	if (valueInterval.intersects (
		GetFoam (timeStep)->GetRange (property)))
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

bool FoamAlongTime::IsQuadratic () const
{
    return m_foams[0]->IsQuadratic ();
}

size_t FoamAlongTime::GetMaxCountPerBinIndividual (
    BodyProperty::Enum property) const
{
    size_t size = GetTimeSteps ();
    size_t max = 0;
    for (size_t i = 0; i < size; ++i)
	max = std::max (
	    max, GetFoam (i)->GetHistogram (property).GetMaxCountPerBin ());
    return max;
}
