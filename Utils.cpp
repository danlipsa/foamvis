/**
 * @file   Utils.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of various utility functions
 */

#include "Edge.h"
#include "Face.h"
#include "DebugStream.h"
#include "Utils.h"
#include "Vertex.h"


const G3D::Vector3int16 Vector3int16Zero (0, 0, 0);

ostream& operator<< (ostream& ostr, const G3D::AABox& box)
{
    cdbg << "AABox(" << box.low () << ", " << box.high () << ")";
    return ostr;
}


ostream& operator<< (ostream& ostr, const G3D::Rect2D& box)
{
    cdbg << "Rect2D (" << box.x0y0 () << ", " << box.x1y1 () << ")";
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

ostream& operator<< (ostream& ostr, const G3D::Matrix4& m)
{
    return ostr
	<< "G3D::Matrix4 (" << endl
	<< m.row (0) << endl
	<< m.row (1) << endl
	<< m.row (2) << endl
	<< m.row (3) << ")" << endl;
}

ostream& operator<< (ostream& ostr, const G3D::Vector4& v)
{
    return ostr << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3];
}

ostream& operator<< (ostream& ostr, const G3D::Vector2& v)
{
    return ostr << v[0] << ", " << v[1];
}

bool isFuzzyZero (const G3D::Vector3& v)
{
    return v.squaredMagnitude () < fuzzyEpsilon * fuzzyEpsilon;
}

template<typename U, typename V>
ostream& operator<< (ostream& ostr, const pair<U, V>& p)
{
    return ostr << "pair(" << p.first << ", " << p.second << ")";
}

template <typename Container, 
	  typename ContainerIterator,
	  typename ContainerKeyType>
ContainerIterator fuzzyFind (const Container& s, const ContainerKeyType& x)
{
    ContainerIterator it = s.lower_bound (x);
    if (it != s.end () && (*it)->fuzzyEq (*x))
	return it;
    if (it != s.begin ())
    {
	--it;
	if ((*(it))->fuzzyEq (*x))
	    return it;
    }
    return s.end ();
}



// Template instantiations
//======================================================================

template std::_Rb_tree_const_iterator<boost::shared_ptr<Edge> > fuzzyFind<std::set<boost::shared_ptr<Edge>, EdgeLessThan, std::allocator<boost::shared_ptr<Edge> > >, std::_Rb_tree_const_iterator<boost::shared_ptr<Edge> >, boost::shared_ptr<Edge> >(std::set<boost::shared_ptr<Edge>, EdgeLessThan, std::allocator<boost::shared_ptr<Edge> > > const&, boost::shared_ptr<Edge> const&);

template std::_Rb_tree_const_iterator<boost::shared_ptr<Face> > fuzzyFind<std::set<boost::shared_ptr<Face>, FaceLessThan, std::allocator<boost::shared_ptr<Face> > >, std::_Rb_tree_const_iterator<boost::shared_ptr<Face> >, boost::shared_ptr<Face> >(std::set<boost::shared_ptr<Face>, FaceLessThan, std::allocator<boost::shared_ptr<Face> > > const&, boost::shared_ptr<Face> const&);

template std::_Rb_tree_const_iterator<boost::shared_ptr<Vertex> > fuzzyFind<std::set<boost::shared_ptr<Vertex>, VertexLessThan, std::allocator<boost::shared_ptr<Vertex> > >, std::_Rb_tree_const_iterator<boost::shared_ptr<Vertex> >, boost::shared_ptr<Vertex> >(std::set<boost::shared_ptr<Vertex>, VertexLessThan, std::allocator<boost::shared_ptr<Vertex> > > const&, boost::shared_ptr<Vertex> const&);
