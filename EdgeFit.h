/**
 * @file   EdgeFit.h
 * @author Dan R. Lipsa
 * @date 18 March 2010
 *
 * Declaration of the EdgeFit class
 */

#ifndef __EDGE_NORMAL_FIT_H__
#define __EDGE_NORMAL_FIT_H__

#include "OrientedEdge.h"

class OrientedFace;
class Body;

class EdgeFit
{
public:
    EdgeFit (
	const OrientedEdge& oe, const G3D::Vector3& normal = NO_NORMAL) :
    m_edge (oe), m_normal (normal) {}
    OrientedFace* FitAndDuplicateFace (Body* body) const;

    bool HasNormal () const
    {
	return m_normal.isFinite ();
    }
    const G3D::Vector3& GetNormal () const
    {
	return m_normal;
    }
    OrientedEdge& GetOrientedEdge ()
    {
	return m_edge;
    }
    const OrientedEdge& GetOrientedEdge () const
    {
	return m_edge;
    }

public:
    static void Initialize (list<EdgeFit>* queue, Body* body);
    friend ostream& operator<< (
	ostream& ostr, const EdgeFit& edgeNormalFit);
    static OrientedFace* FitFromQueue (list<EdgeFit>* queue, Body* body);
    static void AddQueue (list<EdgeFit>* queue, 
			  OrientedFace* fit);

public:
    static const G3D::Vector3 NO_NORMAL;

private:
    OrientedEdge m_edge;
    G3D::Vector3 m_normal;
};


#endif //__EDGE_NORMAL_FIT_H__
