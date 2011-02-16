/**
 * @file   ParsingEnums.cpp
 * @author Dan R. Lipsa
 * @date 21 Jan. 2011
 *
 * Implementations for functionality declared in ParsingEnums.h
 */

#include "ParsingEnums.h"

const QColor Color::COLORS[] = 
{
    QColor::fromRgbF (0.0, 0.0, 0.0),  // BLACK
    QColor::fromRgbF (0.0, 0.0, 1.0),  // BLUE
    QColor::fromRgbF (0.0, 1.0, 0.0),  // GREEN
    QColor::fromRgbF (0.0, 1.0, 1.0),  // CYAN
    QColor::fromRgbF (1.0, 0.0, 0.0),  // RED
    QColor::fromRgbF (1.0, 0.0, 1.0),  // MAGENTA
    QColor::fromRgbF (1.0, 0.5, 0.0),  // BROWN
    QColor::fromRgbF (.6f, .6f, .6f),  // LIGHTGRAY
    QColor::fromRgbF (.3f, .3f, .3f),  // DARKGRAY
    QColor::fromRgbF (.3f, .8f, 1.0),  // LIGHTBLUE
    QColor::fromRgbF (0.5, 1.0, 0.5),  // LIGHTGREEN
    QColor::fromRgbF (0.5, 1.0, 1.0),  // LIGHTCYAN
    QColor::fromRgbF (1.0, 0.5, 0.5),  // LIGHTRED
    QColor::fromRgbF (1.0, 0.5, 1.0),  // LIGHTMAGENTA
    QColor::fromRgbF (1.0, 1.0, 0.0),  // YELLOW
    QColor::fromRgbF (1.0, 1.0, 1.0),  // WHITE
    QColor::fromRgbF (0.0, 0.0, 0.0)   // CLEAR
};

ostream& operator<< (ostream& ostr, Color::Enum color)
{
    switch (color)
    {
    case Color::BLACK:
	ostr << "Black";
	break;
    case Color::BLUE:
	ostr << "Blue";
	break;
    case Color::GREEN:
	ostr << "Green";
	break;
    case Color::CYAN:
	ostr << "Cyan";
	break;
    case Color::RED:
	ostr << "Red";
	break;
    case Color::MAGENTA:
	ostr << "Magenta";
	break;
    case Color::BROWN:
	ostr << "Brown";
	break;
    case Color::LIGHTGRAY:
	ostr << "Lightgray";
	break;
    case Color::DARKGRAY:
	ostr << "Darkgray";
	break;
    case Color::LIGHTBLUE:
	ostr << "Lightblue";
	break;
    case Color::LIGHTGREEN:
	ostr << "Lightgreen";
	break;
    case Color::LIGHTCYAN:
	ostr << "Lightcyan";
	break;
    case Color::LIGHTRED:
	ostr << "Lightred";
	break;
    case Color::LIGHTMAGENTA:
	ostr << "Lightmagenta";
	break;
    case Color::YELLOW:
	ostr << "Yellow";
	break;
    case Color::WHITE:
	ostr << "White";
	break;
    case Color::CLEAR:
	ostr << "Clear";
	break;
    default:
	ostr << "Invalid";
	break;
    }
    return ostr;
}

const QColor& Color::GetValue(Color::Enum color)
{
    return COLORS[color];
}
