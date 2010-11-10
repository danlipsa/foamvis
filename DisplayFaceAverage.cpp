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
#include "GLWidget.h"
#include "PropertySetter.h"


// AddShaderProgram Methods
// ======================================================================
void AddShaderProgram::Init ()
{
    QGLShader *fshader = new QGLShader(QGLShader::Fragment);
    const char *fsrc =
	"uniform sampler2D oldTexUnit;\n"
	"uniform sampler2D stepTexUnit;\n"
        "void main(void)\n"
        "{\n"
	"    vec2 old = texture2D (oldTexUnit, gl_TexCoord[0].st).rg;\n"
	"    vec2 step = texture2D (stepTexUnit, gl_TexCoord[0].st).rg;\n"
        "    gl_FragColor.rg = old + step;\n"
        "}\n";
    fshader->compileSourceCode(fsrc);
    addShader(fshader);
    link();

    m_oldTexUnitIndex = uniformLocation("oldTexUnit");
    m_stepTexUnitIndex = uniformLocation("stepTexUnit");
}

void AddShaderProgram::Bind ()
{
    bool bindSuccessful = bind ();
    RuntimeAssert (bindSuccessful, "Bind failed for DisplayShaderProgram");
    setUniformValue (m_oldTexUnitIndex, GetOldTexUnit ());
    setUniformValue (m_stepTexUnitIndex, GetStepTexUnit ());
}



// StoreShaderProgram Methods
// ======================================================================
void StoreShaderProgram::Init ()
{
    QGLShader *vshader = new QGLShader(QGLShader::Vertex);
    const char *vsrc =
        "attribute float vValue;\n"
        "varying float fValue;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = ftransform();\n"	
        "    fValue = vValue;\n"
        "}\n";
    vshader->compileSourceCode(vsrc);

    QGLShader *fshader = new QGLShader(QGLShader::Fragment);
    const char *fsrc =
	"varying float fValue;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor.rg = vec2 (fValue, 1);\n"
        "}\n";
    fshader->compileSourceCode(fsrc);

    addShader(vshader);
    addShader(fshader);
    link();

    m_vValueIndex = attributeLocation("vValue");
}

void StoreShaderProgram::Bind ()
{
    bool bindSuccessful = bind ();
    RuntimeAssert (bindSuccessful, "Bind failed for StoreShaderProgram");
}

// DisplayShaderProgram Methods
// ======================================================================

void DisplayShaderProgram::Init ()
{
    QGLShader *fshader = new QGLShader(QGLShader::Fragment);
    const char *fsrc =
	"uniform float minValue;\n"
	"uniform float maxValue;\n"
	"uniform sampler1D colorBarTexUnit;\n"
	"uniform sampler2D averageTexUnit;\n"
        "void main(void)\n"
        "{\n"
	"    vec2 averageCount = texture2D (averageTexUnit, gl_TexCoord[0].st).xy;\n"
	"    if (averageCount.y == 0.0)\n"
	"        gl_FragColor = vec4 (1.0, 1.0, 1.0, 1.0);\n"
	"    else\n"
	"    {\n"
	"        float average = averageCount.x / averageCount.y;\n"
	"        float colorBarTexIndex = (average - minValue) / (maxValue - minValue);\n"
        "        gl_FragColor = texture1D (colorBarTexUnit, colorBarTexIndex);\n"
	"    }\n"
        "}\n";
    fshader->compileSourceCode(fsrc);

    addShader(fshader);
    link();

    m_minValueIndex = uniformLocation("minValue");
    m_maxValueIndex = uniformLocation("maxValue");
    m_colorBarTexUnitIndex = uniformLocation("colorBarTexUnit");
    m_averageTexUnitIndex = uniformLocation("averageTexUnit");
}

void DisplayShaderProgram::Bind (GLfloat minValue, GLfloat maxValue)
{
    bool bindSuccessful = bind ();
    RuntimeAssert (bindSuccessful, "Bind failed for DisplayShaderProgram");
    setUniformValue (m_minValueIndex, minValue);
    setUniformValue (m_maxValueIndex, maxValue);
    setUniformValue (m_colorBarTexUnitIndex, GetColorBarTexUnit ());
    setUniformValue (m_averageTexUnitIndex, GetAverageTexUnit ());
}

// DisplayFaceAverage Methods
// ======================================================================

void DisplayFaceAverage::Init (const QSize& size)
{
    glPushAttrib (GL_COLOR_BUFFER_BIT);
    m_step.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RG32F));
    m_new.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RG32F));
    m_old.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_RG32F));
    m_debug.reset (new QGLFramebufferObject (size));
    glPopAttrib ();
    Clear ();
}

void DisplayFaceAverage::Clear ()
{
    clear (*m_step);
    clear (*m_new);
    clear (*m_old);
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
}

void DisplayFaceAverage::Calculate (BodyProperty::Enum bodyProperty,
				    GLfloat minValue, GLfloat maxValue)
{
    const FoamAlongTime& foamAlongTime = m_glWidget.GetFoamAlongTime ();
    size_t count = foamAlongTime.GetTimeSteps ();
    for (size_t i = 0; i < count; ++i)
    {
	Step (i, bodyProperty, minValue, maxValue);
	if (true /*i % 10 == 0*/)
	{
	    Display (minValue, maxValue);
	    const_cast<GLWidget&>(m_glWidget).updateGL ();
	    QCoreApplication::processEvents ();
	}
    }
}

void DisplayFaceAverage::display (
    GLfloat minValue, GLfloat maxValue, QGLFramebufferObject& srcFbo)
{
    m_displayShaderProgram.Bind (minValue, maxValue);
    // bind srcFbo.texture () to texture 1
    const_cast<GLWidget&>(m_glWidget).glActiveTexture (
	TextureEnum (m_displayShaderProgram.GetAverageTexUnit ()));
    m_glWidget.RenderFromFbo (srcFbo);

    const_cast<GLWidget&>(m_glWidget).glActiveTexture (GL_TEXTURE0);
    m_displayShaderProgram.release ();
}


void DisplayFaceAverage::StepDisplay ()
{
    const FoamAlongTime& foamAlongTime = m_glWidget.GetFoamAlongTime ();
    BodyProperty::Enum facesColor = m_glWidget.GetFacesColor ();
    GLfloat minValue = foamAlongTime.GetMin (facesColor);
    GLfloat maxValue = foamAlongTime.GetMax (facesColor);
    size_t timeStep = m_glWidget.GetTimeStep ();
    Step (timeStep, facesColor, minValue, maxValue);
    Display (minValue, maxValue);
}

void DisplayFaceAverage::Step (
    size_t timeStep, BodyProperty::Enum bodyProperty,
    GLfloat minValue, GLfloat maxValue)
{
    const Foam& foam = *m_glWidget.GetFoamAlongTime ().GetFoam (timeStep);

    (void)timeStep;(void)minValue;(void)maxValue;
    QSize size = m_new->size ();
    glPushMatrix ();
    glPushAttrib (GL_CURRENT_BIT | GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);
    //m_glWidget.ViewportTransform (size.width (), size.height ());
    m_glWidget.ModelViewTransformNoRotation ();	
    renderToStep (foam, bodyProperty);
    //save (*m_step, "step", timeStep);
    addToNew ();
    //save (*m_new, "new", timeStep);
    copyToOld ();
    //save (*m_old, "old", timeStep);    
    glPopAttrib ();
    glPopMatrix ();
    detectOpenGLError ();
}

void DisplayFaceAverage::renderToStep (
    const Foam& foam, BodyProperty::Enum bodyProperty)
{
    m_step->bind ();
    glPushAttrib (GL_COLOR_BUFFER_BIT);
    glClearColor (Qt::black);
    glClear(GL_COLOR_BUFFER_BIT);
    m_storeShaderProgram.Bind ();
    const Foam::Bodies& bodies = foam.GetBodies ();
    if (foam.IsQuadratic ())
	writeFacesValues<DisplaySameEdges> (bodies, bodyProperty);
    else
	writeFacesValues<DisplaySameTriangles> (
	    bodies, bodyProperty);
    m_storeShaderProgram.release ();
    glPopAttrib ();
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

void DisplayFaceAverage::clear (QGLFramebufferObject& fbo)
{
    fbo.bind ();
    glPushAttrib (GL_COLOR_BUFFER_BIT); 
    glClearColor (Qt::black);
    glClear(GL_COLOR_BUFFER_BIT);
    glPopAttrib ();
    fbo.release ();    
}

void DisplayFaceAverage::save (
    QGLFramebufferObject& fbo, string fileName, size_t timeStep,
    GLfloat minValue, GLfloat maxValue)
{
    // render to the debug buffer
    m_debug->bind ();
    display (minValue, maxValue, fbo);
    m_debug->release ();
    ostringstream ostr;
    ostr << setfill ('0') << setw (4) << timeStep << fileName << ".jpg";
    m_debug->toImage ().save (ostr.str ().c_str ());    
}

template<typename displaySameEdges>
void DisplayFaceAverage::writeFacesValues (
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
		  m_glWidget, m_glWidget.GetBodySelector (), 
		  VertexAttributeSetter (
		      m_storeShaderProgram, 
		      m_storeShaderProgram.GetVValueIndex ()),
		  bodyProperty,
		  DisplayElement::INVISIBLE_CONTEXT));
    glPopAttrib ();
}
