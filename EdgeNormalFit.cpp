/**
 * @file   EdgeNormalFit.cpp
 * @author Dan R. Lipsa
 * @date 18 March 2010
 *
 * Implementation of the EdgeNormalFit class
 */

#include "EdgeNormalFit.h"
#include "Body.h"

/**
 * Designate the X axis as the normal origin.
 *
 * Order oriented faces  based on the angle between  the normal to the
 * face and the normal origin.
 *
 * Add to the  end of the queue an EdgeNormalFit  with the first edge
 * of the first face and an infinite normal.  An infinite normal means
 * that we  don't know the  normal of  the face we  try to fit  and we
 * choose  the next normal  not processed  for which  we find  an edge
 * fit. This the face most perpendicular on axis X.
 *
 * until we fit all faces
 *   Remove an EdgeNormalFit  from the queue.
 *
 *   If the normal is infinite find a face that fits the edge with the
 *   normal having the smallest angle to the normals already processed
 *   (smallest in the list of normals not processed yet).
 *
 *     Else  (normal is  finite),  fit  a face  on  the same  physical
 *     face.  You do  that by  scanning  through faces  with the  same
 *     normal and match the edge.
 *
 *     If the fit is not possible, that means we discovered a physical
 *     edge and two  physical vertices.  Record those. Add  to the end
 *     of   the   queue   the    physical   edge   and   an   infinite
 *     normal. Continue.
 *
 *   (the fit is  possible) Add to the begining of  the queue edges of
 *   the fitted face with the same normal.
 */

ostream& operator<< (ostream& ostr, const EdgeNormalFit& edgeNormalFit)
{
    ostr << "EdgeNormalFit: " << edgeNormalFit.m_edge 
	 << ", normal " << edgeNormalFit.m_normal;
    return ostr;
}

const G3D::Vector3 EdgeNormalFit::UNKNOWN_NORMAL = G3D::Vector3::inf ();

void EdgeNormalFit::Initialize (list<EdgeNormalFit>* queue, Body* body)
{
    Body::NormalFaceMap& normalFaceMap = body->GetNormalFaceMap ();
    OrientedFace* of = (*normalFaceMap.begin ()).second;
    queue->push_back (EdgeNormalFit (of->GetOrientedEdge (0), UNKNOWN_NORMAL));
}

void EdgeNormalFit::AddQueue (
    list<EdgeNormalFit>* queue, 
    const EdgeNormalFit& src, OrientedFace* fit)
{
    
}

OrientedFace* EdgeNormalFit::FitAndDuplicateFace (Body* body) const
{
    
}
