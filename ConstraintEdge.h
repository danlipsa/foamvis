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
    void FixPointsConcaveOrConvex ();
    G3D::Vector3 ComputePointMulti (
	size_t i, bool* success,
	G3D::Vector2 previousTimeStepPoint) const
    {
	return computePointMulti (i, success, &previousTimeStepPoint);
    }
    void ChoosePoint (size_t i, const G3D::Vector3& p);

private:
    enum Side
    {
	SIDE_PLUS,
	SIDE_MINUS,
	SIDE_ZERO,
	SIDE_INVALID
    };

private:
    G3D::Vector3 computePointMulti (
	size_t i, bool* success,
	const G3D::Vector2* previousTimeStepPoint = 0) const;
    void cachePoints ();
    void fixPoint (size_t i, const vector<Side>& side, Side correctSide);
    void fixPointInTriple (size_t i, Side correctSide);
    void computeSide (vector<Side>* side,
		      size_t* countPlus, size_t* countMinus, 
		      size_t* countZero, size_t* countInvalid);
    Side computeCorrectSide (
	size_t countPlus,  size_t countMinus, size_t countZero);
    float computeScore (
	size_t countPlus,  size_t countMinus, size_t countZero, 
	size_t countInvalid);
    size_t storePointsToFix (
	vector< pair<size_t, size_t> >* pointsToFix, size_t bodyIndex);
    float distanceToNeighbors (G3D::Vector3 point, size_t i);

private:
    ParsingData* m_parsingData;
    vector<bool> m_valid;
};


#endif //__CONSTRAINT_EDGE_H__

// Local Variables:
// mode: c++
// End:
