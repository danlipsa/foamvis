/**
 * @file   DisplayBodyFunctors.h
 * @author Dan R. Lipsa
 * @date  26 Oct. 2010
 *
 * Implementation for functors to display a body
 */

#include "DisplayBodyFunctors.h"

DisplayBodyCenter::DisplayBodyCenter (
    const GLWidget& widget, const BodySelector& bodySelector):
    DisplayBodyBase (widget, TexCoordSetter(widget), 
		     BodyProperty::NONE, bodySelector)
{}

