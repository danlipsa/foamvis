/**
 * @file ForcesOneObject.h
 * @author Dan R. Lipsa
 * @date 7 June 2011
 *
 * Declaration of the ForcesOneObject class
 */
#ifndef __FORCES_ONE_OBJECT_H__
#define __FORCES_ONE_OBJECT_H__

class Body;
/**
 * Names of parameters in a DMP file where the network and the pressure forces
 * are stored.
 */
struct ForcesOneObjectNames
{
    ForcesOneObjectNames ();
    size_t m_bodyId;
    boost::array<string, 2> m_networkForceName;
    boost::array<string, 2> m_pressureForceName;
    string m_networkTorque;
    string m_pressureTorque;
};

struct ForcesOneObject
{
    ForcesOneObject ();
    size_t m_bodyId;
    boost::shared_ptr<Body> m_body;
    G3D::Vector2 m_networkForce;
    G3D::Vector2 m_pressureForce;
    float m_networkTorque;
    float m_pressureTorque;
};

#endif //__FORCES_ONE_OBJECT_H__

// Local Variables:
// mode: c++
// End:
