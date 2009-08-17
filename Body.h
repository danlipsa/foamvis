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

class AttributesInfo;
/**
 * A body is a set of faces
 */
class Body : public Element
{
public:
    /**
     * Creates a new body
     * @param faceIndexes 0 based indexes into a vector of Face objects
     * @param faces vector of Face objects
     */
    Body(const vector<int>& faceIndexes, vector<Face*>& faces);
        ~Body ();
    /**
     * Returns the  vector of oriented faces this body is made of
     * @return a vector of oriented faces
     */
    const vector<OrientedFace*> GetOrientedFaces() const
    {
        return m_faces;
    }
    /**
     * Prety print a body
     */
    friend ostream& operator<< (ostream& ostr, Body& b); 
    /**
     * Specifies the default attributes for the Body object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    static void StoreDefaultAttributes (AttributesInfo& info);
private:
    /**
     * Oriented faces that are part of this body.
     */
    vector<OrientedFace*> m_faces;
    /**
     * Stores information about all body attributes
     */
    static AttributesInfo* m_infos;
};
/**
 * Pretty prints a Body*
 * @param ostr where to print
 * @param b what to print
 * @return where to print something else
 */
inline ostream& operator<< (ostream& ostr, Body* b)
{
    return ostr << *b;
}

#endif

// Local Variables:
// mode: c++
// End:
