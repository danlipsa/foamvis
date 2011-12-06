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
 * Body attributes.
 */
class BodyProperty
{
public:
    enum Enum
    {
	VELOCITY_X,
	    PROPERTY_BEGIN = VELOCITY_X,
	VELOCITY_Y,
	VELOCITY_Z,
	VELOCITY_MAGNITUDE,
	SIDES_PER_BODY,
	DEFORMATION_P_OVER_SQRTA,
	DEFORMATION_EIGEN,

	PRESSURE,
	    DMP_BEGIN = PRESSURE,
	TARGET_VOLUME,
	ACTUAL_VOLUME,
	COUNT
    };
public:
    static const char* ToString (BodyProperty::Enum property);
    static Enum FromSizeT (size_t i);
};

class FaceProperty
{
public:
    enum Enum
    {
	DMP_COLOR = BodyProperty::COUNT,
	EDGES_PER_FACE,
	COUNT
    };
public:
    static const char* ToString (FaceProperty::Enum property);
    static Enum FromSizeT (size_t i);
};

const char* BodyOrFacePropertyToString (size_t property);

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

class Palette
{
public:
    enum Enum
    {
	// multihue
	RAINBOW, FIRST = RAINBOW,
	RAINBOW_EXTENDED,
	// sequential
	BLACK_BODY,
	BREWER_BLUE9,
	// diverging
	BLUE_RED_DIVERGING,
	BLUE_TAN_DIVERGING,
	PURPLE_ORANGE_DIVERGING,
	GREEN_PURPLE_DIVERGING,
	GREEN_RED_DIVERGING, LAST = GREEN_RED_DIVERGING
    };
    static const char* ToString (Palette::Enum name);
};

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
	FACES_STATISTICS,
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
	AREA
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
