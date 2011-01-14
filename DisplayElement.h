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
	TRANSPARENT_CONTEXT,
	INVISIBLE_CONTEXT
    };

    enum TessellationEdgesDisplay
    {
	TEST_DISPLAY_TESSELLATION,
	DONT_DISPLAY_TESSELLATION
    };

public:
    DisplayElement (const GLWidget& widget,
		    bool useZPos = false, double zPos = 0) : 
	m_glWidget (widget),
	m_useZPos (useZPos),
	m_zPos (zPos)
    {
    }

protected:
    const GLWidget& m_glWidget;
    bool m_useZPos;
    double m_zPos;
};


class DisplayElementFocus : public DisplayElement
{
public:
    DisplayElementFocus (
	const GLWidget& widget,
	FocusContext focus = FOCUS, bool useZPos = false, double zPos = 0) :
	
	DisplayElement (widget, useZPos, zPos),
	m_focus (focus)
    {
    }
    
protected:
    FocusContext m_focus;
};

class TexCoordSetter;

template<typename PropertySetter = TexCoordSetter>
class DisplayElementProperty : public DisplayElement
{
public:
    DisplayElementProperty (
	const GLWidget& glWidget,
	PropertySetter propertySetter,
	BodyProperty::Enum property = BodyProperty::NONE,
	bool useZPos = false, double zPos = 0) :

	DisplayElement (glWidget, useZPos, zPos),
	m_propertySetter (propertySetter),
	m_property (property)
    {
    }
protected:
    PropertySetter m_propertySetter;
    BodyProperty::Enum m_property;
};



template<typename PropertySetter = TexCoordSetter>
class DisplayElementPropertyFocus : 
    public DisplayElementProperty<PropertySetter>
{
public:
    DisplayElementPropertyFocus (
	const GLWidget& widget,
	PropertySetter setter,
	BodyProperty::Enum property = BodyProperty::NONE,
	DisplayElement::FocusContext focus = DisplayElement::FOCUS,
	bool useZPos = false, double zPos = 0) :
	
	DisplayElementProperty<PropertySetter> (
	    widget, setter, property, useZPos, zPos),
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
