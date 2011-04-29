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
	ParsingData* parsingData, ExpressionTree* expressionTree) :
	m_parsingData (parsingData), m_expressionTree (expressionTree)
    {
    }

    double operator () (const double& y)
    {
	m_parsingData->SetVariable ("y", y);
	return m_expressionTree->Value ();
    }

private:
    ParsingData* m_parsingData;
    ExpressionTree* m_expressionTree;
};


// Methods
// ======================================================================

ConstraintEdge::ConstraintEdge (
    ParsingData* parsingData,
    const boost::shared_ptr<Vertex>& begin,
    const boost::shared_ptr<Vertex>& end, double min, double max) :

    ApproximationEdge (
	begin, end, 
	G3D::Vector3int16(0, 0, 0), 0, ElementStatus::ORIGINAL),
    m_parsingData (parsingData),
    m_min (min), m_max (max)
{    
    cachePoints ();
}

G3D::Vector3 ConstraintEdge::computePoint (size_t i) const
{
    size_t constraintIndex = GetBegin ()->GetConstraintIndexes ()[0] - 1;
    ExpressionTree* constraint = m_parsingData->GetConstraint (constraintIndex);
    G3D::Vector3 begin = GetBegin ()->GetVector ();
    G3D::Vector3 end = GetEnd ()->GetVector ();
    double x = begin.x + (end.x - begin.x) * i / (GetPointCount () - 1);
    m_parsingData->SetVariable ("x", x);
    mt::eps_tolerance<double> tol(numeric_limits<double>::digits - 3);
    boost::uintmax_t maxIter (100);
    return G3D::Vector3 (
	x, 
	mt::bisect (
	    ConstraintEvaluator (m_parsingData, constraint),
	    m_min, m_max, tol, maxIter).first, 
	0);
}
