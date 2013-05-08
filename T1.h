/**
 * @file T1.h
 * @author Dan R. Lipsa
 * @date 7 May 2013
 * A topological change
 */

#ifndef __T1_H__
#define __T1_H__

#include "Enums.h"


struct T1
{
public:
    T1 () : 
        m_type (T1Type::COUNT)
    {}
    T1 (G3D::Vector3 position, T1Type::Enum type) :
        m_position (position), m_type (type)
    {}

    void SetPosition (const G3D::Vector3& v)
    {
        m_position = v;
    }
    const G3D::Vector3& GetPosition () const
    {
        return m_position;
    }
    void SetType (T1Type::Enum tct)
    {
        m_type = tct;
    }
    T1Type::Enum GetType () const
    {
        return m_type;
    }
    string ToString () const;

    friend ostream& operator<< (ostream& ostr, const T1& d);

private:
    G3D::Vector3 m_position;
    T1Type::Enum m_type;
};




#endif //__T1_H__

// Local Variables:
// mode: c++
// End:
