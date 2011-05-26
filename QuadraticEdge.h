/**
 * @file   Edge.h
 * @author Dan R. Lipsa
 * 
 * Declaration of the QuadraticEdge class
 */
#ifndef __QUADRATIC_EDGE_H__
#define __QUADRATIC_EDGE_H__

#include "ApproximationEdge.h"

class QuadraticEdge : public ApproximationEdge
{
public:
    QuadraticEdge (
	const boost::shared_ptr<Vertex>& begin,
	const boost::shared_ptr<Vertex>& end,
	const boost::shared_ptr<Vertex>& middle,
	const G3D::Vector3int16& endLocation, 
	size_t id,
	ElementStatus::Enum duplicateStatus = ElementStatus::ORIGINAL);
    boost::shared_ptr<Vertex> GetMiddle () const
    {
	return m_middle;
    }
    virtual boost::shared_ptr<Edge> Clone () const;
    virtual void SetEnd(boost::shared_ptr<Vertex> end);

protected:
    QuadraticEdge (const QuadraticEdge& quadraticEdge);
    virtual boost::shared_ptr<Edge> createDuplicate (
	const OOBox& periods,
	const G3D::Vector3& newBegin, VertexSet* vertexSet) const;

private:
    /**
     * Quadratic function applied to parameter t in [0, 2]
     */
    void cachePoints ();
    G3D::Vector3 computePoint (size_t i) const;
    double quadratic (double t, size_t axis) const;
    G3D::Vector3 quadratic (double t) const;
    void setMiddle (boost::shared_ptr<Vertex> middle)
    {
	m_middle = middle;
    }

private:
    boost::shared_ptr<Vertex> m_middle;
};


#endif //__QUADRATIC_EDGE_H__

// Local Variables:
// mode: c++
// End:
