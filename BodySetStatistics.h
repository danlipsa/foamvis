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
    float GetMax (size_t bodyProperty) const
    {
	return m_max[bodyProperty];
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
	const FoamAlongTime& foamAlongTime, size_t bodyId, size_t timeStep);

    void RangeStep (size_t bodyProperty, float newValue);
    void MinStep (size_t bodyProperty, float newValue)
    {
	m_min[bodyProperty] = min (m_min[bodyProperty], newValue);
    }
    void MaxStep (size_t bodyProperty, float newValue)
    {
	m_max[bodyProperty] = max (m_max[bodyProperty], newValue);
    }
    
private:
    /**
     * Increment the correct bin for 'bodyProperty' (@see
     * BodyProperty::Enum) and 'value'.
     */
    void valuePerInterval (size_t bodyProperty, float value);

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
};


#endif //__BODY_SET_STATISTICS_H__

// Local Variables:
// mode: c++
// End:
