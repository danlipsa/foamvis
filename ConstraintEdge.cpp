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
	size_t axis,
	double value) :

	m_parsingData (parsingData),
	m_expressionTree (expressionTree),
	m_axis (axis),
	m_value (value)
    {
    }
    ParsingData* m_parsingData;
    boost::shared_ptr<ExpressionTree> m_expressionTree;
    size_t m_axis;
    double m_value;
};

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
    gsl_vector_set (f, 1, x[params->m_axis] - params->m_value);
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
    const G3D::AABox& box, const G3D::Vector3& center) :

    ApproximationEdge (
	begin, end, 
	G3D::Vector3int16(0, 0, 0), 0, ElementStatus::ORIGINAL),
    m_parsingData (parsingData),
    m_box (box),
    m_center (center)
{
    size_t constraintIndex = GetBegin ()->GetConstraintIndex (0);
    m_constraint = m_parsingData->GetConstraint (constraintIndex);
    m_parsingData->UnsetVariable ("x");
    m_parsingData->UnsetVariable ("y");
    boost::shared_ptr<ExpressionTree> simplified (
	m_constraint->GetSimplified ());
    m_piecewise = simplified->HasConditional ();
    cachePoints ();
    SetAttribute<ColorAttribute, Color::Enum> (
	EdgeAttributeIndex::COLOR, Color::RED);
}

G3D::Vector3 ConstraintEdge::computePoint (size_t i) const
{
    bool success;
    G3D::Vector3 result;
    if (m_piecewise)
	result = computePointMulti (i, &success);
    else
	result = computePointBisection (i, &success);
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
    double dx = abs (end.x - begin.x);
    double dy = abs (end.y - begin.y);
    G3D::Vector3 current = begin + (end - begin) * i / (GetPointCount () - 1);
    size_t constraintIndex = GetBegin ()->GetConstraintIndex (0);
    boost::shared_ptr<ExpressionTree> constraint = 
	m_parsingData->GetConstraint (constraintIndex);
    size_t axis = (dx > dy) ? 0 : 1;
    ConstraintLineParams clp (m_parsingData, constraint,
			      axis, current[axis]);
    gsl_multiroot_function function;
    function.f = &constraintLineEvaluator;
    function.n = 2;
    function.params = &clp;
    GslVector guess (2);
    guess.Set (0, current[0]);
    guess.Set (1, current[1]);
    Solver solver;
    solver.Set (&function, guess.GetVector ());
    if (solver.Solve (&numberIterations, ABSOLUTE_ERROR, RELATIVE_ERROR))
    {
	gsl_vector* root = solver.GetRoot ();
	if (numberIterations == 0)
	{
	    *success = false;	    
	    cdbg << "Solution for constraint " 
		 << constraintIndex << " point " << i
		 << " excedeed " << NUMBER_ITERATIONS << endl;
	}
	else
	    *success = true;
	return G3D::Vector3 (gsl_vector_get (root, 0),
			     gsl_vector_get (root, 1), 0);
    }
    else
    {
	*success = false;
/*
	cdbg <<     "No solution for constraint " 
	     << constraintIndex << " point " << i << endl;
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
    double min = m_box.low ()[other[axis]];
    double max = m_box.high ()[other[axis]];
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
