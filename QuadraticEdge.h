/**
 * @file   Edge.h
 * @author Dan R. Lipsa
 * 
 * Declaration of the QuadraticEdge class
 */
#ifndef __QUADRATIC_EDGE_H__
#define __QUADRATIC_EDGE_H__

#include "Edge.h"

class QuadraticEdge : public Edge
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
    virtual void Display (Color::Enum defaultColor, float alpha) const;

protected:
    QuadraticEdge (const QuadraticEdge& quadraticEdge);
    
private:
    boost::shared_ptr<Vertex> m_middle;
};


#endif //__QUADRATIC_EDGE_H__

// Local Variables:
// mode: c++
// End:
