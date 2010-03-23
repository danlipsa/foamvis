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
#include "OrientedEdge.h"

class ProcessBodyTorus
{
public:
    ProcessBodyTorus (Body* body) : m_body (body) {}
    void Initialize ();
    bool Step ();

private:
    void addQueue (const OrientedFace* of,
		   OrientedFace::const_iterator fitPosition);

private:
    list<EdgeFit> m_queue;
    Body* m_body;
};



#endif //__PROCESS_BODY_TORUS_H__

// Local Variables:
// mode: c++
// End:
