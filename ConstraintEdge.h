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
		    vector< pair<size_t, size_t> >* pointsToFix, 
		    size_t bodyIndex);
    size_t GetConstraintIndex () const;
    void FixPointsConcaveOrConvex ();
    G3D::Vector3 ComputePointMulti (
	size_t i, bool* success,
	G3D::Vector2 previousTimeStepPoint) const
    {
	return computePointMulti (i, success, &previousTimeStepPoint);
    }
    void SetPoint (size_t i, const G3D::Vector3& p, bool valid)
    {
	ApproximationEdge::SetPoint (i, p);
	m_valid[i] = valid;
    }


private:
    G3D::Vector3 computePointMulti (
	size_t i, bool* success,
	const G3D::Vector2* previousTimeStepPoint = 0) const;
    void cachePoints ();
    void fixPoint (size_t i, const vector<int>& side, int correctSide);
    void fixPointInTriple (size_t i, int correctSide);
    void computeSide (vector<int>* side,
		      size_t* countPlus, size_t* countMinus, 
		      size_t* countZero, size_t* countInvalid);
    int computeCorrectSide (
	size_t countPlus,  size_t countMinus, size_t countZero);
    float computeScore (
	size_t countPlus,  size_t countMinus, size_t countZero, 
	size_t countInvalid);
    void storePointsToFix (
	vector< pair<size_t, size_t> >* pointsToFix, size_t bodyIndex);

private:
    ParsingData* m_parsingData;
    vector<bool> m_valid;
};


#endif //__CONSTRAINT_EDGE_H__

// Local Variables:
// mode: c++
// End:
