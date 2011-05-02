/**
 * @file   ConstraintEdge.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the ConstraintEdge class
 */

#include "ConstraintEdge.h"
#include "DebugStream.h"
#include "ExpressionTree.h"
#include "ParsingData.h"
#include "Vertex.h"

// Private Classes
// ======================================================================
class ConstraintEvaluator
 : public unary_function<double, double>
{
public:
    ConstraintEvaluator (
	const char* variableName,
	ParsingData* parsingData, ExpressionTree* expressionTree) :
	m_variableName (variableName),
	m_parsingData (parsingData), m_expressionTree (expressionTree)
    {
    }

    double operator () (const double& y)
    {
	m_parsingData->SetVariable (m_variableName, y);
	return m_expressionTree->Value ();
    }

private:
    string m_variableName;
    ParsingData* m_parsingData;
    ExpressionTree* m_expressionTree;
};


// Methods
// ======================================================================

ConstraintEdge::ConstraintEdge (
    ParsingData* parsingData,
    const boost::shared_ptr<Vertex>& begin,
    const boost::shared_ptr<Vertex>& end, const G3D::AABox& box) :

    ApproximationEdge (
	begin, end, 
	G3D::Vector3int16(0, 0, 0), 0, ElementStatus::ORIGINAL),
    m_parsingData (parsingData),
    m_box (box)
{    
    cachePoints ();
}

G3D::Vector3 ConstraintEdge::computePoint (size_t i) const
{
    G3D::Vector3 begin = GetBegin ()->GetVector ();
    G3D::Vector3 end = GetEnd ()->GetVector ();
    double dx = abs (end.x - begin.x);
    double dy = abs (end.y - begin.y);
    G3D::Vector3 current = begin + (end - begin) * i / (GetPointCount () - 1);
    if (dx > dy)
    {
	double y = computeValue (0, current);
	return G3D::Vector3 (current.x, y, 0);
    }
    else
    {
	double x = computeValue (1, current);
	return G3D::Vector3 (x, current.y, 0);
    }
}


double ConstraintEdge::computeValue (
    size_t axis, const G3D::Vector3& current) const
{
    const char* axisName[] = {"x", "y"};
    size_t other[] = {1, 0};
    boost::uintmax_t maxIter (100);
    mt::eps_tolerance<double> tol(numeric_limits<double>::digits - 2);
    size_t constraintIndex = GetBegin ()->GetConstraintIndexes ()[0] - 1;
    ExpressionTree* constraint = m_parsingData->GetConstraint (constraintIndex);
    double currentX = current[axis];
    m_parsingData->SetVariable (axisName[axis], currentX);
    double currentY = current[other[axis]];
    double min = m_box.low ()[other[axis]];
    double max = m_box.high ()[other[axis]];
    double firstY;
    if (min < currentY)
	firstY = mt::bisect (
	    ConstraintEvaluator (
		axisName[other[axis]], m_parsingData, constraint),
	    min, currentY, tol, maxIter).first;
    else
	firstY = -numeric_limits<double>::max ();
    double secondY;
    if (currentY < max)
	secondY = mt::bisect (
	    ConstraintEvaluator (
		axisName[other[axis]], m_parsingData, constraint),
	    currentY, max, tol, maxIter).first;
    else
	secondY = numeric_limits<double>::max ();
    double y = (currentY - firstY < secondY - currentY) ? firstY : secondY;
    cdbg << min << " " << firstY << " " << currentY  << " " 
	 << secondY << " " << max << endl;
    return y;
}
