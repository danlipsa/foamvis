/**
 * @file Force.h
 * @author Dan R. Lipsa
 * @date 7 June 2011
 *
 * Declaration of the Force class
 */
#ifndef __FORCE_H__
#define __FORCE_H__

class Body;
/**
 * Names of parameters in a DMP file where the network and the pressure forces
 * are stored.
 */
struct ForceNames
{
    size_t m_bodyId;
    boost::array<string, 2> m_networkForceName;
    boost::array<string, 2> m_pressureForceName;
};

struct Force
{
    size_t m_bodyId;
    boost::shared_ptr<Body> m_body;
    G3D::Vector2 m_networkForce;
    G3D::Vector2 m_pressureForce;
};

#endif //__FORCE_H__

// Local Variables:
// mode: c++
// End:
