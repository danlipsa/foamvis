#ifndef __BODY_H__
#define __BODY_H__

#include <vector>
#include "OrientedFace.h"

class Body
{
public:
    Body(std::vector<int>& face_indexes, std::vector<Face*>& faces);
    ~Body();

private:
    /**
     * Faces that are part of this body.
     */
    std::vector<OrientedFace*> m_faces;
};


#endif

// Local Variables:
// mode: c++
// End:
