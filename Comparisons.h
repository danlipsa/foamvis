/**
 * @file   Comparisons.h
 * @author Dan R. Lipsa
 * @date 25 March 2010
 * Comparison functors
 */

#ifndef __COMPARISONS_H__
#define __COMPARISONS_H__

class Foam;
class Face;
class Edge;
class Vertex;
class AdjacentOrientedFace;

#include "SystemDifferences.h"

/**
 * Binary function that compares two C strings ignoring the case
 */
struct LessThanNoCase : binary_function<const char*, const char*, bool>
{
    /**
     * Compares two character pointers
     * @param s1 first C string
     * @param s2 second C string
     * @return true if the first argument is less than the second argument.
     */
    bool operator()(const char* s1, const char* s2) const
    {
        return strcasecmp(s1, s2) < 0;
    }
    bool operator () (const string& s1, const string& s2) const
    {
	return operator () (s1.c_str (), s2.c_str ());
    }
};


struct Vector3int16LessThan
{
    bool operator () (const G3D::Vector3int16& first, 
		      const G3D::Vector3int16& second)
    {
	return 
	    first.x < second.x ||
	    (first.x == second.x && first.y < second.y) ||
	    (first.x == second.x && first.y == second.y && first.z < second.z);
    }
};



struct VertexPtrLessThan
{
    bool operator () (const boost::shared_ptr<const Vertex>& first,
		      const boost::shared_ptr<const Vertex>& second) const;
};


/**
 * Functor that compares two vertices along X, Y or Z axis
 */
class VertexPtrLessThanAlong
{
public:
    /**
     * Constructor
     * Stores the axis we want to do the comparison on.
     */
VertexPtrLessThanAlong(G3D::Vector3::Axis axis) : 
    m_axis(axis) {}
    /**
     * Compares two vertices
     * @param first the first vertex
     * @param second the second vertex
     * @return true if first is less than second false otherwise
     */
    bool operator() (const boost::shared_ptr<Vertex>& first,
		     const boost::shared_ptr<Vertex>& second) const;
    bool operator() (
	const G3D::Vector3& first, const G3D::Vector3& second) const
    {
	return first[m_axis] < second[m_axis];
    }

    bool operator() (
	const G3D::Vector3* first, const G3D::Vector3* second) const
    {
	return operator () (*first, *second);
    }

    double operator () (const G3D::Vector3& x) const
    {
	return x[m_axis];
    }
    double operator () (const G3D::Vector3* x) const
    {
	return operator () (*x);
    }
    double operator () (const boost::shared_ptr<Vertex>& x) const;

private:
    /**
     * Axis along which we make the comparison
     */
    G3D::Vector3::Axis m_axis;
};

class EdgeLessThan
{
public:
    bool operator () (const boost::shared_ptr<const Edge>& first,
		      const boost::shared_ptr<const Edge>& second) const;
};


class FaceLessThan
{
public:
    bool operator () (const boost::shared_ptr<const Face>& first,
		      const boost::shared_ptr<const Face>& second) const;
};


class OrientedFaceIndexLessThan
{
public:
    bool operator () (
	const AdjacentOrientedFace& first, 
	const AdjacentOrientedFace& second) const;
};


template <typename BBObject>
class BBObjectLessThanAlong
{
public:
    typedef const G3D::Vector3& (G3D::AABox::*BoxCorner) () const;
    typedef boost::function<G3D::AABox (const BBObject& object)> GetAABox;

    BBObjectLessThanAlong (G3D::Vector3::Axis axis, BoxCorner corner,
			   GetAABox getAABox) : 
	m_axis (axis), m_corner(corner), m_getAABox (getAABox)
    {}
    /**
     * Functor that compares two data objects
     * @param first first data object
     * @param second second data object
     */
    bool operator () (
	const BBObject* first, const BBObject* second)
    {
	return operator () (*first, *second);
    }

    bool operator () (
	boost::shared_ptr<BBObject> first, 
	boost::shared_ptr<BBObject> second)
    {
	return operator () (*first, *second);
    }

    bool operator() (const BBObject& first, 
		     const BBObject& second);
    double operator () (const BBObject& x);
    double operator () (const boost::shared_ptr<BBObject>& x)
    {
	return operator () (*x);
    }
	
private:
    /**
     * Along which axis to compare
     */
    G3D::Vector3::Axis m_axis;
    /**
     * What corner of the AABox to compare
     */
    BoxCorner m_corner;
    GetAABox m_getAABox;
};

template<typename BBObject>
class BBObjectLessThanAlongLow : public BBObjectLessThanAlong<BBObject>
{
public:
    BBObjectLessThanAlongLow (G3D::Vector3::Axis axis) :
	BBObjectLessThanAlong<BBObject> (
	    axis, &G3D::AABox::low, 
	    boost::bind (&BBObject::GetBoundingBox, _1))
    {
    }
};

template<typename BBObject>
class BBObjectLessThanAlongLowTorus : public BBObjectLessThanAlong<BBObject>
{
public:
    BBObjectLessThanAlongLowTorus (G3D::Vector3::Axis axis) :
	BBObjectLessThanAlong<BBObject> (
	    axis, &G3D::AABox::low,
	    boost::bind (&BBObject::GetBoundingBoxTorus, _1))
    {
    }
};


template<typename BBObject>
class BBObjectLessThanAlongHigh : public BBObjectLessThanAlong<BBObject>
{
public:
    BBObjectLessThanAlongHigh (G3D::Vector3::Axis axis) :
	BBObjectLessThanAlong<BBObject> (
	    axis, &G3D::AABox::high,
	    boost::bind (&BBObject::GetBoundingBox, _1))
    {
    }
};

template<typename BBObject>
class BBObjectLessThanAlongHighTorus : public BBObjectLessThanAlong<BBObject>
{
public:
    BBObjectLessThanAlongHighTorus (G3D::Vector3::Axis axis) :
	BBObjectLessThanAlong<BBObject> (
	    axis, &G3D::AABox::high,
	    boost::bind (&BBObject::GetBoundingBoxTorus, _1))
    {
    }
};


typedef set<boost::shared_ptr<Vertex>, VertexPtrLessThan> VertexSet;
typedef set<boost::shared_ptr<Edge>, EdgeLessThan> EdgeSet;
typedef set<boost::shared_ptr<Face>, FaceLessThan> FaceSet;
class Body;
bool BodyLessThan (const boost::shared_ptr<Body>& first,
		   const boost::shared_ptr<Body>& second);
bool BodyLessThanId (const boost::shared_ptr<Body>& first, size_t bodyId);



#endif //__COMPARISONS_H__

// Local Variables:
// mode: c++
// End:
