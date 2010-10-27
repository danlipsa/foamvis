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
class GLWidget;

class DisplayElement
{
public:
    enum FocusContext
    {
	FOCUS,
	CONTEXT
    };
    enum ContextDisplay
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
    DisplayElement (const GLWidget& widget) : 
	m_glWidget (widget) 
    {
    }

protected:
    const GLWidget& m_glWidget;
};


class DisplayElementFocus : public DisplayElement
{
public:
    DisplayElementFocus (const GLWidget& widget,
			 FocusContext focus = FOCUS) :
	DisplayElement (widget), m_focus (focus)
    {
    }
    
protected:
    FocusContext m_focus;
};

class TexCoordSetter
{
public:
    TexCoordSetter (const GLWidget& glWidget) :
	m_glWidget (glWidget)
    {
    }

    void operator () (double value);

private:
    const GLWidget& m_glWidget;
};


template<typename PropertySetter = TexCoordSetter>
class DisplayElementProperty : public DisplayElement
{
public:
    DisplayElementProperty (
	const GLWidget& glWidget,
	PropertySetter propertySetter,
	BodyProperty::Enum bodyProperty = BodyProperty::NONE) :
	DisplayElement (glWidget),
	m_propertySetter (propertySetter),
	m_bodyProperty (bodyProperty)
    {
    }
protected:
    PropertySetter m_propertySetter;
    BodyProperty::Enum m_bodyProperty;
};



template<typename PropertySetter = TexCoordSetter>
class DisplayElementPropertyFocus : 
    public DisplayElementProperty<PropertySetter>
{
public:
    DisplayElementPropertyFocus (
	const GLWidget& widget,
	PropertySetter setter,
	BodyProperty::Enum bodyProperty = BodyProperty::NONE,
	DisplayElement::FocusContext focus = DisplayElement::FOCUS) :
	DisplayElementProperty<PropertySetter> (widget, setter, bodyProperty),
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
