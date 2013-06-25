/**
 * @file OrientedElement.h
 * @author Dan R. Lipsa
 * @date 21 May 2010
 * @ingroup data model
 * @brief Base class for OrientedFace and OrientedEdge. Allows using a
 *        Face or Edge in direct or reversed order.
 */
#ifndef __ORIENTED_ELEMENT_H__
#define __ORIENTED_ELEMENT_H__

#include "ParsingEnums.h"
class Element;

/**
 * @brief Base class for OrientedFace and OrientedEdge. Allows using a
 *        Face or Edge in direct or reversed order.
 */
class OrientedElement
{
public:
    OrientedElement (
	boost::shared_ptr<Element> element, bool reversed) :
	m_element (element), m_reversed (reversed) {}

    OrientedElement () : m_reversed(false) {}

    boost::shared_ptr<Element> GetElement () const
    {
	return m_element;
    }
    void SetElement (boost::shared_ptr<Element> element)
    {
	m_element = element;
    }

    size_t GetId () const;
    void Reverse ()
    {
	m_reversed = ! m_reversed;
    }

    /**
     * Is  this  in the  same  order  or  reversed compared  with  the
     * Element associated with  it.  @return true for reversed,
     * false otherwise
     */
    bool IsReversed () const
    {
	return m_reversed;
    }

    string GetStringId () const;
    string GetOrientationString () const;

private:
    boost::shared_ptr<Element> m_element;
    bool m_reversed;
};


#endif //__ORIENTED_ELEMENT_H__

// Local Variables:
// mode: c++
// End:
