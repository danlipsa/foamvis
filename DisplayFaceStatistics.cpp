/**
 * @file   DisplayFaceStatistics.h
 * @author Dan R. Lipsa
 * @date  24 Oct. 2010
 *
 * Implementation for the DisplayFaceStatistics class
 */

#include "Debug.h"
#include "DebugStream.h"
#include "DisplayFaceStatistics.h"
#include "DisplayBodyFunctors.h"
#include "DisplayFaceFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "Foam.h"
#include "FoamAlongTime.h"
#include "GLWidget.h"
#include "OpenGLUtils.h"
#include "PropertySetter.h"
#include "ViewSettings.h"


// AddShaderProgram Methods
// ======================================================================
void AddShaderProgram::Init ()
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

void AddShaderProgram::Bind ()
{
    bool bindSuccessful = bind ();
    RuntimeAssert (bindSuccessful, "Bind failed for AddShaderProgram");
    setUniformValue (m_oldTexUnitIndex, GetOldTexUnit ());
    setUniformValue (m_stepTexUnitIndex, GetStepTexUnit ());
}



// RemoveShaderProgram Methods
// ======================================================================
void RemoveShaderProgram::Init ()
{
    m_fshader = boost::make_shared<QGLShader> (QGLShader::Fragment);
    const char *fsrc =
	"uniform sampler2D oldTexUnit;\n"
	"uniform sampler2D stepTexUnit;\n"
        "void main(void)\n"
        "{\n"
	"    vec4 old = texture2D (oldTexUnit, gl_TexCoord[0].st);\n"
	"    vec4 step = texture2D (stepTexUnit, gl_TexCoord[0].st);\n"
	"    vec2 newSumCount = old.rg - step.rg;\n"
	"    float min = old.b;"
	"    float max = old.a;"
        "    gl_FragColor = vec4 (newSumCount, min, max);\n"
        "}\n";
    m_fshader->compileSourceCode(fsrc);
    addShader(m_fshader.get ());
    link();

    m_oldTexUnitIndex = uniformLocation("oldTexUnit");
    m_stepTexUnitIndex = uniformLocation("stepTexUnit");
}

void RemoveShaderProgram::Bind ()
{
    bool bindSuccessful = bind ();
    RuntimeAssert (bindSuccessful, "Bind failed for RemoveShaderProgram");
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
	"// 0: average, 1: min, 2: max, 3: count\n"
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
	"        else if (displayType == 2)\n"
	"           value = result.a;\n"
	"        else\n"
	"           value = result.g;\n"
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

// DisplayFaceStatistics Methods
// ======================================================================

AddShaderProgram DisplayFaceStatistics::m_addShaderProgram;
RemoveShaderProgram DisplayFaceStatistics::m_removeShaderProgram;
StoreShaderProgram DisplayFaceStatistics::m_storeShaderProgram;
DisplayShaderProgram DisplayFaceStatistics::m_displayShaderProgram;
InitShaderProgram DisplayFaceStatistics::m_initShaderProgram;

void DisplayFaceStatistics::Init (const QSize& size)
{
    m_currentHistoryCount = 0;
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

void DisplayFaceStatistics::Clear ()
{
    clearZero (m_new);
    clearMinMax (m_old);
}

void DisplayFaceStatistics::Release ()
{
    m_step.reset ();
    m_new.reset ();
    m_old.reset ();
    m_debug.reset ();
}

void DisplayFaceStatistics::InitShaders ()
{
    m_addShaderProgram.Init ();
    m_removeShaderProgram.Init ();
    m_storeShaderProgram.Init ();
    m_displayShaderProgram.Init ();
    m_initShaderProgram.Init ();
}

void DisplayFaceStatistics::display (
    const G3D::Rect2D& viewRect,
    GLfloat minValue, GLfloat maxValue,
    StatisticsType::Enum displayType, QGLFramebufferObject& srcFbo)
{
    m_displayShaderProgram.Bind (minValue, maxValue, displayType);
    const_cast<GLWidget&>(m_glWidget).glActiveTexture (
	TextureEnum (m_displayShaderProgram.GetResultTexUnit ()));
    m_glWidget.RenderFromFbo (viewRect, srcFbo);
    const_cast<GLWidget&>(m_glWidget).glActiveTexture (GL_TEXTURE0);
    m_displayShaderProgram.release ();
}


void DisplayFaceStatistics::InitStep (
    ViewNumber::Enum view, GLfloat minValue, GLfloat maxValue)
{
    G3D::Rect2D viewRect = m_glWidget.GetViewRect (view);
    Init (QSize (viewRect.width (), viewRect.height ()));
    Step (view, minValue, maxValue);
}

void DisplayFaceStatistics::Step (
    ViewNumber::Enum view, GLfloat minValue, GLfloat maxValue)
{
    Step (view, minValue, maxValue, m_glWidget.GetTimeStep ());
}

void DisplayFaceStatistics::Step (
    ViewNumber::Enum view, GLfloat minValue, GLfloat maxValue, size_t timeStep)
{
    G3D::Rect2D viewRect = m_glWidget.GetViewRect (view);
    // used for display
    (void)minValue;(void)maxValue;
    QSize size = m_new->size ();
    glPushAttrib (GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);
    glPushMatrix ();
    m_glWidget.ModelViewTransform (timeStep);
    renderToStep (view, timeStep);
    save (viewRect, *m_step, "step", timeStep,
	  minValue, maxValue, StatisticsType::AVERAGE);
    glPopMatrix ();
    addStepToNew (viewRect);
    //save (viewRect, *m_new, "new", timeStep,
    //minValue, maxValue, StatisticsType::AVERAGE);
    copyNewToOld ();
    //save (viewRect, *m_old, "old", timeStep, 
    //minValue, maxValue, StatisticsType::AVERAGE);
    ++m_currentHistoryCount;
    if (m_currentHistoryCount > m_historyCount && 
	timeStep >= m_historyCount)
    {
	glPushMatrix ();
	m_glWidget.ModelViewTransform (timeStep - m_historyCount);
	renderToStep (view, timeStep - m_historyCount);
	glPopMatrix ();
	//save (viewRect, *m_step, "step_", timeStep - m_historyCount,
	//minValue, maxValue, StatisticsType::AVERAGE);
	removeStepFromNew (viewRect);
	//save (viewRect, *m_new, "new_", timeStep,
	//minValue, maxValue, StatisticsType::AVERAGE);
	copyNewToOld ();
	//save (viewRect, *m_old, "old_", timeStep, 
	//minValue, maxValue, StatisticsType::AVERAGE);
	--m_currentHistoryCount;
    }
    glPopAttrib ();
    detectOpenGLError ();
}

void DisplayFaceStatistics::renderToStep (ViewNumber::Enum view,
    size_t timeStep)
{
    clearMinMax (m_step);
    m_step->bind ();
    m_storeShaderProgram.Bind ();
    const Foam& foam = *m_glWidget.GetFoamAlongTime ().GetFoam (timeStep);
    const Foam::Bodies& bodies = foam.GetBodies ();
    writeFacesValues<DisplayFaceTriangleFan> (view, bodies);
    m_storeShaderProgram.release ();
    m_step->release ();
}

void DisplayFaceStatistics::addStepToNew (const G3D::Rect2D& viewRect)
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

    m_glWidget.RenderFromFbo (viewRect, *m_step);
    m_addShaderProgram.release ();
    m_new->release ();
}


void DisplayFaceStatistics::removeStepFromNew (const G3D::Rect2D& viewRect)
{
    m_new->bind ();
    m_removeShaderProgram.Bind ();

    // bind old texture
    const_cast<GLWidget&>(m_glWidget).glActiveTexture (
	TextureEnum (m_removeShaderProgram.GetOldTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_old->texture ());

    // bind step texture
    const_cast<GLWidget&>(m_glWidget).glActiveTexture (
	TextureEnum (m_removeShaderProgram.GetStepTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_step->texture ());
    // set the active texture to texture 0
    const_cast<GLWidget&>(m_glWidget).glActiveTexture (GL_TEXTURE0);

    m_glWidget.RenderFromFbo (viewRect, *m_step);
    m_removeShaderProgram.release ();
    m_new->release ();
}



void DisplayFaceStatistics::copyNewToOld ()
{
    QSize size = m_new->size ();
    QRect rect (QPoint (0, 0), size);
    QGLFramebufferObject::blitFramebuffer (
	m_old.get (), rect, m_new.get (), rect);
}

void DisplayFaceStatistics::clearZero (
    const boost::scoped_ptr<QGLFramebufferObject>& fbo)
{
    fbo->bind ();
    glPushAttrib (GL_COLOR_BUFFER_BIT); 
    glClearColor (Qt::black);
    glClear(GL_COLOR_BUFFER_BIT);
    glPopAttrib ();
    fbo->release ();    
}

void DisplayFaceStatistics::clearMinMax (
    const boost::scoped_ptr<QGLFramebufferObject>& fbo)
{
    fbo->bind ();
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
    fbo->release ();
}



void DisplayFaceStatistics::save (const G3D::Rect2D& viewRect,
    QGLFramebufferObject& fbo, const char* fileName, size_t timeStep,
    GLfloat minValue, GLfloat maxValue, StatisticsType::Enum displayType)
{
    // render to the debug buffer
    m_debug->bind ();
    display (viewRect, minValue, maxValue, displayType, fbo);
    m_debug->release ();
    ostringstream ostr;
    ostr << setfill ('0') << setw (4) << timeStep << fileName << ".png";
    m_debug->toImage ().save (ostr.str ().c_str ());    
}

template<typename displaySameEdges>
void DisplayFaceStatistics::writeFacesValues (
    ViewNumber::Enum view, const Foam::Bodies& bodies)
{
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | 
		  GL_ENABLE_BIT | GL_TEXTURE_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_TEXTURE_1D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture (GL_TEXTURE_1D, 
		   m_glWidget.GetViewSettings (view)->GetColorBarTexture ());
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<
	      DisplayFaceBodyPropertyColor<displaySameEdges, 
	      SetterValueVertexAttribute>, SetterValueVertexAttribute> (
		  m_glWidget, m_glWidget.GetBodySelector (), 
		  SetterValueVertexAttribute (
		      m_glWidget, view, &m_storeShaderProgram,
		      m_storeShaderProgram.GetVValueIndex ()),
		  DisplayElement::INVISIBLE_CONTEXT));
    glPopAttrib ();
}
