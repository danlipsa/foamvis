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
		    const boost::shared_ptr<Vertex>& end);

protected:
    virtual G3D::Vector3 computePoint (size_t i) const;

private:
    G3D::Vector3 computePointMulti (size_t i, bool* success) const;
    void fixPoints ();
    void fixPoint (size_t i, const vector<int>& side, int correctSide);
    void computeSide (vector<int>* side, size_t* countPlus,
		      size_t* countMinus, size_t* countZero);


private:
    ParsingData* m_parsingData;
    boost::shared_ptr<ExpressionTree> m_constraint;
};


#endif //__CONSTRAINT_EDGE_H__

// Local Variables:
// mode: c++
// End:
