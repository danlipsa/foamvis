/**
 * @file   OrientedFaceIndex.h
 * @author Dan R. Lipsa
 * @date 19 May 2010
 * 
 * Declaration of the OrientedFaceIndex class
 */
#ifndef __ORIENTED_FACE_INDEX_H__
#define __ORIENTED_FACE_INDEX_H__

class OrientedFace;

struct OrientedFaceIndex
{
    OrientedFaceIndex (const boost::shared_ptr<OrientedFace>& orientedFace,
		       size_t index) :
    m_orientedFace (orientedFace), m_index (index)
    {}
    boost::shared_ptr<OrientedFace> m_orientedFace;
    size_t m_index;
};

#endif //__ORIENTED_FACE_INDEX_H__


// Local Variables:
// mode: c++
// End:
