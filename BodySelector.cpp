/**
 * @file   BodySelector.cpp
 * @author Dan R. Lipsa
 * @date  31 Aug. 2010
 *
 * Functors used to select a body
 */

#include "Body.h"
#include "BodySelector.h"
#include "Debug.h"
#include "DebugStream.h"
#include "GLWidget.h"
#include "Utils.h"


// AllBodySelector
// ======================================================================

boost::shared_ptr<AllBodySelector> AllBodySelector::SELECTOR = 
    boost::shared_ptr<AllBodySelector> (new AllBodySelector ());

// PropertyValueBodySelector
// ======================================================================

bool PropertyValueBodySelector::operator () (
    const boost::shared_ptr<Body>& body) const
{
    if (body->ExistsPropertyValue (m_property))
    {
	double value = body->GetPropertyValue (m_property);
	ValueIntervals::const_iterator it = find_if (
	    m_valueIntervals.begin (), m_valueIntervals.end (),
	    boost::bind (&QwtDoubleInterval::contains, _1, value));
	return it != m_valueIntervals.end ();
    }
    else
	return false;
}

string PropertyValueBodySelector::ToUserString () const
{
    ostringstream ostr;
    ostr << "Selection on " << BodyProperty::ToString (m_property) << endl
	 << "Intervals: ";
    ostream_iterator<QwtDoubleInterval> ido (ostr, " ");
    copy (m_valueIntervals.begin (), m_valueIntervals.end (), ido);
    return ostr.str ();
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
}


bool IdBodySelector::operator () (
    const boost::shared_ptr<Body>& body) const
{
    return binary_search (m_ids.begin (), m_ids.end (), body->GetId ());
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

void IdBodySelector::SetUnion (const IdBodySelector& idsToAdd)
{
    SetUnion (idsToAdd.m_ids);
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

string IdBodySelector::ToString () const
{
    ostringstream ostr;
    ostr << "IdBodySelector: ";
    ostream_iterator<size_t> ido (ostr, " ");
    copy (m_ids.begin (), m_ids.end (), ido);
    ostr << endl;
    return ostr.str ();
}

string IdBodySelector::ToUserString () const
{
    ostringstream ostr;
    ostream_iterator<size_t> ido (ostr, " ");
    copy (m_ids.begin (), m_ids.end (), ido);
    ostr << endl;
    return ostr.str ();
}


// CompositeBodySelector
// ======================================================================

bool CompositeBodySelector::operator () (
    const boost::shared_ptr<Body>& body) const
{
    return (*m_idSelector) (body) && (*m_propertyValueSelector) (body);
}

