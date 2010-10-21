/**
 * @file   OpenGLUtils.cpp
 * @author Dan R. Lipsa
 * @date 21 October 2010
 *
 * Implementation of various OpenGL utility functions
 */

#include "OpenGLUtils.h"
#include "DebugStream.h"

G3D::Vector3 gluProject (const G3D::Vector3& object)
{
    GLdouble model[16];
    glGetDoublev (GL_MODELVIEW_MATRIX, model);
    GLdouble proj[16];
    glGetDoublev (GL_PROJECTION_MATRIX, proj);
    GLint view[4];
    glGetIntegerv (GL_VIEWPORT, view);
    GLdouble x, y, z;
    gluProject (object.x, object.y, object.z, model, proj, view, 
		&x, &y, &z);
    return G3D::Vector3 (x, y, z);
}
