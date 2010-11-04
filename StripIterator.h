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
class BodyAlongTime;
class FoamAlongTime;

/**
 * Iterates over line segments for centers of bubbles along time
 * @todo Consider using a Boost.Iterator Library
 * @see BodyAlongTime
 */
class StripIterator
{
public:
    /**
     * Location of a point in a strip of segments.
     */
    enum Location
    {
	BEGIN,
	END,
	MIDDLE,
	COUNT
    };
    struct Point
    {
	Point () :
	    m_location (COUNT), m_timeStep (0)
	{
	}

	Point (G3D::Vector3 point, Location location, size_t timeStep,
		    const boost::shared_ptr<Body>& body) :
	    m_location (location), m_timeStep (timeStep),
	    m_body (body), m_point (point)
	{
	}
	
	Location m_location;
	size_t m_timeStep;
	boost::shared_ptr<Body> m_body;
	G3D::Vector3 m_point;	
    };

public:
    StripIterator (const BodyAlongTime& bodyAlongTime,
		   const FoamAlongTime& foamAlongTime, size_t timeStep = 0);
    bool HasNext () const;
    Point Next ();    
    template <typename ProcessSegment> 
    void ForEachSegment (ProcessSegment processSegment)
    {
	StripIterator::Point prev = Next ();
	while (HasNext ())
	{
	    StripIterator::Point p = Next ();
	    if (// middle or end of a segment
		p.m_location != StripIterator::BEGIN &&
		// the segment is not between two strips
		prev.m_location != StripIterator::END)
		processSegment (p, prev);
	    prev = p;
	}
    }
public:
    static double GetPropertyValue (BodyProperty::Enum colorBy,
				    const Point& p, const Point& prev);
    static double GetPropertyValue (BodyProperty::Enum colorBy,
				    const Point& p);
    static bool ExistsPropertyValue (BodyProperty::Enum colorBy,
				     const Point& p);

private:
    size_t m_timeStep;
    /*
     * Index into the vector of wraps pointing to the current wrap or
     * to the end of the vector.
     */
    size_t m_currentWrap;
    bool m_isNextBeginOfStrip;
    const BodyAlongTime& m_bodyAlongTime;
    const FoamAlongTime& m_foamAlongTime;
};


#endif //__STRIP_ITERATOR_H__

// Local Variables:
// mode: c++
// End:
