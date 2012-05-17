/**
 * @file   Utils.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of various utility functions
 */

#include "Body.h"
#include "Comparisons.h"
#include "Debug.h"
#include "Edge.h"
#include "Face.h"
#include "Foam.h"
#include "DebugStream.h"
#include "Utils.h"
#include "Vertex.h"



// G3D Helper functions
// ======================================================================

bool IsFuzzyZero (const G3D::Vector3& v)
{
    return v.squaredMagnitude () < fuzzyEpsilon * fuzzyEpsilon;
}

void Matrix2SetColumn (G3D::Matrix2* m, size_t column, const G3D::Vector2& v)
{
    const size_t SIZE = 2;
    for (size_t i = 0; i < SIZE; ++i)
	(*m)[i][column] = v[i];
}

template <typename M>
G3D::Matrix2 ToMatrix2 (const M& matrix)
{
    G3D::Matrix2 m2;
    Matrix2SetColumn (&m2, 0, matrix.column (0).xy ());
    Matrix2SetColumn (&m2, 1, matrix.column (1).xy ());
    return m2;
}

G3D::Matrix2 MatrixFromColumns (
    const G3D::Vector2& col0, const G3D::Vector2& col1)
{
    G3D::Matrix2 r;
    Matrix2SetColumn (&r, 0, col0);
    Matrix2SetColumn (&r, 1, col1);
    return r;
}

G3D::Matrix3 MatrixFromColumns (
    const G3D::Vector3& col0, const G3D::Vector3& col1, 
    const G3D::Vector3& col2)
{
    G3D::Matrix3 r;
    r.setColumn (0, col0);
    r.setColumn (1, col1);
    r.setColumn (2, col2);
    return r;
}


G3D::Matrix2 mult (const G3D::Matrix2& first, const G3D::Matrix2& second)
{
    const size_t SIZE = 2;
    G3D::Matrix2 m;
    //row
    for (size_t i = 0; i < SIZE; ++i)
	// column
	for (size_t j = 0; j < SIZE; ++j)
	{
	    m[i][j] = 0;
	    for (size_t k = 0; k < SIZE; ++k)
		m[i][j] += first[i][k] * second[k][j];
	}
    return m;
}

G3D::Rect2D toRect2D (G3D::AABox box)
{
    return G3D::Rect2D::xyxy (box.low ().xy (), box.high ().xy ());
}


// ToString functionality
// ======================================================================

ostream& operator<< (ostream& ostr, const G3D::AABox& box)
{
    ostr << "AABox(" << box.low () << ", " << box.high () << ")";
    return ostr;
}

ostream& operator<< (ostream& ostr, const G3D::Matrix2& m)
{
    ostr << "Matrix2(" << m[0][0] << ", " << m[0][1] << ", "
	 << m[1][0] << ", " << m[1][1] << ")";
    return ostr;
}

ostream& operator<< (ostream& ostr, const QBox3D& box)
{
    ostr << "QBox3D(" << box.minimum () << ", " << box.maximum () << ")";
    return ostr;
}


ostream& operator<< (ostream& ostr, const G3D::Rect2D& box)
{
    ostr << "Rect2D (" << box.x0y0 () << ", " << box.x1y1 () << ")";
    return ostr;
}

ostream& operator<< (ostream& ostr, const G3D::Plane& plane)
{
    G3D::Vector3 normal;
    float distance;
    plane.getEquation (normal, distance);
    ostr << "Plane (" << normal << ", " << distance << ")";
    return ostr;
}


ostream& operator<< (ostream& ostr, const QColor& color)
{
    return ostr << "QColor(" << color.red () << ", " << color.green () << ", "
		<< color.blue () << ")";
}

ostream& operator<< (ostream& ostr, const QwtDoubleInterval& interval)
{
    return ostr 
	<< "QwtDoubleInterval: "
	<< ((interval.borderFlags () & QwtDoubleInterval::ExcludeMinimum) ?
	    "(" : "[")
	<< interval.minValue () << ", "
	<< interval.maxValue ()
	<< ((interval.borderFlags () & QwtDoubleInterval::ExcludeMaximum) ?
	    ")" : "]");
}

ostream& operator<< (ostream& ostr, const vector<bool>& v)
{
    BOOST_FOREACH (bool b, v)
	ostr << (b ? "1" : "0");
    return ostr;
}

ostream& operator<< (ostream& ostr, const G3D::Matrix4& m)
{
    return ostr
	<< "G3D::Matrix4 (" << endl
	<< m.row (0) << endl
	<< m.row (1) << endl
	<< m.row (2) << endl
	<< m.row (3) << ")" << endl;
}

ostream& operator<< (ostream& ostr, const G3D::Matrix3& m)
{
    return ostr
	<< "G3D::Matrix3 (" << endl
	<< m.row (0) << endl
	<< m.row (1) << endl
	<< m.row (2) << ")" << endl;
}

ostream& operator<< (ostream& ostr, const G3D::Vector4& v)
{
    return ostr << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3];
}

ostream& operator<< (ostream& ostr, const QVector3D& v)
{
    return ostr << "(" << v.x () << ", " << v.y () << ", " << v.z () << ")";
}

ostream& operator<< (ostream& ostr, const G3D::Vector2& v)
{
    return ostr << "(" << v[0] << ", " << v[1] << ")";
}

ostream& operator<< (ostream& ostr, const QPoint& p)
{
    return ostr << "(" << p.x () << ", " << p.y () << ")";
}


template<typename U, typename V>
ostream& operator<< (ostream& ostr, const pair<U, V>& p)
{
    return ostr << "(" << p.first << ", " << p.second << ")";
}


// Unit vectors
// ======================================================================

const G3D::Vector3int16 Vector3int16Zero (0, 0, 0);

const G3D::Vector3int16& Vector3int16Unit (size_t direction)
{
    static const G3D::Vector3int16 unitVector3int16[3] = {
	G3D::Vector3int16 (1, 0, 0),
	G3D::Vector3int16 (0, 1, 0),
	G3D::Vector3int16 (0, 0, 1)
    };
    return unitVector3int16[direction];
}

// 3D Math functionality
// ======================================================================

void Scale (G3D::AABox* aabox, double change)
{
    using G3D::Vector3;
    Vector3 center = aabox->center ();
    Vector3 newLow = aabox->low () * change + center * (1 - change);
    Vector3 newHigh = aabox->high () * change + center * (1 - change);
    aabox->set (newLow, newHigh);
}

void Translate (G3D::AABox* aabox, const G3D::Vector3& translationRatio)
{
    G3D::Vector3 translation (translationRatio * aabox->extent ());
    *aabox = (*aabox + translation);
}

void Scale (G3D::Rect2D* aabox, double change)
{
    using G3D::Vector2;
    Vector2 center = aabox->center ();
    Vector2 newLow = aabox->x0y0 () * change + center * (1 - change);
    Vector2 newHigh = aabox->x1y1 () * change + center * (1 - change);
    *aabox = G3D::Rect2D::xyxy ( newLow, newHigh);
}

bool Intersection (
    const QBox3D& box, const QVector3D& _begin, const QVector3D& _end)
{
    QVector3D begin (_begin);
    QVector3D end (_end);
    // 2D case
    if (box.minimum ().z () == 0 && box.maximum ().z () == 0)
    {
	if (qFuzzyCompare (begin.z () + 1, 1))
	    begin.setZ (0);
	if (qFuzzyCompare (end.z () + 1, 1))
	    end.setZ (0);
    }
    QRay3D ray (QVector3D (begin.x (), begin.y (), 0),  end - begin);
    qreal minimum_t, maximum_t;
    if (box.intersection (ray, &minimum_t, &maximum_t))
    {
	const qreal begin_t = 0;
	const qreal end_t = 1;
	bool intersection = ! (maximum_t < begin_t || minimum_t > end_t);
	if (intersection)
	{
	    cdbg << "min_t: " << minimum_t << " max_t: " << maximum_t << endl;
	    cdbg << "begin: " << begin << " end: " << end << endl;
	    cdbg << "box: " << box << endl;
	}
	return intersection;
    }
    else
	return false;
}


// Conversions Qt - G3D
// ======================================================================

QVector2D ToQt (const G3D::Vector2& v)
{
    return QVector2D (v.x, v.y);
}

QVector3D ToQt (const G3D::Vector3& v)
{
    return QVector3D (v.x, v.y, v.z);
}

QBox3D ToQt (const G3D::AABox& box)
{
    return QBox3D (ToQt (box.low ()), ToQt (box.high ()));
}

G3D::Vector2 ToG3D (const QVector2D& v)
{
    return G3D::Vector2 (v.x (), v.y ());
}

G3D::Vector3 ToG3D (const QVector3D& v)
{
    return G3D::Vector3 (v.x (), v.y (), v.z ());
}

G3D::AABox ToG3D (const QBox3D& box)
{
    return G3D::AABox (ToG3D (box.minimum ()), ToG3D (box.maximum ()));
}


G3D::Vector2 QtToOpenGl (const QPoint& point, int windowHeight)
{
    return G3D::Vector2 (point.x (), windowHeight - point.y ());
}

int OpenGlToQt (int h, int windowHeight)
{
    return windowHeight - h;
}

string ColorToHtml (const QColor& color, const char* text)
{
    ostringstream ostr;
    ostr << "<table><td bgcolor=\"#" << hex << setfill ('0')
	 << setw (2) << color.red ()
	 << setw (2) << color.green ()
	 << setw (2) << color.blue ()
	 << "\"><font color=\"#"
	 << setw (2) << color.red ()
	 << setw (2) << color.green ()
	 << setw (2) << color.blue ()
	 << "\">" << text << "</font></td></table>";
    return ostr.str ();
}


// Conversions Qt - stl
// ======================================================================
QStringList ToQStringList (const vector<string>& v)
{
    QStringList list;
    BOOST_FOREACH (string s, v)
	list << QString::fromStdString (s);
    return list;
}


// Container algorithms
// ======================================================================

template<typename Container, 
	 typename ContainerIterator, 
	 typename ElementComparatorAlong>
void CalculateAggregate<Container, ContainerIterator, ElementComparatorAlong>::
operator() (Aggregate aggregate, Container& container, G3D::Vector3* v)
{
    using G3D::Vector3;
    ContainerIterator it;
    ElementComparatorAlong comparator (Vector3::X_AXIS);
    it = aggregate (container.begin (), container.end (), comparator);
    v->x = comparator (*it);
    comparator = ElementComparatorAlong (Vector3::Y_AXIS);
    it = aggregate (container.begin (), container.end (), comparator);
    v->y = comparator (*it);
    comparator = ElementComparatorAlong (Vector3::Z_AXIS);
    it = aggregate (container.begin (), container.end (), comparator);
    v->z = comparator (*it);
}

template<typename T>
int polyCentroid2D(T x[], T y[], size_t n, T *xCentroid, T *yCentroid, T *area)
{
    register size_t i, j;
    T ai, atmp = 0, xtmp = 0, ytmp = 0;
    if (n < 3)
	return 1;
    for (i = n-1, j = 0; j < n; i = j, j++)
    {
	ai = x[i] * y[j] - x[j] * y[i];
	atmp += ai;
	xtmp += (x[j] + x[i]) * ai;
	ytmp += (y[j] + y[i]) * ai;
    }
    *area = atmp / 2;
    if (atmp != 0)
    {
	*xCentroid =	xtmp / (3 * atmp);
	*yCentroid =	ytmp / (3 * atmp);
	return 0;
    }
    return 2;
}

template<typename T> vector<G3D::Vector3> GetEdgeVectors (const T& t)
{
    EdgeSet edges = t.GetEdgeSet ();
    vector<G3D::Vector3> v;
    BOOST_FOREACH (boost::shared_ptr<Edge> edge, edges)
    {
	for (size_t i = 0; i < edge->GetPointCount (); ++i)
	    v.push_back (edge->GetPoint (i));
    }
    return v;
}

template<typename T> G3D::AABox CalculateBoundingBox (const T& t)
{
    G3D::Vector3 low, high;
    vector<G3D::Vector3> v (GetEdgeVectors (t));
    CalculateAggregate <vector<G3D::Vector3>, vector<G3D::Vector3>::iterator, 
	VertexLessThanAlong> () (min_element, v, &low);
    CalculateAggregate <vector<G3D::Vector3>, vector<G3D::Vector3>::iterator, 
	VertexLessThanAlong>()(max_element, v, &high);
    return G3D::AABox (low, high);
}

QString ReadShader (const QString& resourceUrl)
{
    QFile file (resourceUrl);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	ThrowException ("Invalid resource: ", 
			resourceUrl.toStdString ());
    QTextStream in (&file);
    return in.readAll ();
}

boost::shared_ptr<QGLShader> CreateShader (const QString& resourceUrl,
					   QGLShader::ShaderType type)
{
    boost::shared_ptr<QGLShader> shader(new QGLShader (type));
    QString vsrc = ReadShader (resourceUrl);
    if (! shader->compileSourceCode(vsrc))
	ThrowException ("Compile error for ", 
			resourceUrl.toLatin1 ().constData ());
    return shader;
}


void SetCheckedNoSignals (QButtonGroup* buttonGroup, int buttonId, bool checked)
{
    buttonGroup->blockSignals (true);
    buttonGroup->button (buttonId)->setChecked (checked);
    buttonGroup->blockSignals (false);
}

void SetCurrentIndexNoSignals (QComboBox* comboBox, int index)
{
    comboBox->blockSignals (true);
    comboBox->setCurrentIndex (index);
    comboBox->blockSignals (false);    
}

