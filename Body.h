/**
 * @file   Body.h
 * @author Dan R. Lipsa
 *
 * Declaration of the Body class
 */

#ifndef __BODY_H__
#define __BODY_H__

#include <vector>
#include "OrientedFace.h"
using namespace std;

/**
 * A body is a set of faces
 */
class Body
{
public:
    /**
     * Creates a new body
     * @param faceIndexes 0 based indexes into a vector of Face objects
     * @param faces vector of Face objects
     */
    Body(const vector<int>& faceIndexes, vector<Face*>& faces);
    /**
     * Deletes the memory allocated to store a Body object
     */
    ~Body();
    /**
     * Prety print a body
     */
    friend ostream& operator<< (ostream& ostr, Body& b); 

private:
    /**
     * Faces that are part of this body.
     */
    vector<OrientedFace*> m_faces;
};

#endif

// Local Variables:
// mode: c++
// End:
