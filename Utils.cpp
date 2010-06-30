/**
 * @file   Utils.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of various utility functions
 */

#include "Utils.h"
#include "DebugStream.h"

const G3D::Vector3int16 Vector3int16Zero (0, 0, 0);

ostream& operator<< (ostream& ostr, const G3D::AABox& box)
{
    cdbg << "AABox(" << box.low () << ", " << box.high () << ")";
    return ostr;
}


const G3D::Vector3int16& Vector3int16Unit (size_t direction)
{
    static const G3D::Vector3int16 unitVector3int16[3] = {
	G3D::Vector3int16 (1, 0, 0),
	G3D::Vector3int16 (0, 1, 0),
	G3D::Vector3int16 (0, 0, 1)
    };
    return unitVector3int16[direction];
}

/*
 * @return 1 if value is between begin and end,
 *         0 if value is less than begin - 1 or greater than end + 1
 *         value linearly scaled between [0, 1] if value is in [begin - 1, begin]
 *         or [end, end+1]
 */
double trapezoid (double value, size_t begin, size_t end)
{
    return max (
	0.0, ((end - begin + 2) - (abs (value - begin) + abs (value - end)))
	/ 2);
}

void RainbowColor (double f, QColor* color)
{
    const double dx = 0.8;

    f = (f < 0) ? 0 : (f > 1) ? 1 : f; // clamp f in [0, 1]
    double g = (6 - 2*dx) * f + dx;    // scale f to [dx, 6 - dx]

    color->setRedF (trapezoid (g, 4, 5));
    color->setGreenF (trapezoid (g, 2, 4));
    color->setBlueF (trapezoid (g, 1, 2));
}
