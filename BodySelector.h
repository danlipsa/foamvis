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
    virtual bool operator () (size_t bodyId, size_t timeStep) const = 0;
    virtual BodySelectorType::Enum GetType () const = 0;
};

class AllBodySelector : public BodySelector
{
public:
    virtual bool operator () (size_t bodyId, size_t timeStep) const
    {
	(void) bodyId;
	(void) timeStep;
	return true;
    }
    BodySelectorType::Enum GetType () const
    {
	return BodySelectorType::ALL;
    }

public:
    static boost::shared_ptr<AllBodySelector> Get ()
    {
	return SELECTOR;
    }
private:
    static boost::shared_ptr<AllBodySelector> SELECTOR;
};

class FoamAlongTime;
/**
 * Selects bodies with a propriety value in an interval from an
 * interval list.
 */
class PropertyValueBodySelector : public BodySelector
{
public:
    typedef vector<QwtDoubleInterval> ValueIntervals;
public:
    PropertyValueBodySelector (BodyProperty::Enum property,
			       vector<QwtDoubleInterval> valueIntervals,
			       const FoamAlongTime& foamAlongTime) :
	m_property (property), m_valueIntervals (valueIntervals),
	m_foamAlongTime (foamAlongTime)
    {
    }
    virtual ~PropertyValueBodySelector ()
    {
    }

    virtual bool operator () (size_t bodyId, size_t timeStep) const;
    BodySelectorType::Enum GetType () const
    {
	return BodySelectorType::PROPERTY_VALUE;
    }

private:
    BodyProperty::Enum m_property;
    ValueIntervals m_valueIntervals;
    const FoamAlongTime& m_foamAlongTime;
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

    virtual bool operator () (size_t bodyId, size_t timeStep) const;
    BodySelectorType::Enum GetType () const
    {
	return BodySelectorType::ID;
    }
    void SetUnion (const vector<size_t>& idsToAdd);
    void SetUnion (const IdBodySelector& idsToAdd);
    void SetDifference (const vector<size_t>& idsToRemove);
    string ToString () const;

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

    virtual bool operator () (size_t bodyId, size_t timeStep) const;
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
