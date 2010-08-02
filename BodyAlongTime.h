/**
 * @file   BodyAlongTime.h
 * @author Dan R. Lipsa
 * @date 15 May 2010
 *
 * Declaration for bodies along time classes
 */

#ifndef __BODY_ALONG_TIME_H__
#define __BODY_ALONG_TIME_H__

class Body;
class FoamAlongTime;
#include "StripIterator.h"
#include "Enums.h"

class BodyAlongTimeStatistics
{
public:
    BodyAlongTimeStatistics ();
    float GetMin (size_t i) const
    {
	return m_min[i];
    }
    float GetMax (size_t i) const
    {
	return m_max[i];
    }
    size_t GetValuesPerBin (size_t component, size_t bin) const
    {
	return m_histogram[component][bin];
    }

protected:
    void speedValuesPerInterval (const StripIterator::StripPoint& p,
				     const StripIterator::StripPoint& prev);
    void valuesPerInterval (const boost::shared_ptr<Body>& body);
    /**
     * Increment the correct bin for 'index' (@see CenterPathColor::Enum) and
     * 'value'.
     */
    void valuePerInterval (size_t index, float value);



protected:
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


class BodyAlongTime : public BodyAlongTimeStatistics
{
public:
    typedef vector<boost::shared_ptr<Body> > Bodies;
    typedef vector<size_t> Wraps;
    typedef vector<G3D::Vector3int16> Translations;

public:
    BodyAlongTime (size_t timeSteps);

    boost::shared_ptr<Body>& GetBody (size_t timeStep)
    {
	return m_bodyAlongTime[timeStep];
    }
    size_t GetId () const;
    const boost::shared_ptr<Body>& GetBody (size_t timeStep) const
    {
	return m_bodyAlongTime[timeStep];
    }

    void Resize (size_t timeSteps)
    {
	m_bodyAlongTime.resize (timeSteps);
    }
    const Bodies& GetBodies () const
    {
	return m_bodyAlongTime;
    }
    /**
     * Calculates when from one time step to another a body is wrapped around
     * the original domain. A body wraps around when it moves a distance longer
     * than 1/2 of min all sides of the original domain
     */
    void CalculateBodyWraps (const FoamAlongTime& foamAlongTime);
    void CalculateValueRange (const FoamAlongTime& foamAlongTime);
    void CalculateHistogram (
	const FoamAlongTime& foamAlongTime,
	BodyAlongTimeStatistics* destination);

    StripIterator GetStripIterator (
	const FoamAlongTime& foamAlongTime) const
    {
	return StripIterator (*this, foamAlongTime);
    }

    size_t GetWrapSize () const
    {
	return m_wraps.size ();
    }
    size_t GetWrap (size_t i) const
    {
	return m_wraps[i];
    }

    void Resize ();
    G3D::Vector3int16 GetTranslation (size_t i) const
    {
	return m_translations[i];
    }
    string ToString () const;

public:
    friend ostream& operator<< (
	ostream& ostr, const BodyAlongTime& bodyAlongTime);

private:
    void speedRangeStep (const StripIterator::StripPoint& p,
			 const StripIterator::StripPoint& prev);
    void rangeStep (const boost::shared_ptr<Body>& body);

private:
    Bodies m_bodyAlongTime;
    /**
     * List of times (indexes in Bodies vector) where a body wraps
     * around the torus original domain. The wrap is 
     * between index and index + 1.
     */
    Wraps m_wraps;
    /**
     * Translation between step index and step index + 1.
     * @see Wraps
     */
    Translations m_translations;
};

class BodiesAlongTime : public BodyAlongTimeStatistics
{
public:
    typedef map <size_t, boost::shared_ptr<BodyAlongTime> > BodyMap;

public:
    BodiesAlongTime ();

    size_t GetBodyCount ()
    {
	return m_bodyMap.size ();
    }

    void Allocate (const boost::shared_ptr<Body>  body, size_t timeSteps);
    void Cache (boost::shared_ptr<Body>  body, size_t timeStep);
    BodyMap& GetBodyMap ()
    {
	return m_bodyMap;
    }
    const BodyMap& GetBodyMap () const
    {
	return m_bodyMap;
    }
    void Resize (const boost::shared_ptr<Body>  body);
    BodyAlongTime& GetBodyAlongTime (size_t id)
    {
	return getBodyAlongTime (id);
    }

    const BodyAlongTime& GetBodyAlongTime (size_t id) const
    {
	return getBodyAlongTime (id);
    }
    void CalculateValueRange (const FoamAlongTime& foamAlongTime);
    void CalculateHistogram (
	const FoamAlongTime& foamAlongTime);
    string ToString () const;
    QwtIntervalData GetHistogram (size_t i) const;

private:
    void resize (size_t bodyOriginalIndex, size_t timeSteps);
    BodyAlongTime& getBodyAlongTime (size_t id) const;

private:
    /**
     * Map between the original index of the body and the body along time
     */
    BodyMap m_bodyMap;
};

inline ostream& operator<< (ostream& ostr, const BodyAlongTime& bat)
{
    return ostr << bat.ToString ();
}

inline ostream& operator<< (
    ostream& ostr, const BodiesAlongTime& bodiesAlongTime)
{
    return ostr << bodiesAlongTime.ToString ();
}


#endif //__BODY_ALONG_TIME_H__

// Local Variables:
// mode: c++
// End:
