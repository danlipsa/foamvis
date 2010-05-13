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
    ColoredElement(size_t originalIndex, Data* data,
		   ElementStatus::Name status) :
    Element (originalIndex, data, status) 
    {}

    /**
     * Returns the element color.
     * @param color the color that should be returned if the element doesn't
     *        have any color. The default is a color based on the index of the 
     *        element if the parameter is Color::COUNT
     */
    Color::Name GetColor (Color::Name color = Color::COUNT) const;

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
