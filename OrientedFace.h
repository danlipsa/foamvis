/**
 * @file OrientedFace.h
 * @author Dan R. Lipsa
 * 
 * Declaration for the OrientedFace class
 */
#ifndef __ORIENTED_FACE_H__
#define __ORIENTED_FACE_H__

#include "Face.h"

/**
 * An oriented face is a face  (list of edges) that can have its edges
 * read in direct or reversed order.
 */
class OrientedFace
{
public:
    /**
     * Constructs a OrientedFace object
     * @param face a pointer to a Face
     * @param reversed if true the edges in the Face should be read in
     *        reverse order,  otherwise they should be  read in direct
     *        order
     */
    OrientedFace(Face* face, bool reversed)
    {m_face = face; m_reversed = reversed;}
    const Face* GetFace () const {return m_face;}
    bool IsReversed () const {return m_reversed;}
    const Vertex* GetBegin (unsigned int edgeIndex) const;
    const Vertex* GetEnd (unsigned int edgeIndex) const;
    /**
     * Pretty print for the OrientedFace object
     */
    friend std::ostream& operator<< (std::ostream& ostr, const OrientedFace& of);


private:
    /**
     * Object that has information about the edges in this OrientedFace
     */
    Face* m_face;
    /**
     * If true,  edges in the refering  Face object should  be read in
     * reversed order
     */
    bool m_reversed;
};

inline std::ostream& operator<< (std::ostream& ostr, const OrientedFace* pof)
{
    return ostr << *pof;
}


#endif //__ORIENTED_FACE_H__

// Local Variables:
// mode: c++
// End:
