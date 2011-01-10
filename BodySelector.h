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
 * Abstract class. Selects a list of bodies based on a property.
 */
class BodySelector
{
public:
    /**
     * Returns true if this body is selected.
     */
    virtual bool operator () (size_t bodyId, size_t timeStep) const = 0;
};

class AllBodiesSelected : public BodySelector
{
    virtual bool operator () (size_t bodyId, size_t timeStep) const
    {
	(void) bodyId;
	(void) timeStep;
	return true;
    }
};


class GLWidget;
/**
 * Selects a body specified by GLWidget. (which cycles through all bodies)
 */
class CycleBodySelector : public BodySelector
{
public:
    CycleBodySelector (const GLWidget& glWidget) :
	m_glWidget (glWidget)
    {
    }
    
    virtual bool operator () (size_t bodyId, size_t timeStep) const;
    
private:
    const GLWidget& m_glWidget;
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
    PropertyValueBodySelector (BodyProperty::Enum bodyProperty,
			   vector<QwtDoubleInterval> valueIntervals,
			   const FoamAlongTime& foamAlongTime) :
	m_bodyProperty (bodyProperty), m_valueIntervals (valueIntervals),
	m_foamAlongTime (foamAlongTime)
    {
    }

    virtual bool operator () (size_t bodyId, size_t timeStep) const;

private:
    BodyProperty::Enum m_bodyProperty;
    ValueIntervals m_valueIntervals;
    const FoamAlongTime& m_foamAlongTime;
};


/**
 * Selects bodies with specified ids.
 */
class IdBodySelector : public BodySelector
{
public:
    IdBodySelector (vector<size_t> ids);
    virtual bool operator () (size_t bodyId, size_t timeStep) const;

private:
    vector<size_t> m_ids;
};


#endif //__BODY_SELECTOR_H__
// Local Variables:
// mode: c++
// End:
