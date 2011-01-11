/**
 * @file   BodySelector.cpp
 * @author Dan R. Lipsa
 * @date  31 Aug. 2010
 *
 * Functors used to select a body
 */

#include "BodySelector.h"
#include "FoamAlongTime.h"
#include "GLWidget.h"
#include "DebugStream.h"



// Members
// ======================================================================
bool CycleBodySelector::operator () (size_t bodyId, size_t timeStep) const
{
    (void)timeStep;
    return m_glWidget.IsDisplayedBody (bodyId);
}


bool PropertyValueBodySelector::operator () (
    size_t bodyId, size_t timeStep) const
{
    if (m_foamAlongTime.ExistsBodyProperty (
	    m_bodyProperty, bodyId, timeStep))
    {
	double value = m_foamAlongTime.GetBodyProperty (m_bodyProperty, 
							bodyId, timeStep);
	ValueIntervals::const_iterator it = find_if (
	    m_valueIntervals.begin (), m_valueIntervals.end (),
	    boost::bind (&QwtDoubleInterval::contains, _1, value));
	return it != m_valueIntervals.end ();
    }
    else
	return false;
}

IdBodySelector::IdBodySelector (const vector<size_t>& ids) :
    m_ids (ids)
{
    cdbg << "IdBodySelector: ";
    ostream_iterator<size_t> ido (cdbg, " ");
    copy (m_ids.begin (), m_ids.end (), ido);
    cdbg << endl;
}


bool IdBodySelector::operator () (
    size_t bodyId, size_t timeStep) const
{
    (void)timeStep;
    vector<size_t>::const_iterator it = find (
	m_ids.begin (), m_ids.end (), bodyId);
    return it != m_ids.end ();
}
