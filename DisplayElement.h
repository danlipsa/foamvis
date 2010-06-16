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


#endif //__DISPLAY_ELEMENT_H__

// Local Variables:
// mode: c++
// End:
