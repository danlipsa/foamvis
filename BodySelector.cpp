/**
 * @file   CycleSelector.h
 * @author Dan R. Lipsa
 * @date  31 Aug. 2010
 *
 * Functors used to select a body
 */

#include "BodySelector.h"
#include "FoamAlongTime.h"
#include "GLWidget.h"


// Members
// ======================================================================
bool CycleSelector::operator () (size_t bodyId, size_t timeStep) const
{
    (void)timeStep;
    return m_glWidget.IsDisplayedBody (bodyId);
}


bool PropertyValueSelector::operator () (size_t bodyId, size_t timeStep) const
{
    if (m_foamAlongTime.ExistsBodyProperty (
	    m_bodyProperty, bodyId, timeStep))
    {
	float value = m_foamAlongTime.GetBodyProperty (
	    m_bodyProperty, bodyId, timeStep);
	ValueIntervals::const_iterator it = find_if (
	    m_valueIntervals.begin (), m_valueIntervals.end (),
	    boost::bind (&QwtDoubleInterval::contains, _1, value));
	return it != m_valueIntervals.end ();
    }
    else
	return false;
}
