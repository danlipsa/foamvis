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


const G3D::Vector3int16& Vector3int16Unit (size_t direction);

extern const G3D::Vector3int16 Vector3int16Zero;
/**
 * Implementation of the Rainbow Colormap from 
 * Data Visualization, Principles and Practice,
 * Alexandru C. Telea
 * Section 5.2 Designing Effective Colormaps, page 132
 * Returns the rainbow color coresponding o a value between 0 and 1, 
 * blue=0, green=.5 red=1
 *
 */
QColor RainbowColor (double value);

/**
 * Cool warm color map from
 * Diverging Color Maps for Scientific Visualization
 * Kenneth Moreland
 * ISVC 2009
 *
 * Returns the coolwarm  color based on the index  value between 0 and
 * 256 inclusive.
 */
QColor BlueRedColor (size_t index);

inline void glTranslate (const G3D::Vector3& translate)
{
    glTranslatef (translate.x, translate.y, translate.z);
}

void Scale (G3D::AABox* aabox, float change);
void Scale (G3D::Rect2D* aabox, float change);
void EncloseRotation (G3D::AABox* aabox);
/**
 * Returns the luminance of a color using a formula from 
 * The Visualization Toolkit, 4th edition,
 * An Object-Oriented Approach to 3D Graphics
 * Will Schroeder, Ken Martin, Bill Lorensen
 * Section 8.9 Scalars and Color, page 295
 */
inline float Luminance (const QColor& color)
{
    return color.alphaF () * 
	(0.3 * color.redF () + 0.59 * color.greenF () + 0.11 * color.blueF ());
}

extern const size_t HISTOGRAM_INTERVALS;

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
