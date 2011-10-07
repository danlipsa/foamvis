/**
 * @file   OpenGLUtils.cpp
 * @author Dan R. Lipsa
 * @date 21 October 2010
 *
 * Implementation of various OpenGL utility functions
 */

#include "OpenGLUtils.h"
#include "DebugStream.h"
#include "Debug.h"
#include "OOBox.h"
#include "Utils.h"

// Private Classes
// ======================================================================

/**
 * Stores information about various OpenGL characteristics of the graphic card
 */
class OpenGLFeature
{
public:
    enum Type
    {
	BOOLEAN,
	INTEGER,
	INTEGER2,
	FLOAT,
	STRING,
	SEPARATOR
    };

    OpenGLFeature (GLenum what, Type type, const char* name) :
	m_what (what), m_type (type), m_name (name)
    {
    }

    OpenGLFeature (const char* name) :
	m_what (0), m_type (SEPARATOR), m_name (name)
    {
    }


    /**
     * Reads an OpenGLFeature from OpenGL
     */
    string get () const
    {
	ostringstream ostr;
	switch (m_type)
	{
	case INTEGER:
	{
	    GLint where;
	    glGetIntegerv (m_what, &where);
	    ostr << where;
	    return ostr.str ();
	}
	case FLOAT:
	{
	    GLfloat where;
	    glGetFloatv (m_what, &where);
	    ostr << where;
	    return ostr.str ();
	}
	case INTEGER2:
	{
	    GLint where[2];
	    glGetIntegerv (m_what, where);
	    ostr << where[0] << " " << where[1];
	    return ostr.str ();
	}

	
	case BOOLEAN:
	{
	    GLboolean where;
	    glGetBooleanv (m_what, &where);
	    ostr << (where ? "true" : "false");
	    return ostr.str ();
	}
	
	case STRING:
	    ostr << glGetString (m_what);
	    return ostr.str ();
	
	case SEPARATOR:
	    return "";
	
	default:
	{
	    ThrowException ("Invalid storage type for OpenGLFeature");
	    return 0;
	}
	}
    }

    void print (ostream* ostr) const
    {
	(*ostr) << m_name;
	if (m_what != 0)
	    (*ostr) << ": " << get ();
	(*ostr) << endl;
    }

private:
    /**
     * What OpenGL characteristic
     */
    GLenum m_what;
    /**
     * Data type of the required information
     */
    Type m_type;
    /**
     * Name of the OpenGL characteristic
     */
    string m_name;
};


G3D::Vector3 gluProject (G3D::Vector3 object)
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

G3D::Vector2 toEye (G3D::Vector2 object)
{
    G3D::Matrix4 m;
    G3D::Vector4 o (object, 0, 1);
    G3D::glGetMatrix (GL_MODELVIEW_MATRIX, m);
    return (m * o).xy ();
}

G3D::Vector3 gluUnProject (
    G3D::Vector2 screenCoord, 
    GluUnProjectZOperation::Enum zOperation)
{
    GLdouble model[16];
    glGetDoublev (GL_MODELVIEW_MATRIX, model);
    GLdouble proj[16];
    glGetDoublev (GL_PROJECTION_MATRIX, proj);
    GLint view[4];
    glGetIntegerv (GL_VIEWPORT, view);
    GLfloat zScreenCoord;
    if (zOperation == GluUnProjectZOperation::READ)
	glReadPixels (screenCoord.x, screenCoord.y, 1, 1, 
		      GL_DEPTH_COMPONENT, GL_FLOAT, &zScreenCoord);
    else
	zScreenCoord = 0;
    double x = 0, y = 0, z = 0;
    GLint ret = gluUnProject (screenCoord.x, screenCoord.y, zScreenCoord, 
			      model, proj, view, 
			      &x, &y, &z);
    RuntimeAssert (ret == GLU_TRUE, "gluUnproject");
    return G3D::Vector3 (x, y, z);
}


void WarnOnOpenGLError (string message)
{
    GLenum errCode;
    if ((errCode = glGetError()) != GL_NO_ERROR)
        cdbg << "OpenGL Error " << message.c_str () << ":"
	     << gluErrorString(errCode) << endl;
}

void ThrowOnOpenGLError (string message)
{
    GLenum errCode;
    ostringstream ostr;
    if ((errCode = glGetError()) != GL_NO_ERROR)
    {
        ostr << "OpenGL Error " << message.c_str () << ":"
	     << gluErrorString(errCode);
	ThrowException (ostr.str ());
    }
}


void printOpenGLInfo (ostream& ostr)
{
    boost::array<OpenGLFeature, 33> info = {{
	OpenGLFeature (GL_VENDOR, OpenGLFeature::STRING, "GL_VENDOR"),
	OpenGLFeature (GL_RENDERER, OpenGLFeature::STRING, "GL_RENDERER"),
	OpenGLFeature (GL_VERSION, OpenGLFeature::STRING, "GL_VERSION"),
	OpenGLFeature (GL_SHADING_LANGUAGE_VERSION, 
		       OpenGLFeature::STRING, "GL_SHADING_LANGUAGE_VERSION"),

	OpenGLFeature ("--- Line width ---"),
	OpenGLFeature (GL_ALIASED_LINE_WIDTH_RANGE, OpenGLFeature::INTEGER2,
		       "GL_ALIASED_LINE_WIDTH_RANGE"),
	OpenGLFeature (GL_LINE_WIDTH_RANGE, OpenGLFeature::INTEGER2,
		       "GL_LINE_WIDTH_RANGE"),
	OpenGLFeature (GL_LINE_WIDTH_GRANULARITY, OpenGLFeature::FLOAT,
		       "GL_LINE_WIDTH_GRANULARITY"),


	OpenGLFeature ("--- Texture / Viewport ---"),
	OpenGLFeature (GL_MAX_VIEWPORT_DIMS, OpenGLFeature::INTEGER2,
		       "GL_MAX_VIEWPORT_DIMS"),
	OpenGLFeature (GL_MAX_TEXTURE_SIZE, OpenGLFeature::INTEGER,
		       "GL_MAX_TEXTURE_SIZE"),
	OpenGLFeature (GL_MAX_TEXTURE_UNITS, OpenGLFeature::INTEGER,
		       "GL_MAX_TEXTURE_UNITS"),


	OpenGLFeature ("--- Framebuffer Objects ---"),
	OpenGLFeature (GL_MAX_COLOR_ATTACHMENTS_EXT, OpenGLFeature::INTEGER,
		       "GL_MAX_COLOR_ATTACHMENTS_EXT"),

	OpenGLFeature ("--- Vertex Shader ---"),
	OpenGLFeature (GL_MAX_VERTEX_ATTRIBS, OpenGLFeature::INTEGER,
		       "GL_MAX_VERTEX_ATTRIBS"),

	OpenGLFeature ("--- Geometry Shader ---"),
	OpenGLFeature (GL_MAX_GEOMETRY_OUTPUT_VERTICES, OpenGLFeature::INTEGER,
		       "GL_MAX_GEOMETRY_OUTPUT_VERTIES"),

	OpenGLFeature ("--- Multisampling ---"),	
	OpenGLFeature (GL_SAMPLE_BUFFERS, OpenGLFeature::INTEGER, 
		       "GL_SAMPLE_BUFFERS"),
	OpenGLFeature (GL_SAMPLES, OpenGLFeature::INTEGER, "GL_SAMPLES"),
	
	OpenGLFeature ("--- Other ---"),	
	OpenGLFeature (GL_AUX_BUFFERS, OpenGLFeature::INTEGER, "AUX_BUFFERS"),
        OpenGLFeature (GL_RED_BITS, OpenGLFeature::INTEGER, "RED_BITS"),
        OpenGLFeature (GL_GREEN_BITS, OpenGLFeature::INTEGER, "GREEN_BITS"),
        OpenGLFeature (GL_BLUE_BITS, OpenGLFeature::INTEGER, "BLUE_BITS"),
        OpenGLFeature (GL_ALPHA_BITS, OpenGLFeature::INTEGER, "ALPHA_BITS"),

        OpenGLFeature (GL_INDEX_BITS, OpenGLFeature::INTEGER, "INDEX_BITS"),
        OpenGLFeature (GL_DEPTH_BITS, OpenGLFeature::INTEGER, "DEPTH_BITS"),
        OpenGLFeature (GL_STENCIL_BITS, OpenGLFeature::INTEGER, "STENCIL_BITS"),
	
	OpenGLFeature (GL_STEREO, OpenGLFeature::BOOLEAN, "GL_STEREO"),
	OpenGLFeature (GL_DOUBLEBUFFER, OpenGLFeature::BOOLEAN,
		       "GL_DOUBLEBUFFER"),
	OpenGLFeature (GL_EXTENSIONS, OpenGLFeature::STRING, "GL_EXTENSIONS"),
	}};
    for_each (info.begin (), info.end (),
	      boost::bind (&OpenGLFeature::print, _1, &ostr));
}


void DisplayOpositeFaces (G3D::Vector3 origin,
			  G3D::Vector3 faceFirst,
			  G3D::Vector3 faceSecond,
			  G3D::Vector3 translation)
{
    G3D::Vector3 faceOrigin;
    G3D::Vector3 faceSum = faceFirst + faceSecond;
    G3D::Vector3 translations[] = {origin, translation};
    for (int i = 0; i < 2; i++)
    {
	faceOrigin += translations[i];
	faceFirst += translations[i];
	faceSecond += translations[i];
	faceSum += translations[i];
	
	glBegin (GL_LINE_STRIP);
	::glVertex (faceOrigin);
	::glVertex (faceFirst);
	::glVertex (faceSum);
	::glVertex (faceSecond);
	::glVertex (faceOrigin);
	glEnd ();
    }
}

void DisplayBox (const OOBox& oobox)
{
    glPushAttrib (GL_POLYGON_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glLineWidth (1.0);
    glColor (Qt::black);
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

    DisplayOpositeFaces (G3D::Vector3::zero (), 
			 oobox[0], oobox[1], oobox[2]);
    DisplayOpositeFaces (G3D::Vector3::zero (),
			 oobox[1], oobox[2], oobox[0]);
    DisplayOpositeFaces (G3D::Vector3::zero (), 
			 oobox[2], oobox[0], oobox[1]);
    glPopAttrib ();
}

void DisplayBox (const G3D::AABox& aabb, const QColor& color, GLfloat lineWidth)
{
    using G3D::Vector3;
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);
    glColor (color);
    glLineWidth (lineWidth);
    Vector3 diagonal = aabb.high () - aabb.low ();
    Vector3 first = diagonal.x * Vector3::unitX ();
    Vector3 second = diagonal.y * Vector3::unitY ();
    Vector3 third = diagonal.z * Vector3::unitZ ();    
    DisplayOpositeFaces (aabb.low (), first, second, third);
    DisplayOpositeFaces (aabb.low (), second, third, first);
    DisplayOpositeFaces (aabb.low (), third, first, second);
    glPopAttrib ();
}

void DisplayBox (const G3D::Rect2D& rect)
{
    glBegin (GL_POLYGON);
    ::glVertex (rect.x0y0 ());
    ::glVertex (rect.x1y0 ());
    ::glVertex (rect.x1y1 ());
    ::glVertex (rect.x0y1 ());
    glEnd ();
}

void ClearColorBuffer (Qt::GlobalColor clearColor)
{
    glPushAttrib (GL_COLOR_BUFFER_BIT); 
    glClearColor (clearColor);
    glClear(GL_COLOR_BUFFER_BIT);
    glPopAttrib ();
}

void ClearStencilBuffer (GLint clearStencil)
{
    glPushAttrib (GL_STENCIL_BUFFER_BIT); 
    glClearStencil (clearStencil);
    glClear(GL_STENCIL_BUFFER_BIT);
    glPopAttrib ();
}


void ClearColorStencilBuffers (QColor clearColor, GLint clearStencil)
{
    glPushAttrib (GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 
    glClearColor (clearColor);
    glClearStencil (clearStencil);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glPopAttrib ();
}

void drawEllipsis2D (float l1, float l2, float c)
{
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; ++i)
    {
	float t = i * M_PI / 180;
	glVertex2f( l1 * cos (t) * c, l2 * sin (t) * c);
    }
    glEnd();
}

// Based on OpenGL FAQ, 9.090 How do I draw a full-screen quad?
void ActivateShader (G3D::Rect2D destRect)
{
    glPushAttrib (GL_VIEWPORT_BIT);
    glViewport (destRect.x0 (), destRect.y0 (),
		destRect.width (), destRect.height ());

    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glLoadIdentity ();
    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity ();
    glBegin (GL_QUADS);
    glTexCoord2i (0, 0);glVertex3i (-1, -1, -1);
    glTexCoord2i (1, 0);glVertex3i (1, -1, -1);
    glTexCoord2i (1, 1);glVertex3i (1, 1, -1);
    glTexCoord2i (0, 1);glVertex3i (-1, 1, -1);
    glEnd ();
    glPopMatrix ();
    glMatrixMode (GL_MODELVIEW);
    glPopMatrix ();
    glPopAttrib ();
}

