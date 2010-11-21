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

G3D::Vector3 gluUnProject (const G3D::Vector3& window)
{
    GLdouble model[16];
    glGetDoublev (GL_MODELVIEW_MATRIX, model);
    GLdouble proj[16];
    glGetDoublev (GL_PROJECTION_MATRIX, proj);
    GLint view[4];
    glGetIntegerv (GL_VIEWPORT, view);
    GLdouble x, y, z;
    gluUnProject (window.x, window.y, window.z, model, proj, view, 
		  &x, &y, &z);
    return G3D::Vector3 (x, y, z);
}


/**
 * Check the OpenGL  error code and prints a message  to cdbg if there
 * is an error
 */
void detectOpenGLError (string message)
{
    GLenum errCode;
    if ((errCode = glGetError()) != GL_NO_ERROR)
        cdbg << "OpenGL Error " << message.c_str () << ":"
		    << gluErrorString(errCode);
}

void printOpenGLInfo (ostream& ostr)
{
    boost::array<OpenGLFeature, 26> info = {{
	OpenGLFeature (GL_VENDOR, OpenGLFeature::STRING, "GL_VENDOR"),
	OpenGLFeature (GL_RENDERER, OpenGLFeature::STRING, "GL_RENDERER"),
	OpenGLFeature (GL_VERSION, OpenGLFeature::STRING, "GL_VERSION"),
	OpenGLFeature (GL_SHADING_LANGUAGE_VERSION, 
		       OpenGLFeature::STRING, "GL_SHADING_LANGUAGE_VERSION"),

	OpenGLFeature ("--- Texture ---"),
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
