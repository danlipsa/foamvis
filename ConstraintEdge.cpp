/**
 * @file   ConstraintEdge.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the ConstraintEdge class
 */
#include "Attribute.h"
#include "ConstraintEdge.h"
#include "Debug.h"
#include "DebugStream.h"
#include "ExpressionTree.h"
#include "ParsingData.h"
#include "Utils.h"
#include "Vertex.h"


// Private Functions and Classes
// ======================================================================
// ConstraintLineParams
// ======================================================================
struct ConstraintLineParams
{
    ConstraintLineParams (
	ParsingData* parsingData,
	const boost::shared_ptr<ExpressionTree>& expressionTree,
	G3D::Vector3 normal, G3D::Vector3 point) :

	m_parsingData (parsingData),
	m_expressionTree (expressionTree),
	m_normal (normal),
	m_point (point)
    {
	//cdbg << "normal=" << m_normal << " point=" << m_point << endl;
    }
    ParsingData* m_parsingData;
    boost::shared_ptr<ExpressionTree> m_expressionTree;
    G3D::Vector3 m_normal;
    G3D::Vector3 m_point;
};

// GslVector
// ======================================================================

struct GslVector
{
    GslVector (size_t n)
    {
	m_vector = gsl_vector_alloc (n);
    }
    ~GslVector ()
    {
	gsl_vector_free (m_vector);
    }
    void Set (size_t i, double value)
    {
	gsl_vector_set (m_vector, i, value);
    }
    gsl_vector* GetVector () const
    {
	return m_vector;
    }
private:
    gsl_vector* m_vector;
};

ostream& operator<< (ostream& ostr, const gsl_vector* v)
{
    G3D::Vector3 result (gsl_vector_get (v, 0),
			 gsl_vector_get (v, 1), 0);
    return ostr << result;
}


// constraintLineEvaluator
// ======================================================================
int constraintLineEvaluator (const gsl_vector* gslX, void* p, gsl_vector* f)
{
    ConstraintLineParams* params = static_cast<ConstraintLineParams*> (p);
    boost::array<double, 2> x;
    x[0] = gsl_vector_get(gslX, 0);
    x[1] = gsl_vector_get(gslX, 1);
    params->m_parsingData->SetVariable ("x", x[0]);
    params->m_parsingData->SetVariable ("y", x[1]);
    gsl_vector_set (f, 0, params->m_expressionTree->Value ());


    //size_t axis = (abs(params->m_normal.x) > abs(params->m_normal.y)) ? 0 : 1;
    //double value = params->m_point[axis];
    //gsl_vector_set (f, 1, x[axis] - value);

    gsl_vector_set (
	f, 1, 
	(x[0] - params->m_point[0])*params->m_normal[0] + 
	(x[1] - params->m_point[1])*params->m_normal[1]);
    //cdbg << "f (" << gslX << ")=" << f << endl;
    return GSL_SUCCESS;
}



// Solver
// ======================================================================
class Solver
{
public:
    Solver ();
    ~Solver ();
    void Set (gsl_multiroot_function *function, const gsl_vector *guess);
    bool Solve (size_t* maxIter, double absoluteError, double relativeError);
    gsl_vector* GetRoot () const;
    
    static string ErrorToString (int error);

private:
    gsl_multiroot_fsolver* m_solver;
};

Solver::Solver ()
{
    m_solver = gsl_multiroot_fsolver_alloc (gsl_multiroot_fsolver_hybrids, 2);
}

Solver::~Solver ()
{
    gsl_multiroot_fsolver_free (m_solver);
}

void Solver::Set (gsl_multiroot_function *function, const gsl_vector *guess)
{
    gsl_multiroot_fsolver_set (m_solver, function, guess);
}

bool Solver::Solve (
    size_t* maxIter, double absoluteError, double relativeError)
{
    int result;
    gsl_vector* x;
    gsl_vector* dx;
    do
    {
	result = gsl_multiroot_fsolver_iterate (m_solver);
	if (result != 0)
	    break;
	x = gsl_multiroot_fsolver_root (m_solver);
	dx = gsl_multiroot_fsolver_dx (m_solver);
	//cdbg << "x=" << x << "dx=" << dx << endl;
	result = gsl_multiroot_test_delta (dx, x, absoluteError, relativeError);
	--(*maxIter);
    } while (result == GSL_CONTINUE && (*maxIter) > 0);
    return result == GSL_SUCCESS;
}

gsl_vector* Solver::GetRoot () const
{
    return gsl_multiroot_fsolver_root (m_solver);
}

string Solver::ErrorToString (int error)
{
    ostringstream ostr;
    switch (error)
    {
    case GSL_EBADFUNC:
	ostr << "GSL_EBADFUNC";
	break;
    case GSL_ENOPROG:
	ostr << "Iteration is not making any progress (GSL_ENOPROG)";
	break;
    default:
	ostr << "Error: " << error << endl;
	break;
    }
    return ostr.str ();
}


// ConstraintEvaluator
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
	return m_expressionTree->Value ();
    }

private:
    string m_variableName;
    ParsingData* m_parsingData;
    boost::shared_ptr<ExpressionTree> m_expressionTree;
};

// Methods
// ======================================================================

ConstraintEdge::ConstraintEdge (
    ParsingData* parsingData,
    const boost::shared_ptr<Vertex>& begin,
    const boost::shared_ptr<Vertex>& end, 
    const G3D::AABox& foamBox, const G3D::AABox& bubbleBox) :

    ApproximationEdge (
	begin, end, 
	G3D::Vector3int16(0, 0, 0), 0, ElementStatus::ORIGINAL),
    m_parsingData (parsingData),
    m_foamBox (foamBox),
    m_bubbleBox (bubbleBox)
{
    size_t constraintIndex = GetBegin ()->GetConstraintIndex (0);
    m_constraint = m_parsingData->GetConstraint (constraintIndex);
    m_parsingData->UnsetVariable ("x");
    m_parsingData->UnsetVariable ("y");
    boost::shared_ptr<ExpressionTree> simplified (
	m_constraint->GetSimplified ());
    m_piecewise = simplified->HasConditional ();
    cachePoints ();
    fixPoints ();
    SetAttribute<ColorAttribute, Color::Enum> (
	EdgeAttributeIndex::COLOR, Color::RED);
}

void ConstraintEdge::fixPoints ()
{
    G3D::Vector3 begin = GetBegin ()->GetVector ();
    G3D::Vector3 end = GetEnd ()->GetVector ();
    size_t longAxis = 
	(abs (end[0] - begin[0]) > abs (end[1] - begin[1])) ? 0 : 1;
    vector<int> side (GetPointCount ());
    size_t other[] = {1, 0};
    size_t 
	countPlus = 0,
	countMinus = 0,
	countZero = 0;
    int correctSide;
    for (size_t i = 1; i < GetPointCount () - 1; ++i)
    {
	G3D::Vector3 point = GetPoint (i);
	float pointValue = 
	    point[other[longAxis]] - begin[other[longAxis]] -
	    (point[longAxis] - begin[longAxis]) * 
	    (end[other[longAxis]] - begin[other[longAxis]]) / 
	    (end[longAxis] - begin[longAxis]);
	side[i] = G3D::fuzzyGt (pointValue, 0.0) ? 1 : 
	    (G3D::fuzzyLt (pointValue, 0.0) ? -1 : 0);
	if (side[i] > 0)
	    ++countPlus;
	else if (side[i] < 0)
	    ++countMinus;
	else
	    ++countZero;
    }


    size_t maxCount = max (countPlus, max (countMinus, countZero));
    if (maxCount == countPlus)
	correctSide = 1;
    else if (maxCount == countMinus)
	correctSide = -1;
    else
	correctSide = 0;
    side[0] = side[side.size () - 1] = correctSide;
    
    for (size_t i = 1; i < GetPointCount () - 1; ++i)
	if (side[i] != correctSide)
	{
	    fixPoint (i, side, correctSide);
	    side[i] = correctSide;
	}
}

void ConstraintEdge::fixPoint (
    size_t i, const vector<int>& side, int correctSide)
{
    size_t left = i - 1;
    size_t right = i + 1;
    while (right < GetPointCount () && side[right] != correctSide)
	++right;
    SetPoint (i, 
	      GetPoint (left) + 
	      (GetPoint (right) - GetPoint (left)) / (right - left));
}


G3D::Vector3 ConstraintEdge::computePoint (size_t i) const
{
    bool success;
    G3D::Vector3 result;
    //if (m_piecewise)
	result = computePointMulti (i, &success);
	//else
	//result = computePointBisection (i, &success);
    return result;
}


G3D::Vector3 ConstraintEdge::computePointMulti (
    size_t i, bool* success) const
{
    const size_t NUMBER_ITERATIONS = 50;
    size_t numberIterations = NUMBER_ITERATIONS;
    const double ABSOLUTE_ERROR = GSL_SQRT_DBL_EPSILON;
    const double RELATIVE_ERROR = GSL_SQRT_DBL_EPSILON;
    G3D::Vector3 begin = GetBegin ()->GetVector ();
    G3D::Vector3 end = GetEnd ()->GetVector ();
    G3D::Vector3 current = begin + (end - begin) * i / (GetPointCount () - 1);
    size_t constraintIndex = GetBegin ()->GetConstraintIndex (0);
    boost::shared_ptr<ExpressionTree> constraint = 
	m_parsingData->GetConstraint (constraintIndex);
    ConstraintLineParams clp (m_parsingData, constraint,
			      end - begin, current);
    gsl_multiroot_function function;
    function.f = &constraintLineEvaluator;
    function.n = 2;
    function.params = &clp;
    GslVector guess (2);
    guess.Set (0, current[0]);
    guess.Set (1, current[1]);
    Solver solver;
    //cdbg << "guess=" << guess.GetVector () << endl;
    solver.Set (&function, guess.GetVector ());
    if (solver.Solve (&numberIterations, ABSOLUTE_ERROR, RELATIVE_ERROR))
    {
	gsl_vector* root = solver.GetRoot ();
	if (numberIterations == 0)
	{
	    *success = false;
	    cdbg << "Multi-root fail: # iterations,"
		 << " constraint=" << constraintIndex << ", index=" << i << endl;
	    return current;
	}
	else
	{
	    G3D::Vector3 result (gsl_vector_get (root, 0),
				 gsl_vector_get (root, 1), 0);
/*
	    if ((current - result).squaredLength () > 
		(m_bubbleBox.high () - m_bubbleBox.low ()).squaredLength () / 3)
	    {
		cdbg << "Multi-root fail: distant root, aabb bubble=" 
		     << m_bubbleBox << " root=" << result
		     << " constraint=" << constraintIndex 
		     << ", index=" << i << endl;
		*success = false;
		return current;
	    }
	    else
*/
	    {
/*
		cdbg << "Multi-root success, iterations " 
		     << (NUMBER_ITERATIONS - numberIterations) << " " 
		     << result
		     << " constraint=" << constraintIndex 
		     << ", index=" << i << endl;
*/
		*success = true;
		return result;
	    }

	    return result;
	}
	
    }
    else
    {
/*
	*success = false;
	cdbg << "Multi-root fail: no solution, " 
	     << " constraint=" << constraintIndex << ", index=" << i << endl;
*/
	return current;
    }
}


G3D::Vector3 ConstraintEdge::computePointBisection (
    size_t i, bool* success) const
{
    G3D::Vector3 begin = GetBegin ()->GetVector ();
    G3D::Vector3 end = GetEnd ()->GetVector ();
    double dx = abs (end.x - begin.x);
    double dy = abs (end.y - begin.y);
    G3D::Vector3 current = begin + (end - begin) * i / (GetPointCount () - 1);
    if (dx > dy)
    {
	double y = computeValueBisection (0, current, success);
	return G3D::Vector3 (current.x, y, 0);
    }
    else
    {
	double x = computeValueBisection (1, current, success);
	return G3D::Vector3 (x, current.y, 0);
    }
}

double ConstraintEdge::computeValueBisection (
    size_t axis, const G3D::Vector3& current, bool* success) const
{
    const char* AXIS_NAME[] = {"x", "y"};
    size_t other[] = {1, 0};
    boost::uintmax_t maxIter (100);
    mt::eps_tolerance<double> tol(numeric_limits<double>::digits - 3);
    size_t constraintIndex = GetBegin ()->GetConstraintIndex (0);
    boost::shared_ptr<ExpressionTree> constraint = 
	m_parsingData->GetConstraint (constraintIndex);
    double currentX = current[axis];
    m_parsingData->SetVariable (AXIS_NAME[axis], currentX);
    double middle = current[other[axis]];
    double min = m_foamBox.low ()[other[axis]];
    double max = m_foamBox.high ()[other[axis]];
    //double min = GetBegin ()->GetVector ()[other[axis]];
    //double max = GetEnd ()->GetVector ()[other[axis]];
    //if (min > max)
    //swap (min, max);

    double firstY;
    ConstraintEvaluator evaluator (
	AXIS_NAME[other[axis]], m_parsingData, constraint);
    *success = false;
    try
    {
	if (min < middle)
	{
	    firstY = mt::bisect (evaluator, min, middle, tol, maxIter).first;
	    *success = true;
	}
	else
	    firstY = -numeric_limits<double>::max ();
    }
    catch (exception& err)
    {
/*
	cdbg << endl << AXIS_NAME[other[axis]] << ": "
	     << min << " " << middle << " " << err.what () << endl;
	cdbg << evaluator (min) << ", " << evaluator (middle) << endl;	
	m_parsingData->UnsetVariable (AXIS_NAME[other[axis]]);
	boost::shared_ptr<ExpressionTree> simplifiedConstraint (
	    constraint->GetSimplified ());
	cdbg << simplifiedConstraint->ToString () << endl << endl;
*/

	firstY = -numeric_limits<double>::max ();	
    }

    double secondY;
    try 
    {
	if (middle < max)
	{
	    secondY = mt::bisect (evaluator, middle, max, tol, maxIter).first;
	    *success = true;
	}
	else
	    secondY = numeric_limits<double>::max ();
    }
    catch (exception& err)
    {
/*
	cdbg << endl  << AXIS_NAME[other[axis]] << ": "
	     << middle << " " << max << " " << err.what () << endl;
	cdbg << evaluator (middle) << ", " << evaluator (max) << endl;
	m_parsingData->UnsetVariable (AXIS_NAME[other[axis]]);
	boost::shared_ptr<ExpressionTree> simplifiedConstraint (
	    constraint->GetSimplified ());
	cdbg << simplifiedConstraint->ToString () << endl << endl;
*/
	secondY = numeric_limits<double>::max ();
    }
    double y = (*success) ? 
	((middle - firstY < secondY - middle) ? firstY : secondY) : 
	current[other[axis]];
    return y;
}
