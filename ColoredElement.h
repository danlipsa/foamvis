/**
 * @file Element.h
 * @author Dan R. Lipsa
 * @date 19 April 2010
 * 
 * Base class for Edge and Face
 */
#ifndef __COLORED_ELEMENT_H__
#define __COLORED_ELEMENT_H__

#include "Element.h"
#include "Enums.h"

class ColoredElement : public Element
{
public:
    ColoredElement(size_t id, ElementStatus::Name status) :
    Element (id, status) 
    {}

    /**
     * Returns  the  element color.   
     *
     * @param defaultColor  the color that  should be returned  if the
     * element doesn't have any  color. If the default is Color::COUNT
     * the color returned is based on the index of the element
     */
    Color::Name GetColor (Color::Name defaultColor = Color::CLEAR) const;
    string GetStringId () const;
public:
    /**
     * Specifies the default attributes for an Face object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    static void StoreDefaultAttributes (AttributesInfo* info);

private:
    /**
     * Index where the color attribute is stored
     */
    const static size_t COLOR_INDEX = 0;
};


#endif //__COLORED_ELEMENT_H__

// Local Variables:
// mode: c++
// End:
