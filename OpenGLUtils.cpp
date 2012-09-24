/**
 * @file   OpenGLUtils.cpp
 * @author Dan R. Lipsa
 * @date 21 October 2010
 *
 * Implementation of various OpenGL utility functions
 */

#include "DataProperties.h"
#include "DebugStream.h"
#include "Debug.h"
#include "OOBox.h"
#include "OpenGLUtils.h"
#include "Utils.h"

//#define __LOG__(code) code
#define __LOG__(code)


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

void getCurrentMatrices (GLdouble model[], GLdouble proj[], GLint view[])
{
    glGetDoublev (GL_MODELVIEW_MATRIX, model);
    glGetDoublev (GL_PROJECTION_MATRIX, proj);
    glGetIntegerv (GL_VIEWPORT, view);
}


G3D::Vector3 gluProject (G3D::Vector3 objectCoord)
{
    GLdouble model[16];GLdouble proj[16];GLint view[4];
    getCurrentMatrices (model, proj, view);
    GLdouble x, y, z;
    gluProject (objectCoord.x, objectCoord.y, objectCoord.z, model, proj, view, 
		&x, &y, &z);
    return G3D::Vector3 (x, y, z);
}

template<typename InputIterator, typename OutputIterator>
void gluProject (InputIterator objectCoordBegin, InputIterator objectCoordEnd,
                 OutputIterator windowCoordBegin)
{
    GLdouble model[16];GLdouble proj[16];GLint view[4];
    getCurrentMatrices (model, proj, view);
    OutputIterator outIt = windowCoordBegin;
    for (InputIterator it = objectCoordBegin; it != objectCoordEnd; ++it)
    {
        GLdouble x, y, z;
        gluProject (it->x, it->y, it->z,
                    model, proj, view, &x, &y, &z);
        (*outIt) = G3D::Vector3 (x, y, z);
        ++outIt;
    }
}

G3D::Rect2D gluProject (const G3D::Rect2D& oc)
{
    boost::array<G3D::Vector3, 4> objectCoord = {{
            G3D::Vector3 (oc.x0y0 (), 0),
            G3D::Vector3 (oc.x0y1 (), 0),
            G3D::Vector3 (oc.x1y1 (), 0),
            G3D::Vector3 (oc.x1y0 (), 0)
        }};
    boost::array<G3D::Vector3, 4> windowCoord;
    gluProject (objectCoord.begin (), objectCoord.end (), windowCoord.begin ());
    return G3D::Rect2D::xyxy (windowCoord[0].x, windowCoord[0].y,
                              windowCoord[2].x, windowCoord[2].y);
}


G3D::Vector3 toEye (G3D::Vector3 object)
{
    G3D::Matrix4 m;
    G3D::Vector4 o (object, 1);
    G3D::glGetMatrix (GL_MODELVIEW_MATRIX, m);
    return (m * o).xyz ();
}

bool isMatrixValid (GLenum matrixType)
{
    GLdouble model[16];
    glGetDoublev (matrixType, model);
    for (size_t i = 0; i < 16; ++i)
    {
	if (! bm::isfinite (model[i]))
	{
	    cdbg << "model: ";
	    for (size_t j = 0; j < 16; ++j)
		cdbg << model[j] << " ";
	    cdbg << endl;
	    return false;
	}
    }
    return true;
}

float GetOnePixelInObjectSpace ()
{
    G3D::Vector3 first = toObject (G3D::Vector2 (0, 1));
    G3D::Vector3 second = toObject (G3D::Vector2 (1, 1));
    float onePixelInObjectSpace = (second - first).length ();
    return onePixelInObjectSpace;
}


G3D::Vector3 toObject (const G3D::Vector2& positionGl)
{
    bool is2D = DATA_PROPERTIES.Is2D ();
    G3D::Vector3 op = gluUnProject (positionGl, 
				    is2D ? GluUnProjectZOperation::SET0 : 
				    GluUnProjectZOperation::READ);
    return op;
}


G3D::Vector3 toObject (const QPoint& positionQt, int windowHeight)
{
    G3D::Vector2 positionGl = QtToOpenGl (positionQt, windowHeight);
    return toObject (positionGl);
}


G3D::Vector3 gluUnProject (
    G3D::Vector2 screenCoord, 
    GluUnProjectZOperation::Enum zOperation)
{
    GLdouble model[16];GLdouble proj[16];GLint view[4];
    getCurrentMatrices (model, proj, view);
    GLfloat zScreenCoord = 0;
    if (zOperation == GluUnProjectZOperation::READ)
	glReadPixels (screenCoord.x, screenCoord.y, 1, 1, 
		      GL_DEPTH_COMPONENT, GL_FLOAT, &zScreenCoord);
    else
	zScreenCoord = 0;
    double x = 0, y = 0, z = 0;
    GLint ret = gluUnProject (screenCoord.x, screenCoord.y, zScreenCoord, 
			      model, proj, view, 
			      &x, &y, &z);
    G3D::Vector3 v = 
	G3D::Vector3 (x, y, 
		      zOperation == GluUnProjectZOperation::READ ? z : 0);
    if (ret != GLU_TRUE)
    {
	cdbg << "model: ";
	for (size_t i = 0; i < 16; ++i)
	    cdbg << model[i] << " ";
	cdbg << endl << "projection: ";
	for (size_t i = 0; i < 16; ++i)
	    cdbg << proj[i] << " ";
	cdbg << endl << "viewport: ";
	for (size_t i = 0; i < 4; ++i)
	    cdbg << view[i] << " ";
	cdbg << endl;
	WarnOnOpenGLError ("gluUnProject");
	RuntimeAssert (false, "gluUnproject: ", v, " zOp:", zOperation);
    }
    __LOG__(
	cdbg << screenCoord << ", " << zScreenCoord << ", " << v << endl;)
    return v;
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

	OpenGLFeature ("--- Shaders ---"),
	OpenGLFeature (GL_MAX_VERTEX_ATTRIBS, OpenGLFeature::INTEGER,
		       "GL_MAX_VERTEX_ATTRIBS"),
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
        OpenGLFeature (GL_MAX_CLIP_PLANES, OpenGLFeature::INTEGER, 
                       "MAX_CLIP_PLANES"),
	
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
    glBegin (GL_QUADS);
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

void drawEllipsis2D (float l1, float l2, float size, float lineWidth)
{
    glLineWidth (lineWidth);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; ++i)
    {
	float t = i * M_PI / 180;
	glVertex2f( l1 * cos (t) * size, l2 * sin (t) * size);
    }
    glEnd();
    glLineWidth (1.0);
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

vtkSmartPointer<vtkMatrix4x4> OpenGlToVtk (const boost::array<GLdouble,16>& mv)
{
    VTK_CREATE(vtkMatrix4x4, m);
    m->DeepCopy (&mv[0]);
    m->Transpose ();    
    return m;
}

G3D::Matrix4 OpenGlToG3D (const boost::array<GLdouble,16>& mv)
{
    G3D::Matrix4 m (&mv[0]);
    return m.transpose ();
}

QString ReadShader (const QString& resourceUrl)
{
    QFile file (resourceUrl);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	ThrowException ("Invalid resource: ", 
			resourceUrl.toStdString ());
    QTextStream in (&file);
    return in.readAll ();
}

boost::shared_ptr<QGLShader> CreateShader (const QString& resourceUrl,
					   QGLShader::ShaderType type)
{
    boost::shared_ptr<QGLShader> shader(new QGLShader (type));
    QString vsrc = ReadShader (resourceUrl);
    if (! shader->compileSourceCode(vsrc))
	ThrowException ("Compile error for ", 
			resourceUrl.toLatin1 ().constData ());
    return shader;
}
