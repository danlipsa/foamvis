/**
 * @file   ForceOneObject.cpp
 * @author Dan R. Lipsa
 * @date  8 March 2012
 *
 * Implementation for the ForceOneObject class 
 *
 */

#include "Body.h"
#include "Debug.h"
#include "ForceOneObject.h"
#include "Utils.h"


// ForceNamesOneObject
// ===========================================================================
ForceNamesOneObject::ForceNamesOneObject () :
    m_bodyId (INVALID_INDEX)
{
}


// ForceOneObject
// ===========================================================================
ForceOneObject::ForceOneObject () :
    m_networkTorque (0),
    m_pressureTorque (0)
{
}

ForceOneObject::ForceOneObject (boost::shared_ptr<Body> body) :
    m_body (body),
    m_networkTorque (0),
    m_pressureTorque (0)
{
}

ForceOneObject& ForceOneObject::operator+= (const ForceOneObject& other)
{
    m_networkForce += other.m_networkForce;
    m_pressureForce += other.m_pressureForce;
    m_networkTorque += other.m_networkTorque;
    m_pressureTorque += other.m_pressureTorque;
    return *this;
}

ForceOneObject ForceOneObject::operator/ (float value) const
{
    ForceOneObject foo = *this;
    foo.m_networkForce /= value;
    foo.m_pressureForce /= value;
    foo.m_networkTorque /= value;
    foo.m_pressureTorque /= value;
    return foo;
}

ForceOneObject& ForceOneObject::operator-= (const ForceOneObject& other)
{
    m_networkForce -= other.m_networkForce;
    m_pressureForce -= other.m_pressureForce;
    m_networkTorque -= other.m_networkTorque;
    m_pressureTorque -= other.m_pressureTorque;
    return *this;
}

G3D::Vector3 ForceOneObject::GetForce (ForceType::Enum type) const
{
    switch (type)
    {
    case ForceType::NETWORK:
        return m_networkForce;
    case ForceType::PRESSURE:
        return m_pressureForce;
    case ForceType::RESULT:
        return m_networkForce + m_pressureForce;
    default:
        RuntimeAssert (
            false, "ForceOneObject::GetForce: Invalid ForceType::Enum: ", type);
        return G3D::Vector3::zero ();
    }
}

float ForceOneObject::GetTorque (ForceType::Enum type) const
{
    switch (type)
    {
    case ForceType::NETWORK:
        return m_networkTorque;
    case ForceType::PRESSURE:
        return m_pressureTorque;
    case ForceType::RESULT:
        return m_networkTorque + m_pressureTorque;
    default:
        RuntimeAssert (
            false, "ForceOneObject::GetTorque: Invalid ForceType::Enum: ", type);
        return 0;
    }
}


void ForceOneObject::SetForce (ForceType::Enum type, 
                               float x, float y, float z) 
{
    G3D::Vector3 value (x, y, z);
    switch (type)
    {
    case ForceType::NETWORK:
        m_networkForce = value;
        break;
    case ForceType::PRESSURE:
        m_pressureForce = value;
        break;
    default:
        RuntimeAssert (
            false, "ForceOneObject::SetForce: Invalid ForceType::Enum: ", type);
    }
}

void ForceOneObject::SetTorque (ForceType::Enum type, float t) 
{
    switch (type)
    {
    case ForceType::NETWORK:
        m_networkTorque = t;
        break;
    case ForceType::PRESSURE:
        m_pressureTorque = t;
        break;
    default:
        RuntimeAssert (
            false, "ForceOneObject::SetTorque: Invalid ForceType::Enum: ", type);
    }
}

string ForceOneObject::ToString () const
{
    ostringstream ostr;
    ostr << "body id: " << m_body->GetId () << endl
         << "network force: " << m_networkForce 
         << ", length: " << m_networkForce.length () << endl
         << "pressure force: " << m_pressureForce 
         << ", length: " << m_pressureForce.length () << endl
         << "network torque: " << m_networkTorque << endl
         << "pressure torque: " << m_pressureTorque << endl;
    return ostr.str ();
}

ostream& operator<< (ostream& ostr, const ForceOneObject& foo)
{
    return ostr << foo.ToString ();
}
