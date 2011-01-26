/**
 * @file   BodySelector.cpp
 * @author Dan R. Lipsa
 * @date  31 Aug. 2010
 *
 * Functors used to select a body
 */

#include "BodySelector.h"
#include "Debug.h"
#include "DebugStream.h"
#include "FoamAlongTime.h"
#include "GLWidget.h"



// AllBodySelector
// ======================================================================

boost::shared_ptr<AllBodySelector> AllBodySelector::SELECTOR = 
    boost::shared_ptr<AllBodySelector> (new AllBodySelector ());

// PropertyValueBodySelector
// ======================================================================

bool PropertyValueBodySelector::operator () (
    size_t bodyId, size_t timeStep) const
{
    if (m_foamAlongTime.ExistsBodyProperty (
	    m_property, bodyId, timeStep))
    {
	double value = m_foamAlongTime.GetBodyPropertyValue (
	    m_property, bodyId, timeStep);
	ValueIntervals::const_iterator it = find_if (
	    m_valueIntervals.begin (), m_valueIntervals.end (),
	    boost::bind (&QwtDoubleInterval::contains, _1, value));
	return it != m_valueIntervals.end ();
    }
    else
	return false;
}

// IdBodySelector
// ======================================================================

IdBodySelector::IdBodySelector ()
{
}

IdBodySelector::IdBodySelector (size_t id)
{
    m_ids.push_back (id);
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
    return binary_search (m_ids.begin (), m_ids.end (), bodyId);
}

void IdBodySelector::SetUnion (const vector<size_t>& idsToAdd)
{
    vector<size_t> result (m_ids.size () + idsToAdd.size ());
    vector<size_t>::iterator resultEnd = set_union (
	m_ids.begin (), m_ids.end (), idsToAdd.begin (), idsToAdd.end (),
	result.begin ());
    result.resize (resultEnd - result.begin ());
    m_ids = result;
}

void IdBodySelector::SetDifference (const vector<size_t>& idsToRemove)
{
    vector<size_t> result (m_ids.size ());
    vector<size_t>::iterator resultEnd = set_difference (
	m_ids.begin (), m_ids.end (), idsToRemove.begin (), idsToRemove.end (),
	result.begin ());
    result.resize (resultEnd - result.begin ());
    m_ids = result;
}

// CompositeBodySelector
// ======================================================================

bool CompositeBodySelector::operator () (
    size_t bodyId, size_t timeStep) const
{
    return (*m_idSelector) (bodyId, timeStep) && 
	(*m_propertyValueSelector) (bodyId, timeStep);
}

