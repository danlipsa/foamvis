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

boost::array<const char*, BodyScalar::COUNT> BodyScalar::NAME2D = {{
	"Velocity along x",
	"Velocity along y",
	"Velocity along z",
	"Velocity magnitude",
	"Edges per face",
	"Deformation P / sqrt(A)",
	"Deformation eigen",
	"Pressure",
	"Target volume",
	"Actual volume"
    }};

boost::array<const char*, BodyScalar::COUNT> BodyScalar::NAME3D = {{
	"Velocity along x",
	"Velocity along y",
	"Velocity along z",
	"Velocity magnitude",
	"Faces per body",
	"Deformation A / V^(2/3)",
	"Deformation eigen",
	"Pressure",
	"Target volume",
	"Actual volume",
    }};

boost::array<const char*, BodyScalar::COUNT>* BodyScalar::NAME = &NAME2D;

void BodyScalar::Set3D ()
{
    NAME = &NAME3D;
}

const char* BodyScalar::ToString (BodyScalar::Enum property)
{
    RuntimeAssert (property < static_cast<int> (NAME->size ()), 
		   "Invalid BodyScalar: ", property);
    return (*NAME)[property];
}

BodyScalar::Enum BodyScalar::FromSizeT (size_t i)
{
    RuntimeAssert (i < COUNT, "Value outside of BodyScalar::Enum: ", i);
    return BodyScalar::Enum (i);
}

const char* DisplayFaceScalar::ToString (DisplayFaceScalar::Enum faceProperty)
{
    switch (faceProperty)
    {
    case DMP_COLOR:
	return "Dmp color";
    default:
	return "Error";
    }
}

boost::array<BodyAttribute::Info,BodyAttribute::COUNT> BodyAttribute::INFO = {{
	{BodyAttribute::SIDES_PER_BUBBLE,
	 BodyScalar::ToString (BodyScalar::SIDES_PER_BUBBLE), 1},
	{BodyAttribute::DEFORMATION_SIMPLE,
	 BodyScalar::ToString (BodyScalar::DEFORMATION_SIMPLE), 1},
	{BodyAttribute::DEFORMATION_EIGEN,
	 BodyScalar::ToString (BodyScalar::DEFORMATION_EIGEN), 1},
	{BodyAttribute::PRESSURE,
	 BodyScalar::ToString (BodyScalar::PRESSURE), 1},
	{BodyAttribute::TARGET_VOLUME,
	 BodyScalar::ToString (BodyScalar::TARGET_VOLUME),1},
	{BodyAttribute::ACTUAL_VOLUME,
	 BodyScalar::ToString (BodyScalar::ACTUAL_VOLUME),1},
	{BodyAttribute::VELOCITY, "Velocity",3},
	{BodyAttribute::DEFORMATION, "Deformation",9}
    }};

const char* BodyAttribute::ToString (BodyAttribute::Enum attribute)
{
    return INFO[attribute].m_name;
}

size_t BodyAttribute::GetNumberOfComponents (BodyAttribute::Enum attribute)
{
    return INFO[attribute].m_numberOfComponents;
}

BodyScalar::Enum BodyAttribute::ToBodyScalar (
    BodyAttribute::Enum attribute)
{
    BodyScalar::Enum v[] = {
	BodyScalar::SIDES_PER_BUBBLE,
	BodyScalar::DEFORMATION_SIMPLE,
	BodyScalar::DEFORMATION_EIGEN,
	BodyScalar::PRESSURE,
	BodyScalar::TARGET_VOLUME,
	BodyScalar::ACTUAL_VOLUME,
	BodyScalar::COUNT,
	BodyScalar::COUNT,
	BodyScalar::COUNT
    };
    BodyScalar::Enum displayBodyScalar = v[attribute];
    RuntimeAssert (displayBodyScalar != BodyScalar::COUNT,
		   "BodyAttribute::Enum is not a scalar: ", attribute);
    return displayBodyScalar;
}

BodyAttribute::Enum BodyAttribute::FromBodyScalar (
    BodyScalar::Enum s)
{
    BodyAttribute::Enum v[] = {
	BodyAttribute::COUNT,
	BodyAttribute::COUNT,
	BodyAttribute::COUNT,
	BodyAttribute::COUNT,

	BodyAttribute::SIDES_PER_BUBBLE,
	BodyAttribute::DEFORMATION_SIMPLE,
	BodyAttribute::DEFORMATION_EIGEN,
	BodyAttribute::PRESSURE,
	BodyAttribute::TARGET_VOLUME,
	BodyAttribute::ACTUAL_VOLUME,
	BodyAttribute::COUNT
    };
    BodyAttribute::Enum attribute = v[s];
    RuntimeAssert (attribute != BodyAttribute::COUNT,
		   "BodyScalar::Enum is not a body attribute: ", s);
    return attribute;
}




const char* BodyOrFaceScalarToString (size_t i)
{
    if (i < BodyScalar::COUNT)
	return BodyScalar::ToString (BodyScalar::Enum (i));
    else
	return DisplayFaceScalar::ToString (DisplayFaceScalar::Enum (i));
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
