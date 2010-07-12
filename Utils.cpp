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
    double color = max (
	0.0,
	((end - begin + 2) - (abs (value - begin) + abs (value - end))) / 2);
    return color;
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

void Scale (G3D::AABox* aabox, float change)
{
    using G3D::Vector3;
    Vector3 center = aabox->center ();
    Vector3 newLow = aabox->low () * change + center * (1 - change);
    Vector3 newHigh = aabox->high () * change + center * (1 - change);
    aabox->set (newLow, newHigh);
}

void Scale (G3D::Rect2D* aabox, float change)
{
    using G3D::Vector2;
    Vector2 center = aabox->center ();
    Vector2 newLow = aabox->x0y0 () * change + center * (1 - change);
    Vector2 newHigh = aabox->x1y1 () * change + center * (1 - change);
    *aabox = G3D::Rect2D::xyxy ( newLow, newHigh);
}

void EncloseRotation (G3D::AABox* aabox)
{
    using G3D::Vector3;
    Vector3 center = aabox->center ();
    float halfSideLength = (aabox->high () - center).length ();
    Vector3 halfDiagonal = halfSideLength * 
	(Vector3::unitX () + Vector3::unitY () + Vector3::unitZ ());
    aabox->set (center - halfDiagonal, center + halfDiagonal);
}
