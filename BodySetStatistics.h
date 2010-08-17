/**
 * @file   BodySetStatistics.h
 * @author Dan R. Lipsa
 * @date 15 May 2010
 *
 * Declaration for BodySetStatistics
 */

#ifndef __BODY_SET_STATISTICS_H__
#define __BODY_SET_STATISTICS_H__

class Body;
#include "StripIterator.h"

class BodySetStatistics
{
public:
    BodySetStatistics ();
    float GetMin (size_t bodyProperty) const
    {
	return m_min[bodyProperty];
    }
    const vector<float>& GetMin() const
    {
	return m_min;
    }
    vector<float>& GetMin()
    {
	return m_min;
    }
    float GetMax (size_t bodyProperty) const
    {
	return m_max[bodyProperty];
    }
    const vector<float>& GetMax () const
    {
	return m_max;
    }
    vector<float>& GetMax ()
    {
	return m_max;
    }
    size_t Size () const
    {
	return m_min.size ();
    }
    size_t GetValuesPerBin (size_t bodyProperty, size_t bin) const
    {
	return m_histogram[bodyProperty][bin];
    }

    void SpeedHistogramStep (const StripIterator::Point& p,
			     const StripIterator::Point& prev);
    void HistogramStep (const boost::shared_ptr<Body>& body);
    void HistogramStep (
	const FoamAlongTime& foamAlongTime, size_t bodyId, size_t timeStep,
	const BodySetStatistics& rangeStatistics);

    void RangeStep (size_t bodyProperty, float newValue);
    void MinStep (size_t bodyProperty, float newValue)
    {
	m_min[bodyProperty] = min (m_min[bodyProperty], newValue);
    }
    void MaxStep (size_t bodyProperty, float newValue)
    {
	m_max[bodyProperty] = max (m_max[bodyProperty], newValue);
    }
    QwtIntervalData GetHistogram (
	size_t bodyProperty, const BodySetStatistics* rangeStatistics = 0) const;
    void CalculateMaxCountPerBin ();
    size_t GetMaxCountPerBin (size_t bodyProperty)
    {
	return m_maxCountPerBin[bodyProperty];
    }

public:
    static size_t GetBin (float value, size_t binCount,
			  float beginInterval, float endInterval);
    
private:
    /**
     * Increment the correct bin for 'bodyProperty' (@see
     * BodyProperty::Enum) and 'value'.
     */
    void valuePerInterval (size_t bodyProperty, float value,
			   float beginInterval, float endInterval);

private:
    /**
     * Min speed along X, Y, Z, min total speed, min pressure
     */
    vector<float> m_min;
    /**
     * Max speed along X, Y, Z, max total speed, max pressure
     */
    vector<float> m_max;
    /**
     * Divide the value range in HISTOGRAM_INTERVALS intervals.
     * This array tells us how many values you have in each interval for
     * speed along x, y, z, total speed and pressure
     */
    vector< vector <size_t> > m_histogram;
    /**
     * Each element of the array corresponds to a histogram for a property.
     * It tells us, the maximum number of values we have in a single bin.
     */
    vector<size_t> m_maxCountPerBin;
};


#endif //__BODY_SET_STATISTICS_H__

// Local Variables:
// mode: c++
// End:
