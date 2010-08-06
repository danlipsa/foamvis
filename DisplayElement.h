/**
 * @file   DisplayElement.h
 * @author Dan R. Lipsa
 * @date  4 March 2010
 *
 * Interface for functors that display an element (body, face, edge or vertex)
 */

#ifndef __DISPLAY_ELEMENT_H__
#define __DISPLAY_ELEMENT_H__

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
    DisplayElement (const GLWidget& widget) : m_widget (widget) {}
protected:
    const GLWidget& m_widget;
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


class DisplayElementProperties : public DisplayElementFocus
{
public:
    DisplayElementProperties (
	const GLWidget& widget,
	FocusContext focus = FOCUS,
	BodyProperty::Enum bodyProperty = BodyProperty::NONE) :
	DisplayElementFocus (widget, focus),
	m_bodyProperty (bodyProperty)
    {
    }
protected:
    BodyProperty::Enum m_bodyProperty;
};


#endif //__DISPLAY_ELEMENT_H__

// Local Variables:
// mode: c++
// End:
