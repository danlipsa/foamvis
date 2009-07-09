#ifndef __BODY_H__
#define __BODY_H__

#include <list>
using namespace std;

#include "OrientedFace.h"

class Body
{
public:

private:
    /**
     * Faces that are part of this body.
     */
    list<OrientedFace*> m_faces;
};


#endif
