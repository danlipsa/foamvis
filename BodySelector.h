/**
 * @file   BodySelector.h
 * @author Dan R. Lipsa
 * @date  31 Aug. 2010
 *
 * Functors used to select a body
 */
#ifndef __BODY_SELECTOR_H__
#define __BODY_SELECTOR_H__

#include "Enums.h"
class Body;


/**
 * Abstract class. Selects a list of bodies based on a criteria.
 */
class BodySelector
{

public:
    virtual ~BodySelector ()
    {
    }
    /**
     * Returns true if this body is selected.
     */
    virtual bool operator () (const boost::shared_ptr<Body>& body) const = 0;
    virtual BodySelectorType::Enum GetType () const = 0;
    boost::shared_ptr<BodySelector> Clone () const;
};

class BodySelectorPredicate
{
public:
    BodySelectorPredicate (const BodySelector& bs) :
	m_bs (bs)
    {
    }
    bool operator () (const boost::shared_ptr<Body>& body) const
    {
	return m_bs.operator () (body);
    }
private:
    const BodySelector& m_bs;
};


class AllBodySelector : public BodySelector
{
public:
    virtual bool operator () (const boost::shared_ptr<Body>& body) const
    {
	(void) body;
	return true;
    }
    virtual BodySelectorType::Enum GetType () const
    {
	return BodySelectorType::ALL;
    }
    boost::shared_ptr<AllBodySelector> Clone () const
    {
	return SELECTOR;
    }

public:
    static boost::shared_ptr<AllBodySelector> Get ()
    {
	return SELECTOR;
    }
private:
    static boost::shared_ptr<AllBodySelector> SELECTOR;
};

/**
 * Selects bodies with a propriety value in an interval from an
 * interval list.
 */
class PropertyValueBodySelector : public BodySelector
{
public:
    typedef vector<QwtDoubleInterval> ValueIntervals;
    typedef vector<pair<size_t, size_t> > Bins;
public:
    PropertyValueBodySelector (BodyScalar::Enum property,
			       const ValueIntervals& valueIntervals,
                               const Bins& bins) :
	m_property (property),
        m_valueIntervals (valueIntervals),
        m_bins (bins)
    {
    }
    virtual ~PropertyValueBodySelector ()
    {
    }

    virtual bool operator () (const boost::shared_ptr<Body>& body) const;
    virtual BodySelectorType::Enum GetType () const
    {
	return BodySelectorType::PROPERTY_VALUE;
    }
    boost::shared_ptr<PropertyValueBodySelector> Clone () const;
    string ToUserString () const;
    const ValueIntervals& GetIntervals () const
    {
	return m_valueIntervals;
    }
    const Bins& GetBins () const
    {
        return m_bins;
    }

private:
    BodyScalar::Enum m_property;
    ValueIntervals m_valueIntervals;
    /**
     * Useful for setting the selection on a histogram.
     */
    Bins m_bins;
};


/**
 * Selects bodies with specified ids.
 */
class IdBodySelector : public BodySelector
{
public:
    IdBodySelector ();
    IdBodySelector (size_t id);
    IdBodySelector (const vector<size_t>& ids);
    virtual ~IdBodySelector ()
    {
    }

    virtual bool operator () (const boost::shared_ptr<Body>& body) const;
    BodySelectorType::Enum GetType () const
    {
	return BodySelectorType::ID;
    }
    boost::shared_ptr<IdBodySelector> Clone () const;

    void SetUnion (const vector<size_t>& idsToAdd);
    void SetUnion (const IdBodySelector& idsToAdd);
    void SetDifference (const vector<size_t>& idsToRemove);
    const vector<size_t>& GetIds () const
    {
	return m_ids;
    }
    string ToString () const;
    string ToUserString () const;

private:
    /**
     * Selected body ids ordered ascending.
     */
    vector<size_t> m_ids;
};


/**
 * Selects a body based on Id and PropertyValue.
 */
class CompositeBodySelector : public BodySelector
{
public:
    CompositeBodySelector (
	boost::shared_ptr<IdBodySelector> idSelector,
	boost::shared_ptr<PropertyValueBodySelector> propertyValueSelector) :
	m_idSelector (idSelector),
	m_propertyValueSelector (propertyValueSelector)
    {
    }
    virtual ~CompositeBodySelector ()
    {
    }
    boost::shared_ptr<CompositeBodySelector> Clone () const;


    boost::shared_ptr<PropertyValueBodySelector> 
    GetPropertyValueSelector () const
    {
	return m_propertyValueSelector;
    }
    boost::shared_ptr<IdBodySelector> GetIdSelector () const
    {
	return m_idSelector;
    }

    void SetSelector (
	boost::shared_ptr<PropertyValueBodySelector> propertyValueSelector)
    {
	m_propertyValueSelector = propertyValueSelector;
    }

    void SetSelector (boost::shared_ptr<IdBodySelector> idSelector)
    {
	m_idSelector = idSelector;
    }

    virtual bool operator () (const boost::shared_ptr<Body>& body) const;
    BodySelectorType::Enum GetType () const
    {
	return BodySelectorType::COMPOSITE;
    }


private:
    boost::shared_ptr<IdBodySelector> m_idSelector;
    boost::shared_ptr<PropertyValueBodySelector> m_propertyValueSelector;
};

inline ostream& operator<< (ostream& ostr, const IdBodySelector& selector)
{
    return ostr << selector.ToString ();
}



#endif //__BODY_SELECTOR_H__

// Local Variables:
// mode: c++
// End:
