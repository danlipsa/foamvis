/**
 * @file OrientedElement.h
 * @author Dan R. Lipsa
 * @date 21 May 2010
 * 
 * Declaration for the OrientedElement class
 */
#ifndef __ORIENTED_ELEMENT_H__
#define __ORIENTED_ELEMENT_H__

#include "ParsingEnums.h"
class ColoredElement;

/**
 * Base class for OrientedFace and OrientedEdge
 */
class OrientedElement
{
public:
    OrientedElement (
	boost::shared_ptr<ColoredElement> coloredElement, bool reversed) :
	m_coloredElement (coloredElement), m_reversed (reversed) {}

    OrientedElement () : m_reversed(false) {}

    boost::shared_ptr<ColoredElement> GetColoredElement () const
    {
	return m_coloredElement;
    }
    void SetElement (boost::shared_ptr<ColoredElement> coloredElement)
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

    QColor GetColor (const QColor& defaultColor) const;


    string GetStringId () const;

private:
    boost::shared_ptr<ColoredElement> m_coloredElement;
    bool m_reversed;
};


#endif //__ORIENTED_ELEMENT_H__

// Local Variables:
// mode: c++
// End:
