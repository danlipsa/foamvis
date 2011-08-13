/**
 * @file Utils.h
 * @author Dan R. Lipsa
 * 
 * Utility functions
 */
#ifndef __UTILS_H__
#define __UTILS_H__

// Fuzzy equality functionality
// ======================================================================

/**
 * Fix of G3D version 8.0, 
 * Vector3::isZero () which tests against fuzzyEpsilon instead of 
 * fuzzyEpsilon squared.
 */
bool IsFuzzyZero (const G3D::Vector3& v);

// ToString functionality
// ======================================================================

/**
 * Pretty prints a G3D::AABox
 * @param ostr where to print
 * @param box what to print
 * @return where to print next
 */
ostream& operator<< (ostream& ostr, const G3D::AABox& box);
ostream& operator<< (ostream& ostr, const G3D::Rect2D& rect);
ostream& operator<< (ostream& ostr, const G3D::Plane& p);
ostream& operator<< (ostream& ostr, const G3D::Matrix4& m);
ostream& operator<< (ostream& ostr, const G3D::Vector4& v);
ostream& operator<< (ostream& ostr, const G3D::Vector2& v);
ostream& operator<< (ostream& ostr, const G3D::Matrix2& m);
ostream& operator<< (ostream& ostr, const QColor& color);
ostream& operator<< (ostream& ostr, const QVector3D& v);
ostream& operator<< (ostream& ostr, const QPoint& p);
ostream& operator<< (ostream& ostr, const QBox3D& p);
ostream& operator<< (ostream& ostr, const QwtDoubleInterval& interval);
ostream& operator<< (ostream& ostr, const vector<bool>& v);

template<typename U, typename V>
ostream& operator<< (ostream& ostr, const pair<U, V>& p);


// Unit vectors
// ======================================================================

const G3D::Vector3int16& Vector3int16Unit (size_t direction);
extern const G3D::Vector3int16 Vector3int16Zero;
void Matrix2SetColumn (G3D::Matrix2* m, size_t i, const G3D::Vector2& v);
template <typename M> G3D::Matrix2 ToMatrix2 (const M& m);
G3D::Matrix2 ToMatrix2 (const G3D::Vector2& col1, const G3D::Vector2& col2);
G3D::Matrix2 mult (const G3D::Matrix2& first, const G3D::Matrix2& second);


// 3D Math functionality
// ======================================================================

void Scale (G3D::AABox* aabox, double change);
void Scale (G3D::Rect2D* aabox, double change);
void Translate (G3D::AABox* aabox, const G3D::Vector3& translationRatio);
void AddBorder (G3D::AABox* aabox);
bool Intersection (
    const QBox3D& box, const QVector3D& begin, const QVector3D& end);
/**
 * ANSI C code from the article
 * "Centroid of a Polygon"
 * by Gerard Bashein and Paul R. Detmer,
	(gb@locke.hs.washington.edu, pdetmer@u.washington.edu)
 * in "Graphics Gems IV", Academic Press, 1994
 *
 *********************************************************************
 polyCentroid2D: Calculates the centroid (xCentroid, yCentroid) and area
 of a polygon, given its vertices (x[0], y[0]) ... (x[n-1], y[n-1]). It
 is assumed that the contour is closed, i.e., that the vertex following
 (x[n-1], y[n-1]) is (x[0], y[0]).  The algebraic sign of the area is
 positive for counterclockwise ordering of vertices in x-y plane;
 otherwise negative.

 Returned values:  0 for normal execution;  1 if the polygon is
 degenerate (number of vertices < 3);  and 2 if area = 0 (and the
 centroid is undefined).
**********************************************************************/
template<typename T>
int polyCentroid2D(T x[], T y[], size_t n,
		 T* xCentroid, T* yCentroid, T* area);



// Conversions Qt - G3D
// ======================================================================

QVector2D ToQt (const G3D::Vector2& v);
QVector3D ToQt (const G3D::Vector3& v);
QBox3D ToQt (const G3D::AABox& box);
G3D::Vector2 ToG3D (const QVector2D& v);
G3D::Vector3 ToG3D (const QVector3D& v);
G3D::AABox ToG3D (const QBox3D& box);


// Conversions Qt - OpenGl
// ======================================================================
G3D::Vector2 QtToOpenGl (const QPoint& point, int windowHeight);
int OpenGlToQt (int h, int windowHeight);

string ColorToHtml (const QColor& color, const char* text);


// Container algorithms
// ======================================================================

template<typename Container, typename ContainerIterator, 
	 typename ElementComparatorAlong>
struct CalculateAggregate
{
    typedef ContainerIterator (*Aggregate) (
	ContainerIterator first, ContainerIterator second, 
	ElementComparatorAlong lessThan);
    
    void operator() (Aggregate aggregate, Container& container, 
		     G3D::Vector3* result);
};

template<typename Container> void resizeAllowIndex (Container* c, size_t i)
{
    if (i >= c->size ())
        c->resize (i + 1);
}

template<typename T> vector<G3D::Vector3> GetEdgeVectors (const T& t);
template<typename T> G3D::AABox CalculateBoundingBox (const T& t);


// Other
// ======================================================================

#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

const static size_t HISTOGRAM_INTERVALS = 256;
const static size_t INVALID_INDEX = numeric_limits<size_t>::max ();

#define CALL_MEMBER_FN(object,ptrToMember)  ((object)->*(ptrToMember))

QString ReadShader (const QString& resourceUrl);
boost::shared_ptr<QGLShader> CreateShader (const QString& resourceUrl,
					   QGLShader::ShaderType type);

//#define RESOURCE(name) ":/" name
#define RESOURCE(name) name

void SetCheckedNoSignals (QButtonGroup* buttonGroup, int buttonId, 
			  bool checked);
void SetCurrentIndexNoSignals (QComboBox* comboBox, int index);
void SetCheckedNoSignals (QCheckBox* checkBox, bool checked);

#endif //__UTILS_H__

// Local Variables:
// mode: c++
// End:
