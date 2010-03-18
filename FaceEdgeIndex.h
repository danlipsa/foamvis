/**
 * @file   FaceEdgeIndex.h
 * @author Dan R. Lipsa
 * @date 18 March 2010
 *
 * Declaration of the FaceEdgeIndex.h
 */

#ifndef __FACE_EDGE_INDEX_H__
#define __FACE_EDGE_INDEX_H__

class OrientedFace;
class Body;

class FaceEdgeIndex
{
public:
    FaceEdgeIndex () : m_face (0), m_edgeIndex (0) {}
    FaceEdgeIndex (const OrientedFace* face, size_t edgeIndex) :
    m_face (face), m_edgeIndex (edgeIndex) {}
    void AddQueue (
	list<FaceEdgeIndex>* queue, OrientedFace* fit);
    OrientedFace* FitAndDuplicateFace (Body* body) const;


public:
    friend ostream& operator<< (ostream& ostr, const FaceEdgeIndex& fei);
    static bool GetFaceIntersection (
	const OrientedFace& firstFace, const OrientedFace& secondFace,
	FaceEdgeIndex* first, FaceEdgeIndex* second);
    static void Initialize (list<FaceEdgeIndex>* queue,
			    Body* body);
    static void TwoConnectedFaces (
	Body* body, FaceEdgeIndex* first, FaceEdgeIndex* second);

public:
    const OrientedFace* m_face;
    /**
     * Index in the oriented face
     */
    size_t m_edgeIndex; 
};


#endif //__FACE_EDGE_INDEX_H__
