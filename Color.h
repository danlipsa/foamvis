#ifndef __COLOR_H__
#define __COLOR_H__

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
    static const float* GetValue(Color::Name name);
};


#endif
