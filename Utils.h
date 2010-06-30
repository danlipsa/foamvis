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

const G3D::Vector3int16& Vector3int16Unit (size_t direction);

extern const G3D::Vector3int16 Vector3int16Zero;
/**
 * Implementation of the Rainbow Colormap from 
 * Data Visualization, Principles and Practice,
 * Alexandru C. Telea
 */
void RainbowColor (double value, QColor* color);

#endif //__ELEMENT_UTILS_H__

// Local Variables:
// mode: c++
// End:
