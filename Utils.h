/**
 * @file Utils.h
 * @author Dan R. Lipsa
 * 
 * Template function definitions that apply to Vertex, Edge, Face and Body.
 */
#ifndef __ELEMENT_UTILS_H__
#define __ELEMENT_UTILS_H__

/**
 * Pretty prints a G3D::AABox
 * @param ostr where to print
 * @param box what to print
 * @return where to print next
 */
ostream& operator<< (ostream& ostr, const G3D::AABox& box);
ostream& operator<< (ostream& ostr, const QColor& color);
ostream& operator<< (ostream& ostr, const QwtDoubleInterval& interval);

template<typename U, typename V>
ostream& operator<< (ostream& ostr, const pair<U, V>& p)
{
    return ostr << "pair(" << p.first << ", " << p.second << ")";
}

inline ostream& operator<< (ostream& ostr, const vector<bool>& v)
{
    BOOST_FOREACH (bool b, v)
	ostr << (b ? "1" : "0");
    return ostr;
}


const G3D::Vector3int16& Vector3int16Unit (size_t direction);

extern const G3D::Vector3int16 Vector3int16Zero;

inline void glTranslate (const G3D::Vector3& translate)
{
    glTranslatef (translate.x, translate.y, translate.z);
}

void Scale (G3D::AABox* aabox, double change);
void Scale (G3D::Rect2D* aabox, double change);
void EncloseRotation (G3D::AABox* aabox);
/**
 * Returns the luminance of a color using a formula from 
 * The Visualization Toolkit, 4th edition,
 * An Object-Oriented Approach to 3D Graphics
 * Will Schroeder, Ken Martin, Bill Lorensen
 * Section 8.9 Scalars and Color, page 295
 */
inline double Luminance (const QColor& color)
{
    return color.alphaF () * 
	(0.3 * color.redF () + 0.59 * color.greenF () + 0.11 * color.blueF ());
}

template <typename Container, 
	  typename ContainerIterator,
	  typename ContainerKeyType>
ContainerIterator fuzzyFind (const Container& s, const ContainerKeyType& x)
{
    ContainerIterator it = s.lower_bound (x);
    if (it != s.end () && (*it)->fuzzyEq (*x))
	return it;
    if (it != s.begin ())
    {
	--it;
	if ((*(it))->fuzzyEq (*x))
	    return it;
    }
    return s.end ();
}


#endif //__ELEMENT_UTILS_H__

// Local Variables:
// mode: c++
// End:
