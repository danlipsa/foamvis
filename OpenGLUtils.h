/**
 * @file   OpenGLUtils.h
 * @author Dan R. Lipsa
 * @date 21 October 2010
 * @brief General OpenGL functionality
 * @ingroup utils
 */

#ifndef __OPENGL_UTILS_H__
#define __OPENGL_UTILS_H__

#include "Enums.h"

class OOBox;

inline void glColor (QColor color)
{
    glColor4f (color.redF (), color.greenF (), color.blueF (), color.alphaF ());
}

inline void glScale (double ratio)
{
    glScaled (ratio, ratio, ratio);
}

// eliminate Valgrind errors with G3D version
inline void glVertex (const G3D::Vector2& v)
{
    glVertex2f (v.x, v.y);
}

// eliminate Valgrind errors with G3D version
inline void glVertex (const G3D::Vector3& v)
{
    glVertex3f (v.x, v.y, v.z);
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
    glTranslatef (translate.x, translate.y, 0.);
}

inline void glViewport (G3D::Rect2D viewport)
{
    glViewport (ceil (viewport.x0 ()), ceil (viewport.y0 ()), 
		viewport.width (), viewport.height ());
}

float GetOnePixelInObjectSpace (bool is2D);
G3D::Vector3 toObject (const G3D::Vector2& positionGl, bool is2D);
G3D::Vector3 toObject (const QPoint& positionQt, int windowHeight, bool is2D);

/**
 * Maps object coordinates to window coordinates
 */
G3D::Vector3 gluProject (G3D::Vector3 objectCoord);
template<typename InputIterator, typename OutputIterator>
void gluProject (InputIterator objectCoordBegin, InputIterator objectCoordEnd,
                 OutputIterator windowCoordBegin);
G3D::Rect2D gluProject (const G3D::Rect2D& objectCoord);

/**
 * @brief Operation for the Z component (set to zero or read from the
 * graphics card)
 */
struct GluUnProjectZOperation
{
    enum Enum
    {
	SET0,
	READ
    };
};

/**
 * Returns the object coordinate associated with a window coordinate.
 * Uses the depth buffer to find out the Z screen coordinate (and then
 * the z coordinate) if zOperation == READ otherwise it uses a Z screen 
 * coordinate equal with 0.
 * see OpenGL FAQ, 9.110
 */
G3D::Vector3 gluUnProject (
    G3D::Vector2 windowCoord, GluUnProjectZOperation::Enum zOperation);
template<typename InputIterator, typename OutputIterator>
void gluUnProject (
    InputIterator windowCoordBegin, InputIterator windowCoordEnd,
    OutputIterator objectCoordBegin, GluUnProjectZOperation::Enum zOperation);
G3D::Rect2D gluUnProject (const G3D::Rect2D& windowCoord,
                          GluUnProjectZOperation::Enum zOperation);

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

GLenum GetColorComponent (size_t index);

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

void ClearColorBuffer (Qt::GlobalColor clearColor);
void ClearStencilBuffer (GLint clearStencil);
void ClearColorStencilBuffers (
    QColor clearColor, GLint clearStencil);

void drawEllipsis2D (float l1, float l2, float size, float lineWidth);

void ActivateShader (G3D::Rect2D destRect);
bool isMatrixValid (GLenum matrixType);

    
vtkSmartPointer<vtkMatrix4x4> OpenGlToVtk (const boost::array<GLdouble,16>& mv);
G3D::Matrix4 OpenGlToG3D (const boost::array<GLdouble,16>& mv);

QString ReadShader (const QString& resourceUrl);
boost::shared_ptr<QGLShader> CreateShader (const QString& resourceUrl,
                                           QGLShader::ShaderType type);
/**
 * Maps object coordinates to eye coordinates.
 */
G3D::Vector3 ObjectToEye (G3D::Vector3 object);
G3D::Vector2 ObjectToEye (G3D::Vector2 object);

#endif //__OPENGL_UTIL_H__

// Local Variables:
// mode: c++
// End:
