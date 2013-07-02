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
#include "WidgetGl.h"
#include "Utils.h"


// BodySelector
// ======================================================================

const BinRegions BodySelector::ALL_BINS (
    1, pair<size_t, size_t> (0, HISTOGRAM_INTERVALS));

boost::shared_ptr<BodySelector> BodySelector::Clone () const
{
    switch (GetType ())
    {
    case BodySelectorType::ALL:
	return static_cast<const AllBodySelector*> (this)->Clone ();
	
    case BodySelectorType::ID:
	return static_cast<const IdBodySelector*> (this)->Clone ();
	
    case BodySelectorType::PROPERTY_VALUE:
	return static_cast<const ValueBodySelector*> (this)->Clone ();
	
    case BodySelectorType::COMPOSITE:
	return static_cast<const CompositeBodySelector*> (this)->Clone ();

    default:
	return boost::shared_ptr<BodySelector> ();
    }
}

const BinRegions& BodySelector::GetBins () const
{
    if (GetType  () == BodySelectorType::COMPOSITE ||
        GetType  () == BodySelectorType::PROPERTY_VALUE)
    {
        const ValueBodySelector* selector = 0; 
        if (GetType () == BodySelectorType::COMPOSITE)
            selector = static_cast<const CompositeBodySelector*> (this)
                ->GetValueSelector ().get ();
        else
            selector = static_cast<const ValueBodySelector*> (this);    
        const BinRegions& v = selector->GetBins ();
        return v;
    }
    else
    {
        return ALL_BINS;
    }
}


// AllBodySelector
// ======================================================================

boost::shared_ptr<AllBodySelector> AllBodySelector::SELECTOR = 
    boost::shared_ptr<AllBodySelector> (new AllBodySelector ());

// ValueBodySelector
// ======================================================================

ValueBodySelector::ValueBodySelector (
    BodyScalar::Enum scalar, bool is2D, const ValueIntervals& valueIntervals,
    const BinRegions& bins) :

    m_scalar (scalar),
    m_is2D (is2D),
    m_valueIntervals (valueIntervals),
    m_bins (bins)
{
}


bool ValueBodySelector::operator () (
    const boost::shared_ptr<Body>& body) const
{
    if (body->IsObject ())
	return true;
    if (body->HasScalarValue (m_scalar))
    {
	double value = body->GetScalarValue (m_scalar);
	ValueIntervals::const_iterator it = find_if (
	    m_valueIntervals.begin (), m_valueIntervals.end (),
	    boost::bind (&QwtDoubleInterval::contains, _1, value));
	return it != m_valueIntervals.end ();
    }
    else
	return false;
}

string ValueBodySelector::ToUserString () const
{
    ostringstream ostr;
    ostr << "Selection on " << BodyScalar::ToString (m_scalar) << endl
	 << "Intervals: ";
    ostream_iterator<QwtDoubleInterval> ido (ostr, " ");
    copy (m_valueIntervals.begin (), m_valueIntervals.end (), ido);
    return ostr.str ();
}

boost::shared_ptr<ValueBodySelector> ValueBodySelector::Clone (
    ) const
{
    boost::shared_ptr<ValueBodySelector> p (
	new ValueBodySelector (m_scalar, m_is2D, m_valueIntervals, m_bins));
    return p;
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

boost::shared_ptr<IdBodySelector> IdBodySelector::Clone () const
{
    boost::shared_ptr<IdBodySelector> p (new IdBodySelector (m_ids));
    return p;
}


// CompositeBodySelector
// ======================================================================

bool CompositeBodySelector::operator () (
    const boost::shared_ptr<Body>& body) const
{
    return (*m_idSelector) (body) && (*m_valueSelector) (body);
}

boost::shared_ptr<CompositeBodySelector> CompositeBodySelector::Clone () const
{
    boost::shared_ptr<CompositeBodySelector> p (
	new CompositeBodySelector (m_idSelector->Clone (),
				   m_valueSelector->Clone ()));
    return p;
}
