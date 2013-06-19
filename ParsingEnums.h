/**
 * @file ParsingEnums.h
 * @author Dan R. Lipsa
 * @date 21 Jan 2011
 * @brief Enums used for parsing
 * @ingroup parser
 */

#ifndef __PARSING_ENUMS_H__
#define __PARSING_ENUMS_H__

/**
 * @brief Colors of faces or edges
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
    static const QColor& GetValue(Color::Enum name);    
    friend ostream& operator<< (ostream& ostr, Color::Enum name);

private:
    static const QColor COLORS[];
};

/**
 * @brief Types of attributes defined in a Surface Evolver DMP
 *        file.
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



#endif //__PARSING_ENUMS_H__

// Local Variables:
// mode: c++
// End:
