/**
 * @file   ProcessBodyTorus.h
 * @author Dan R. Lipsa
 * @date 16 March 2010
 * @brief Processing done to "unwrap" bodies in torus model.
 * @ingroup data
 */

#ifndef __PROCESS_BODY_TORUS_H__
#define __PROCESS_BODY_TORUS_H__

#include "AdjacentOrientedFace.h"
class Body;
class Foam;
class OrientedFace;

/**
 * @brief Processing done to "unwrap" bodies in torus model.
 */
class ProcessBodyTorus
{
public:
    ProcessBodyTorus (const Foam& foam, const boost::shared_ptr<Body>& body);
    void Initialize ();
    bool Step (VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet);
    void Unwrap (VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet);

private:
    void push (boost::shared_ptr<OrientedFace>  of);
    bool pop (AdjacentOrientedFace* orientedFaceIndex,
	      AdjacentOrientedFace* nextOrientedFaceIndex);
    void restrictFacesAroundAnEdge (const AdjacentOrientedFace& ofi, 
				    vector<AdjacentOrientedFace>* possibilities);
    bool chooseFaceNeighbor (
	const AdjacentOrientedFace& ofi, 
	const vector<AdjacentOrientedFace>& possibilities,
	AdjacentOrientedFace* nextOrientedFaceIndex);

private:
    const Foam& m_foam;
    boost::shared_ptr<Body> m_body;
    /**
     * Queue of edges together with the face they are part of.
     */
    queue<AdjacentOrientedFace> m_queue;
    vector<bool> m_traversed;
};



#endif //__PROCESS_BODY_TORUS_H__

// Local Variables:
// mode: c++
// End:
