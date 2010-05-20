/**
 * @file   OrientedFaceIndex.h
 * @author Dan R. Lipsa
 * @date 20 May 2010
 * 
 * Declaration of the OrientedFaceIndex class
 */
#ifndef __ORIENTED_FACE_INDEX_H__
#define __ORIENTED_FACE_INDEX_H__

class Face;

struct OrientedFaceIndex
{
    OrientedFaceIndex (Face* face, bool faceReversed,
		       size_t edgeIndex) :
	m_face (face), m_faceReversed (faceReversed), m_edgeIndex (edgeIndex)
    {}
    OrientedFaceIndex () :
	m_face(0), m_faceReversed(false), m_edgeIndex(0)
    {}
    void clear ()
    {
	m_face = 0;
	m_faceReversed = false;
	m_edgeIndex = 0;
    }

    friend ostream& operator<< (ostream& ostr, const OrientedFaceIndex& ofi);

    Face* m_face;
    bool m_faceReversed;
    size_t m_edgeIndex;


};

#endif //__ORIENTED_FACE_INDEX_H__

// Local Variables:
// mode: c++
// End:
