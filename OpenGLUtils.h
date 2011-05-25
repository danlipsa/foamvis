/**
 * @file   OpenGLUtils.h
 * @author Dan R. Lipsa
 * @date 21 October 2010
 *
 * Declaration of the OpenGLUtils class
 */

#ifndef __OPENGL_UTILS_H__
#define __OPENGL_UTILS_H__

class OOBox;

inline void glColor (QColor color)
{
    glColor4f (color.redF (), color.greenF (), color.blueF (), color.alphaF ());
}

inline void glVertex (G3D::Vector3 p)
{
    glVertex3f (p.x, p.y, p.z);
}

inline void glVertex (G3D::Vector2 p)
{
    glVertex2f (p.x, p.y);
}

inline void glScale (double ratio)
{
    glScaled (ratio, ratio, ratio);
}

inline void glClearColor (QColor color)
{
    glClearColor (color.redF (), color.greenF (), 
		  color.blueF (), color.alphaF ());
}

inline void glTranslate (G3D::Vector3 translate)
{
    glTranslatef (translate.x, translate.y, translate.z);
}

inline void glTranslate (G3D::Vector2 translate)
{
    glTranslatef (translate.x, translate.y, 0);
}

inline void glViewport (G3D::Rect2D viewport)
{
    glViewport (ceil (viewport.x0 ()), ceil (viewport.y0 ()), 
		viewport.width (), viewport.height ());
}

/**
 * Maps object coordinates to window coordinates
 */
G3D::Vector3 gluProject (G3D::Vector3 object);

struct GluUnProjectZOperation
{
    enum Enum
    {
	SET0,
	READ
    };
};

/**
 * Returns the world coordinate associated with a screen coordinate.
 * Uses the depth buffer to find out the Z screen coordinate (and then
 * the z coordinate) if zOperation == READ otherwise it uses a Z screen 
 * coordinate equal with 0.
 * see OpenGL FAQ, 9.110
 */
G3D::Vector3 gluUnProject (
    G3D::Vector2 screenCoord, 
    GluUnProjectZOperation::Enum zOperation = GluUnProjectZOperation::READ);

/**
 * Check the OpenGL  error code and prints a message  to cdbg if there
 * is an error
 */
void WarnOnOpenGLError (string message = "");
void ThrowOnOpenGLError (string message = "");



void printOpenGLInfo (ostream& ostr);

inline GLenum TextureEnum (GLint texture)
{
    return GL_TEXTURE0 + texture;
}

void DisplayBox (const OOBox& oobox);
void DisplayBox (const G3D::AABox& aabb, const QColor& color,
		 GLfloat lineWidth = 1.0);
void DisplayBox (const G3D::Rect2D& rect);


template<typename GraphicsObject>
void DisplayBox (
    boost::shared_ptr<GraphicsObject> o, 
    const QColor& color, GLfloat lineWidth = 1.0)
{
    DisplayBox (o->GetBoundingBox (), color, lineWidth);
}


template<typename GraphicsObject>
void DisplayBox (
    const GraphicsObject& o, const QColor& color, GLfloat lineWidth = 1.0)
{
    DisplayBox (o.GetBoundingBox (), color, lineWidth);
}

void RenderFromFbo (G3D::Rect2D viewRect, QGLFramebufferObject& fbo);
void RenderFromFboAndRotate (
    G3D::Rect2D viewRect, QGLFramebufferObject& fbo,
    G3D::Vector2 rotationCenter, float angleDegrees);

void ClearColorBuffer (Qt::GlobalColor clearColor);
void ClearColorStencilBuffers (
    Qt::GlobalColor clearColor, GLint clearStencil);

#endif //__OPENGL_UTIL_H__

// Local Variables:
// mode: c++
// End:
