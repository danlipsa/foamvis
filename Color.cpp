/**
 * @file   Color.h
 * @author Dan R. Lipsa
 * 
 * Definitions for class members for the class describing the color of faces
 */
#include "Color.h"

float colors[][4] = 
{
    {0.0,0.0,0.0,1.},
    {0.0,0.0,1.,1.},
    {0.0,1.,0.0,1.},
    {0.0,1.,1.,1.}, 
    {1.,0.0,0.0,1.},
    {1.,0.0,1.,1.},
    {1.,0.5,0.,1.},
    {.6f,.6f,.6f,1.},
    {.3f,.3f,.3f,1.},
    {.3f,.8f,1.,1.}, 
    {.5,1.,.5,1.},
    {.5,1.,1.,1.},
    {1.,.5,.5,1.},
    {1.,.5,1.,1.},
    {1.,1.,.0,1.},
    {1.,1.,1.,1.},
    {0.,0.,0.,0.}
};

ostream& operator<< (ostream& ostr, Color::Name color)
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
	ostr << "Invalid color";
	break;
    }
    return ostr;
}

float* Color::GetValue(Color::Name color)
{
    return colors[color];
}
