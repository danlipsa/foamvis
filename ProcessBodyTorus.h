/**
 * @file   ProcessBodyTorus.h
 * @author Dan R. Lipsa
 * @date 16 March 2010
 *
 * Declaration of the ProcessBodyTorus.h
 */

#ifndef __PROCESS_BODY_TORUS_H__
#define __PROCESS_BODY_TORUS_H__

class OrientedFace;
#include "Body.h"
#include "DebugStream.h"
#include "FaceEdgeIndex.h"



template<typename Fit>
class ProcessBodyTorus
{
public:
    ProcessBodyTorus (Body* body) : m_body (body) {}
    void Initialize ()
    {
	Fit::Initialize (&m_queue, m_body);
    }
    bool Step ()
    {
	if (m_body->GetPlacedOrientedFaces () == 
	    m_body->GetOrientedFaces ().size ())
	    return false;

	cdbg << "Queue " << m_queue.size () << " elements" << endl;
	ostream_iterator<Fit> output (cdbg, "\n");
	copy (m_queue.begin (), m_queue.end (), output);

	// place the face that fits over that margin (this might
	// create a duplicate face)
	RuntimeAssert (m_queue.size () > 0,
		       "Process body torus queue is empty");
	Fit fit = m_queue.front ();
	m_queue.pop_front ();
	OrientedFace* face = fit.FitAndDuplicateFace (m_body);

	// if the face was not placed before
	// add two more angles in the queue
	if (face == 0)
	    cdbg << "No face fitted" << endl;
	else
	{ 
	    if (! face->IsPlaced ())
	    {
		if (face->GetFace ()->IsDuplicate ())
		    cdbg << "Fitted face: " << endl << *face << endl;
		else
		{
		    Face& f = *face->GetFace ();
		    cdbg << "Fitted face " << f.GetOriginalIndex () 
			 << " " << f.GetColor () << " "
			 << " not a DUPLICATE" << endl;
		}
		m_body->SetPlacedOrientedFace (face);
		fit.AddQueue (&m_queue, &fit, face);
	    }
	    else
	    {
		cdbg << "Fitted face " << face->GetFace ()->GetOriginalIndex () 
		     << " " << face->GetFace ()->GetColor () << " "
		     << " already PLACED" << endl;
	    }
	}
	return true;
    }

    void End ()
    {
	m_body->ResetPlacedOrientedFaces ();
    }

private:
    list<Fit> m_queue;
    Body* m_body;
};



#endif //__PROCESS_BODY_TORUS_H__

// Local Variables:
// mode: c++
// End:
