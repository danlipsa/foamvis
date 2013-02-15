/**
 * @file ForceOneObject.h
 * @author Dan R. Lipsa
 * @date 7 June 2011
 *
 * Declaration of the ForceOneObject class
 */
#ifndef __FORCES_ONE_OBJECT_H__
#define __FORCES_ONE_OBJECT_H__

class Body;
/**
 * Names of parameters in a DMP file where the network and the pressure forces
 * are stored.
 */
struct ForceNamesOneObject
{
    ForceNamesOneObject ();
    size_t m_bodyId;
    boost::array<string, 3> m_networkForceName;
    boost::array<string, 3> m_pressureForceName;
    string m_networkTorqueName;
    string m_pressureTorqueName;
};

struct ForceOneObject
{
    ForceOneObject ();
    ForceOneObject (size_t bodyId, boost::shared_ptr<Body> body);
    ForceOneObject& operator+= (const ForceOneObject& other);
    ForceOneObject& operator-= (const ForceOneObject& other);

    size_t m_bodyId;
    boost::shared_ptr<Body> m_body;
    G3D::Vector3 m_networkForce;
    G3D::Vector3 m_pressureForce;
    float m_networkTorque;
    float m_pressureTorque;
};

#endif //__FORCES_ONE_OBJECT_H__

// Local Variables:
// mode: c++
// End:
