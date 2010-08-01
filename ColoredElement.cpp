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
#include "Debug.h"
#include "ParsingDriver.h"

// Static fields
// ======================================================================
const size_t ColoredElement::COLOR_INDEX = 0;


// Methods
// ======================================================================

Color::Enum ColoredElement::GetColor (Color::Enum defaultColor) const
{
    if (m_attributes != 0)
	return *boost::static_pointer_cast<ColorAttribute> (
	    (*m_attributes)[COLOR_INDEX]);
    else
    {
	if (defaultColor == Color::COUNT)
	    return static_cast<Color::Enum>((GetId ()+1) % Color::COUNT);
	else
	    return defaultColor;
    }
}

string ColoredElement::GetStringId () const
{
    ostringstream id, ostr;
    id << Element::GetStringId () << " "
       << GetColor ();
    ostr << setw (15) << id.str ();
    return ostr.str ();
}



// Static and Friends methods
// ======================================================================

void ColoredElement::AddDefaultAttributes (AttributesInfo* infos)
{
    using EvolverData::parser;
    const char* colorString = 
        ParsingDriver::GetKeywordString(parser::token::COLOR);
    auto_ptr<AttributeCreator> ac (new ColorAttributeCreator());
    size_t colorIndex = infos->AddAttributeInfoLoad (colorString, ac);
    RuntimeAssert (colorIndex == COLOR_INDEX,
		   "Color should be stored at index ", COLOR_INDEX);
}
