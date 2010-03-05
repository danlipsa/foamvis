/**
 * @file ElementUtils.h
 * @author Dan R. Lipsa
 * 
 * Template function definitions that apply to Vertex, Edge, Face and Body.
 */
#ifndef __ELEMENT_UTILS_H__
#define __ELEMENT_UTILS_H__

#include "SystemDifferences.h"


/**
 * Unary function  that prints an  object of type  E&. Used to  print a
 * sequence of objects.
 */
template <typename E>
struct PrintElement : public unary_function<E&, void>
{
public:
    /**
     * Constructs the object
     * @param ostr output stream where the object should be printed to
     * @param useEndOfLine use end of line or comma to separate object
     * of type E.
     */
    PrintElement (ostream& ostr, bool useEndOfLine) : 
    m_ostr(ostr), m_index(0), m_useEndOfLine(useEndOfLine) {
    }
    /**
     * Pretty prints an object
     * @param e the object to be printed.
     */
    void operator() (E& e) 
    {
        if (m_index > 0)
        {
            if (m_useEndOfLine)
                m_ostr << endl;
            else
                m_ostr << ", ";
        }
        m_ostr << m_index << ": " << e;
        m_index++;
    }
private:
    /**
     * Output stream to print the object to
     */
    ostream& m_ostr;
    /**
     * Keeps track  of how  many objects where  printed, and  prints an
     * index in front of each object printed.
     */
    int m_index;
    /**
     * Separates objects with an end of line or with a comma
     */
    bool m_useEndOfLine;
};

/**
 * Pretty prints a list of objects.
 * @param ostr output stream where to print the objects to
 * @param v vector of objects to be printed
 * @param elementName string describing an object in the list of objects printed
 * @param useEndOfLine objects are separated by an end of line or by a comma
 */
template <typename E>
ostream& PrintElements (
    ostream& ostr, vector<E>& v, const char* elementName, 
    bool useEndOfLine)
{
    ostr << v.size() << " " << elementName << ":"<< endl;
    for_each(v.begin (), v.end (), PrintElement<E>(ostr, useEndOfLine));
    ostr << endl;
    return ostr;
}

/**
 * Pretty prints a list of objects in reverse order
 * @param ostr output stream where to print the objects to
 * @param v vector of objects to  be printed in reverse order than how
 *        they are stored in the vector.
 * @param elementName string describing an object in the list of objects printed
 * @param useEndOfLine objects are separated by an end of line or by a comma
 */
template <typename E>
ostream& ReversePrintElements (ostream& ostr, vector<E>& v, 
			       const char* elementName, bool useEndOfLine)
{
    ostr << v.size() << " " << elementName << ":"<< endl;
    for_each(v.rbegin (), v.rend (), PrintElement<E>(ostr, useEndOfLine));
    ostr << endl;
    return ostr;
}

/**
 * Binary function that compares two C strings ignoring the case
 */
struct LessThanNoCase : binary_function<const char*, const char*, bool>
{
    /**
     * Compares two character pointers
     * @param s1 first C string
     * @param s2 second C string
     * @return true if the first argument is less than the second argument.
     */
    bool operator()(const char* s1, const char* s2) 
    {
        return strcasecmp(s1, s2) < 0;
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

G3D::Vector3 aNormal (const G3D::Vector3& v);

#endif //__ELEMENT_UTILS_H__

// Local Variables:
// mode: c++
// End:
