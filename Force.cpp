/**
 * @file   Force.cpp
 * @author Dan R. Lipsa
 * @date  8 March 2012
 *
 * Implementation for the Force class 
 *
 */

#include "Force.h"
#include "Utils.h"

ForceNames::ForceNames () :
    m_bodyId (INVALID_INDEX)
{
}

Force::Force () :
    m_bodyId (INVALID_INDEX),
    m_networkTorque (0),
    m_pressureTorque (0)
{
}
