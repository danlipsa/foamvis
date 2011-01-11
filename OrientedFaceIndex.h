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

/**
 * Keeps track of all faces an edge is part of.
 */
class OrientedFaceIndex
{
public:
    OrientedFaceIndex (const boost::shared_ptr<OrientedFace>& face,
		       size_t edgeIndex) :
	m_orientedFace (face),
	m_orientedEdgeIndex (edgeIndex)
    {
    }

    OrientedFaceIndex () : 
	m_orientedEdgeIndex(0)
    {
    }

    boost::shared_ptr<OrientedFace> GetOrientedFace () const
    {
	return m_orientedFace.lock ();
    }
    boost::shared_ptr<Face> GetFace () const;
    OrientedEdge GetOrientedEdge () const;
    size_t GetOrientedEdgeIndex () const
    {
	return m_orientedEdgeIndex;
    }
    bool IsOrientedEdgeReversed () const;
    bool IsStandalone () const;

    const BodyIndex& GetBodyIndex () const;
    size_t GetBodyId () const;

    void clear ()
    {
	m_orientedFace.reset ();
	m_orientedEdgeIndex = 0;
    }
    bool IsValidNext (const OrientedFaceIndex& next) const;
    string ToString () const;

private:
    boost::weak_ptr<OrientedFace> m_orientedFace;
    size_t m_orientedEdgeIndex;


};

typedef multiset<OrientedFaceIndex,
		 OrientedFaceIndexLessThan> OrientedFaceIndexList;
inline ostream& operator<< (ostream& ostr, const OrientedFaceIndex& ofi)
{
    return ostr << ofi.ToString ();
}

#endif //__ORIENTED_FACE_INDEX_H__

// Local Variables:
// mode: c++
// End:
