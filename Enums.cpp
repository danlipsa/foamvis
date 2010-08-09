/**
 * @file   Enums.cpp
 * @author Dan R. Lipsa
 * @date 13 May 2010
 *
 * Implementations for functionality declared in Enums.h
 */

#include "Enums.h"
#include "Debug.h"


const G3D::Color3 Color::COLORS[] = 
{
    G3D::Color3(0.0, 0.0, 0.0),  // BLACK
    G3D::Color3(0.0, 0.0, 1.0),  // BLUE
    G3D::Color3(0.0, 1.0, 0.0),  // GREEN
    G3D::Color3(0.0, 1.0, 1.0),  // CYAN
    G3D::Color3(1.0, 0.0, 0.0),  // RED
    G3D::Color3(1.0, 0.0, 1.0),  // MAGENTA
    G3D::Color3(1.0, 0.5, 0.0),  // BROWN
    G3D::Color3(.6f, .6f, .6f),  // LIGHTGRAY
    G3D::Color3(.3f, .3f, .3f),  // DARKGRAY
    G3D::Color3(.3f, .8f, 1.0),  // LIGHTBLUE
    G3D::Color3(0.5, 1.0, 0.5),  // LIGHTGREEN
    G3D::Color3(0.5, 1.0, 1.0),  // LIGHTCYAN
    G3D::Color3(1.0, 0.5, 0.5),  // LIGHTRED
    G3D::Color3(1.0, 0.5, 1.0),  // LIGHTMAGENTA
    G3D::Color3(1.0, 1.0, 0.0),  // YELLOW
    G3D::Color3(1.0, 1.0, 1.0),  // WHITE
    G3D::Color3(0.0, 0.0, 0.0)   // CLEAR
};

ostream& operator<< (ostream& ostr, Color::Enum color)
{
    switch (color)
    {
    case Color::BLACK:
	ostr << "Black";
	break;
    case Color::BLUE:
	ostr << "Blue";
	break;
    case Color::GREEN:
	ostr << "Green";
	break;
    case Color::CYAN:
	ostr << "Cyan";
	break;
    case Color::RED:
	ostr << "Red";
	break;
    case Color::MAGENTA:
	ostr << "Magenta";
	break;
    case Color::BROWN:
	ostr << "Brown";
	break;
    case Color::LIGHTGRAY:
	ostr << "Lightgray";
	break;
    case Color::DARKGRAY:
	ostr << "Darkgray";
	break;
    case Color::LIGHTBLUE:
	ostr << "Lightblue";
	break;
    case Color::LIGHTGREEN:
	ostr << "Lightgreen";
	break;
    case Color::LIGHTCYAN:
	ostr << "Lightcyan";
	break;
    case Color::LIGHTRED:
	ostr << "Lightred";
	break;
    case Color::LIGHTMAGENTA:
	ostr << "Lightmagenta";
	break;
    case Color::YELLOW:
	ostr << "Yellow";
	break;
    case Color::WHITE:
	ostr << "White";
	break;
    case Color::CLEAR:
	ostr << "Clear";
	break;
    default:
	ostr << "Invalid";
	break;
    }
    return ostr;
}

const G3D::Color3& Color::GetValue(Color::Enum color)
{
    return COLORS[color];
}


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

string BodyProperty::ToString (BodyProperty::Enum property)
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
    return static_cast<BodyProperty::Enum>(i);
}
