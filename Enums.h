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

	ROTATE_LIGHT,
	TRANSLATE_LIGHT,

	SELECT,
	DESELECT,

	COUNT
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
	METHOD_OR_QUANTITY_NAME,
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
	VELOCITY_ALONG_X,
	    PROPERTY_BEGIN = VELOCITY_ALONG_X,
	VELOCITY_ALONG_Y,
	VELOCITY_ALONG_Z,
	VELOCITY_MAGNITUDE,
	NUMBER_OF_SIDES,
	ELONGATION,

	PRESSURE,
	    PER_BODY_BEGIN = PRESSURE,
	TARGET_VOLUME,
	ACTUAL_VOLUME,
	NONE,
	    PER_BODY_END = NONE, PROPERTY_END = NONE,
	COUNT
    };
public:
    static const char* ToString (BodyProperty::Enum property);
    static Enum FromSizeT (size_t i);
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
	FACES_STATISTICS,

	CENTER_PATHS,
        COUNT
    };
    static Enum FromInt (int i);
    static bool IsGlobal (ViewType::Enum t);
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
	/**
	 *  y
	 *    x
	 * z
	 */
	TWO_D,
	/**
	 *  z
	 *    x
	 * y
	 */
	TWO_D_TIME_DISPLACEMENT,
	/**
	 *  z
	 *    y
	 * x
	 */
	TWO_D_ROTATE_RIGHT90,
	/**
	 *  z
	 *    y
	 * x
	 */
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
	FOUR
    };
    static size_t GetCount (ViewCount::Enum);
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
	COLOR
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

#endif //__ENUMS_H__

// Local Variables:
// mode: c++
// End:
