/**
 * @file   BodySelector.h
 * @author Dan R. Lipsa
 * @date  31 Aug. 2010
 * @ingroup settings model
 * @brief Functors that specify selected bubbles. 
 */
#ifndef __BODY_SELECTOR_H__
#define __BODY_SELECTOR_H__

#include "Enums.h"
class Body;


/**
 * @brief Specifies selected bubbles
 */
class BodySelector
{
public:
    typedef vector<QwtDoubleInterval> ValueIntervals;

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
    const BinRegions& GetBins () const;
private:
    static const BinRegions ALL_BINS;
};

/**
 * @brief Specifies selected bubbles (used in std libraries algorithms)
 */
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

/**
 * @brief Specifies that all bubbles are selected.
 */
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
 * @brief Specifies that bubbles with a scalar value in a specified 
 *        list of intervals are selected.
 */
class ValueBodySelector : public BodySelector
{
public:
    ValueBodySelector (BodyScalar::Enum scalar, bool is2D,
                       const ValueIntervals& valueIntervals,
                       const BinRegions& bins);
    virtual ~ValueBodySelector ()
    {
    }
    virtual bool operator () (const boost::shared_ptr<Body>& body) const;
    virtual BodySelectorType::Enum GetType () const
    {
	return BodySelectorType::PROPERTY_VALUE;
    }
    boost::shared_ptr<ValueBodySelector> Clone () const;
    string ToUserString () const;
    const ValueIntervals& GetIntervals () const
    {
	return m_valueIntervals;
    }
    const BinRegions& GetBins () const
    {
        return m_bins;
    }
    BodyScalar::Enum GetScalar () const
    {
        return m_scalar;
    }

private:
    BodyScalar::Enum m_scalar;
    const bool m_is2D;
    ValueIntervals m_valueIntervals;
    /**
     * Useful for setting the selection on a histogram.
     */
    BinRegions m_bins;
};


/**
 * @brief Specifies that bubbles with given IDs are selected
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
 * @brief Specifies that bubbles with given IDs and scalar values are selected.
 */
class CompositeBodySelector : public BodySelector
{
public:
    CompositeBodySelector (
	boost::shared_ptr<IdBodySelector> idSelector,
	boost::shared_ptr<ValueBodySelector> propertyValueSelector) :

	m_idSelector (idSelector),
	m_valueSelector (propertyValueSelector)
    {
    }
    virtual ~CompositeBodySelector ()
    {
    }
    boost::shared_ptr<CompositeBodySelector> Clone () const;


    boost::shared_ptr<ValueBodySelector> GetValueSelector () const
    {
	return m_valueSelector;
    }
    boost::shared_ptr<IdBodySelector> GetIdSelector () const
    {
	return m_idSelector;
    }

    void SetSelector (boost::shared_ptr<ValueBodySelector> valueSelector)
    {
	m_valueSelector = valueSelector;
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
    boost::shared_ptr<ValueBodySelector> m_valueSelector;
};

inline ostream& operator<< (ostream& ostr, const IdBodySelector& selector)
{
    return ostr << selector.ToString ();
}



#endif //__BODY_SELECTOR_H__

// Local Variables:
// mode: c++
// End:
