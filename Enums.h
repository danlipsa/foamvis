/**
 * @file Enums.h
 * @author Dan R. Lipsa
 * @date 1 May 2010
 * @brief Enums used in the program.
 * @ingroup utils
 */

#ifndef __ENUMS_H__
#define __ENUMS_H__

/**
 * @brief Specifies if this is an element stored in the dmp file or a duplicate
 * 
 */
struct ElementStatus
{
    enum Enum
    {
	/**
	 * Stored in the dmp file, no duplicate was made
	 */
	ORIGINAL,
	/**
	 * Not in the dmp file, a duplicate of an element in the dmp file
	 */
	DUPLICATE
    };
    friend ostream& operator<< (ostream& ostr,
				ElementStatus::Enum duplicateStatus);    
};


/**
 * @brief Ways to interact with the data (navigation operations)
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

/**
 * @brief Object the user interacts with (focus, context, light, grid)
 */
struct InteractionObject
{
    enum Enum {
	FOCUS,
	CONTEXT,
	LIGHT,
	GRID
    };
};


/**
 * @brief Types of attributes for vertices, edges, faces and bodies
 */
struct AttributeType
{
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
 * @brief Body scalars.
 */
class BodyScalar
{
public:
    enum Enum
    {
	VELOCITY_X,     PROPERTY_BEGIN = VELOCITY_X,
	VELOCITY_Y,
	VELOCITY_Z,
	VELOCITY_MAGNITUDE,

	SIDES_PER_BUBBLE,   // edges per face for 2D, faces per body for 3D
	DEFORMATION_SIMPLE, // P / sqrt (A) for 2D, A / V^(2/3) for 3D
	DEFORMATION_EIGEN,  // l - l_min / l_max where (l_i are the eigen values
	                    // for the deformation tensor
	PRESSURE,       DMP_BEGIN = PRESSURE,
	TARGET_VOLUME,
	ACTUAL_VOLUME,
	GROWTH_RATE,
        COUNT                // 11
    };
public:
    static const char* ToString (BodyScalar::Enum property);
    static Enum FromSizeT (size_t i);

private:
    static boost::array<const char*, COUNT> NAME;
};

/**
 * @brief Vector and tensor attributes (enum values starts after the
 *        last BodyScalar value)
 */
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
	COUNT                // 13
    };
    static const char* ToString (BodyAttribute::Enum attribute);
    static const char* ToString (size_t attribute);
    static string ValueToString (size_t attribute, float* value);
    static size_t DependsOn (size_t attribute);
    static ConvertType Convert (size_t attribute);

    static size_t GetNumberOfComponents (BodyAttribute::Enum attribute);
    static size_t GetNumberOfComponents (size_t attribute);
    static BodyAttribute::Enum FromSizeT (size_t i);


    static vtkDataSetAttributes::AttributeTypes GetType (size_t attribute);
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

/**
 * @brief Other way to display a body (DMP_COLOR) or compute an
 *        average (T1_KDE) that is conveniently stored as a body attribute.
 */
class OtherScalar
{
public:
    enum Enum
    {
	DMP_COLOR = BodyAttribute::COUNT,
        T1_KDE,
	COUNT                // 15
    };
public:
    static const char* ToString (OtherScalar::Enum property);
    static const char* ToString (size_t property);
    static Enum FromSizeT (size_t i);
};

/**
 * @brief Options for displaying a Histogram
 */
class HistogramType
{
public:
    enum Option
    {
	UNICOLOR_TIME_STEP = 0x0,
	COLOR_MAPPED = 0x1,
	ALL_TIME_STEPS_SHOWN = 0x2
    };
    Q_DECLARE_FLAGS (Options, Option);
};
Q_DECLARE_OPERATORS_FOR_FLAGS (HistogramType::Options);

/**
 * @brief Types of palettes for color maps
 */
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

/**
 * @brief Types of sequencial palettes for color maps
 */
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

/**
 * @brief Types of diverging palettes for color maps
 */
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

/**
 * @brief A palette (sequential or diverging) for color maps
 */
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


/**
 * @brief Type of visualization used.
 */
class ViewType
{
public:
    enum Enum {
        EDGES,
        FACES,
	BUBBLE_PATHS,
	AVERAGE,
	T1_KDE,
        COUNT
    };
    static Enum FromSizeT (size_t i);
    static bool IsTimeDependent (ViewType::Enum t);
    static const char* ToString (ViewType::Enum viewType);
};

/**
 * @brief Type of vector visualizations
 */
struct VectorVis
{
    enum Enum
    {
        GLYPH,
        STREAMLINE,
        PATHLINE,
        COUNT
    };
};

/**
 * @brief Type of edge visualization used
 */
class EdgeVis
{
public:
    enum Enum
    {
        EDGE_NORMAL,
        EDGE_TORUS,
        EDGE_TORUS_FACE
    };
};

/**
 * @brief Type of statistics visualization used (average, min or max)
 */
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

/**
 * @brief Types of averages (do we use a per voxel or a global count).
 *
 * We have two type of averages. For the LOCAL type, a count is stored in each
 * voxel. This is used for average of attributes where a voxel is not covered
 * in each time step. For the GLOBAL type, a global count is used, equal with
 * the number of steps in Average::GetCurrentTimeWindow. This is used for T1KDE.
 */
struct AverageCountType
{
    enum Enum
    {
        LOCAL,
        GLOBAL
    };
};

/**
 * @brief Transformations we apply to the data before we display it.
 */
struct AxisOrderName
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

/**
 * @brief Light number
 */
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

/**
 * @brief Body selector type
 */
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
 * @brief Location of a point in a strip of segments.
 */
struct StripPointLocation
{
    enum Enum
    {
	BEGIN_POINT,
	MIDDLE_POINT,
	END_POINT,
	COUNT
    };
};

/**
 * @brief Specifies which ends of a tube segment are perpendicular on it.
 */
struct SegmentPerpendicularEnd
{
    enum Enum
    {
	BEGIN_SEGMENT,
	END_SEGMENT,
	BEGIN_END_SEGMENT,
	NONE,
	COUNT
    };
};

/**
 * @brief Number of views displayed
 */
struct ViewCount
{
    enum Enum
    {
	ONE = 1,
	TWO,
	THREE,
	FOUR,
	COUNT
    };
    static ViewCount::Enum FromSizeT (size_t count);
};

/**
 * @brief Are views laid out horizontally or vertically
 */
struct ViewLayout
{
    enum Enum
    {
	HORIZONTAL,
	VERTICAL
    };
};

/**
 * @brief View number
 */
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
    static const char* ToString (ViewNumber::Enum viewNumber);
};

/**
 * @brief Highlight color number. Each palette has its own highlight
 *        colors that can be used with the colors used in the palette.
 */
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

/**
 * @brief Light type (ambient, diffuse, specular)
 */
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

/**
 * @brief Color (red, green or blue)
 */
struct ColorNumber
{
    enum Enum
    {
	RED,
	GREEN,
	BLUE
    };
};

/**
 * @brief Window size. We can set either the window or the view to be
 *        720x480 pixels. Used for producing movies.
 */
struct WindowSize
{
    enum Enum
    {
	WINDOW_720x480,
	GL_720x480
    };
};

/**
 * @brief Body attributes index for attributes present in the DMP file.
 */
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

/**
 * @brief Face attributes index for attributes present in the DMP file.
 */
struct FaceAttributeIndex
{
    enum Enum
    {
	COLOR,
	AREA,
	CONSTRAINTS
    };
};

/**
 * @brief Edge attributes index for attributes present in the DMP file.
 */
struct EdgeAttributeIndex
{
    enum Enum
    {
	COLOR,
	CONSTRAINTS
    };
};

/**
 * @brief Vertex attributes index for attributes present in the DMP file.
 */
struct VertexAttributeIndex
{
    enum Enum
    {
	CONSTRAINTS
    };
};

/**
 * @brief If the viewing volume encloses a rotation of the data
 *        bounding box or not.
 */
struct ViewingVolumeOperation
{
    enum Enum
    {
	DONT_ENCLOSE2D,
	ENCLOSE2D
    };
};

/**
 * @brief Types of color maps for scalar time-step or average visualizations.
 */
struct ColorMapScalarType
{
    enum Enum
    {
	PROPERTY,
	STATISTICS_COUNT,
	T1_KDE,
	NONE
    };
};

/**
 * @brief Time linkage
 */
struct TimeLinkage
{
    enum Enum
    {
	INDEPENDENT,
	LINKED,
    };
};


/**
 * @brief Where do we duplicate the original domain
 */
struct DuplicateDomain
{
    enum Enum
    {
        LEFT,
        RIGHT,
        TOP,
        BOTTOM,
        COUNT
    };
};

/**
 * @brief VTK pipeline type (only average 3d for now)
 */
struct PipelineType
{
    enum Enum
    {
        AVERAGE_3D,
        COUNT
    };
};

/**
 * @brief Types of force and torque displayed
 */
struct ForceType
{
    enum Enum
    {
        NETWORK,
        PRESSURE,
        RESULT,
        DIFFERENCE,
        COUNT
    };
    static Enum FromSizeT (size_t i);
};

// a region is all bins i such that first <= i < second
typedef vector<pair<size_t, size_t> > BinRegions;

/**
 * @brief Type of topological changes
 */
struct T1Type
{
    /**
     * 2D: POP_VERTEX
     * 3D: any of these, COUNT means the type is unknown
     */
    enum Enum
    {
        QUAD_TO_QUAD,
        TRI_TO_EDGE,
        EDGE_TO_TRI,
        POP_EDGE,
        POP_VERTEX,
        COUNT
    };
    static Enum FromSizeT (size_t i);
    static QColor ToColor (T1Type::Enum);
    static const char* ToString (T1Type::Enum type);
    static vtkSmartPointer<vtkLookupTable> GetLookupTable ();

private:
    static boost::array<const char*, COUNT> NAME;
    static boost::array<QColor, COUNT> COLOR;
};


/**
 * @brief Data dimension
 */
class Dimension
{
public:
    enum Enum
    {
        D2D = 2,
        D3D
    };
};

/**
 * @brief What is part of context (everything or unselected objects)
 */
class Context
{
public:
    enum Enum
    {
        UNSELECTED,
        ALL
    };
};

/**
 * @brief If the context is invisible or not
 */
class ContextInvisible
{
public:
    enum Enum
    {
	USER_DEFINED,
        ALWAYS
    };
};

/**
 * @brief What do we average (scalar, vector, tensor, t1kde)
 */
struct AverageType
{
    enum Enum
    {
        SCALAR,
        T1KDE,
        VECTOR,
        TENSOR,
        COUNT
    };
    static const char* ToString (AverageType::Enum type);
};


#endif //__ENUMS_H__

// Local Variables:
// mode: c++
// End:
