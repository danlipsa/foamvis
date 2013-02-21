/**
 * @file   Enums.cpp
 * @author Dan R. Lipsa
 * @date 13 May 2010
 *
 * Implementations for functionality declared in Enums.h
 */

#include "DataProperties.h"
#include "Debug.h"
#include "Enums.h"
#include "Utils.h"

// Private Classes/Functions
// ======================================================================

template<size_t component>
void vectorExtract (
    double from[BodyAttribute::MAX_NUMBER_OF_COMPONENTS], 
    double to[BodyAttribute::MAX_NUMBER_OF_COMPONENTS])
{
    to[0] = from[component];
}

void vectorMagnitude (
    double from[BodyAttribute::MAX_NUMBER_OF_COMPONENTS], 
    double to[BodyAttribute::MAX_NUMBER_OF_COMPONENTS])
{
    to[0] = sqrt (from[0] * from[0] + from[1] * from[1] + from[2] * from[2]);
}

void deformationEigen (
    double from[BodyAttribute::MAX_NUMBER_OF_COMPONENTS], 
    double to[BodyAttribute::MAX_NUMBER_OF_COMPONENTS])
{
    SymmetricMatrixEigen c;
    float values[3];
    G3D::Vector3 vectors[3];
    c.Calculate (G3D::Matrix3 (from[0], from[1], from[2],
			       from[3], from[4], from[5],
			       from[6], from[7], from[8]),
		 values, vectors);
    to[0] = 1.0 - values[2] / values[0];
}



// Methods ElementStatus
// ======================================================================

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

// Methods AttributeType
// ======================================================================

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

// Methods BodyScalar
// ======================================================================
boost::array<const char*, BodyScalar::COUNT> BodyScalar::NAME = {{
	"Velocity along x",
	"Velocity along y",
	"Velocity along z",
	"Velocity magnitude",
	"Edges per face",
	"Deformation", // P / sqrt(A) (2D) or A / V^(2/3) (3D)
	"Deformation eigen",
	"Pressure",
	"Target volume",
	"Actual volume",
	"Growth rate"
    }};


const char* BodyScalar::ToString (BodyScalar::Enum property)
{
    RuntimeAssert (property < static_cast<int> (NAME.size ()), 
		   "Invalid BodyScalar: ", property);
    return NAME[property];
}

BodyScalar::Enum BodyScalar::FromSizeT (size_t i)
{
    RuntimeAssert (i < COUNT, "Value outside of BodyScalar::Enum: ", i);
    return BodyScalar::Enum (i);
}

// Methods FaceScalar
// ======================================================================
const char* FaceScalar::ToString (FaceScalar::Enum faceProperty)
{
    switch (faceProperty)
    {
    case DMP_COLOR:
	return "Dmp color";
    default:
	return "Error";
    }
}

const char* FaceScalar::ToString (size_t i)
{
    if (i < BodyScalar::COUNT)
	return BodyScalar::ToString (BodyScalar::FromSizeT (i));
    else
	return FaceScalar::ToString (FaceScalar::FromSizeT (i));
}

FaceScalar::Enum FaceScalar::FromSizeT (size_t i)
{
    RuntimeAssert (i < COUNT && i >= BodyScalar::COUNT, 
		   "Value outside of FaceScalar::Enum: ", i);
    return FaceScalar::Enum (i);
}

// Methods BodyAttribute
// ======================================================================
boost::array<BodyAttribute::DependsOnInfo, BodyAttribute::COUNT> 
BodyAttribute::DEPENDS_ON_INFO = {{
	{BodyAttribute::VELOCITY, vectorExtract<0>},
	{BodyAttribute::VELOCITY, vectorExtract<1>},
	{BodyAttribute::VELOCITY, vectorExtract<2>},
	{BodyAttribute::VELOCITY, vectorMagnitude},
	{COUNT, 0}, // SIDES_PER_BUBBLE
	{COUNT, 0}, // DEFORMATION_SIMPLE
	{COUNT, 0}, //{BodyAttribute::DEFORMATION, deformationEigen}, // too long
	{COUNT, 0}, // PRESSURE
	{COUNT, 0}, // TARGET_VOLUME
	{COUNT, 0}, // ACTUAL_VOLUME
	{COUNT, 0}, // GROWTH_RATE
	{COUNT, 0}, // VELOCITY
	{COUNT, 0}  // DEFORMATION
    }};

boost::array<BodyAttribute::Info, BodyAttribute::COUNT> BodyAttribute::INFO = {{
	{BodyAttribute::VELOCITY, "Velocity", 
	 BodyAttribute::VECTOR_NUMBER_OF_COMPONENTS},
	{BodyAttribute::DEFORMATION, "Deformation", 
	 BodyAttribute::TENSOR_NUMBER_OF_COMPONENTS}
    }};

vtkDataSetAttributes::AttributeTypes BodyAttribute::GetType (size_t attribute)
{
    size_t components = GetNumberOfComponents (attribute);
    switch (components)
    {
    case SCALAR_NUMBER_OF_COMPONENTS:
	return vtkDataSetAttributes::SCALARS;
    case VECTOR_NUMBER_OF_COMPONENTS:
	return vtkDataSetAttributes::VECTORS;
    case TENSOR_NUMBER_OF_COMPONENTS:
	return vtkDataSetAttributes::TENSORS;
    default:
	return vtkDataSetAttributes::NUM_ATTRIBUTES;
    }
}

size_t BodyAttribute::DependsOn (size_t attribute)
{
    return DEPENDS_ON_INFO[attribute].m_dependsOnAttribute;
}

BodyAttribute::ConvertType BodyAttribute::Convert (size_t attribute)
{
    return DEPENDS_ON_INFO[attribute].m_convert;
}

BodyAttribute::Enum BodyAttribute::FromSizeT (size_t i)
{
    RuntimeAssert (i < COUNT && i >= BodyScalar::COUNT, 
		   "Value outside of BodyAttribute::Enum: ", i);
    return BodyAttribute::Enum (i);
}


const char* BodyAttribute::ToString (BodyAttribute::Enum attribute)
{
    return INFO[attribute - BodyScalar::COUNT].m_name;
}

string BodyAttribute::ValueToString (size_t attribute, float* value)
{
    ostringstream ostr;
    size_t numberOfComponents = BodyAttribute::GetNumberOfComponents (attribute);
    if (numberOfComponents == 1)
	ostr << *value;
    else if (numberOfComponents == 3)
    {
	G3D::Vector3 v(value);
	ostr << v;
    }
    else if (numberOfComponents == 9)
    {
	G3D::Matrix3 m (value[0], value[1], value[2],
			value[3], value[4], value[5],
			value[6], value[7], value[8]);
	ostr << m;
    }
    else
	ThrowException ("Invalid number of components: ", numberOfComponents);
    return ostr.str ();
}

const char* BodyAttribute::ToString (size_t attribute)
{
    if (attribute < BodyScalar::COUNT)
	return BodyScalar::ToString (BodyScalar::FromSizeT (attribute));
    else
	return BodyAttribute::ToString (BodyAttribute::FromSizeT (attribute));
}

bool BodyAttribute::IsRedundant (size_t attribute)
{
    return BodyAttribute::DependsOn (attribute) != COUNT;
}


size_t BodyAttribute::GetNumberOfComponents (BodyAttribute::Enum attribute)
{
    return INFO[attribute  - BodyScalar::COUNT].m_numberOfComponents;
}

size_t BodyAttribute::GetNumberOfComponents (size_t attribute)
{
    if (attribute < BodyScalar::COUNT)
	return 1;
    else
	return GetNumberOfComponents (BodyAttribute::FromSizeT (attribute));
}

// Methods ViewNumber
// ======================================================================
ViewNumber::Enum ViewNumber::FromSizeT (size_t i)
{
    RuntimeAssert (i < ViewNumber::COUNT,
		   "Value outside of ViewNumber::Enum", i);
    return ViewNumber::Enum (i);
}

// Methods ViewType
// ======================================================================

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
    case T1S_KDE:
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
	return "Average";
    case T1S_KDE:
	return "T1s KDE";
    default:
	return "Invalid ViewType";
    }
}

// Methods Palette
// ======================================================================

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

// Methods PaletteType
// ======================================================================

const char* PaletteType::ToString (PaletteType::Enum type)
{
    switch (type)
    {
    case SEQUENTIAL:
	return "Sequential";
    case DIVERGING:
	return "Diverging";
    }
    return "Invalid";
}

// Methods PaletteSequential
// ======================================================================

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

// Methods PaletteDiverging
// ======================================================================

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

// Methods ViewCount
// ======================================================================


ViewCount::Enum ViewCount::FromSizeT (size_t count)
{
    RuntimeAssert (count < COUNT,
		   "Value outside of ViewCount::Enum: ", count);
    return ViewCount::Enum (count);
}

// Methods LightType
// ======================================================================

GLenum LightType::ToOpenGL (LightType::Enum lightType)
{
    const GLenum glLightType[] = {GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR};
    RuntimeAssert (lightType < LightType::COUNT, 
		   "Invalid LightType: ", lightType);
    return glLightType[lightType];
}

// Methods LightNumber
// ======================================================================
LightNumber::Enum LightNumber::FromSizeT (size_t i)
{
    RuntimeAssert (i <= LightNumber::COUNT,
		   "Value outside of LightNumber::Enum: ", i);
    return LightNumber::Enum (i);
}

// Methods ForceType
// ======================================================================
ForceType::Enum ForceType::FromSizeT (size_t i)
{
    RuntimeAssert (i <= ForceType::COUNT,
		   "Value outside of ForceType::Enum: ", i);
    return ForceType::Enum (i);
}
