/**
 * @file Element.h
 * @author Dan R. Lipsa
 * 
 * Template function definitions that apply to Vertex, Edge, Face and Body.
 */
#ifndef __ELEMENT_UTILS_H__
#define __ELEMENT_UTILS_H__

#include <functional>
#include <algorithm>
#include <iostream>
#include <vector>
#include "SystemDifferences.h"

/**
 * Deletes an object of type E*.
 * @param pe pointer to the object to be deleted
 */
template <class E> 
void DeleteElementPtr (const E* pe) 
{delete pe;}


/**
 * Unary function  that prints an  object of type  E&. Used to  print a
 * sequence of objects.
 */
template <class E>
struct PrintElement : public std::unary_function<E&, void>
{
public:
    /**
     * Constructs the object
     * @param ostr output stream where the object should be printed to
     * @param useEndOfLine use end of line or comma to separate object
     * of type E.
     */
    PrintElement (std::ostream& ostr, bool useEndOfLine) : 
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
    std::ostream& m_ostr;
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
template <class E>
std::ostream& PrintElements (
    std::ostream& ostr, std::vector<E>& v, const char* elementName, 
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
template <class E>
std::ostream& ReversePrintElements (std::ostream& ostr, std::vector<E>& v, 
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
struct LessThanNoCase : std::binary_function<const char*, const char*, bool>
{
    /**
     * Compares two character pointers
     * @param s1 first C string
     * @param s2 second C string
     * @return true if the first argument is less than the second argument.
     */
    bool operator()(const char* s1, const char* s2) const
    {
        return strcasecmp(s1, s2) < 0;
    }
};

#endif //__ELEMENT_UTILS_H__

// Local Variables:
// mode: c++
// End:
