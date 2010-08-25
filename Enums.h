/**
 * @file Enums.h
 * @author Dan R. Lipsa
 * @date 1 May 2010
 * Enums used in the program.
 */

#ifndef __ENUMS_H__
#define __ENUMS_H__

/**
 * Colors of faces or edges
 */
class Color
{
public:
    /**
     * Names of colors
     */
    enum Enum
    {
        BLACK,    /* dark colors */
        BLUE,
        GREEN,
        CYAN,
        RED,
        MAGENTA,
        BROWN,
        LIGHTGRAY,
        DARKGRAY, /* light colors */
        LIGHTBLUE,
        LIGHTGREEN,
        LIGHTCYAN,
        LIGHTRED,
        LIGHTMAGENTA,
        YELLOW,
        WHITE,
        CLEAR,
        COUNT
    };

public:
    /**
     * Gets a vector of RGBA components for the given color
     * @param name color name
     * @return vector of RGBA components for the given color
     */
    static const G3D::Color3& GetValue(Color::Enum name);    
    friend ostream& operator<< (ostream& ostr, Color::Enum name);

private:
    static const G3D::Color3 COLORS[];
};



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
	SELECT_BRUSH,
	SELECT_ERASER,
	INTERACTION_MODE_COUNT
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
 * Types of  attributes defined in a  Surface Evolver DMP  file. Use a
 * struct to enclose the enum.
 */
struct DefineAttribute
{
    /**
     * Types of attributes defined in a Surface Evolver DMP file
     */
    enum Enum {
        VERTEX,
        EDGE,
        FACE,
        BODY,
        COUNT
    };
};

class BodyProperty
{
public:
    enum Enum
    {
	VELOCITY_ALONG_X, 
	    VELOCITY_BEGIN = VELOCITY_ALONG_X, ENUM_BEGIN = VELOCITY_ALONG_X,
	VELOCITY_ALONG_Y,
	VELOCITY_ALONG_Z,
	VELOCITY_MAGNITUDE,
	PRESSURE, VELOCITY_END = PRESSURE, PER_BODY_BEGIN = PRESSURE,
	VOLUME,
	NONE, PER_BODY_END = NONE,
	COUNT
    };
public:
    static const char* ToString (BodyProperty::Enum property);
    static Enum FromSizeT (size_t i);
};





#endif //__ENUMS_H__

// Local Variables:
// mode: c++
// End:
