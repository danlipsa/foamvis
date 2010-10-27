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


class VertexAttributeSetter
{
public:
    VertexAttributeSetter (QGLShaderProgram& program, int attr) :
	m_program (program), m_attr (attr)
    {
    }

    void operator () (double value)
    {
	m_program.setAttributeValue (m_attr, value);
    }

private:
    QGLShaderProgram& m_program;
    int m_attr;
};



void DisplayFaceAverage::Init (const QSize& size)
{
    m_new.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RG32F));
    m_old.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RG32F));
    step ();
}

void DisplayFaceAverage::InitShaders ()
{
    initAddShader ();
    initDisplayShader ();    
}

void DisplayFaceAverage::initAddShader ()
{
    QGLShader *vshaderAdd = new QGLShader(QGLShader::Vertex, 
					  const_cast<GLWidget*>(&m_glWidget));
    const char *vsrcAdd =
        "attribute float vValue;\n"
        "varying float fValue;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = ftransform();\n"	
        "    fValue = vValue;\n"
        "}\n";
    vshaderAdd->compileSourceCode(vsrcAdd);

    QGLShader *fshaderAdd = new QGLShader(QGLShader::Fragment, 
					  const_cast<GLWidget*>(&m_glWidget));
    const char *fsrcAdd =
	"varying float fValue;\n"
	"uniform sampler2D oldTexUnit;\n"
        "void main(void)\n"
        "{\n"
        "    vec2 old = texture2D (oldTexUnit, gl_FragCoord.st).xy;\n"
        "    vec2 new = vec2 (old.x + fValue, old.y + 1);\n"
        "    gl_FragColor.rg = new;\n"
        "}\n";
    fshaderAdd->compileSourceCode(fsrcAdd);

    m_add.shader.addShader(vshaderAdd);
    m_add.shader.addShader(fshaderAdd);
    m_add.shader.link();

    m_add.vValueIndex = m_add.shader.attributeLocation("vValue");
    m_add.oldTexUnitIndex = m_add.shader.uniformLocation("oldTexUnit");
}

void DisplayFaceAverage::Calculate (BodyProperty::Enum bodyProperty)
{
    const FoamAlongTime& foamAlongTime = m_glWidget.GetFoamAlongTime ();
    m_add.shader.bind ();
    m_add.shader.setUniformValue (m_add.oldTexUnitIndex, 1);
    const_cast<GLWidget&>(m_glWidget).glActiveTexture (GL_TEXTURE1);
    glBindTexture (GL_TEXTURE_2D, m_old->texture ());

    size_t i = 0;
    BOOST_FOREACH (const boost::shared_ptr<const Foam>& foam, 
		   foamAlongTime.GetFoams ())
    {
	step (foam.get (), bodyProperty);
	if (i >= 2)
	    break;
	++i;
    }
    const_cast<GLWidget&>(m_glWidget).glActiveTexture (GL_TEXTURE0);
    m_add.shader.release ();
}

void DisplayFaceAverage::step (
    const Foam* foam, BodyProperty::Enum bodyProperty)
{
    QSize size = m_new->size ();
    {
	glPushMatrix ();
	glPushAttrib (GL_CURRENT_BIT | GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);
	m_glWidget.ViewportTransform (size.width (), size.height ());
	m_glWidget.ModelViewTransformNoRotation ();
	{
	    m_new->bind ();
	    glClearColor (Qt::black);
	    // render to the new buffer
	    glClear(GL_COLOR_BUFFER_BIT);
	    if (foam != 0)
	    {
		const Foam::Bodies& bodies = foam->GetBodies ();
		if (foam->IsQuadratic ())
		    displayFacesValues<DisplaySameEdges> (bodies, bodyProperty);
		else
		    displayFacesValues<DisplaySameTriangles> (
			bodies, bodyProperty);
	    }
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


void DisplayFaceAverage::initDisplayShader ()
{
    QGLShader *fshaderDisplay = new QGLShader(QGLShader::Fragment);
    const char *fsrcDisplay =
	"uniform float minValue;\n"
	"uniform float maxValue;\n"
	"uniform sampler1D colorBarTexUnit;\n"
	"uniform sampler2D averageTexUnit;\n"
        "void main(void)\n"
        "{\n"
	"    vec2 averageCount = texture2D (averageTexUnit, gl_TexCoord[0].st).xy;\n"
	"    if (averageCount.y == 0)\n"
	"        gl_FragColor = vec4 (1.0, 1.0, 1.0, 1.0);\n"
	"    else\n"
	"    {\n"
	"        float average = averageCount.x / averageCount.y;\n"
	"        float colorBarTexIndex = (average - minValue) / (maxValue - minValue);\n"
        "        gl_FragColor = texture1D (colorBarTexUnit, colorBarTexIndex);\n"
	"    }\n"
        "}\n";
    fshaderDisplay->compileSourceCode(fsrcDisplay);

    m_display.shader.addShader(fshaderDisplay);
    m_display.shader.link();

    m_display.minValueIndex = 
	m_display.shader.uniformLocation("minValue");
    m_display.maxValueIndex = 
	m_display.shader.uniformLocation("maxValue");
    m_display.colorBarTexUnitIndex = 
	m_display.shader.uniformLocation("colorBarTexUnit");
    m_display.averageTexUnitIndex = 
	m_display.shader.uniformLocation("averageTexUnit");
}

void DisplayFaceAverage::Display (GLfloat minValue, GLfloat maxValue,
				  GLint colorBarTexUnit)
{
    m_display.shader.bind ();
    m_display.shader.setUniformValue (m_display.minValueIndex, minValue);
    m_display.shader.setUniformValue (m_display.maxValueIndex, maxValue);
    m_display.shader.setUniformValue (
	m_display.colorBarTexUnitIndex, colorBarTexUnit);
    m_display.shader.setUniformValue (m_display.averageTexUnitIndex, 1);
    const_cast<GLWidget&>(m_glWidget).glActiveTexture (GL_TEXTURE1);
    m_glWidget.RenderFromFbo (*m_new);
    const_cast<GLWidget&>(m_glWidget).glActiveTexture (GL_TEXTURE0);
    m_display.shader.release ();
}



void DisplayFaceAverage::Release ()
{
    m_new.reset ();
    m_old.reset ();
}


template<typename displaySameEdges>
void DisplayFaceAverage::displayFacesValues (
    const Foam::Bodies& bodies, BodyProperty::Enum bodyProperty)
{
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_TEXTURE_1D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture (GL_TEXTURE_1D, m_glWidget.GetColorBarTexture ());
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<
	      DisplayFaceWithColor<displaySameEdges, VertexAttributeSetter>,
	      VertexAttributeSetter> (
		  m_glWidget, AllBodiesSelected (), 
		  VertexAttributeSetter (m_add.shader, m_add.vValueIndex),
		  bodyProperty,
		  DisplayElement::INVISIBLE_CONTEXT));
    glPopAttrib ();
}
