/**
 * @file   DisplayFaceAverage.h
 * @author Dan R. Lipsa
 * @date  24 Oct. 2010
 *
 * Implementation for the DisplayFaceAverage class
 */

#include "Debug.h"
#include "DisplayFaceAverage.h"
#include "DisplayBodyFunctors.h"
#include "DisplayFaceFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "Foam.h"
#include "FoamAlongTime.h"
#include "GLWidget.h"
#include "OpenGLUtils.h"
#include "PropertySetter.h"


// ComposeShaderProgram Methods
// ======================================================================
void ComposeShaderProgram::Init ()
{
    m_fshader = boost::make_shared<QGLShader> (QGLShader::Fragment);
    const char *fsrc =
	"uniform sampler2D oldTexUnit;\n"
	"uniform sampler2D stepTexUnit;\n"
        "void main(void)\n"
        "{\n"
	"    vec4 old = texture2D (oldTexUnit, gl_TexCoord[0].st);\n"
	"    vec4 step = texture2D (stepTexUnit, gl_TexCoord[0].st);\n"
	"    vec2 newSumCount = old.rg + step.rg;\n"
	"    float min = min (old.b, step.b);"
	"    float max = max (old.a, step.a);"
        "    gl_FragColor = vec4 (newSumCount, min, max);\n"
        "}\n";
    m_fshader->compileSourceCode(fsrc);
    addShader(m_fshader.get ());
    link();

    m_oldTexUnitIndex = uniformLocation("oldTexUnit");
    m_stepTexUnitIndex = uniformLocation("stepTexUnit");
}

void ComposeShaderProgram::Bind ()
{
    bool bindSuccessful = bind ();
    RuntimeAssert (bindSuccessful, "Bind failed for ComposeShaderProgram");
    setUniformValue (m_oldTexUnitIndex, GetOldTexUnit ());
    setUniformValue (m_stepTexUnitIndex, GetStepTexUnit ());
}



// StoreShaderProgram Methods
// ======================================================================
void StoreShaderProgram::Init ()
{
    m_vshader = boost::make_shared<QGLShader> (QGLShader::Vertex);
    const char *vsrc =
        "attribute float vValue;\n"
        "varying float fValue;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = ftransform();\n"	
        "    fValue = vValue;\n"
        "}\n";
    m_vshader->compileSourceCode(vsrc);

    m_fshader = boost::make_shared<QGLShader> (QGLShader::Fragment);
    const char *fsrc =
	"varying float fValue;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = vec4 (fValue, 1, fValue, fValue);\n"
        "}\n";
    m_fshader->compileSourceCode(fsrc);

    addShader(m_vshader.get ());
    addShader(m_fshader.get ());
    link();

    m_vValueIndex = attributeLocation("vValue");
}

void StoreShaderProgram::Bind ()
{
    bool bindSuccessful = bind ();
    RuntimeAssert (bindSuccessful, "Bind failed for StoreShaderProgram");
}

// InitShaderProgram Methods
// ======================================================================
void InitShaderProgram::Init ()
{
    m_fshader = boost::make_shared<QGLShader> (QGLShader::Fragment);
    const char *fsrc =
        "void main(void)\n"
        "{\n"
        "    float max = 3.40282e+38;"
        "    gl_FragColor = vec4 (0, 0, max, -max);\n"
        "}\n";
    m_fshader->compileSourceCode(fsrc);
    addShader(m_fshader.get ());
    link();
}

void InitShaderProgram::Bind ()
{
    bool bindSuccessful = bind ();
    RuntimeAssert (bindSuccessful, "Bind failed for InitShaderProgram");
}


// DisplayShaderProgram Methods
// ======================================================================

void DisplayShaderProgram::Init ()
{
    m_fshader = boost::make_shared<QGLShader> (QGLShader::Fragment);
    // this should match StatisticsType::Enum order
    const char *fsrc =
	"// 0: average, 1: min, 2: max\n"
	"uniform int displayType;\n"
	"uniform float minValue;\n"
	"uniform float maxValue;\n"
	"uniform sampler1D colorBarTexUnit;\n"
	"uniform sampler2D resultTexUnit;\n"
        "void main(void)\n"
        "{\n"
	"    vec4 result = texture2D (resultTexUnit, gl_TexCoord[0].st);\n"
	"    if (result.g == 0.0)\n"
	"        gl_FragColor = vec4 (1.0, 1.0, 1.0, 1.0);\n"
	"    else\n"
	"    {\n"
	"        float value;\n"
	"        if (displayType == 0)\n"
	"           value = result.r / result.g;\n"
	"        else if (displayType == 1)\n"
	"           value = result.b;\n"
	"        else\n"
	"           value = result.a;\n"
	"        float colorBarTexIndex = (value - minValue) / (maxValue - minValue);\n"
        "        gl_FragColor = texture1D (colorBarTexUnit, colorBarTexIndex);\n"
	"    }\n"
        "}\n";
    m_fshader->compileSourceCode(fsrc);

    addShader(m_fshader.get ());
    link();

    m_displayTypeIndex = uniformLocation ("displayType");
    m_minValueIndex = uniformLocation("minValue");
    m_maxValueIndex = uniformLocation("maxValue");
    m_colorBarTexUnitIndex = uniformLocation("colorBarTexUnit");
    m_resultTexUnitIndex = uniformLocation("resultTexUnit");
}

void DisplayShaderProgram::Bind (GLfloat minValue, GLfloat maxValue,
				 StatisticsType::Enum displayType)
{
    bool bindSuccessful = bind ();
    RuntimeAssert (bindSuccessful, "Bind failed for DisplayShaderProgram");
    setUniformValue (m_displayTypeIndex, displayType);
    setUniformValue (m_minValueIndex, minValue);
    setUniformValue (m_maxValueIndex, maxValue);
    setUniformValue (m_colorBarTexUnitIndex, GetColorBarTexUnit ());
    setUniformValue (m_resultTexUnitIndex, GetResultTexUnit ());
}

// DisplayFaceAverage Methods
// ======================================================================

void DisplayFaceAverage::Init (const QSize& size)
{
    glPushAttrib (GL_COLOR_BUFFER_BIT);
    m_step.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, 
	    GL_RGBA32F));
    m_new.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D,
	    GL_RGBA32F));
    m_old.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D,
	    GL_RGBA32F));
    m_debug.reset (new QGLFramebufferObject (size));
    glPopAttrib ();
    Clear ();
}

void DisplayFaceAverage::Clear ()
{
    clearZero (*m_new);
    clearMinMax (*m_old);
}

void DisplayFaceAverage::Release ()
{
    m_step.reset ();
    m_new.reset ();
    m_old.reset ();
}

void DisplayFaceAverage::InitShaders ()
{
    m_addShaderProgram.Init ();
    m_storeShaderProgram.Init ();
    m_displayShaderProgram.Init ();
    m_initShaderProgram.Init ();
}

void DisplayFaceAverage::Calculate (BodyProperty::Enum property,
				    GLfloat minValue, GLfloat maxValue)
{
    const FoamAlongTime& foamAlongTime = m_glWidget.GetFoamAlongTime ();
    size_t count = foamAlongTime.GetTimeSteps ();
    for (size_t i = 0; i < count; ++i)
    {
	Step (i, property, minValue, maxValue);
	if (true /*i % 10 == 0*/)
	{
	    Display (minValue, maxValue);
	    const_cast<GLWidget&>(m_glWidget).updateGL ();
	    QCoreApplication::processEvents ();
	}
    }
}

void DisplayFaceAverage::display (
    GLfloat minValue, GLfloat maxValue,
    StatisticsType::Enum displayType, QGLFramebufferObject& srcFbo)
{
    m_displayShaderProgram.Bind (minValue, maxValue, displayType);
    const_cast<GLWidget&>(m_glWidget).glActiveTexture (
	TextureEnum (m_displayShaderProgram.GetResultTexUnit ()));
    m_glWidget.RenderFromFbo (srcFbo);
    const_cast<GLWidget&>(m_glWidget).glActiveTexture (GL_TEXTURE0);
    m_displayShaderProgram.release ();
}


void DisplayFaceAverage::StepDisplay ()
{
    const FoamAlongTime& foamAlongTime = m_glWidget.GetFoamAlongTime ();
    BodyProperty::Enum facesColor = m_glWidget.GetColoredBy ();
    GLfloat minValue = foamAlongTime.GetMin (facesColor);
    GLfloat maxValue = foamAlongTime.GetMax (facesColor);
    size_t timeStep = m_glWidget.GetTimeStep ();
    Step (timeStep, facesColor, minValue, maxValue);
    Display (minValue, maxValue, m_glWidget.GetStatisticsType ());
}

void DisplayFaceAverage::Step (
    size_t timeStep, BodyProperty::Enum property,
    GLfloat minValue, GLfloat maxValue)
{
    const Foam& foam = *m_glWidget.GetFoamAlongTime ().GetFoam (timeStep);

    (void)timeStep;(void)minValue;(void)maxValue;
    QSize size = m_new->size ();
    glPushMatrix ();
    {
	glPushAttrib (GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);
	m_glWidget.ModelViewTransformNoRotation ();	
	renderToStep (foam, property);
	//save (*m_step, "step", timeStep);
	addToNew ();
	//save (*m_new, "new", timeStep);
	copyToOld ();
	//save (*m_old, "old", timeStep);    
	glPopAttrib ();
    }
    glPopMatrix ();
    detectOpenGLError ();
}

void DisplayFaceAverage::renderToStep (
    const Foam& foam, BodyProperty::Enum property)
{
    clearMinMax (*m_step);
    m_step->bind ();
    m_storeShaderProgram.Bind ();
    const Foam::Bodies& bodies = foam.GetBodies ();
    if (foam.IsQuadratic ())
	writeFacesValues<DisplaySameEdges> (bodies, property);
    else
	writeFacesValues<DisplaySameTriangles> (bodies, property);
    m_storeShaderProgram.release ();
    m_step->release ();
}

void DisplayFaceAverage::addToNew ()
{
    m_new->bind ();
    m_addShaderProgram.Bind ();

    // bind old texture
    const_cast<GLWidget&>(m_glWidget).glActiveTexture (
	TextureEnum (m_addShaderProgram.GetOldTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_old->texture ());

    // bind step texture
    const_cast<GLWidget&>(m_glWidget).glActiveTexture (
	TextureEnum (m_addShaderProgram.GetStepTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_step->texture ());
    // set the active texture to texture 0
    const_cast<GLWidget&>(m_glWidget).glActiveTexture (GL_TEXTURE0);

    m_glWidget.RenderFromFbo (*m_step);
    m_addShaderProgram.release ();
    m_new->release ();
}

void DisplayFaceAverage::copyToOld ()
{
    QSize size = m_new->size ();
    QRect rect (QPoint (0, 0), size);
    QGLFramebufferObject::blitFramebuffer (
	m_old.get (), rect, m_new.get (), rect);
}

void DisplayFaceAverage::clearZero (QGLFramebufferObject& fbo)
{
    fbo.bind ();
    glPushAttrib (GL_COLOR_BUFFER_BIT); 
    glClearColor (Qt::black);
    glClear(GL_COLOR_BUFFER_BIT);
    glPopAttrib ();
    fbo.release ();    
}

void DisplayFaceAverage::clearMinMax (QGLFramebufferObject& fbo)
{
    fbo.bind ();
    m_initShaderProgram.Bind ();
    glPushAttrib (GL_VIEWPORT_BIT);
    glViewport (0, 0, m_glWidget.width (), m_glWidget.height ());
    // Based on OpenGL FAQ, 9.090 How do I draw a full-screen quad?
    glPushMatrix ();
    {
	glLoadIdentity ();
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	{
	    glLoadIdentity ();
	    glBegin (GL_QUADS);
	    glVertex3i (-1, -1, -1);
	    glVertex3i (1, -1, -1);
	    glVertex3i (1, 1, -1);
	    glVertex3i (-1, 1, -1);
	    glEnd ();
	}
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
    }
    glPopAttrib ();
    glPopMatrix ();
    m_initShaderProgram.release ();
    fbo.release ();
}



void DisplayFaceAverage::save (
    QGLFramebufferObject& fbo, string fileName, size_t timeStep,
    GLfloat minValue, GLfloat maxValue, StatisticsType::Enum displayType)
{
    // render to the debug buffer
    m_debug->bind ();
    display (minValue, maxValue, displayType, fbo);
    m_debug->release ();
    ostringstream ostr;
    ostr << setfill ('0') << setw (4) << timeStep << fileName << ".jpg";
    m_debug->toImage ().save (ostr.str ().c_str ());    
}

template<typename displaySameEdges>
void DisplayFaceAverage::writeFacesValues (
    const Foam::Bodies& bodies, BodyProperty::Enum property)
{
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | 
		  GL_ENABLE_BIT | GL_TEXTURE_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_TEXTURE_1D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture (GL_TEXTURE_1D, m_glWidget.GetColorBarTexture ());
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<
	      DisplayFaceWithColor<displaySameEdges, VertexAttributeSetter>,
	      VertexAttributeSetter> (
		  m_glWidget, m_glWidget.GetBodySelector (), 
		  VertexAttributeSetter (
		      m_storeShaderProgram, 
		      m_storeShaderProgram.GetVValueIndex (), m_glWidget),
		  property,
		  DisplayElement::INVISIBLE_CONTEXT));
    glPopAttrib ();
}
