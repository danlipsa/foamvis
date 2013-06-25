/**
 * @file   ApproximationEdge.h
 * @author Dan R. Lipsa
 * @date 28 Apr. 2011
 * @ingroup data model
 * @brief Curved edge approximated with a sequence of points.
 */
#ifndef __APPROXIMATION_EDGE_H__
#define __APPROXIMATION_EDGE_H__

#include "Edge.h"

/**
 * @brief Curved edge approximated with a sequence of points.
 */
class ApproximationEdge : public Edge
{
public:
    ApproximationEdge (
	size_t pointCount,
	const boost::shared_ptr<Vertex>& begin,
	const boost::shared_ptr<Vertex>& end,
	const G3D::Vector3int16& endLocation, 
	size_t id, Edge::Type type,
	ElementStatus::Enum duplicateStatus = ElementStatus::ORIGINAL);
    virtual size_t GetPointCount () const;
    virtual G3D::Vector3 GetPoint (size_t i) const;
    void SetPoint (size_t i, const G3D::Vector3& p)
    {
	m_points[i] = p;
    }

protected:
    ApproximationEdge (const ApproximationEdge& approximationEdge);
    virtual boost::shared_ptr<Edge> createDuplicate (
	const OOBox& originalDomain,
	const G3D::Vector3& newBegin, VertexSet* vertexSet) const;


protected:
    vector<G3D::Vector3> m_points;
};


#endif //__APPROXIMATION_EDGE_H__

// Local Variables:
// mode: c++
// End:
