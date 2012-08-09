/**
 * @file Enums.h
 * @author Dan R. Lipsa
 * @date 1 May 2010
 * Enums used in the program.
 */

#ifndef __ENUMS_H__
#define __ENUMS_H__

/**
 * Status of vertices, edges, faces and bodies.
 * 
 */
struct ElementStatus
{
    enum Enum
    {
	/**
	 * Is in the data file, no duplicate was made
	 */
	ORIGINAL,
	/**
	 * Not in the data file, a duplicate of an element in the data file
	 */
	DUPLICATE
    };
    friend ostream& operator<< (ostream& ostr,
				ElementStatus::Enum duplicateStatus);    
};


/**
 * Ways to interact with the interface
 */
class InteractionMode
{
public:
    /**
     * Names of colors
     */
    enum Enum
    {
	ROTATE,
	SCALE,
	TRANSLATE,

	SELECT,
	DESELECT,

	COUNT
    };
};

struct InteractionObject
{
    enum Enum {
	FOCUS,
	CONTEXT,
	LIGHT,
	GRID
    };
};


struct AttributeType
{
    /**
     * Types of attributes
     */
    enum Enum
    {
        INT,
        REAL,
        COLOR,
        INT_ARRAY,
        REAL_ARRAY,
        COUNT
    };

    /**
     * Pretty print for a Type
     * @param ostr where to print
     * @param type what to print
     * @return where to print next
     */
    friend ostream& operator<< (ostream& ostr, AttributeType::Enum type);
};


/**
 * Body scalars.
 */
class BodyScalar
{
public:
    enum Enum
    {
	VELOCITY_X,
	    PROPERTY_BEGIN = VELOCITY_X,
	VELOCITY_Y,
	VELOCITY_Z,
	VELOCITY_MAGNITUDE,

	SIDES_PER_BUBBLE, // edges per face for 2D, faces per body for 3D
	DEFORMATION_SIMPLE, // P / sqrt (A) for 2D, A / V^(2/3) for 3D
	DEFORMATION_EIGEN,  // l - l_min / l_max where (l_i are the eigen values
	                    // for the deformation tensor
	PRESSURE,
	    DMP_BEGIN = PRESSURE,
	TARGET_VOLUME,
	ACTUAL_VOLUME,
	GROWTH_RATE,
	COUNT
    };
public:
    static const char* ToString (BodyScalar::Enum property);
    static Enum FromSizeT (size_t i);

private:
    static boost::array<const char*, COUNT> NAME2D;
    static boost::array<const char*,COUNT> NAME3D;
    static const boost::array<const char*, COUNT>& NAME ();
};

class FaceScalar
{
public:
    enum Enum
    {
	DMP_COLOR = BodyScalar::COUNT,
	COUNT
    };
public:
    static const char* ToString (FaceScalar::Enum property);
    static const char* ToString (size_t property);
    static Enum FromSizeT (size_t i);
};

struct BodyAttribute
{
    enum NumberOfComponents
    {
	SCALAR_NUMBER_OF_COMPONENTS = 1,
	VECTOR_NUMBER_OF_COMPONENTS = 3,
	TENSOR_NUMBER_OF_COMPONENTS = 9,
	MAX_NUMBER_OF_COMPONENTS = 9
    };
    typedef boost::function<void (
	double[MAX_NUMBER_OF_COMPONENTS], 
	double[MAX_NUMBER_OF_COMPONENTS])> ConvertType;
    enum Enum
    {
	VELOCITY = BodyScalar::COUNT,
	DEFORMATION,
	COUNT
    };
    static const char* ToString (BodyAttribute::Enum attribute);
    static const char* ToString (size_t attribute);
    static size_t DependsOn (size_t attribute);
    static ConvertType Convert (size_t attribute);

    static size_t GetNumberOfComponents (BodyAttribute::Enum attribute);
    static size_t GetNumberOfComponents (size_t attribute);
    static BodyAttribute::Enum FromSizeT (size_t i);


    static bool IsScalar (size_t attribute)
    {
	return GetNumberOfComponents (attribute) == SCALAR_NUMBER_OF_COMPONENTS;
    }
    static bool IsVector (size_t attribute)
    {
	return GetNumberOfComponents (attribute) == VECTOR_NUMBER_OF_COMPONENTS;
    }
    static bool IsTensor (size_t attribute)
    {
	return GetNumberOfComponents (attribute) == TENSOR_NUMBER_OF_COMPONENTS;
    }    
    static bool IsRedundant (size_t attribute);

private:
    struct Info
    {
	Enum m_attribute;
	const char* m_name;
	size_t m_numberOfComponents;
    };
    struct DependsOnInfo
    {
	size_t m_dependsOnAttribute;
	ConvertType m_convert;
    };
    static boost::array<Info, COUNT> INFO;
    static boost::array<DependsOnInfo, COUNT> DEPENDS_ON_INFO;
};


class HistogramType
{
public:
    enum Enum
    {
	NONE,
	UNICOLOR,
	COLOR_CODED
    };
};

class PaletteType
{
public:
    enum Enum
    {
	SEQUENTIAL,
	DIVERGING
    };
    static const char* ToString (PaletteType::Enum type);
};

struct PaletteSequential
{
    enum Enum
    {
	BLACK_BODY,
	BREWER_BLUES9,
	BREWER_YLORRD9,
	COUNT
    };
    static const char* ToString (PaletteSequential::Enum name);
};

struct PaletteDiverging
{
    enum Enum
    {
	BLUE_RED,
	BLUE_TAN,
	PURPLE_ORANGE,
	GREEN_PURPLE,
	GREEN_RED,
	COUNT
    };
    static const char* ToString (PaletteDiverging::Enum name);
};

struct Palette
{
    Palette ();
    Palette (PaletteType::Enum type, int palette);
    Palette (PaletteType::Enum type, PaletteSequential::Enum sequential,
	     PaletteDiverging::Enum diverging);
    PaletteType::Enum m_type;
    PaletteSequential::Enum m_sequential;
    PaletteDiverging::Enum m_diverging;
    string ToString () const;
};

inline ostream& operator<< (ostream& ostr, const Palette& b)
{
    return ostr << b.ToString ();
}


class ViewType
{
public:
    /**
     * WHAT kind of objects do we display
     */
    enum Enum {
        EDGES,
	EDGES_TORUS,
	FACES_TORUS,
        FACES,

	CENTER_PATHS,
	AVERAGE,
	T1S_PDE,
        COUNT
    };
    static Enum FromSizeT (size_t i);
    static bool IsTimeDependent (ViewType::Enum t);
    static const char* ToString (ViewType::Enum viewType);
};


class StatisticsType
{
public:
    enum Enum
    {
	AVERAGE,
	MIN,
	MAX,
	COUNT
    };
};

struct AxesOrder
{
    enum Enum
    {
	TWO_D,
	TWO_D_TIME_DISPLACEMENT,
	TWO_D_ROTATE_RIGHT90,
	TWO_D_ROTATE_RIGHT90_REFLECTION,
	TWO_D_ROTATE_LEFT90,
	THREE_D,
	COUNT
    };
};

struct LightNumber
{
    enum Enum
    {
	LIGHT0,
	LIGHT1,
	LIGHT2,
	LIGHT3,
	COUNT
    };
    static Enum FromSizeT (size_t i);
};

class BodySelectorType
{
public:
    enum Enum
    {
	ALL,
	ID,
	PROPERTY_VALUE,
	COMPOSITE
    };
};

/**
 * Location of a point in a strip of segments.
 */
struct StripPointLocation
{
    enum Enum
    {
	BEGIN,
	MIDDLE,
	END,
	COUNT
    };
};

/**
 * Specifies which ends of a tube segment are perpendicular on it.
 */
struct SegmentPerpendicularEnd
{
    enum Enum
    {
	BEGIN,
	END,
	BEGIN_END,
	NONE,
	COUNT
    };
};

struct ViewCount
{
    enum Enum
    {
	ONE,
	TWO,
	THREE,
	FOUR, MAX = FOUR
    };
    static size_t GetCount (ViewCount::Enum);
    static ViewCount::Enum FromSizeT (size_t count);
};

struct ViewLayout
{
    enum Enum
    {
	HORIZONTAL,
	VERTICAL
    };
};

struct ViewNumber
{
    enum Enum
    {
	VIEW0,
	VIEW1,
	VIEW2,
	VIEW3,
	COUNT
    };
    static Enum FromSizeT (size_t count);
};

struct HighlightNumber
{
    enum Enum
    {
	H0,
	H1,
	H2,
	COUNT
    };
};

struct LightType
{
    enum Enum
    {
	AMBIENT,
	DIFFUSE,
	SPECULAR,
	COUNT
    };
    static GLenum ToOpenGL (LightType::Enum lightType);
};

struct ColorNumber
{
    enum Enum
    {
	RED,
	GREEN,
	BLUE
    };
};

struct WindowSize
{
    enum Enum
    {
	WINDOW_720x480,
	GL_720x480
    };
};


struct BodyAttributeIndex
{
    enum Enum
    {
	PRESSURE,
	TARGET_VOLUME,
	ACTUAL_VOLUME,
	ORIGINAL
    };
};

struct FaceAttributeIndex
{
    enum Enum
    {
	COLOR,
	AREA,
	CONSTRAINTS
    };
};

struct EdgeAttributeIndex
{
    enum Enum
    {
	COLOR,
	CONSTRAINTS
    };
};

struct VertexAttributeIndex
{
    enum Enum
    {
	CONSTRAINTS
    };
};


struct ViewingVolumeOperation
{
    enum Enum
    {
	DONT_ENCLOSE2D,
	ENCLOSE2D
    };
};

struct ColorBarType
{
    enum Enum
    {
	PROPERTY,
	STATISTICS_COUNT,
	T1S_PDE,
	NONE
    };

};

struct TimeLinkage
{
    enum Enum
    {
	INDEPENDENT,
	LINKED,
    };
};




#endif //__ENUMS_H__

// Local Variables:
// mode: c++
// End:
