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
