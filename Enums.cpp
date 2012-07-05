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
	ostr << "ORIG";
	break;
    case ElementStatus::DUPLICATE:
	ostr << "DUP";
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
    case VELOCITY_X:
	return "Velocity along x";
    case VELOCITY_Y:
	return "Velocity along y";
    case VELOCITY_Z:
	return "Velocity along z";
    case VELOCITY_MAGNITUDE:
	return "Velocity magnitude";
    case SIDES_PER_BODY:
	return "Number of sides";
    case DEFORMATION_P_OVER_SQRTA:
	return "Deformation P / sqrt(A)";
    case DEFORMATION_EIGEN:
	return "Deformation eigen";
    case PRESSURE:
	return "Pressure";
    case TARGET_VOLUME:
	return "Target volume";
    case ACTUAL_VOLUME:
	return "Actual volume";
    case COUNT:
	return "Error";
    }
    return "Error";
}

BodyProperty::Enum BodyProperty::FromSizeT (size_t i)
{
    RuntimeAssert (i < COUNT, "Value outside of BodyProperty::Enum: ", i);
    return BodyProperty::Enum (i);
}

const char* FaceProperty::ToString (FaceProperty::Enum faceProperty)
{
    switch (faceProperty)
    {
    case DMP_COLOR:
	return "Dmp color";
    default:
	return "Error";
    }
}

const char* BodyOrFacePropertyToString (size_t i)
{
    if (i < BodyProperty::COUNT)
	return BodyProperty::ToString (BodyProperty::Enum (i));
    else
	return FaceProperty::ToString (FaceProperty::Enum (i));
}

ViewType::Enum ViewType::FromSizeT (size_t i)
{
    RuntimeAssert (i < COUNT, "Value outside of ViewType::Enum", i);
    return ViewType::Enum (i);
}

bool ViewType::IsTimeDependent (ViewType::Enum t)
{
    switch (t)
    {
    case CENTER_PATHS:
    case AVERAGE:
    case T1S_PDE:
	return true;
    default:
	return false;
    }
}

const char* ViewType::ToString (ViewType::Enum t)
{
    switch (t)
    {
    case EDGES:
	return "Edges";
    case EDGES_TORUS:
	return "Edges torus";
    case FACES_TORUS:
	return "Faces torus";
    case FACES:
	return "Faces";	
    case CENTER_PATHS:
	return "Center paths";
    case AVERAGE:
	return "Faces statistics";
    case T1S_PDE:
	return "T1s PDE";
    default:
	return "Invalid ViewType";
    }
}

Palette::Palette () :
    m_type (PaletteType::SEQUENTIAL),
    m_sequential (PaletteSequential::BLACK_BODY),
    m_diverging (PaletteDiverging::BLUE_RED)
{
}

Palette::Palette (PaletteType::Enum type, int palette)
{
    m_type = type;
    m_sequential = PaletteSequential::BLACK_BODY;
    m_diverging = PaletteDiverging::BLUE_RED;
    if (type == PaletteType::SEQUENTIAL)
	m_sequential = PaletteSequential::Enum (palette);
    else
	m_diverging = PaletteDiverging::Enum (palette);
}

Palette::Palette (PaletteType::Enum type, PaletteSequential::Enum sequential,
		  PaletteDiverging::Enum diverging) :
    m_type (type),
    m_sequential (sequential),
    m_diverging (diverging)
{
}

string Palette::ToString () const
{
    ostringstream ostr;
    ostr << PaletteType::ToString (m_type) << ", ";
    if (m_type == PaletteType::SEQUENTIAL)
	ostr << PaletteSequential::ToString (m_sequential);
    else
	ostr << PaletteDiverging::ToString (m_diverging);
    return ostr.str ();
}

const char* PaletteType::ToString (PaletteType::Enum type)
{
    switch (type)
    {
    case SEQUENTIAL:
	return "Sequential";
    case DIVERGING:
	return "Diverging";
    }
}

const char* PaletteSequential::ToString (PaletteSequential::Enum type)
{
    switch (type)
    {
    case BLACK_BODY:
	return "Black body radiator";
    case BREWER_BLUES9:
	return "Blues";
    case BREWER_YLORRD9:
	return "Yellow Orange Red";
    default:
	RuntimeAssert (false, "Invalid Palette: ", type);
	return 0;
    }
}

const char* PaletteDiverging::ToString (PaletteDiverging::Enum type)
{
    switch (type)
    {
    case BLUE_RED:
	return "Blue-red";
    case BLUE_TAN:
	return "Blue-tan";
    case PURPLE_ORANGE:
	return "Purple-orange";
    case GREEN_PURPLE:
	return "Green-purple";
    case GREEN_RED:
	return "Green-red";
    default:
	RuntimeAssert (false, "Invalid Palette: ", type);
	return 0;
    }
}


size_t ViewCount::GetCount (ViewCount::Enum viewCount)
{
    return viewCount + 1;
}


ViewCount::Enum ViewCount::FromSizeT (size_t count)
{
    RuntimeAssert (count <= (MAX + 1),
		   "Value outside of ViewCount::Enum: ", count);
    return ViewCount::Enum (count - 1);
}

GLenum LightType::ToOpenGL (LightType::Enum lightType)
{
    const GLenum glLightType[] = {GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR};
    RuntimeAssert (lightType < LightType::COUNT, 
		   "Invalid LightType: ", lightType);
    return glLightType[lightType];
}
