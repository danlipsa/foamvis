/**
 * @file FoamParameters.h
 * @author Dan R. Lipsa
 *
 * Declaration of the FoamParameters class
 */
#ifndef __FOAM_PARAMETERS_H__
#define __FOAM_PARAMETERS_H__

class FoamParameters
{
public:
    FoamParameters () :
	m_spaceDimension (3),
	m_quadratic (false)
    {
    }

    void SetSpaceDimension (size_t spaceDimension) 
    {
	m_spaceDimension = spaceDimension;
    }
    size_t GetSpaceDimension () const
    {
	return m_spaceDimension;
    }
    bool Is2D () const
    {
	return m_spaceDimension == 2;
    }
    bool IsQuadratic () const
    {
	return m_quadratic;
    }
    void SetQuadratic (bool quadratic)
    {
	m_quadratic = quadratic;
    }

    bool operator== (const FoamParameters& other)
    {
	return m_spaceDimension == other.m_spaceDimension &&
	    m_quadratic == other.m_quadratic;
    }

    bool operator!= (const FoamParameters& other)
    {
	return ! operator== (other);
    }

private:
    size_t m_spaceDimension;
    bool m_quadratic;
};



#endif //__FOAM_PARAMETERS_H__

// Local Variables:
// mode: c++
// End:
