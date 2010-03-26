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
    DisplayElement (const GLWidget& widget) : m_widget (widget) {}
protected:
    const GLWidget& m_widget;
};

#endif //__DISPLAY_ELEMENT_H__
