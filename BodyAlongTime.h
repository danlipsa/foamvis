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

class BodyAlongTime
{
public:
    typedef vector<boost::shared_ptr<Body> > Bodies;
    typedef vector<size_t> Wraps;
    typedef vector<G3D::Vector3int16> Translations;

public:
    BodyAlongTime (size_t timeSteps) :
	m_bodyAlongTime (timeSteps)
    {}
    boost::shared_ptr<Body>& GetBody (size_t timeStep)
    {
	return m_bodyAlongTime[timeStep];
    }
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
    void CalculateSpeedRange (const FoamAlongTime& foamAlongTime);

    StripIterator GetStripIterator (
	CenterPathColorBy::Object colorBy,
	const FoamAlongTime& foamAlongTime) const
    {
	return StripIterator (colorBy, *this, foamAlongTime);
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
    float GetMinSpeed (VectorMeasure::Type i) const
    {
	return m_minSpeed[i];
    }
    float GetMaxSpeed (VectorMeasure::Type i) const
    {
	return m_maxSpeed[i];
    }

public:
    friend ostream& operator<< (
	ostream& ostr, const BodyAlongTime& bodyAlongTime);

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
    /**
     * Min speed along X, Y, Z and total
     */
    boost::array<float, 4> m_minSpeed;
    /**
     * Max speed along X, Y, Z and total
     */
    boost::array<float, 4> m_maxSpeed;
};


class BodiesAlongTime
{
public:
    typedef map <size_t, boost::shared_ptr<BodyAlongTime> > BodyMap;

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
    void CalculateSpeedRange (const FoamAlongTime& foamAlongTime);
    float GetMinSpeed (VectorMeasure::Type i) const
    {
	return m_minSpeed[i];
    }
    float GetMaxSpeed (VectorMeasure::Type i) const
    {
	return m_maxSpeed[i];
    }
    string ToString () const;

private:
    void resize (size_t bodyOriginalIndex, size_t timeSteps);
    BodyAlongTime& getBodyAlongTime (size_t id) const;

private:
    /**
     * Map between the original index of the body and the body along time
     */
    BodyMap m_bodyMap;
    /**
     * Min speed along X, Y, Z and total
     */
    boost::array<float, 4> m_minSpeed;
    /**
     * Max speed along X, Y, Z and total
     */
    boost::array<float, 4> m_maxSpeed;
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
