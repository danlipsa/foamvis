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


void Scale (G3D::AABox* aabox, double change)
{
    using G3D::Vector3;
    Vector3 center = aabox->center ();
    Vector3 newLow = aabox->low () * change + center * (1 - change);
    Vector3 newHigh = aabox->high () * change + center * (1 - change);
    aabox->set (newLow, newHigh);
}

void Scale (G3D::Rect2D* aabox, double change)
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
    double halfSideLength = (aabox->high () - center).length ();
    Vector3 halfDiagonal = halfSideLength * 
	(Vector3::unitX () + Vector3::unitY () + Vector3::unitZ ());
    aabox->set (center - halfDiagonal, center + halfDiagonal);
}

ostream& operator<< (ostream& ostr, const QColor& color)
{
    return ostr << "QColor(" << color.red () << ", " << color.green () << ", "
		<< color.blue () << ")";
}

ostream& operator<< (ostream& ostr, const QwtDoubleInterval& interval)
{
    return ostr 
	<< "QwtDoubleInterval: "
	<< ((interval.borderFlags () & QwtDoubleInterval::ExcludeMinimum) ?
	    "(" : "[")
	<< interval.minValue () << ", "
	<< interval.maxValue ()
	<< ((interval.borderFlags () & QwtDoubleInterval::ExcludeMaximum) ?
	    ")" : "]");
}

ostream& operator<< (ostream& ostr, const vector<bool>& v)
{
    BOOST_FOREACH (bool b, v)
	ostr << (b ? "1" : "0");
    return ostr;
}
