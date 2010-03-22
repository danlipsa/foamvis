/**
 * @file   EdgeFit.cpp
 * @author Dan R. Lipsa
 * @date 18 March 2010
 *
 * Implementation of the EdgeFit class
 */

#include "EdgeFit.h"
#include "Body.h"
#include "Debug.h"
#include "Data.h"

/**
 * Designate the X axis as the normal origin.
 *
 * Order oriented faces  based on the angle between  the normal to the
 * face and the normal origin.
 *
 * Add to the end of the queue an EdgeFit with the first edge of
 * the first  face and an  no normal.  No  normal means that  we don't
 * know the normal  of the face we  try to fit and we  choose the next
 * normal  not  processed for  which  we find  an  edge  fit. For  G3D
 * NO_NORMAL  is stored  using  Vector3::inf (). 
 *
 * until we fit all faces
 *   Remove an EdgeFit  from the queue.
 *
 *   If there is no normal find a face that fits the edge with the
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

ostream& operator<< (ostream& ostr, const EdgeFit& edgeNormalFit)
{
    ostr << "EdgeFit: " << edgeNormalFit.m_edge 
	 << ", normal " << edgeNormalFit.m_normal;
    return ostr;
}

const G3D::Vector3 EdgeFit::NO_NORMAL = G3D::Vector3::inf ();

void EdgeFit::Initialize (list<EdgeFit>* queue, Body* body)
{
    OrientedFace* of = body->GetFirstFace ();
    body->IncrementNormalFace ();
    for (OrientedFace::iterator it = of->begin (); it != of->end (); ++it)
	queue->push_back (EdgeFit (*it, EdgeFit::NO_NORMAL));
}

void EdgeFit::AddQueue (
    list<EdgeFit>* queue, OrientedFace* fit)
{
    for (OrientedFace::iterator it = fit->begin ();
	 it != fit->end (); ++it)
	queue->push_front (EdgeFit (*it, EdgeFit::NO_NORMAL));
}


OrientedFace* EdgeFit::FitFromQueue (list<EdgeFit>* queue, Body* body)
{
    // find an edge that fits this face
    OrientedFace* candidate = body->GetCurrentNormalFace ()->second;
    for (list<EdgeFit>::iterator it = queue->begin ();
	 it != queue->end ();
	 it++)
    {
	G3D::Vector3 translation;
	if (Body::FitFace (*candidate, it->m_edge, &translation))
	{
	    EdgeFit fit = *it;
	    if (! translation.isZero ())
	    {
		Face* face = candidate->GetFace ();
		//found a possible fit
		candidate->SetFace (
		    body->GetData ()->GetFaceDuplicate (
			*face, 
			*(face->GetOrientedEdge(0)->GetBegin ()) + 
			translation));
	    }
	    
	    queue->erase (it);
	    body->IncrementNormalFace ();
	    return candidate;
	}
    }
    RuntimeAssert (false, "No match found for: ", *candidate);
    return candidate;
}
