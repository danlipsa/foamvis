/**
 * @file   BodyAlongTime.h
 * @author Dan R. Lipsa
 * @date 15 May 2010
 * @ingroup data model
 * @brief A bubble path
 */

#ifndef __BODY_ALONG_TIME_H__
#define __BODY_ALONG_TIME_H__

#include "StripIterator.h"
#include "Enums.h"

class Body;
class Simulation;

/**
 * @brief A bubble path
 */
class BodyAlongTime
{
public:
    typedef vector<boost::shared_ptr<Body> > Bodies;
    typedef vector<size_t> Wraps;
    typedef vector<G3D::Vector3int16> Translations;

public:
    BodyAlongTime (size_t timeSteps);

    size_t GetId () const;
    const boost::shared_ptr<Body>& GetBody (size_t timeStep) const
    {
	return m_bodyAlongTime[timeStep];
    }
    void SetBody (size_t timeStep, const boost::shared_ptr<Body>& body);
    size_t GetTimeBegin () const
    {
	return m_timeBegin;
    }
    size_t GetTimeEnd () const
    {
	return m_timeEnd;
    }
    /**
     * Calculates when from one time step to another a body is wrapped around
     * the original domain. A body wraps around when it moves a distance longer
     * than 1/2 of min all sides of the original domain
     */
    void CalculateBodyWraps (const Simulation& simulation);

    StripIterator GetStripIterator (const Simulation& simulation) const
    {
	return StripIterator (*this, simulation);
    }

    size_t GetWrapSize () const
    {
	return m_wraps.size ();
    }
    size_t GetWrap (size_t i) const
    {
	return m_wraps[i];
    }

    G3D::Vector3int16 GetTranslation (size_t currentWrap) const
    {
	return m_translations[currentWrap];
    }
    string ToString () const;    
    void AssertDeadBubblesStayDead () const;

public:
    friend ostream& operator<< (
	ostream& ostr, const BodyAlongTime& bodyAlongTime);

private:
    Bodies m_bodyAlongTime;
    /**
     * A bubble can appear after time 0 and disappear before time n.
     * I assume bubble IDs are not reused. :-) That is, once a bubble has 
     * dissapeared, it cannot appear again.
     * A bubble exists between m_timeBeing <= time < m_timeEnd.
     */
    size_t m_timeBegin;
    size_t m_timeEnd;
    /**
     * List of times (indexes in Bodies vector) where a body wraps
     * around the torus original domain. The wrap is between index and
     * index + 1. If there are no wraps this vector has 0 length.
     */
    Wraps m_wraps;
    /**
     * Translation between step index and step index + 1.
     * @see Wraps
     */
    Translations m_translations;
};


/**
 * @brief A map between the bubble ID and the bubble path
 */
class BodiesAlongTime
{
public:
    typedef map <size_t, boost::shared_ptr<BodyAlongTime> > BodyMap;

public:
    BodiesAlongTime ();

    size_t GetBodyCount ()
    {
	return m_bodyMap.size ();
    }

    void AllocateBodyAlongTime (size_t bodyId, size_t timeSteps);
    void CacheBody (
	boost::shared_ptr<Body> body, size_t timeStep, size_t timeSteps);
    BodyMap& GetBodyMap ()
    {
	return m_bodyMap;
    }
    const BodyMap& GetBodyMap () const
    {
	return m_bodyMap;
    }
    BodyAlongTime& GetBodyAlongTime (size_t id)
    {
	return getBodyAlongTime (id);
    }

    const BodyAlongTime& GetBodyAlongTime (size_t id) const
    {
	return getBodyAlongTime (id);
    }

    string ToString () const;
    void AssertDeadBubblesStayDead () const;

private:
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
