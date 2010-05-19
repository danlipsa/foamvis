/**
 * @file   ProcessBodyTorus.cpp
 * @author Dan R. Lipsa
 * @date 22 March 2010
 *
 * Implementation of the ProcessBodyTorus class
 */

#include "Body.h"
#include "Debug.h"
#include "DebugStream.h"
#include "Foam.h"
#include "ProcessBodyTorus.h"
#include "OrientedFace.h"
#include "Vertex.h"

ProcessBodyTorus::ProcessBodyTorus (Body* body) : 
    m_body (body)
{
    Body::OrientedFaces& oFaces = m_body->GetOrientedFaces ();
    BOOST_FOREACH (OrientedFace* of, oFaces)
	m_idTraversedMap[of->GetSignedId ()] = false;
}


void ProcessBodyTorus::Initialize ()
{
    Body::OrientedFaces& oFaces = m_body->GetOrientedFaces ();
    for_each (oFaces.begin (), oFaces.end (), 
	      boost::mem_fn (&OrientedFace::UpdateEdgeAdjacency));
    OrientedFace* of = m_body->GetOrientedFace (0);
    m_idTraversedMap[of->GetSignedId ()] = true;
    for (size_t i = 0; i < of->size (); i++)
    {
	OrientedEdge oe;
	of->GetOrientedEdge (i, &oe);
	m_queue.push (oe);
    }
}


bool ProcessBodyTorus::Step ()
{
/*
    OrientedEdge oe;
    while (m_queue.size () > 0)
    {
	oe = m_queue.front ();
	if (oe.second->IsTraversed ())
	    m_queue.pop ();
	else
	    break;
    }
    if (m_queue.size () == 0)
	return false;
    OrientedFace* of = oe.second;





    const OrientedEdge& edge = oe.first;
    G3D::Vector3 translation;
    of->CalculateTranslation (edge, &translation);
    if (! translation.isZero ())
    {
	const G3D::Vector3& begin = *of->GetOrientedEdge (0).GetBegin ();
	Face* f = m_body->GetFoam ()->GetFaceDuplicate (*of->GetFace (), begin);
	of->SetFace (f);
    }

    return true;
*/
}

void ProcessBodyTorus::Cleanup ()
{
    Body::OrientedFaces& oFaces = m_body->GetOrientedFaces ();
    for_each (oFaces.begin (), oFaces.end (), 
	      boost::mem_fn (&OrientedFace::ClearEdgeAdjacency));
}


