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
// constraintLineEvaluator
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
    size_t Solve (size_t maxIter, double absoluteError, double relativeError);
    gsl_vector* GetRoot () const;
    
    static const char* ErrorToString (int error);

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

size_t Solver::Solve (size_t maxIter, double absoluteError, double relativeError)
{
    int result;
    gsl_vector* x;
    gsl_vector* dx;
    do
    {
	result = gsl_multiroot_fsolver_iterate (m_solver);
	if (result != 0 && result != GSL_ENOPROG)
	{
	    ostringstream ostr;
	    ostr << "Error gsl_multiroot_fsolver_iterate: " 
		 << ErrorToString (result);
	    ThrowException (ostr.str ());
	}
	x = gsl_multiroot_fsolver_root (m_solver);
	dx = gsl_multiroot_fsolver_dx (m_solver);
	result = gsl_multiroot_test_delta (dx, x, absoluteError, relativeError);
	--maxIter;
    }
    while (result == GSL_CONTINUE && maxIter > 0);
    return maxIter;
}

gsl_vector* Solver::GetRoot () const
{
    return gsl_multiroot_fsolver_root (m_solver);
}

const char* Solver::ErrorToString (int error)
{
    switch (error)
    {
    case GSL_EBADFUNC:
	return "GSL_EBADFUNC";
    case GSL_ENOPROG:
	return "Iteration is not making any progress (GSL_ENOPROG)";
    default:
	return "no error";
    }
}

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
    size_t constraintIndex = GetBegin ()->GetConstraintIndexes ()[0] - 1;
    m_constraint = m_parsingData->GetConstraint (constraintIndex);

boost::shared_ptr<ExpressionTree> simplifiedConstraint (
    m_constraint.GetFunction ()->GetSimplified ());
cdbg << simplifiedConstraint->ToString () << endl << endl;

    cachePoints ();
    SetAttribute<ColorAttribute, Color::Enum> (
	EdgeAttributeIndex::COLOR, Color::RED);
}

G3D::Vector3 ConstraintEdge::computePoint (size_t i) const
{
    const size_t NUMBER_ITERATIONS = 100;
    const double ABSOLUTE_ERROR = 1e-7;
    const double RELATIVE_ERROR = 1e-10;
    G3D::Vector3 begin = GetBegin ()->GetVector ();
    G3D::Vector3 end = GetEnd ()->GetVector ();
    double dx = abs (end.x - begin.x);
    double dy = abs (end.y - begin.y);
    G3D::Vector3 current = begin + (end - begin) * i / (GetPointCount () - 1);
    size_t constraintIndex = GetBegin ()->GetConstraintIndexes ()[0] - 1;
    Constraint constraint = m_parsingData->GetConstraint (constraintIndex);
    size_t axis = (dx > dy) ? 0 : 1;
    ConstraintLineParams clp (m_parsingData, constraint.GetFunction (),
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
    solver.Solve (NUMBER_ITERATIONS, ABSOLUTE_ERROR, RELATIVE_ERROR);
    gsl_vector* root = solver.GetRoot ();
    return G3D::Vector3 (gsl_vector_get (root, 0),
			 gsl_vector_get (root, 1), 0);
}
