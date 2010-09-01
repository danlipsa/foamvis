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

class BodySelector
{
public:
    /**
     * Returns true if this body is selected.
     */
    virtual bool operator () (size_t bodyId, size_t timeStep) const = 0;
};

class GLWidget;
class CycleSelector : public BodySelector
{
public:
    CycleSelector (const GLWidget& glWidget) :
	m_glWidget (glWidget)
    {
    }
    
    virtual bool operator () (size_t bodyId, size_t timeStep) const;
    
private:
    const GLWidget& m_glWidget;
};

class FoamAlongTime;
class PropertyValueSelector : public BodySelector
{
public:
    typedef vector<QwtDoubleInterval> ValueIntervals;
public:
    PropertyValueSelector (BodyProperty::Enum bodyProperty,
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

#endif //__BODY_SELECTOR_H__
// Local Variables:
// mode: c++
// End:
