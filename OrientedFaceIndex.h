/**
 * @file   OrientedFaceIndex.h
 * @author Dan R. Lipsa
 * @date 20 May 2010
 * 
 * Declaration of the OrientedFaceIndex class
 */
#ifndef __ORIENTED_FACE_INDEX_H__
#define __ORIENTED_FACE_INDEX_H__

#include "Comparisons.h"
class BodyIndex;
class Face;
class OrientedEdge;
class OrientedFace;


class OrientedFaceIndex
{
public:
    OrientedFaceIndex (OrientedFace* face, size_t edgeIndex) :
	m_orientedFace (face),
	m_orientedEdgeIndex (edgeIndex)
    {
    }
    OrientedFaceIndex () :
	m_orientedFace(0), m_orientedEdgeIndex(0)
    {
    }
    OrientedFace* GetOrientedFace () const
    {
	return m_orientedFace;
    }
    Face* GetFace () const;
    OrientedEdge GetOrientedEdge () const;
    size_t GetOrientedEdgeIndex () const
    {
	return m_orientedEdgeIndex;
    }
    bool IsOrientedEdgeReversed () const;

    const BodyIndex& GetBodyIndex () const;
    size_t GetBodyId () const;

    void clear ()
    {
	m_orientedFace = 0;
	m_orientedEdgeIndex = 0;
    }
    bool IsValidNext (const OrientedFaceIndex& next) const;


public:
    friend ostream& operator<< (ostream& ostr, const OrientedFaceIndex& ofi);

private:
    OrientedFace* m_orientedFace;
    size_t m_orientedEdgeIndex;


};

typedef multiset<OrientedFaceIndex, OrientedFaceIndexLessThan> 
OrientedFaceIndexList;


#endif //__ORIENTED_FACE_INDEX_H__

// Local Variables:
// mode: c++
// End:
