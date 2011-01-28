/**
 * @file Utils.h
 * @author Dan R. Lipsa
 * 
 * Utility functions
 */
#ifndef __UTILS_H__
#define __UTILS_H__

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

const G3D::Vector3int16& Vector3int16Unit (size_t direction);

/**
 * Fix of G3D version 8.0, 
 * Vector3::isZero () which tests against fuzzyEpsilon instead of 
 * fuzzyEpsilon squared.
 */
bool isFuzzyZero (const G3D::Vector3& v);


extern const G3D::Vector3int16 Vector3int16Zero;

void Scale (G3D::AABox* aabox, double change);
void Scale (G3D::Rect2D* aabox, double change);
void EncloseRotation (G3D::AABox* aabox);
void AddBorder (G3D::AABox* aabox);

template <typename Container, 
	  typename ContainerIterator,
	  typename ContainerKeyType>
ContainerIterator fuzzyFind (const Container& s, const ContainerKeyType& x);

#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

const size_t HISTOGRAM_INTERVALS = 256;

QVector2D ToQt (const G3D::Vector2& v);
QVector3D ToQt (const G3D::Vector3& v);
QBox3D ToQt (const G3D::AABox& box);
G3D::Vector2 ToG3D (const QVector2D& v);
G3D::Vector3 ToG3D (const QVector3D& v);
G3D::AABox ToG3D (const QBox3D& box);
QVector2D MapToOpenGl (const QPoint& point, int windowHeight = 0);



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

bool intersection (
    const QBox3D& box, const QVector3D& begin, const QVector3D& end);


#endif //__UTILS_H__

// Local Variables:
// mode: c++
// End:
