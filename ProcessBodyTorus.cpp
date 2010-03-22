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
#include "ProcessBodyTorus.h"
#include "EdgeFit.h"
#include "OrientedFace.h"

void ProcessBodyTorus::Initialize ()
{
    OrientedFace* of = m_body->GetFirstFace ();
    addQueue (of);
}

bool ProcessBodyTorus::Step ()
{
    Body::NormalFaceMap::iterator normalFaceIt = 
	m_body->GetCurrentNormalFace ();
    if ( normalFaceIt == m_body->GetNormalFaceMap ().end ())
	return false;
	
    cdbg << "Queue " << m_queue.size () << " elements" << endl;
    ostream_iterator<OrientedEdge> output (cdbg, "\n");

    copy (m_queue.begin (), m_queue.end (), output);

    // place the face that fits over that margin (this might
    // create a duplicate face)
    RuntimeAssert (m_queue.size () > 0,
		   "Process body torus queue is empty");
    OrientedFace* face = m_body->FitFromQueue (&m_queue);
	
    if (face->GetFace ()->IsDuplicate ())
	cdbg << "Fitted face: " << endl << *face << endl;
    else
    {
	Face& f = *face->GetFace ();
	cdbg << "Fitted face " << f.GetOriginalIndex () 
	     << " " << f.GetColor () << " "
	     << " not a DUPLICATE" << endl;
    }
    addQueue (face);
    return true;
}


void ProcessBodyTorus::addQueue (OrientedFace* of)
{
    for (OrientedFace::iterator it = of->begin (); it != of->end (); ++it)
    {
	OrientedEdge oe = *it;
	G3D::Vector3 edgeVector = *oe.GetEnd () - *oe.GetBegin ();
	m_queue.push_back (
	    EdgeFit(oe, 
		    of->GetNormal ().cross (edgeVector).unit ()));
    }
    m_body->IncrementNormalFace ();
}

