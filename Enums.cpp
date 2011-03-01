/**
 * @file   Enums.cpp
 * @author Dan R. Lipsa
 * @date 13 May 2010
 *
 * Implementations for functionality declared in Enums.h
 */

#include "Enums.h"
#include "Debug.h"

ostream& operator<< (ostream& ostr, ElementStatus::Enum duplicateStatus)
{
    switch (duplicateStatus)
    {
    case ElementStatus::ORIGINAL:
	ostr << "ORIGINAL";
	break;
    case ElementStatus::DUPLICATE:
	ostr << "DUPLICATE";
	break;
    default:
	RuntimeAssert (false, "Invalid ElementStatus: ", duplicateStatus);
    }
    return ostr;
}

ostream& operator<< (ostream& ostr, AttributeType::Enum type)
{
    switch (type)
    {
    case AttributeType::INT:
        ostr << "INT";
        break;
    case AttributeType::REAL:
        ostr << "REAL";
        break;
    case AttributeType::COLOR:
        ostr << "COLOR";
        break;
    case AttributeType::INT_ARRAY:
        ostr << "INT_ARRAY";
        break;
    case AttributeType::REAL_ARRAY:
        ostr << "REAL_ARRAY";
        break;
    default:
	RuntimeAssert (false, "Invalid AttributeType: ", type);
    }
    return ostr;
}

const char* BodyProperty::ToString (BodyProperty::Enum property)
{
    switch (property)
    {
    case VELOCITY_ALONG_X:
	return "Velocity along x";
    case VELOCITY_ALONG_Y:
	return "Velocity along y";
    case VELOCITY_ALONG_Z:
	return "Velocity along z";
    case VELOCITY_MAGNITUDE:
	return "Velocity magnitude";
    case NUMBER_OF_SIDES:
	return "Number of sides";
    case PERIMETER_OVER_AREA:
	return "Perimeter / area";
    case PRESSURE:
	return "Pressure";
    case VOLUME:
	return "Volume";
    case NONE:
	return "None";
    case COUNT:
	return "Count";
    }
    return "Error";
}

BodyProperty::Enum BodyProperty::FromSizeT (size_t i)
{
    RuntimeAssert (i < COUNT, "Value outside of BodyProperty::Enum", i);
    return BodyProperty::Enum (i);
}

ViewType::Enum ViewType::FromInt (int i)
{
    RuntimeAssert (i < COUNT && i >= 0, "Value outside of ViewType::Enum", i);
    return ViewType::Enum (i);
}


const char* Palette::ToString (Palette::Enum type)
{
    switch (type)
    {
    case RAINBOW:
	return "Rainbow";
    case RAINBOW_EXTENDED:
	return "Rainbow extended";
    case BLACK_BODY:
	return "Black body radiator";
    case BLUE_RED_DIVERGING:
	return "Blue-Red Diverging";
    case BLUE_TAN_DIVERGING:
	return "Blue-Tan Diverging";
    case PURPLE_ORANGE_DIVERGING:
	return "Purple-Orange Diverging";
    case GREEN_PURPLE_DIVERGING:
	return "Green-Purple Diverging";
    case GREEN_RED_DIVERGING:
	return "Green-Red Diverging";
    default:
	RuntimeAssert (false, "Invalid Palette: ", type);
	return 0;
    }
}