template<typename T>
void SetCheckedNoSignals (T* checkBox, bool checked, bool enabled)
{
    checkBox->blockSignals (true);
    checkBox->setEnabled (enabled);
    checkBox->setChecked (checked);
    checkBox->blockSignals (false);    
}

template<typename T>
void SetValueNoSignals (T* t, size_t value)
{
    t->blockSignals (true);
    t->setValue (value);
    t->blockSignals (false);
}

template<typename T>
void SetValueAndMaxNoSignals (T* t, size_t value, size_t max)
{
    t->blockSignals (true);
    t->setMaximum (max);
    t->setValue (value);
    t->blockSignals (false);
}


G3D::Rect2D TexRectFromInsideRect (G3D::Rect2D insideRect)
{
    G3D::Rect2D enclosingRect = EncloseRotation (insideRect);
    return G3D::Rect2D::xyxy (
	TexCoord (enclosingRect, insideRect.x0y0 ()),
	TexCoord (enclosingRect, insideRect.x1y1 ()));
}

G3D::Vector2 TexCoord (G3D::Rect2D enclosingRect, G3D::Vector2 v)
{
    float width = enclosingRect.width ();
    float height = enclosingRect.height ();
    return G3D::Vector2 ((v.x - enclosingRect.x0 ()) / width,
			 (v.y - enclosingRect.y0 ()) / height);
}


G3D::AABox EncloseRotation (const G3D::AABox& box)
{
    using G3D::Vector3;
    Vector3 center = box.center ();
    double halfSideLength = (box.high () - center).length ();
    Vector3 halfDiagonal = halfSideLength * 
	(Vector3::unitX () + Vector3::unitY () + Vector3::unitZ ());
    return G3D::AABox (center - halfDiagonal, center + halfDiagonal);
}

G3D::AABox EncloseRotation2D (const G3D::AABox& box)
{
    G3D::Vector3 center = box.center ();
    float halfSideLength = (box.high ().xy () - center.xy ()).length ();
    G3D::Vector3 halfDiagonal = halfSideLength * 
	(G3D::Vector3::unitX () + 
	 G3D::Vector3::unitY () + G3D::Vector3::unitZ ());
    return G3D::AABox (center - halfDiagonal, center + halfDiagonal);
}

G3D::Rect2D EncloseRotation (const G3D::Rect2D& rect)
{
    G3D::Vector2 center = rect.center ();
    float halfSideLength = (rect.x1y1 () - center).length ();
    G3D::Vector2 halfDiagonal = halfSideLength * 
	(G3D::Vector2::unitX () + G3D::Vector2::unitY ());
    return G3D::Rect2D::xyxy (center - halfDiagonal, center + halfDiagonal);
}

float IndexExponent2Value (
    const QSlider* slider, const pair<float,float>& exponentMinMax)
{
    int index = slider->value ();
    size_t maxSlider = slider->maximum ();
    float exp = exponentMinMax.first + (double (index) / maxSlider) * 
	(exponentMinMax.second - exponentMinMax.first);
    return pow (2, exp);
}

int Value2ExponentIndex (QSlider* slider, 
		 const pair<float,float>& exponentMinMax, float value)
{
    size_t maxSlider = slider->maximum ();
    size_t minSlider = slider->minimum ();
    float exp = log (value) / log (2.0);
    return minSlider + 
	floor (
	    (exp - exponentMinMax.first) / 
	    (exponentMinMax.second - exponentMinMax.first) * 
	    (maxSlider - minSlider));
}

float Index2Value (const QSlider* slider, const pair<float,float>& minMax)
{
    int index = slider->value ();
    size_t maxSlider = slider->maximum ();
    return minMax.first + (double (index) / maxSlider) * 
	(minMax.second - minMax.first);
}

int Value2Index (QSlider* slider, 
		 const pair<float,float>& minMax, float value)
{
    size_t maxSlider = slider->maximum ();
    size_t minSlider = slider->minimum ();
    return minSlider + 
	floor ((value - minMax.first) / (minMax.second - minMax.first) * 
	       (maxSlider - minSlider)); 
}

G3D::Vector2 rotateRadians (G3D::Vector2 v, float radiansCounterClockwise)
{
    G3D::Matrix2 m (
	cos (radiansCounterClockwise), -sin (radiansCounterClockwise), 
	sin (radiansCounterClockwise), cos (radiansCounterClockwise));
    return m*v;
}

G3D::Vector2 rotateDegrees (G3D::Vector2 v, float degrees)
{
    return rotateRadians (v, G3D::toRadians (degrees));
}

// Template instantiations
//======================================================================

/// @cond

template void SetCheckedNoSignals<QCheckBox> (QCheckBox*, bool, bool);
template void SetCheckedNoSignals<QRadioButton> (QRadioButton*, bool, bool);
template void SetValueNoSignals<QSlider>(QSlider*, unsigned long);
template void SetValueAndMaxNoSignals<QSpinBox>(
    QSpinBox*, unsigned long, unsigned long);

template int polyCentroid2D<double>(
    double*, double*, unsigned long, double*, double*, double*);

template std::vector<G3D::Vector3, std::allocator<G3D::Vector3> > GetEdgeVectors<Body>(Body const&);

template G3D::AABox CalculateBoundingBox<Body>(Body const&);

template void CalculateAggregate<std::set<boost::shared_ptr<Vertex>, VertexLessThan, std::allocator<boost::shared_ptr<Vertex> > >, std::_Rb_tree_const_iterator<boost::shared_ptr<Vertex> >, VertexLessThanAlong>::operator()(std::_Rb_tree_const_iterator<boost::shared_ptr<Vertex> > (*)(std::_Rb_tree_const_iterator<boost::shared_ptr<Vertex> >, std::_Rb_tree_const_iterator<boost::shared_ptr<Vertex> >, VertexLessThanAlong), std::set<boost::shared_ptr<Vertex>, VertexLessThan, std::allocator<boost::shared_ptr<Vertex> > >&, G3D::Vector3*);


template void CalculateAggregate<std::vector<G3D::Vector3*, std::allocator<G3D::Vector3*> >, __gnu_cxx::__normal_iterator<G3D::Vector3**, std::vector<G3D::Vector3*, std::allocator<G3D::Vector3*> > >, VertexLessThanAlong>::operator()(__gnu_cxx::__normal_iterator<G3D::Vector3**, std::vector<G3D::Vector3*, std::allocator<G3D::Vector3*> > > (*)(__gnu_cxx::__normal_iterator<G3D::Vector3**, std::vector<G3D::Vector3*, std::allocator<G3D::Vector3*> > >, __gnu_cxx::__normal_iterator<G3D::Vector3**, std::vector<G3D::Vector3*, std::allocator<G3D::Vector3*> > >, VertexLessThanAlong), std::vector<G3D::Vector3*, std::allocator<G3D::Vector3*> >&, G3D::Vector3*);

template void CalculateAggregate<std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Body>*, std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > > >, BBObjectLessThanAlongLow<Body> >::operator()(__gnu_cxx::__normal_iterator<boost::shared_ptr<Body>*, std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > > > (*)(__gnu_cxx::__normal_iterator<boost::shared_ptr<Body>*, std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Body>*, std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > > >, BBObjectLessThanAlongLow<Body>), std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > >&, G3D::Vector3*);

template void CalculateAggregate<std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Body>*, std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > > >, BBObjectLessThanAlongHigh<Body> >::operator()(__gnu_cxx::__normal_iterator<boost::shared_ptr<Body>*, std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > > > (*)(__gnu_cxx::__normal_iterator<boost::shared_ptr<Body>*, std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Body>*, std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > > >, BBObjectLessThanAlongHigh<Body>), std::vector<boost::shared_ptr<Body>, std::allocator<boost::shared_ptr<Body> > >&, G3D::Vector3*);

template void CalculateAggregate<std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > >, BBObjectLessThanAlongLow<Foam> >::operator()(__gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > > (*)(__gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > >, BBObjectLessThanAlongLow<Foam>), std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > >&, G3D::Vector3*);

template void CalculateAggregate<std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > >, BBObjectLessThanAlongHighTorus<Foam> >::operator()(__gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > > (*)(__gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > >, BBObjectLessThanAlongHighTorus<Foam>), std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > >&, G3D::Vector3*);


template void CalculateAggregate<std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > >, BBObjectLessThanAlongLowTorus<Foam> >::operator()(__gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > > (*)(__gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > >, BBObjectLessThanAlongLowTorus<Foam>), std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > >&, G3D::Vector3*);

template void CalculateAggregate<std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > >, BBObjectLessThanAlongHigh<Foam> >::operator()(__gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > > (*)(__gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > >, __gnu_cxx::__normal_iterator<boost::shared_ptr<Foam>*, std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > > >, BBObjectLessThanAlongHigh<Foam>), std::vector<boost::shared_ptr<Foam>, std::allocator<boost::shared_ptr<Foam> > >&, G3D::Vector3*);

template G3D::Matrix2 ToMatrix2<G3D::Matrix4>(G3D::Matrix4 const&);
template G3D::Matrix2 ToMatrix2<G3D::Matrix3>(G3D::Matrix3 const&);
/// @endcond
