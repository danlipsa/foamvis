/**
 * @file   Color.h
 * @author Dan R. Lipsa
 * 
 * Colors of faces.
 */
#ifndef __COLOR_H__
#define __COLOR_H__

/**
 * Colors of faces
 */
class Color
{
public:
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
    /**
     * Gets a vector of RGBA components for the given color
     * @param name color name
     * @return vector of RGBA components for the given color
     */
    static const float* GetValue(Color::Name name);
};

#endif //__COLOR_H__

// Local Variables:
// mode: c++
// End:
