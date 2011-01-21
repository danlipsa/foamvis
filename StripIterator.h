/**
 * @file   BodyAlongTime.h
 * @author Dan R. Lipsa
 * @date 16 July 2010
 *
 * StripIterator declaration
 */

#ifndef __STRIP_ITERATOR_H__
#define __STRIP_ITERATOR_H__

#include "Enums.h"
class Body;
class BodyAlongTime;
class FoamAlongTime;

struct StripIteratorPoint
{
    StripIteratorPoint () :
	m_location (StripPointLocation::COUNT), m_timeStep (0)
    {
    }

    StripIteratorPoint (
	const G3D::Vector3& point, 
	StripPointLocation::Enum location, size_t timeStep,
	const boost::shared_ptr<Body>& body) :

	m_location (location), m_timeStep (timeStep),
	m_body (body), m_point (point)
    {
    }

    bool IsEmpty () const
    {
	return m_location == StripPointLocation::COUNT;
    }

    StripPointLocation::Enum m_location;
    size_t m_timeStep;
    boost::shared_ptr<Body> m_body;
    G3D::Vector3 m_point;	
};




/**
 * Iterates over line segments for centers of bubbles along time
 * @todo Consider using a Boost.Iterator Library
 * @see BodyAlongTime
 */
class StripIterator
{
public:

public:
    StripIterator (const BodyAlongTime& bodyAlongTime,
		   const FoamAlongTime& foamAlongTime, size_t timeStep = 0);
    bool HasNext () const;
    StripIteratorPoint Next ();    

    template <typename ProcessSegment> 
    void ForEachSegment (ProcessSegment processSegment);

public:
    static SegmentPerpendicularEnd::Enum GetSegmentPerpendicularEnd (
	const StripIteratorPoint& begin, const StripIteratorPoint& end);

private:
    size_t m_timeStep;
    /*
     * Index into the vector of wraps pointing to the current wrap or
     * to the end of the vector.
     */
    size_t m_currentWrap;
    /*
     * Next () will return the first point from a strip
     */
    bool m_isNextBeginOfStrip;
    const BodyAlongTime& m_bodyAlongTime;
    const FoamAlongTime& m_foamAlongTime;
};


#endif //__STRIP_ITERATOR_H__

// Local Variables:
// mode: c++
// End:
