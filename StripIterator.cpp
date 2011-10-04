/**
 * @file   StripIterator.cpp
 * @author Dan R. Lipsa
 * @date 16 July 2010
 *
 * Implementation of the StripIterator class
 */

#include "Body.h"
#include "BodyAlongTime.h"
#include "Debug.h"
#include "DisplayBodyFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "Foam.h"
#include "Simulation.h"
#include "OOBox.h"
#include "StripIterator.h"
#include "Utils.h"

StripIterator::StripIterator (
    const BodyAlongTime& bodyAlongTime,
    const Simulation& simulation, size_t timeStep) : 

    m_timeStep (timeStep),
    m_bodyAlongTime (bodyAlongTime),
    m_simulation (simulation)
{
    m_currentWrap = 0;
    size_t wrapSize = m_bodyAlongTime.GetWrapSize ();
    if (wrapSize == 0)
	m_isNextBeginOfStrip = (m_timeStep == 0);
    else
    {
	while (m_timeStep > m_bodyAlongTime.GetWrap (m_currentWrap) &&
	       m_currentWrap < wrapSize)
	    ++m_currentWrap;
	if (m_timeStep == 0 ||
	    m_timeStep == m_bodyAlongTime.GetWrap (m_currentWrap - 1) + 1)
	    m_isNextBeginOfStrip = true;
	else
	    m_isNextBeginOfStrip = false;
    }
}

template <typename ProcessSegment> 
void StripIterator::ForEachSegment (ProcessSegment processSegment)
{
    StripIteratorPoint beforeBegin;
    StripIteratorPoint begin = Next ();
    StripIteratorPoint end = HasNext () ? Next () : StripIteratorPoint ();
    while (end.m_location != StripPointLocation::COUNT)
    {
	StripIteratorPoint afterEnd = 
	    HasNext () ? Next () : StripIteratorPoint ();
	if (// middle or end of a segment
	    end.m_location != StripPointLocation::BEGIN &&
	    // the segment is not between two strips
	    begin.m_location != StripPointLocation::END)
	    processSegment (beforeBegin, begin, end, afterEnd);
	beforeBegin = begin;
	begin = end;
	end = afterEnd;
    }
}

bool StripIterator::HasNext () const
{
    return m_timeStep < m_bodyAlongTime.GetBodies ().size ();
}


StripIteratorPoint StripIterator::Next ()
{
    StripIteratorPoint point;
    boost::shared_ptr<Body> body;
    if (// last wrap
	m_currentWrap == m_bodyAlongTime.GetWrapSize () ||
	// not at the end of a middle wrap
	m_timeStep < m_bodyAlongTime.GetWrap (m_currentWrap) + 1)
    {
	StripPointLocation::Enum location;
	if (// at the end of last wrap
	    m_timeStep == m_bodyAlongTime.GetBodies ().size () - 1)
	{
	    location = StripPointLocation::END;
	    m_isNextBeginOfStrip = false;
	}
	else
	{   // not at the end of a middle wrap or last wrap
	    location = m_isNextBeginOfStrip ? 
		StripPointLocation::BEGIN : StripPointLocation::MIDDLE;
	    m_isNextBeginOfStrip = false;
	}
	body = m_bodyAlongTime.GetBody (m_timeStep);
	point = StripIteratorPoint (
	    body->GetCenter (), location, m_timeStep++, body);
    }
    else
    { // at the end of a middle wrap
	m_isNextBeginOfStrip = true;
	const OOBox& originalDomain = 
	    m_simulation.GetFoam (m_timeStep).GetOriginalDomain ();
	body = m_bodyAlongTime.GetBody (m_timeStep);
	point = StripIteratorPoint (
	    originalDomain.TorusTranslate (
		body->GetCenter (),
		Vector3int16Zero - m_bodyAlongTime.GetTranslation (
		    m_currentWrap++)), 
	    StripPointLocation::END, m_timeStep, body);
    }
    return point;
}

SegmentPerpendicularEnd::Enum StripIterator::GetSegmentPerpendicularEnd (
    const StripIteratorPoint& begin, const StripIteratorPoint& end)
{
    if (begin.m_location == StripPointLocation::BEGIN)
    {
	if (end.m_location == StripPointLocation::END)
	    return SegmentPerpendicularEnd::BEGIN_END;
	else
	    return SegmentPerpendicularEnd::BEGIN;
    }
    else if (end.m_location == StripPointLocation::END)
	return SegmentPerpendicularEnd::END;
    else
	return SegmentPerpendicularEnd::NONE;
}

// Template instantiations
// ======================================================================
/// @cond
template void StripIterator::ForEachSegment<boost::_bi::bind_t<void, boost::_mfi::mf4<void, DisplayCenterPath<SetterTextureCoordinate, DisplaySegmentQuadric>, StripIteratorPoint const&, StripIteratorPoint const&, StripIteratorPoint const&, StripIteratorPoint const&>, boost::_bi::list5<boost::_bi::value<DisplayCenterPath<SetterTextureCoordinate, DisplaySegmentQuadric>*>, boost::arg<1>, boost::arg<2>, boost::arg<3>, boost::arg<4> > > > (boost::_bi::bind_t<void, boost::_mfi::mf4<void, DisplayCenterPath<SetterTextureCoordinate, DisplaySegmentQuadric>, StripIteratorPoint const&, StripIteratorPoint const&, StripIteratorPoint const&, StripIteratorPoint const&>, boost::_bi::list5<boost::_bi::value<DisplayCenterPath<SetterTextureCoordinate, DisplaySegmentQuadric>*>, boost::arg<1>, boost::arg<2>, boost::arg<3>, boost::arg<4> > >);

template void StripIterator::ForEachSegment<boost::_bi::bind_t<void, boost::_mfi::mf4<void, DisplayCenterPath<SetterTextureCoordinate, DisplaySegment>, StripIteratorPoint const&, StripIteratorPoint const&, StripIteratorPoint const&, StripIteratorPoint const&>, boost::_bi::list5<boost::_bi::value<DisplayCenterPath<SetterTextureCoordinate, DisplaySegment>*>, boost::arg<1>, boost::arg<2>, boost::arg<3>, boost::arg<4> > > >
(boost::_bi::bind_t<void, boost::_mfi::mf4<void, DisplayCenterPath<SetterTextureCoordinate, DisplaySegment>, StripIteratorPoint const&, StripIteratorPoint const&, StripIteratorPoint const&, StripIteratorPoint const&>, boost::_bi::list5<boost::_bi::value<DisplayCenterPath<SetterTextureCoordinate, DisplaySegment>*>, boost::arg<1>, boost::arg<2>, boost::arg<3>, boost::arg<4> > >);

template void StripIterator::ForEachSegment<boost::_bi::bind_t<void, boost::_mfi::mf4<void, Simulation, StripIteratorPoint const&, StripIteratorPoint const&, StripIteratorPoint const&, StripIteratorPoint const&>, boost::_bi::list5<boost::_bi::value<Simulation*>, boost::arg<1>, boost::arg<2>, boost::arg<3>, boost::arg<4> > > > (boost::_bi::bind_t<void, boost::_mfi::mf4<void, Simulation, StripIteratorPoint const&, StripIteratorPoint const&, StripIteratorPoint const&, StripIteratorPoint const&>, boost::_bi::list5<boost::_bi::value<Simulation*>, boost::arg<1>, boost::arg<2>, boost::arg<3>, boost::arg<4> > >);

template void StripIterator::ForEachSegment<boost::_bi::bind_t<void, boost::_mfi::mf4<void, DisplayCenterPath<SetterTextureCoordinate, DisplaySegmentTube>, StripIteratorPoint const&, StripIteratorPoint const&, StripIteratorPoint const&, StripIteratorPoint const&>, boost::_bi::list5<boost::_bi::value<DisplayCenterPath<SetterTextureCoordinate, DisplaySegmentTube>*>, boost::arg<1>, boost::arg<2>, boost::arg<3>, boost::arg<4> > > >(boost::_bi::bind_t<void, boost::_mfi::mf4<void, DisplayCenterPath<SetterTextureCoordinate, DisplaySegmentTube>, StripIteratorPoint const&, StripIteratorPoint const&, StripIteratorPoint const&, StripIteratorPoint const&>, boost::_bi::list5<boost::_bi::value<DisplayCenterPath<SetterTextureCoordinate, DisplaySegmentTube>*>, boost::arg<1>, boost::arg<2>, boost::arg<3>, boost::arg<4> > >);
/// @endcond
