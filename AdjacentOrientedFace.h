/**
 * @file   AdjacentOrientedFace.h
 * @author Dan R. Lipsa
 * @date 20 May 2010
 * 
 * Declaration of the AdjacentOrientedFace class
 */
#ifndef __ADJACENT_ORIENTED_FACE_H__
#define __ADJACENT_ORIENTED_FACE_H__

#include "Comparisons.h"
class AdjacentBody;
class Face;
class OrientedEdge;
class OrientedFace;

/**
 * An (oriented) edge in a (oriented) face. Keeps track of all faces
 * an edge is part of.
 */
class AdjacentOrientedFace
{
public:
    AdjacentOrientedFace (const boost::shared_ptr<OrientedFace>& face,
		       size_t edgeIndex) :
	m_orientedFace (face),
	m_orientedEdgeIndex (edgeIndex)
    {
    }

    AdjacentOrientedFace () : 
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

    const AdjacentBody& GetAdjacentBody () const;
    size_t GetBodyId () const;

    void clear ()
    {
	m_orientedFace.reset ();
	m_orientedEdgeIndex = 0;
    }
    /**
     * Returns true if the angle between normals is less than 90 degrees.
     */
    bool IsValidNext (const AdjacentOrientedFace& next) const;
    string ToString () const;

private:
    boost::weak_ptr<OrientedFace> m_orientedFace;
    size_t m_orientedEdgeIndex;


};

typedef multiset<AdjacentOrientedFace,
		 OrientedFaceIndexLessThan> OrientedFaceIndexList;
inline ostream& operator<< (ostream& ostr, const AdjacentOrientedFace& ofi)
{
    return ostr << ofi.ToString ();
}

#endif //__ADJACENT_ORIENTED_FACE_H__

// Local Variables:
// mode: c++
// End:
