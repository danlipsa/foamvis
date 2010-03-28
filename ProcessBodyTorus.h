/**
 * @file   ProcessBodyTorus.h
 * @author Dan R. Lipsa
 * @date 16 March 2010
 *
 * Declaration of the ProcessBodyTorus.h
 */

#ifndef __PROCESS_BODY_TORUS_H__
#define __PROCESS_BODY_TORUS_H__

#include "OrientedEdge.h"
class Body;
class OrientedFace;

class ProcessBodyTorus
{
public:
    typedef pair<OrientedEdge, OrientedFace*> QueueElement;

public:
    ProcessBodyTorus (Body* body) : m_body (body) {}
    void Initialize ();
    bool Step ();

private:
    Body* m_body;
    queue<QueueElement> m_queue;
};



#endif //__PROCESS_BODY_TORUS_H__

// Local Variables:
// mode: c++
// End:
