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
	Body::NormalFaceMap::iterator normalFaceIt = 
	    m_body->GetCurrentNormalFace ();
	if ( normalFaceIt == m_body->GetNormalFaceMap ().end ())
	    return false;
	
	cdbg << "Queue " << m_queue.size () << " elements" << endl;
	ostream_iterator<Fit> output (cdbg, "\n");
	copy (m_queue.begin (), m_queue.end (), output);

	// place the face that fits over that margin (this might
	// create a duplicate face)
	RuntimeAssert (m_queue.size () > 0,
		       "Process body torus queue is empty");
	OrientedFace* face = Fit::FitFromQueue (&m_queue, m_body);
	
	if (face->GetFace ()->IsDuplicate ())
	    cdbg << "Fitted face: " << endl << *face << endl;
	else
	{
	    Face& f = *face->GetFace ();
	    cdbg << "Fitted face " << f.GetOriginalIndex () 
		 << " " << f.GetColor () << " "
		 << " not a DUPLICATE" << endl;
	}
	Fit::AddQueue (&m_queue, face);
	return true;
    }

    void End ()
    {
    }

private:
    list<Fit> m_queue;
    Body* m_body;
};



#endif //__PROCESS_BODY_TORUS_H__

// Local Variables:
// mode: c++
// End:
