/**
 * @file   ColoredElement.cpp
 * @author Dan R. Lipsa
 * @data 19 April 2010
 *
 * Implementation of the ColoredElement class
 */

#include "Attribute.h"
#include "AttributeInfo.h"
#include "AttributeCreator.h"
#include "ColoredElement.h"
#include "ParsingDriver.h"


void ColoredElement::StoreDefaultAttributes (AttributesInfo* infos)
{
    using EvolverData::parser;
    const char* colorString = 
        ParsingDriver::GetKeywordString(parser::token::COLOR);
    infos->Load (colorString);
    infos->AddAttributeInfo (colorString, new ColorAttributeCreator());
}


Color::Name ColoredElement::GetColor (Color::Name color) const
{
    if (m_attributes != 0)
	return dynamic_cast<ColorAttribute*>(
	    (*m_attributes)[COLOR_INDEX].get ())->GetColor ();
    else
    {
	if (color == Color::COUNT)
	    return static_cast<Color::Name>(
		(GetOriginalIndex ()+1) % Color::COUNT);
	else
	    return color;
    }
}
