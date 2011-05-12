/**
 * @file   ConstraintEdge.h
 * @author Dan R. Lipsa
 * @date 28 Apr. 2011
 * 
 * Declaration of the ConstraintEdge class
 */
#ifndef __CONSTRAINT_EDGE_H__
#define __CONSTRAINT_EDGE_H__

#include "ApproximationEdge.h"

class ExpressionTree;
class ParsingData;

class ConstraintEdge : public ApproximationEdge
{
public:
    ConstraintEdge (ParsingData* parsingData,
		    const boost::shared_ptr<Vertex>& begin,
		    const boost::shared_ptr<Vertex>& end,
		    const G3D::AABox& foamBox, const G3D::AABox& bubbleBox);

protected:
    virtual G3D::Vector3 computePoint (size_t i) const;
    G3D::Vector3 computePointMulti (size_t i, bool* success) const;
    G3D::Vector3 computePointBisection (size_t i, bool* success) const;
    double computeValueBisection (
	size_t axis, const G3D::Vector3& current, bool* success) const;
    void fixPoints ();
    void fixPoint (size_t i, const vector<int>& side, int correctSide);


private:
    ParsingData* m_parsingData;
    G3D::AABox m_foamBox;
    G3D::AABox m_bubbleBox;
    G3D::Vector3 m_center;
    boost::shared_ptr<ExpressionTree> m_constraint;
    bool m_piecewise;
};


#endif //__CONSTRAINT_EDGE_H__

// Local Variables:
// mode: c++
// End:
