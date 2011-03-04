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
/**
 * Finds an item in a container using fuzzy comparison
 */
template <typename Container, 
	  typename ContainerIterator,
	  typename ContainerKeyType>
ContainerIterator fuzzyFind (const Container& s, const ContainerKeyType& x);

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
ostream& operator<< (ostream& ostr, const QColor& color);
ostream& operator<< (ostream& ostr, const QwtDoubleInterval& interval);
ostream& operator<< (ostream& ostr, const vector<bool>& v);
ostream& operator<< (ostream& ostr, const G3D::Matrix4& m);
ostream& operator<< (ostream& ostr, const G3D::Vector4& v);
ostream& operator<< (ostream& ostr, const G3D::Vector2& v);
ostream& operator<< (ostream& ostr, const QVector3D& v);
ostream& operator<< (ostream& ostr, const QPoint& p);
ostream& operator<< (ostream& ostr, const QBox3D& p);

template<typename U, typename V>
ostream& operator<< (ostream& ostr, const pair<U, V>& p);


// Unit vectors
// ======================================================================

const G3D::Vector3int16& Vector3int16Unit (size_t direction);
extern const G3D::Vector3int16 Vector3int16Zero;

// 3D Math functionality
// ======================================================================

void Scale (G3D::AABox* aabox, double change);
void Scale (G3D::Rect2D* aabox, double change);
void Translate (G3D::AABox* aabox, const G3D::Vector3& translationRatio);
G3D::AABox EncloseRotation (const G3D::AABox& aabox, double t);
void AddBorder (G3D::AABox* aabox);
bool Intersection (
    const QBox3D& box, const QVector3D& begin, const QVector3D& end);

// Conversions Qt - G3D
// ======================================================================

QVector2D ToQt (const G3D::Vector2& v);
QVector3D ToQt (const G3D::Vector3& v);
QBox3D ToQt (const G3D::AABox& box);
G3D::Vector2 ToG3D (const QVector2D& v);
G3D::Vector3 ToG3D (const QVector3D& v);
G3D::AABox ToG3D (const QBox3D& box);
G3D::Vector2 ToOpenGl (const QPoint& point, int windowHeight);

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

// Other
// ======================================================================

#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

const size_t HISTOGRAM_INTERVALS = 256;


#endif //__UTILS_H__

// Local Variables:
// mode: c++
// End:
