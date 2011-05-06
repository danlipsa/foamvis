/**
 * @file   ConstraintEdge.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the ConstraintEdge class
 */
#include "Attribute.h"
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
	ParsingData* parsingData, 
	boost::shared_ptr<ExpressionTree> expressionTree) :
	m_variableName (variableName),
	m_parsingData (parsingData), m_expressionTree (expressionTree)
    {
    }

    double operator () (const double& y)
    {
	m_parsingData->SetVariable (m_variableName, y);
	cdbg << endl;
	return m_expressionTree->Value ();
    }

private:
    string m_variableName;
    ParsingData* m_parsingData;
    boost::shared_ptr<ExpressionTree> m_expressionTree;
};

// Static fields
// ======================================================================
const char* ConstraintEdge::AXIS_NAME[] = {"x", "y"};


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
    SetAttribute<ColorAttribute, Color::Enum> (
	EdgeAttributeIndex::COLOR, Color::RED);
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
    size_t other[] = {1, 0};
    boost::uintmax_t maxIter (100);
    mt::eps_tolerance<double> tol(numeric_limits<double>::digits - 3);
    size_t constraintIndex = GetBegin ()->GetConstraintIndexes ()[0] - 1;
    boost::shared_ptr<ExpressionTree> constraint = 
	m_parsingData->GetConstraint (constraintIndex);
    double currentX = current[axis];
    m_parsingData->SetVariable (AXIS_NAME[axis], currentX);

    double currentY = current[other[axis]];
    double min = m_box.low ()[other[axis]];
    double max = m_box.high ()[other[axis]];
    //double min = GetBegin ()->GetVector ()[other[axis]];
    //double max = GetEnd ()->GetVector ()[other[axis]];
    if (min > max)
	swap (min, max);

    double firstY;
    ConstraintEvaluator evaluator (
	AXIS_NAME[other[axis]], m_parsingData, constraint);
    try
    {
	if (min < currentY)
	    firstY = mt::bisect (evaluator, min, currentY, tol, maxIter).first;
	else
	    firstY = -numeric_limits<double>::max ();
    }
    catch (exception& err)
    {
	cdbg << endl << min << " " << currentY << " " << err.what () << endl;
	cdbg << evaluator (min) << ", " << evaluator (currentY) << endl;

	
m_parsingData->UnsetVariable (AXIS_NAME[other[axis]]);
boost::shared_ptr<ExpressionTree> simplifiedConstraint (constraint->GetSimplified ());
cdbg << simplifiedConstraint->ToString () << endl << endl;


	firstY = -numeric_limits<double>::max ();	
    }

    double secondY;
    try 
    {
	if (currentY < max)
	    secondY = mt::bisect (evaluator, currentY, max, tol, maxIter).first;
	else
	    secondY = numeric_limits<double>::max ();
    }
    catch (exception& err)
    {
	cdbg << endl << currentY << " " << max << " " << err.what () << endl;
	cdbg << evaluator (currentY) << ", " << evaluator (max) << endl;


m_parsingData->UnsetVariable (AXIS_NAME[other[axis]]);
boost::shared_ptr<ExpressionTree> simplifiedConstraint (constraint->GetSimplified ());
cdbg << simplifiedConstraint->ToString () << endl << endl;

	secondY = numeric_limits<double>::max ();
    }
    double y = (currentY - firstY < secondY - currentY) ? firstY : secondY;

    return y;
}
