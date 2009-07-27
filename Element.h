/**
 * @file Element.h
 * @author Dan R. Lipsa
 * 
 * Template function definitions that apply to Point, Edge, Face and Body.
 */
#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include <functional>
#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;


/**
 * Deletes an object of type E*.
 * @param pe pointer to the object to be deleted
 */
template <class E> 
void DeleteElementPtr (E* pe) 
{delete pe;}


/**
 * Unary function  that prints an  object of type  E*. Used to  print a
 * sequence of objects.
 */
template <class E>
struct PrintElementPtr : public unary_function<E*, void>
{
public:
    /**
     * Constructs the object
     * @param ostr output stream where the object should be printed to
     * @param useEndOfLine use end of line or comma to separate object
     * of type E.
     */
    PrintElementPtr (ostream& ostr, bool useEndOfLine) : 
    m_ostr(ostr), m_index(0), m_useEndOfLine(useEndOfLine) {
    }
    /**
     * Pretty prints an object
     * @param e the object to be printed.
     */
    void operator() (E* e) 
    {
	if (m_index > 0)
	{
	    if (m_useEndOfLine)
		m_ostr << endl;
	    else
		m_ostr << ", ";
	}
	m_ostr << m_index << ": " << *e;
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
 * Pretty prints a list of object stored as pointers.
 * @param ostr output stream where to print the objects to
 * @param v vector of objects to be printed
 * @param elementName string describing an object in the list of objects printed
 * @param useEndOfLine objects are separated by an end of line or by a comma
 */
template <class E>
ostream& PrintElementPtrs (
    ostream& ostr, vector<E*> v, const char* elementName, 
    bool useEndOfLine)
{
    ostr << v.size() << " " << elementName << ":"<< endl;
    for_each(v.begin (), v.end (), PrintElementPtr<E>(ostr, useEndOfLine));
    ostr << endl;
    return ostr;
}

/**
 * Pretty prints a list of objects stored as pointers in reverse order
 * @param ostr output stream where to print the objects to
 * @param v vector of objects to  be printed in reverse order than how
 *        they are stored in the vector.
 * @param elementName string describing an object in the list of objects printed
 * @param useEndOfLine objects are separated by an end of line or by a comma
 */
template <class E>
ostream& ReversePrintElementPtrs (ostream& ostr, vector<E*> v, 
				  const char* elementName, bool useEndOfLine)
{
    ostr << v.size() << " " << elementName << ":"<< endl;
    for_each(v.rbegin (), v.rend (), PrintElementPtr<E>(ostr, useEndOfLine));
    ostr << endl;
    return ostr;
}

#endif

// Local Variables:
// mode: c++
// End:
