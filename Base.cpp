/**
 * @file   Base.cpp
 * @author Dan R. Lipsa
 * @date 11 Dec. 2012
 * 
 * Definitions for a class that stores settings and simulations
 */


#include "Base.h"
#include "Settings.h"
#include "ViewSettings.h"


ViewNumber::Enum Base::GetViewNumber () const
{
    return GetSettings ()->GetViewNumber ();
}

ViewSettings& Base::GetViewSettings (ViewNumber::Enum viewNumber) const
{
    return GetSettings ()->GetViewSettings (viewNumber);
}

size_t Base::GetTime (ViewNumber::Enum viewNumber) const
{
    return GetViewSettings (viewNumber).GetTime ();
}

size_t Base::GetViewCount () const
{
    return GetSettings ()->GetViewCount ();
}
