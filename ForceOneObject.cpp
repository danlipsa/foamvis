/**
 * @file   ForceOneObject.cpp
 * @author Dan R. Lipsa
 * @date  8 March 2012
 *
 * Implementation for the ForceOneObject class 
 *
 */

#include "ForceOneObject.h"
#include "Utils.h"

ForceNamesOneObject::ForceNamesOneObject () :
    m_bodyId (INVALID_INDEX)
{
}

ForceOneObject::ForceOneObject () :
    m_bodyId (INVALID_INDEX),
    m_networkTorque (0),
    m_pressureTorque (0)
{
}

ForceOneObject::ForceOneObject (size_t bodyId, boost::shared_ptr<Body> body) :
    m_bodyId (bodyId),
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

ForceOneObject& ForceOneObject::operator-= (const ForceOneObject& other)
{
    m_networkForce -= other.m_networkForce;
    m_pressureForce -= other.m_pressureForce;
    m_networkTorque -= other.m_networkTorque;
    m_pressureTorque -= other.m_pressureTorque;
    return *this;
}
