#include "Attribute.h"



/**
 * Unary function  that prints an  object of type  E*. Used to  print a
 * sequence of objects.
 */
template <class E>
struct printElement : public unary_function<E, void>
{
public:
    /**
     * Constructs the object
     * @param ostr where to print
     */
    printElement (ostream& ostr) : m_ostr(ostr), m_index(0) {}
    /**
     * Pretty prints an object
     * @param e the object to be printed.
     */
    void operator() (E e) 
    {
        if (m_index > 0)
        {
            m_ostr << ", ";
        }
        m_ostr << e;
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
};


ostream& IntegerArrayAttribute::Print (ostream& ostr) const
{
    for_each (m_values->begin (), m_values->end (), printElement<int> (ostr));
    return ostr;
}

ostream& RealArrayAttribute::Print (ostream& ostr) const
{
    for_each (m_values->begin (), m_values->end (), printElement<float> (ostr));
    return ostr;
}

ostream& operator<< (ostream& ostr, const Attribute& attribute)
{
    if (&attribute == 0)
        return ostr << "null";
    else
        return attribute.Print(ostr);
}
