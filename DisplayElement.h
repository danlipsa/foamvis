/**
 * @file   DisplayElement.h
 * @author Dan R. Lipsa
 * @date  4 March 2010
 *
 * Interface for functors that display an element (body, face, edge or vertex)
 */

#ifndef __DISPLAY_ELEMENT_H__
#define __DISPLAY_ELEMENT_H__

#include "Enums.h"
#include "PropertySetter.h"
class GLWidget;
class Foam;

class DisplayElement
{
public:
    enum FocusContext
    {
	FOCUS,
	CONTEXT
    };
    enum ContextType
    {
	USER_DEFINED_CONTEXT,
	INVISIBLE_CONTEXT
    };

    enum TessellationEdgesDisplay
    {
	DISPLAY_TESSELLATION_EDGES,
	DONT_DISPLAY_TESSELLATION_EDGES
    };

public:
    DisplayElement (
	const GLWidget& widget, const Foam& foam,
	bool useZPos = false, double zPos = 0) : 
	m_widgetGl (widget), m_foam (foam),
	m_useZPos (useZPos),
	m_zPos (zPos)
    {
    }

protected:
    const GLWidget& m_widgetGl;
    const Foam& m_foam;
    bool m_useZPos;
    double m_zPos;
};


class DisplayElementFocus : public DisplayElement
{
public:
    DisplayElementFocus (
	const GLWidget& widget, const Foam& foam,
	FocusContext focus = FOCUS, bool useZPos = false, double zPos = 0) :
	
	DisplayElement (widget, foam, useZPos, zPos),
	m_focus (focus)
    {
    }
    
protected:
    FocusContext m_focus;
};

class SetterTextureCoordinate;

template<typename PropertySetter = SetterTextureCoordinate>
class DisplayElementProperty : public DisplayElement
{
public:
    DisplayElementProperty (
	const GLWidget& widgetGl, const Foam& foam, 
	PropertySetter propertySetter,
	bool useZPos = false, double zPos = 0) :

	DisplayElement (widgetGl, foam, useZPos, zPos), 
	m_propertySetter (propertySetter)
    {
    }
protected:
    PropertySetter m_propertySetter;
};



template<typename PropertySetter = SetterTextureCoordinate>
class DisplayElementPropertyFocus : 
    public DisplayElementProperty<PropertySetter>
{
public:
    DisplayElementPropertyFocus (
	const GLWidget& widget, const Foam& foam,
	PropertySetter setter,
	DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	bool useZPos = false, double zPos = 0) :
	
	DisplayElementProperty<PropertySetter> (
	    widget, foam, setter, useZPos, zPos),
	m_focus (focus)
    {
    }
protected:
    DisplayElement::FocusContext m_focus;
};


#endif //__DISPLAY_ELEMENT_H__

// Local Variables:
// mode: c++
// End:
