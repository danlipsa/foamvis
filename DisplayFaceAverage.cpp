/**
 * @file   DisplayFaceAverage.h
 * @author Dan R. Lipsa
 * @date  24 Oct. 2010
 *
 * Implementation for the DisplayFaceAverage class
 */

#include "DisplayFaceAverage.h"
#include "DisplayBodyFunctors.h"
#include "DisplayFaceFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "Foam.h"
#include "GLWidget.h"

void DisplayFaceAverage::Init (const QSize& size)
{
    m_new.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RGB32F));
    m_old.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RGB32F));
}

void DisplayFaceAverage::InitShaders ()
{
    initAddShader ();
    initTextureShader ();    
}

void DisplayFaceAverage::initAddShader ()
{
    QGLShader *vshaderAdd = new QGLShader(QGLShader::Vertex, 
					  const_cast<GLWidget*>(&m_glWidget));
    const char *vsrcAdd =
        "uniform float minValue;\n"
	"uniform float maxValue;\n"
        "attribute float value;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = ftransform();\n"	
        "    gl_TexCoord[0].s = (value - minValue) / (maxValue - minValue);\n"
        "}\n";
    vshaderAdd->compileSourceCode(vsrcAdd);

    QGLShader *fshaderAdd = new QGLShader(QGLShader::Fragment, 
					  const_cast<GLWidget*>(&m_glWidget));
    const char *fsrcAdd =
	"uniform sampler1D colorBarTexture;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = texture1D (colorBarTexture, gl_TexCoord[0].s);\n"
        "}\n";
    fshaderAdd->compileSourceCode(fsrcAdd);

    m_addShader.addShader(vshaderAdd);
    m_addShader.addShader(fshaderAdd);
    m_addShader.link();

    m_minValueAttr = m_addShader.uniformLocation("minValue");
    m_maxValueAttr = m_addShader.uniformLocation("maxValue");
    m_valueAttr = m_addShader.attributeLocation("value");
    m_colorBarTextureAttr = m_addShader.uniformLocation("colorBarTexture");
}

void DisplayFaceAverage::initTextureShader ()
{
    QGLShader *vshaderTexture = new QGLShader(QGLShader::Vertex);
    const char *vsrcTexture =
        "attribute vec4 vertex;\n"
        "attribute vec4 texCoord;\n"
        "attribute vec3 normal;\n"
        "uniform mat4 matrix;\n"
        "varying vec4 texc;\n"
        "varying float angle;\n"
        "void main(void)\n"
        "{\n"
        "    vec3 toLight = normalize(vec3(0.0, 0.3, 1.0));\n"
        "    angle = max(dot(normal, toLight), 0.0);\n"
        "    gl_Position = matrix * vertex;\n"
        "    texc = texCoord;\n"
        "}\n";
    vshaderTexture->compileSourceCode(vsrcTexture);

    QGLShader *fshaderTexture = new QGLShader(QGLShader::Fragment);
    const char *fsrcTexture =
        "varying vec4 texc;\n"
        "uniform sampler2D tex;\n"
        "varying float angle;\n"
        "void main(void)\n"
        "{\n"
        "    highp vec3 color = texture2D(tex, texc.st).rgb;\n"
        "    color = color * 0.2 + color * 0.8 * angle;\n"
        "    gl_FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);\n"
        "}\n";
    fshaderTexture->compileSourceCode(fsrcTexture);

    m_textureShader.addShader(vshaderTexture);
    m_textureShader.addShader(fshaderTexture);
    m_textureShader.link();
}


void DisplayFaceAverage::Release ()
{
    m_new.reset ();
    m_old.reset ();
}

void DisplayFaceAverage::Step (const Foam& foam)
{
    QSize size = m_new->size ();
    const Foam::Bodies& bodies = foam.GetBodies ();
    {
	glPushMatrix ();
	glPushAttrib (GL_CURRENT_BIT | GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);
	m_glWidget.ViewportTransform (size.width (), size.height ());
	m_glWidget.ModelViewTransformNoRotation ();
	{
	    m_new->bind ();
	    glClearColor (Qt::white);
	    // render to the new buffer
	    glClear(GL_COLOR_BUFFER_BIT);
	    if (foam.IsQuadratic ())
		displayFacesValues<DisplaySameEdges> (bodies);
	    else
		displayFacesValues<DisplaySameTriangles> (bodies);
	    m_new->release ();
	}
	m_new->toImage ().save ("new.jpg");
        // copy current --> previous buffer
	QRect rect (QPoint (0, 0), size);
	QGLFramebufferObject::blitFramebuffer (
	    m_old.get (), rect, m_new.get (), rect);
	m_old->toImage ().save ("old.jpg");
	glPopAttrib ();
	glPopMatrix ();
    }
    detectOpenGLError ();
}

void DisplayFaceAverage::Display ()
{
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    m_glWidget.RenderFromFbo (*m_new);
}

template<typename displaySameEdges>
void DisplayFaceAverage::displayFacesValues (const Foam::Bodies& bodies) const
{
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_TEXTURE_1D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture (GL_TEXTURE_1D, m_glWidget.GetColorBarTexture ());
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<DisplayFaceWithColor<displaySameEdges> > (
		  m_glWidget, AllBodiesSelected (), 
		  DisplayElement::INVISIBLE_CONTEXT, 
		  m_glWidget.GetFacesColor ()));
    glPopAttrib ();
}
