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

struct Vector3int16Hash
{
    std::size_t operator() (const G3D::Vector3int16& v) const
    {
	std::size_t seed = 0;
	boost::hash_combine (seed, v.x);
	boost::hash_combine (seed, v.y);
	boost::hash_combine (seed, v.z);
	return seed;
    }
};

#endif //__ELEMENT_UTILS_H__

// Local Variables:
// mode: c++
// End:
