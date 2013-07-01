/**
 * @file DataProperties.h
 * @author Dan R. Lipsa
 * @ingroup data model
 * @brief Basic properties of the simulation data such as dimensions
 *        and if edges are quadratic or not.
 */
#ifndef __DATA_PROPERTIES_H__
#define __DATA_PROPERTIES_H__

#include "Enums.h"

/**
 * @brief Basic properties of the simulation data such as dimensions
 *        and if it is quadratic or not.
 */
class DataProperties
{
public:
    DataProperties () :
	m_dimension (Dimension::D3D),
	m_quadratic (false)
    {
    }

    Dimension::Enum GetDimension () const
    {
        return m_dimension;
    }
    bool Is2D () const
    {
	return m_dimension == Dimension::D2D;
    }
    bool Is3D () const
    {
	return ! Is2D ();
    }
    bool IsQuadratic () const
    {
	return m_quadratic;
    }
    void SetQuadratic (bool quadratic)
    {
	m_quadratic = quadratic;
    }
    void SetDimension (size_t dimension);

    bool operator== (const DataProperties& other)
    {
	return m_dimension == other.m_dimension &&
	    m_quadratic == other.m_quadratic;
    }

    bool operator!= (const DataProperties& other)
    {
	return ! operator== (other);
    }

private:
    Dimension::Enum m_dimension;
    bool m_quadratic;
};



#endif //__DATA_PROPERTIES_H__

// Local Variables:
// mode: c++
// End:
