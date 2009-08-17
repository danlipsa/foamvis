#ifndef __COLOR_H__
#define __COLOR_H__

class QColor;

class Color
{
public:
    enum Name
    {
        COLOR_BLACK,    /* dark colors */
        COLOR_BLUE,
        COLOR_GREEN,
        COLOR_CYAN,
        COLOR_RED,
        COLOR_MAGENTA,
        COLOR_BROWN,
        COLOR_LIGHTGRAY,
        COLOR_DARKGRAY, /* light colors */
        COLOR_LIGHTBLUE,
        COLOR_LIGHTGREEN,
        COLOR_LIGHTCYAN,
        COLOR_LIGHTRED,
        COLOR_LIGHTMAGENTA,
        COLOR_YELLOW,
        COLOR_WHITE,
        COLOR_CLEAR,
        COLOR_COUNT
    };
    static const float* GetValue(Color::Name name);
};


#endif
