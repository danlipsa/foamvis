/**
 * @file   BodyAlongTime.h
 * @author Dan R. Lipsa
 * @date 16 July 2010
 *
 * StripIterator declaration
 */

#ifndef __STRIP_ITERATOR_H__
#define __STRIP_ITERATOR_H__

/**
 * Iterates over line segments for centers of bubbles along time
 * @see BodyAlongTime
 */

class BodyAlongTime;
class FoamAlongTime;

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
    struct StripPoint
    {
	StripPoint () :
	    m_location (COUNT)
	{
	}

	StripPoint (G3D::Vector3 point, Location location,
		    const boost::shared_ptr<Body>& body) :
	    m_point (point), m_location (location), m_body (body)
	{
	}
	
	G3D::Vector3 m_point;
	Location m_location;
	boost::shared_ptr<Body> m_body;
    };

public:
    StripIterator (const BodyAlongTime& bodyAlongTime,
		   const FoamAlongTime& foamAlongTime) : 
	m_timeStep (0), m_currentWrap (0), m_isNextBeginOfStrip (true),
	m_bodyAlongTime (bodyAlongTime), m_foamAlongTime (foamAlongTime)
	
    {
    }
    bool HasNext () const;
    StripPoint Next ();    
    template <typename ProcessSegment> 
    void ForEachSegment (ProcessSegment processSegment)
    {
	StripIterator::StripPoint prev = Next ();
	while (HasNext ())
	{
	    StripIterator::StripPoint p = Next ();
	    if (// middle or end of a segment
		p.m_location != StripIterator::BEGIN &&
		// the segment is not between two strips
		prev.m_location != StripIterator::END)
		processSegment (p, prev);
	    prev = p;
	}
    }
public:
    static float GetColorByValue (CenterPathColor::Enum colorBy,
				  const StripPoint& p, const StripPoint& prev);
    static float GetColorByValue (CenterPathColor::Enum colorBy,
				  const StripPoint& p);
    static bool ExistsColorByValue (CenterPathColor::Enum colorBy,
				    const StripPoint& p);

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
