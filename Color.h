/**
 * @file   Color.h
 * @author Dan R. Lipsa
 * 
 * Declaration of the color class
 */
#ifndef __COLOR_H__
#define __COLOR_H__

/**
 * Colors of faces or edges
 */
class Color
{
public:
    /**
     * Names of colors
     */
    enum Name
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
    static  float* GetValue(Color::Name name);
    friend ostream& operator<< (ostream& ostr, Color::Name name);
};

#endif //__COLOR_H__

// Local Variables:
// mode: c++
// End:
