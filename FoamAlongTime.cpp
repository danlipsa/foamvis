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

// version 1668 reverts to by hand calculation of statistics

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
    m_maxCountPerBinIndividual (BodyProperty::PROPERTY_END),
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
    CalculateAABox ();
    CacheBodiesAlongTime ();
    calculateBodyWraps ();
    calculateVelocity ();
    calculateStatistics ();
    calculateStatisticsOld ();

    // compare the two statistics methods
    const size_t timeStep = 9;
    const size_t bin = 255;
    for (size_t i = BodyProperty::PROPERTY_BEGIN; 
	 i < BodyProperty::PROPERTY_END; ++i)
    {
	BodyProperty::Enum property = BodyProperty::FromSizeT (i);
	
	cdbg << "Property: " << BodyProperty::ToString (property) << endl
	     << "MinMax global" << endl	     
	     << "Accumulators" << " "
	     << acc::min (m_histogram[property]) << ", "
	     << acc::max (m_histogram[property]) << endl
	     << "By hand" << " "
	     << GetMin (property) << ", " << GetMax (property) << endl;
	cdbg << "========================================" << endl;
	const MinMaxStatistics& minMax = 
	    GetFoam (timeStep)->GetMinMax (property);
	cdbg << "MinMax timestep: " << timeStep << endl
	     << "Accumulators" << " "
	     << acc::min (minMax) << ", "
	     << acc::max (minMax) << endl
	     << "By hand" << " "
	     << GetMin (property, timeStep) << ", " 
	     << GetMax (property, timeStep) << endl;
	cdbg << "========================================" << endl;
	HistogramStatistics statistics = m_histogram[property];
	HistogramStatistics::Result histogram = 
	    acc::density (statistics);
	HistogramStatistics::Result::iterator it = (histogram.begin () + bin + 1);
	QwtIntervalData byHand = GetHistogram (property);
	cdbg << "Histogram global, bin: " << bin << endl
	     << "Accumulators" << " "
	     << (*it).first << ", "
	     << statistics.GetValuesPerBin (bin) << endl
	     << "By hand" << " "
	     << byHand.interval (bin).minValue () << ", " 
	     << byHand.value (bin) << endl;
	cdbg << "========================================" << endl;
	statistics = GetFoam (timeStep)->GetHistogram (property);
	histogram = acc::density (statistics);
	it = (histogram.begin () + bin + 1);
	byHand = GetHistogram (property, timeStep);
	cdbg << "Histogram timestep: " << timeStep << ", bin: " << bin << endl
	     << "Accumulators" << " "
	     << (*it).first << ", "
	     << statistics.GetValuesPerBin (bin) << endl
	     << "By hand" << " "
	     << byHand.interval (bin).minValue () << ", " 
	     << byHand.value (bin) << endl << endl;
    }
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
	BOOST_FOREACH (const boost::shared_ptr<Foam>& foam, m_foams)
	{
	    BodyProperty::Enum property = BodyProperty::FromSizeT (i);
	    double min = acc::min(m_histogram[property]);
	    double max = acc::max(m_histogram[property]);
	    foam->CalculateStatistics (property, min, max);
	}
    }
}


void FoamAlongTime::calculateStatisticsOld ()
{
    cdbg << "Calculating overall statistics..." << endl;
    GetBodiesAlongTime ().CalculateOverallRange (*this);
    GetBodiesAlongTime ().CalculateOverallHistogram (*this);
    GetBodiesAlongTime ().CalculateMaxCountPerBin ();

    cdbg << "Calculating per time step statistics..." << endl;
    calculatePerTimeStepRanges ();
    calculatePerTimeStepHistograms ();
    calculatePerTimeStepMaxCountPerBin ();
}



template <typename Accumulator>
void FoamAlongTime::forAllBodiesAccumulate (
    Accumulator* acc, BodyProperty::Enum property)
{
    BOOST_FOREACH (const boost::shared_ptr<Foam>& foam, m_foams)
    {
	BOOST_FOREACH (
	    const boost::shared_ptr<Body>& body, foam->GetBodies ())
	{
	    if (body->ExistsPropertyValue (property))
		(*acc) (body->GetPropertyValue (property));
	}
    }
}



void FoamAlongTime::calculateVelocity ()
{
    BodiesAlongTime::BodyMap& map = GetBodiesAlongTime ().GetBodyMap ();
    BodiesAlongTime::BodyMap::iterator it;
    for (it = map.begin (); it != map.end (); ++it)
    {
	const BodyAlongTime& bat = *(*it).second;
	StripIterator stripIt = bat.GetStripIterator (*this);
	stripIt.ForEachSegment (boost::bind (&FoamAlongTime::storeVelocity,
					     this, _1, _2, _3, _4));
    }
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

void FoamAlongTime::initializeStatistics ()
{
    for (size_t timeStep = 0; timeStep < GetTimeSteps (); ++timeStep)
	m_foamsStatistics[timeStep].Initialize ();
    GetBodiesAlongTime ().Initialize ();
    BodiesAlongTime::BodyMap bodyMap = GetBodiesAlongTime ().GetBodyMap ();
    pair <size_t, boost::shared_ptr<BodyAlongTime> > idBody;
    BOOST_FOREACH (idBody, bodyMap)
	idBody.second->Initialize ();
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
    {
	m_foamsStatistics[timeStep].ApproximateMedian ();
	cdbg << "median pressure timestep : " << timeStep << ": "
	     << m_foamsStatistics[timeStep].
	    GetMedian (BodyProperty::PRESSURE) << endl;
    }
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
	for (size_t i = BodyProperty::PROPERTY_BEGIN; 
	     i < BodyProperty::PROPERTY_END; ++i)
	{
	    BodyProperty::Enum bodyProperty = 
		BodyProperty::FromSizeT(i);
	    if (ExistsBodyProperty (bodyProperty, bodyId, timeStep))
		m_foamsStatistics[timeStep].RangeStep (
		    bodyProperty,
		    GetBodyPropertyValue (bodyProperty, bodyId, timeStep));
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


    for (size_t bodyProperty = BodyProperty::PROPERTY_BEGIN;
	 bodyProperty < BodyProperty::PROPERTY_END; ++bodyProperty)
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

QwtIntervalData FoamAlongTime::GetHistogram (size_t bodyProperty) const
{
    return GetBodiesAlongTime ().GetHistogram (bodyProperty);
    //return m_histogram[bodyProperty].ToQwtIntervalData ();
}
