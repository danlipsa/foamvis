/**
 * @file ForceOneObject.h
 * @author Dan R. Lipsa
 * @date 7 June 2011
 * @brief Forces and torque acting on one object
 * @ingroup data model
 */
#ifndef __FORCE_ONE_OBJECT_H__
#define __FORCE_ONE_OBJECT_H__

#include "Enums.h"

class Body;
/**
 * @brief Names of parameters in a DMP file where the network and the
 *        pressure forces are stored.
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

/**
 * @brief Forces and torque acting on one object
 */
class ForceOneObject
{
public:
    ForceOneObject ();
    ForceOneObject (boost::shared_ptr<Body> body);
    ForceOneObject& operator+= (const ForceOneObject& other);
    ForceOneObject& operator-= (const ForceOneObject& other);
    ForceOneObject operator/ (float value) const;

    G3D::Vector3 GetForce (ForceType::Enum type) const;
    void SetForce (ForceType::Enum type, float x, float y, float z);
    
    float GetTorque (ForceType::Enum type) const;
    void SetTorque (ForceType::Enum type, float t);

    void SetBody (boost::shared_ptr<Body> body)
    {
        m_body = body;
    }
    boost::shared_ptr<Body> GetBody () const
    {
        return m_body;
    }
    string ToString () const;
    friend ostream& operator<< (ostream& ostr, const ForceOneObject& foo);

private:
    boost::shared_ptr<Body> m_body;
    G3D::Vector3 m_networkForce;
    G3D::Vector3 m_pressureForce;
    float m_networkTorque;
    float m_pressureTorque;
};

#endif //__FORCE_ONE_OBJECT_H__

// Local Variables:
// mode: c++
// End:
