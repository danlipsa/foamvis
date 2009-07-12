#ifndef __ORIENTED_FACE_H__
#define __ORIENTED_FACE_H__

#include "Face.h"

class OrientedFace
{
public:
    OrientedFace(Face* face, bool reversed)
    {m_face = face; m_reversed = reversed;}

private:
    Face* m_face;
    bool m_reversed;
};


#endif

// Local Variables:
// mode: c++
// End:
