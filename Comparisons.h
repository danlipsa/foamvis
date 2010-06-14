/**
 * @file   Comparisons.h
 * @author Dan R. Lipsa
 * @date 25 March 2010
 * Comparison functors
 */

#ifndef __COMPARISONS_H__
#define __COMPARISONS_H__

class Foam;
class Edge;
class Vertex;
class OrientedFaceIndex;

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
    bool operator()(const char* s1, const char* s2) 
    {
        return strcasecmp(s1, s2) < 0;
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



class VectorLessThanAngle
{
public:
    VectorLessThanAngle (const G3D::Vector3& originNormal) : 
    m_originNormal (originNormal) {}
    bool operator () (
	const G3D::Vector3& first, const G3D::Vector3& second) const;
    static double angle (
	const G3D::Vector3& first, const G3D::Vector3& second);
    static double angle0pi (
	const G3D::Vector3& first, const G3D::Vector3& second)
    {
	return acos (first.dot (second));
    }
private:
    G3D::Vector3 m_originNormal;
};


struct VertexLessThan
{
    bool operator () (const boost::shared_ptr<Vertex>& first,
		      const boost::shared_ptr<Vertex>& second) const;
};


/**
 * Functor that compares two vertices along X, Y or Z axis
 */
class VertexLessThanAlong
{
public:
    /**
     * Constructor
     * Stores the axis we want to do the comparison on.
     */
VertexLessThanAlong(G3D::Vector3::Axis axis) : 
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
	const G3D::Vector3* first, const G3D::Vector3* second) const
    {
	return (*first)[m_axis] < (*second)[m_axis];	    
    }
private:
    /**
     * Axis along which we make the comparison
     */
    G3D::Vector3::Axis m_axis;
};

class EdgeLessThan
{
public:
    bool operator () (const Edge* first, const Edge* second) const;
};


class FoamLessThanAlong
{
public:
    typedef const G3D::Vector3& (Foam::*Corner) () const;
    /**
     * Constructor
     * @param axis along which axis to compare
     * @param corner which corner of the AABox to compare
     */
    FoamLessThanAlong (G3D::Vector3::Axis axis, Corner corner) : 
    m_axis (axis), m_corner(corner) {}
    /**
     * Functor that compares two data objects
     * @param first first data object
     * @param second second data object
     */
    bool operator () (const Foam* first, const Foam* second)
    {
	return operator () (*first, *second);
    }

    bool operator () (
	boost::shared_ptr<Foam> first, boost::shared_ptr<Foam> second)
    {
	return operator () (*first, *second);
    }

    bool operator() (const Foam& first, const Foam& second)
    {
	return 
	    (first.*m_corner) ()[m_axis] < (second.*m_corner) ()[m_axis];
    }
	
private:
    /**
     * Along which axis to compare
     */
    G3D::Vector3::Axis m_axis;
    /**
     * What corner of the AABox to compare
     */
    Corner m_corner;
};


class OrientedFaceIndexLessThan
{
public:
    bool operator () (
	const OrientedFaceIndex& first, const OrientedFaceIndex& second) const;
};


#endif //__COMPARISONS_H__
