/**
 * @file   ForcesOneObject.cpp
 * @author Dan R. Lipsa
 * @date  8 March 2012
 *
 * Implementation for the ForcesOneObject class 
 *
 */

#include "ForcesOneObject.h"
#include "Utils.h"

ForcesOneObjectNames::ForcesOneObjectNames () :
    m_bodyId (INVALID_INDEX)
{
}

ForcesOneObject::ForcesOneObject () :
    m_bodyId (INVALID_INDEX),
    m_networkTorque (0),
    m_pressureTorque (0)
{
}

ForcesOneObject::ForcesOneObject (size_t bodyId, boost::shared_ptr<Body> body) :
    m_bodyId (bodyId),
    m_body (body),
    m_networkTorque (0),
    m_pressureTorque (0)
{
}

ForcesOneObject& ForcesOneObject::operator+= (const ForcesOneObject& other)
{
    m_networkForce += other.m_networkForce;
    m_pressureForce += other.m_pressureForce;
    m_networkTorque += other.m_networkTorque;
    m_pressureTorque += other.m_pressureTorque;
    return *this;
}

ForcesOneObject& ForcesOneObject::operator-= (const ForcesOneObject& other)
{
    m_networkForce -= other.m_networkForce;
    m_pressureForce -= other.m_pressureForce;
    m_networkTorque -= other.m_networkTorque;
    m_pressureTorque -= other.m_pressureTorque;
    return *this;
}
