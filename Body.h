/**
 * @file   Body.h
 * @author Dan R. Lipsa
 *
 * Declaration of the Body class
 */

#ifndef __BODY_H__
#define __BODY_H__

#include "OrientedFace.h"

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
    Body(const std::vector<int>& faceIndexes, std::vector<Face*>& faces);
        ~Body ();
    /**
     * Returns the  vector of oriented faces this body is made of
     * @return a vector of oriented faces
     */
    const std::vector<OrientedFace*>& GetOrientedFaces() const
    {
        return m_faces;
    }
    /**
     * Prety prints a Body
     * @param ostr where to print
     * @param b what to print
     * @return the stream where we printed.
     */
    friend std::ostream& operator<< (std::ostream& ostr, Body& b); 
    /**
     * Specifies the default attributes for the Body object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    static void StoreDefaultAttributes (AttributesInfo& info);
    virtual void CalculateAverage () {};
private:
    /**
     * Oriented faces that are part of this body.
     */
    std::vector<OrientedFace*> m_faces;
};
/**
 * Pretty prints a Body*
 * @param ostr where to print
 * @param b what to print
 * @return where to print something else
 */
inline std::ostream& operator<< (std::ostream& ostr, Body* b)
{
    return ostr << *b;
}

#endif //__BODY_H__

// Local Variables:
// mode: c++
// End:
