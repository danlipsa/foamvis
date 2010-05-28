/**
 * @file OrientedElement.h
 * @author Dan R. Lipsa
 * @date 21 May 2010
 * 
 * Declaration for the OrientedElement class
 */
#ifndef __ORIENTED_ELEMENT_H__
#define __ORIENTED_ELEMENT_H__

#include "Enums.h"
class ColoredElement;

/**
 * Base class for OrientedFace and OrientedEdge
 */
class OrientedElement
{
public:
    OrientedElement (ColoredElement* coloredElement, bool reversed) :
	m_coloredElement (coloredElement), m_reversed (reversed) {}
    OrientedElement () : m_coloredElement (0), m_reversed(false) {}

    ColoredElement* GetColoredElement () const
    {
	return m_coloredElement;
    }
    void SetElement (ColoredElement* coloredElement)
    {
	m_coloredElement = coloredElement;
    }

    size_t GetId () const;
    void Reverse ()
    {
	m_reversed = ! m_reversed;
    }

    /**
     * Is  this  in the  same  order  or  reversed compared  with  the
     * ColoredElement associated with  it.  @return true for reversed,
     * false otherwise
     */
    bool IsReversed () const
    {
	return m_reversed;
    }

    Color::Name GetColor (Color::Name defaultColor = Color::CLEAR) const;


    string GetStringId () const;

private:
    ColoredElement* m_coloredElement;
    bool m_reversed;
};


#endif //__ORIENTED_ELEMENT_H__

// Local Variables:
// mode: c++
// End:
