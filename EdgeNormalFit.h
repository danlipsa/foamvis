/**
 * @file   EdgeNormalFit.h
 * @author Dan R. Lipsa
 * @date 18 March 2010
 *
 * Declaration of the EdgeNormalFit class
 */

#ifndef __EDGE_NORMAL_FIT_H__
#define __EDGE_NORMAL_FIT_H__

#include "OrientedEdge.h"

class OrientedFace;
class Body;

class EdgeNormalFit
{
public:
    EdgeNormalFit (
	const OrientedEdge& oe, const G3D::Vector3& normal) :
    m_edge (oe), m_normal (normal) {}
    void AddQueue (list<EdgeNormalFit>* queue, 
		   const EdgeNormalFit& edgeNormalFit, OrientedFace* fit);
    OrientedFace* FitAndDuplicateFace (Body* body) const;
    bool hasKnownNormal ()
    {
	return m_normal.isFinite ();
    }

public:
    static void Initialize (list<EdgeNormalFit>* queue, Body* body);
    friend ostream& operator<< (
	ostream& ostr, const EdgeNormalFit& edgeNormalFit);

public:
    static const G3D::Vector3 UNKNOWN_NORMAL;

private:
    OrientedEdge m_edge;
    G3D::Vector3 m_normal;
};


#endif //__EDGE_NORMAL_FIT_H__
