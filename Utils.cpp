/**
 * @file   Utils.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of various utility functions
 */

#include "Comparisons.h"
#include "Edge.h"
#include "Face.h"
#include "Foam.h"
#include "DebugStream.h"
#include "Utils.h"
#include "Vertex.h"


// Fuzzy equality functionality
// ======================================================================

bool IsFuzzyZero (const G3D::Vector3& v)
{
    return v.squaredMagnitude () < fuzzyEpsilon * fuzzyEpsilon;
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

// ToString functionality
// ======================================================================

ostream& operator<< (ostream& ostr, const G3D::AABox& box)
{
    cdbg << "AABox(" << box.low () << ", " << box.high () << ")";
    return ostr;
}

ostream& operator<< (ostream& ostr, const QBox3D& box)
{
    cdbg << "QBox3D(" << box.minimum () << ", " << box.maximum () << ")";
    return ostr;
}


ostream& operator<< (ostream& ostr, const G3D::Rect2D& box)
{
    cdbg << "Rect2D (" << box.x0y0 () << ", " << box.x1y1 () << ")";
    return ostr;
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

ostream& operator<< (ostream& ostr, const QVector3D& v)
{
    return ostr << "QVector3D(" << v.x () << ", " << v.y () << ", " << v.z () 
		<< ")";
}

ostream& operator<< (ostream& ostr, const QPoint& p)
{
    return ostr << "QPoint (" << p.x () << ", " << p.y () << ")";
}


template<typename U, typename V>
ostream& operator<< (ostream& ostr, const pair<U, V>& p)
{
    return ostr << "pair(" << p.first << ", " << p.second << ")";
}


// Unit vectors
// ======================================================================

const G3D::Vector3int16 Vector3int16Zero (0, 0, 0);

const G3D::Vector3int16& Vector3int16Unit (size_t direction)
{
    static const G3D::Vector3int16 unitVector3int16[3] = {
	G3D::Vector3int16 (1, 0, 0),
	G3D::Vector3int16 (0, 1, 0),
	G3D::Vector3int16 (0, 0, 1)
    };
    return unitVector3int16[direction];
}

// 3D Math functionality
// ======================================================================

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

bool Intersection (
    const QBox3D& box, const QVector3D& _begin, const QVector3D& _end)
{
    QVector3D begin (_begin);
    QVector3D end (_end);
    // 2D case
    if (box.minimum ().z () == 0 && box.maximum ().z () == 0)
    {
	if (qFuzzyCompare (begin.z () + 1, 1))
	    begin.setZ (0);
	if (qFuzzyCompare (end.z () + 1, 1))
	    end.setZ (0);
    }
    QRay3D ray (QVector3D (begin.x (), begin.y (), 0),  end - begin);
    qreal minimum_t, maximum_t;
    if (box.intersection (ray, &minimum_t, &maximum_t))
    {
	const qreal begin_t = 0;
	const qreal end_t = 1;
	bool intersection = ! (maximum_t < begin_t || minimum_t > end_t);
	if (intersection)
	{
	    cdbg << "min_t: " << minimum_t << " max_t: " << maximum_t << endl;
	    cdbg << "begin: " << begin << " end: " << end << endl;
	    cdbg << "box: " << box << endl;
	}
	return intersection;
    }
    else
	return false;
}


// Conversions Qt - G3D
// ======================================================================

QVector2D ToQt (const G3D::Vector2& v)
{
    return QVector2D (v.x, v.y);
}

QVector3D ToQt (const G3D::Vector3& v)
{
    return QVector3D (v.x, v.y, v.z);
}

QBox3D ToQt (const G3D::AABox& box)
{
    return QBox3D (ToQt (box.low ()), ToQt (box.high ()));
}

G3D::Vector2 ToG3D (const QVector2D& v)
{
    return G3D::Vector2 (v.x (), v.y ());
}

G3D::Vector3 ToG3D (const QVector3D& v)
{
    return G3D::Vector3 (v.x (), v.y (), v.z ());
}

G3D::AABox ToG3D (const QBox3D& box)
{
    return G3D::AABox (ToG3D (box.minimum ()), ToG3D (box.maximum ()));
}


G3D::Vector2 MapToOpenGl (const QPoint& point, int windowHeight)
{
    return G3D::Vector2 (point.x (), windowHeight - point.y ());
}

// Container algorithms
// ======================================================================

template<typename Container, 
	 typename ContainerIterator, 
	 typename ElementComparatorAlong>
void CalculateAggregate<Container, ContainerIterator, ElementComparatorAlong>::
operator() (Aggregate aggregate, Container& container, G3D::Vector3* v)
{
    using G3D::Vector3;
    ContainerIterator it;
    ElementComparatorAlong comparator (Vector3::X_AXIS);
    it = aggregate (container.begin (), container.end (), comparator);
    v->x = comparator (*it);
    comparator = ElementComparatorAlong (Vector3::Y_AXIS);
    it = aggregate (container.begin (), container.end (), comparator);
    v->y = comparator (*it);
    comparator = ElementComparatorAlong (Vector3::Z_AXIS);
    it = aggregate (container.begin (), container.end (), comparator);
    v->z = comparator (*it);
}


// Template instantiations
//======================================================================

// fuzzyFind
// ======================================================================

template std::_Rb_tree_const_iterator<boost::shared_ptr<Edge> > fuzzyFind<std::set<boost::shared_ptr<Edge>, EdgeLessThan, std::allocator<boost::shared_ptr<Edge> > >, std::_Rb_tree_const_iterator<boost::shared_ptr<Edge> >, boost::shared_ptr<Edge> >(std::set<boost::shared_ptr<Edge>, EdgeLessThan, std::allocator<boost::shared_ptr<Edge> > > const&, boost::shared_ptr<Edge> const&);

template std::_Rb_tree_const_iterator<boost::shared_ptr<Face> > fuzzyFind<std::set<boost::shared_ptr<Face>, FaceLessThan, std::allocator<boost::shared_ptr<Face> > >, std::_Rb_tree_const_iterator<boost::shared_ptr<Face> >, boost::shared_ptr<Face> >(std::set<boost::shared_ptr<Face>, FaceLessThan, std::allocator<boost::shared_ptr<Face> > > const&, boost::shared_ptr<Face> const&);

template std::_Rb_tree_const_iterator<boost::shared_ptr<Vertex> > fuzzyFind<std::set<boost::shared_ptr<Vertex>, VertexLessThan, std::allocator<boost::shared_ptr<Vertex> > >, std::_Rb_tree_const_iterator<boost::shared_ptr<Vertex> >, boost::shared_ptr<Vertex> >(std::set<boost::shared_ptr<Vertex>, VertexLessThan, std::allocator<boost::shared_ptr<Vertex> > > const&, boost::shared_ptr<Vertex> const&);

// CalculateAggregate
// ======================================================================

template void CalculateAggregate<std::set<boost::shared_ptr<Vertex>, VertexLessThan, std::allocator<boost::shared_ptr<Vertex> > >, std::_Rb_tree_const_iterator<boost::shared_ptr<Vertex> >, VertexLessThanAlong>::operator()(std::_Rb_tree_const_iterator<boost::shared_ptr<Vertex> > (*)(std::_Rb_tree_const_iterator<boost::shared_ptr<Vertex> >, std::_Rb_tree_const_iterator<boost::shared_ptr<Vertex> >, VertexLessThanAlong), std::set<boost::shared_ptr<Vertex>, VertexLessThan, std::allocator<boost::shared_ptr<Vertex> > >&, G3D::Vector3*);

template void CalculateAggregate<std::vector<G3D::Vector3*, std::allocator<G3D::Vector3*> >, __gnu_cxx::__normal_iterator<G3D::Vector3**, std::vector<G3D::Vector3*, std::allocator<G3D::Vector3*> > >, VertexLessThanAlong>::operator()(__gnu_cxx::__normal_iterator<G3D::Vector3**, std::vector<G3D::Vector3*, std::allocator<G3D::Vector3*> > > (*)(__gnu_cxx::__normal_iterator<G3D::Vector3**, std::vector<G3D::Vector3*, std::allocator<G3D::Vector3*> > >, __gnu_cxx::__normal_iterator<G3D::Vector3**, std::vector<G3D::Vector3*, std::allocator<G3D::Vector3*> > >, VertexLessThanAlong), std::vector<G3D::Vector3*, std::allocator<G3D::Vector3*> >&, G3D::Vector3*);

template void CalculateAggregate<std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Body>*, std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > > >, BBObjectLessThanAlongLow<Body> >::operator()(__gnu_cxx::__normal_iterator<boost::shared_ptr<Body>*, std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > > > (*)(__gnu_cxx::__normal_iterator<boost::shared_ptr<Body>*, std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Body>*, std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > > >, BBObjectLessThanAlongLow<Body>), std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > >&, G3D::Vector3*);

template void CalculateAggregate<std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Body>*, std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > > >, BBObjectLessThanAlongHigh<Body> >::operator()(__gnu_cxx::__normal_iterator<boost::shared_ptr<Body>*, std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > > > (*)(__gnu_cxx::__normal_iterator<boost::shared_ptr<Body>*, std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Body>*, std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > > >, BBObjectLessThanAlongHigh<Body>), std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > >&, G3D::Vector3*);

template void CalculateAggregate<std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > >, BBObjectLessThanAlongLow<Foam> >::operator()(__gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > > (*)(__gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > >, BBObjectLessThanAlongLow<Foam>), std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > >&, G3D::Vector3*);

template void CalculateAggregate<std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > >, BBObjectLessThanAlongHigh<Foam> >::operator()(__gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > > (*)(__gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > >, BBObjectLessThanAlongHigh<Foam>), std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > >&, G3D::Vector3*);


