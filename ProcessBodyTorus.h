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
    ProcessBodyTorus (Body* body);
    void Initialize ();
    bool Step ();
    void Cleanup ();

private:
    Body* m_body;
    queue<OrientedEdge> m_queue;
    map<int, bool> m_idTraversedMap;
};



#endif //__PROCESS_BODY_TORUS_H__

// Local Variables:
// mode: c++
// End:
