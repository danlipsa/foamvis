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
    struct StripPoint
    {
	StripPoint (G3D::Vector3 point, bool newStrip):
	    m_point (point), m_newStrip (newStrip)
	{
	}
	
	G3D::Vector3 m_point;
	/**
	 * A strip ends and a new one begins. This variable is false for
	 * the first vertex in the first strip.
	 */
	bool m_newStrip;
    };

public:
    StripIterator (const BodyAlongTime& bodyAlongTime,
		   const FoamAlongTime& foamAlongTime) : 
	m_timeStep (0), m_currentWrap (0), m_newStrip (false),
	m_bodyAlongTime (bodyAlongTime), m_foamAlongTime (foamAlongTime)
    {
    }
    bool HasNext () const;
    StripPoint Next ();

private:
    size_t m_timeStep;
    /*
     * Index into the vector of wraps pointing to the current wrap or
     * to the end of the vector.
     */
    size_t m_currentWrap;
    /**
     * A strip ends and a new one begins.
     */
    bool m_newStrip;
    const BodyAlongTime& m_bodyAlongTime;
    const FoamAlongTime& m_foamAlongTime;
};


#endif //__STRIP_ITERATOR_H__

// Local Variables:
// mode: c++
// End:
