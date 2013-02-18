/**
 * @file DataProperties.h
 * @author Dan R. Lipsa
 *
 * Declaration of the DataProperties class
 */
#ifndef __DATA_PROPERTIES_H__
#define __DATA_PROPERTIES_H__

class DataProperties
{
public:
    DataProperties () :
	m_spaceDimension (3),
	m_quadratic (false)
    {
    }

    size_t GetSpaceDimension () const
    {
	return m_spaceDimension;
    }
    bool Is2D () const
    {
	return m_spaceDimension == 2;
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
    void SetSpaceDimension (size_t spaceDimension) 
    {
	m_spaceDimension = spaceDimension;
    }


    bool operator== (const DataProperties& other)
    {
	return m_spaceDimension == other.m_spaceDimension &&
	    m_quadratic == other.m_quadratic;
    }

    bool operator!= (const DataProperties& other)
    {
	return ! operator== (other);
    }

private:
    size_t m_spaceDimension;
    bool m_quadratic;
};



#endif //__DATA_PROPERTIES_H__

// Local Variables:
// mode: c++
// End:
