/**
 * @file   DisplayElement.h
 * @author Dan R. Lipsa
 * @date  26 Oct 2010
 *
 * Implementation for functors that display an element 
 * (body, face, edge or vertex)
 */

#include "DisplayElement.h"
#include "GLWidget.h"

void TexCoordSetter::operator () (double value)
{
    double texCoord = m_glWidget.TexCoord (value);
    glTexCoord1f (texCoord); 
}
