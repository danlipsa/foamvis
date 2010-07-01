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


class BodyAlongTime
{
public:
    typedef vector<boost::shared_ptr<Body> > Bodies;
    /**
     * List of times (indexes in Bodies vector) where a body wraps
     * around the torus original domain. The wrap is 
     * between index and index + 1. It includes the index equal with the 
     * number of time steps + 1.
     */
    typedef vector<size_t> Wraps;

public:
    BodyAlongTime (size_t timeSteps) :
	m_bodyAlongTime (timeSteps)
    {}
    boost::shared_ptr<Body> & GetBody (size_t timeStep)
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
    const Wraps& GetWraps ()
    {
	return m_wraps;
    }
    void Resize ();

public:
    friend ostream& operator<< (
	ostream& ostr, const BodyAlongTime& bodyAlongTime);

private:
    Bodies m_bodyAlongTime;
    Wraps m_wraps;
};


class BodiesAlongTime
{
public:
    typedef BodyAlongTime OneBody;
    typedef boost::shared_ptr<OneBody> OneBodyPtr;
    typedef map <size_t, OneBodyPtr> BodyMap;

    size_t GetBodyCount ()
    {
	return m_bodyMap.size ();
    }

    void Allocate (const boost::shared_ptr<Body>  body, size_t timeSteps);
    void Cache (boost::shared_ptr<Body>  body, size_t timeStep);
    OneBody& GetOneBody (size_t id);
    BodyMap& GetBodyMap ()
    {
	return m_bodyMap;
    }
    void Resize (const boost::shared_ptr<Body>  body);

public:
    friend ostream& operator<< (
	ostream& ostr, const BodiesAlongTime& bodiesAlongTime);

private:
    void resize (size_t bodyOriginalIndex, size_t timeSteps);

private:
    /**
     * Map between the original index of the body and the body along time
     */
    BodyMap m_bodyMap;
};


#endif //__BODY_ALONG_TIME_H__

// Local Variables:
// mode: c++
// End:
