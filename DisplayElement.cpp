/**
 * @file DisplayElement.cpp
 * @author Dan R. Lipsa
 * @date 1 Feb. 2013
 * 
 * Definitions for methods of the DisplayElement class.
 */

#include "DisplayElement.h"

template<typename PropertySetter>
ViewNumber::Enum DisplayElementProperty<PropertySetter>::GetViewNumber () const
{
    return m_propertySetter.GetViewNumber ();
}

template<typename PropertySetter>
bool DisplayElementProperty<PropertySetter>::Is2D () const
{
    return m_propertySetter.Is2D ();
}



// Template instantiations
// ======================================================================
template class DisplayElementProperty<SetterTextureCoordinate>;
template class DisplayElementProperty<SetterVelocity>;
template class DisplayElementProperty<SetterVertexAttribute>;
template class DisplayElementProperty<SetterNop>;
template class DisplayElementProperty<SetterDeformation>;
