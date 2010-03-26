/**
 * @file ElementUtils.h
 * @author Dan R. Lipsa
 * 
 * Template function definitions that apply to Vertex, Edge, Face and Body.
 */
#ifndef __ELEMENT_UTILS_H__
#define __ELEMENT_UTILS_H__


template<typename O>
struct DeletePointer : public unary_function<O*, void>
{
    void operator() (O* p) const
    {
	delete p;
    }
};

/**
 * Pretty prints a G3D::AABox
 * @param ostr where to print
 * @param box what to print
 * @return where to print next
 */
ostream& operator<< (ostream& ostr,  G3D::AABox& box);

G3D::Matrix2 inverse(const G3D::Matrix2& original);


#endif //__ELEMENT_UTILS_H__

// Local Variables:
// mode: c++
// End:
