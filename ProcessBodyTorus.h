/**
 * @file   ProcessBodyTorus.h
 * @author Dan R. Lipsa
 * @date 16 March 2010
 *
 * Declaration of the ProcessBodyTorus.h
 */

#ifndef __PROCESS_BODY_TORUS_H__
#define __PROCESS_BODY_TORUS_H__

#include "OrientedFaceIndex.h"
class Body;
class Foam;
class OrientedFace;

class ProcessBodyTorus
{
public:
    ProcessBodyTorus (const Foam& foam, const boost::shared_ptr<Body>& body);
    void Initialize ();
    bool Step (VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet);
    void Unwrap (VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet);

private:
    void push (boost::shared_ptr<OrientedFace>  of);
    bool pop (OrientedFaceIndex* orientedFaceIndex,
	      OrientedFaceIndex* nextOrientedFaceIndex);
    void restrictFacesAroundAnEdge (const OrientedFaceIndex& ofi, 
				    vector<OrientedFaceIndex>* possibilities);
    bool chooseFaceNeighbor (
	const OrientedFaceIndex& ofi, 
	const vector<OrientedFaceIndex>& possibilities,
	OrientedFaceIndex* nextOrientedFaceIndex);

private:
    const Foam& m_foam;
    boost::shared_ptr<Body> m_body;
    /**
     * Queue of edges together with the face they are part of.
     */
    queue<OrientedFaceIndex> m_queue;
    vector<bool> m_traversed;
};



#endif //__PROCESS_BODY_TORUS_H__

// Local Variables:
// mode: c++
// End:
