/**
 * @file   DisplayFaceStatistics.h
 * @author Dan R. Lipsa
 * @date  24 Oct. 2010
 *
 * Implementation for the DisplayFaceStatistics class 
 *
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
	"uniform sampler2D previousTexUnit;\n"
	"uniform sampler2D stepTexUnit;\n"
        "void main(void)\n"
        "{\n"
	"    vec4 previous = texture2D (previousTexUnit, gl_TexCoord[0].st);\n"
	"    vec4 step = texture2D (stepTexUnit, gl_TexCoord[0].st);\n"
	"    vec2 currentSumCount = previous.rg + step.rg;\n"
	"    float min = min (previous.b, step.b);"
	"    float max = max (previous.a, step.a);"
        "    gl_FragColor = vec4 (currentSumCount, min, max);\n"
        "}\n";
    m_fshader->compileSourceCode(fsrc);
    addShader(m_fshader.get ());
    link();

    m_previousTexUnitIndex = uniformLocation("previousTexUnit");
    m_stepTexUnitIndex = uniformLocation("stepTexUnit");
}

void AddShaderProgram::Bind ()
{
    bool bindSuccessful = bind ();
    RuntimeAssert (bindSuccessful, "Bind failed for AddShaderProgram");
    setUniformValue (m_previousTexUnitIndex, GetPreviousTexUnit ());
    setUniformValue (m_stepTexUnitIndex, GetStepTexUnit ());
}



// RemoveShaderProgram Methods
// ======================================================================
void RemoveShaderProgram::Init ()
{
    m_fshader = boost::make_shared<QGLShader> (QGLShader::Fragment);
    const char *fsrc =
	"uniform sampler2D previousTexUnit;\n"
	"uniform sampler2D stepTexUnit;\n"
        "void main(void)\n"
        "{\n"
	"    vec4 previous = texture2D (previousTexUnit, gl_TexCoord[0].st);\n"
	"    vec4 step = texture2D (stepTexUnit, gl_TexCoord[0].st);\n"
	"    vec2 currentSumCount = previous.rg - step.rg;\n"
	"    float min = previous.b;\n"
	"    float max = previous.a;\n"
        "    gl_FragColor = vec4 (currentSumCount, min, max);\n"
        "}\n";
    m_fshader->compileSourceCode(fsrc);
    addShader(m_fshader.get ());
    link();

    m_previousTexUnitIndex = uniformLocation("previousTexUnit");
    m_stepTexUnitIndex = uniformLocation("stepTexUnit");
}

void RemoveShaderProgram::Bind ()
{
    bool bindSuccessful = bind ();
    RuntimeAssert (bindSuccessful, "Bind failed for RemoveShaderProgram");
    setUniformValue (m_previousTexUnitIndex, GetPreviousTexUnit ());
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
	"    float maxFloat = 3.40282e+38;\n"
	"    if (fValue == maxFloat)\n"
	"        gl_FragColor = vec4 (0, 0, maxFloat, -maxFloat);\n"
	"    else\n"
        "        gl_FragColor = vec4 (fValue, 1, fValue, fValue);\n"
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
        "    float maxFloat = 3.40282e+38;"
        "    gl_FragColor = vec4 (0, 0, maxFloat, -maxFloat);\n"
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
	"// displayType values: 0=average, 1=min, 2=max, 3=count\n"
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

void DisplayFaceStatistics::init (ViewNumber::Enum viewNumber)
{
    Average::init (viewNumber);
    const G3D::Rect2D viewRect = GetGLWidget ().GetViewRect (viewNumber);
    QSize size (viewRect.width (), viewRect.height ());
    glPushAttrib (GL_COLOR_BUFFER_BIT);
    m_step.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::CombinedDepthStencil, GL_TEXTURE_2D, 
	    GL_RGBA32F));

    if (m_step->attachment () != QGLFramebufferObject::CombinedDepthStencil)
	cdbg << "No stencil attachement available" << endl;

    m_current.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D,
	    GL_RGBA32F));
    m_previous.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D,
	    GL_RGBA32F));
    m_debug.reset (new QGLFramebufferObject (size));
    glPopAttrib ();
    clear (viewRect);
}

void DisplayFaceStatistics::clear (const G3D::Rect2D& viewRect)
{
    m_step->bind ();ClearColorStencilBuffers (Qt::black, 0);
    m_step->release ();

    m_current->bind ();ClearColorBuffer (Qt::black);
    m_current->release ();
    clearColorBufferMinMax (viewRect, m_previous);
}

void DisplayFaceStatistics::Release ()
{
    m_step.reset ();
    m_current.reset ();
    m_previous.reset ();
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
    glActiveTexture (TextureEnum (m_displayShaderProgram.GetResultTexUnit ()));
    RenderFromFbo (viewRect, srcFbo);
    glActiveTexture (GL_TEXTURE0);
    m_displayShaderProgram.release ();
}

void DisplayFaceStatistics::displayAndRotate (
    const G3D::Rect2D& viewRect,
    GLfloat minValue, GLfloat maxValue,
    StatisticsType::Enum displayType, QGLFramebufferObject& srcFbo,
    G3D::Vector2 rotationCenter, float angleDegrees)
{
    m_displayShaderProgram.Bind (minValue, maxValue, displayType);
    glActiveTexture (TextureEnum (m_displayShaderProgram.GetResultTexUnit ()));
    RenderFromFboAndRotate (viewRect, srcFbo, 
			    rotationCenter, angleDegrees);
    glActiveTexture (GL_TEXTURE0);
    m_displayShaderProgram.release ();
}


typedef void (DisplayFaceStatistics::*Operation) (const G3D::Rect2D& viewRect);

void DisplayFaceStatistics::addStep (ViewNumber::Enum viewNumber, 
				     size_t timeStep)
{
    pair<double, double> minMax = getStatisticsMinMax (viewNumber);
    G3D::Rect2D viewRect = GetGLWidget ().GetViewRect (viewNumber);
    glPushAttrib (GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_VIEWPORT_BIT);
    renderToStep (viewNumber, timeStep);
    //save (viewRect, *m_step, "step", timeStep,
    //minMax.first, minMax.second, StatisticsType::AVERAGE);
    addStepToCurrent (viewRect);
    //save (viewRect, *m_current, "current", timeStep,
    //minMax.first, minMax.second, StatisticsType::AVERAGE);
    copyCurrentToPrevious ();
    //save (viewRect, *m_previous, "previous", timeStep, 
    //minMax.first, minMax.second, StatisticsType::AVERAGE);    
    glPopAttrib ();
    WarnOnOpenGLError ("DisplayFaceStatistics::addStep");
}

void DisplayFaceStatistics::removeStep (ViewNumber::Enum viewNumber, 
				     size_t timeStep)
{
    pair<double, double> minMax = getStatisticsMinMax (viewNumber);
    G3D::Rect2D viewRect = GetGLWidget ().GetViewRect (viewNumber);
    glPushAttrib (GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_VIEWPORT_BIT);
    renderToStep (viewNumber, timeStep);
    //save (viewRect, *m_step, "step_", timeStepy - m_timeWindow,
    //minMax.first, minMax.second, StatisticsType::AVERAGE);
    removeStepFromCurrent (viewRect);
    //save (viewRect, *m_current, "current_", timeStep,
    //minMax.first, minMax.second, StatisticsType::AVERAGE);
    copyCurrentToPrevious ();
    //save (viewRect, *m_previous, "previous_", timeStep, 
    //minMax.first, minMax.second, StatisticsType::AVERAGE);
    glPopAttrib ();
    WarnOnOpenGLError ("DisplayFaceStatistics::addStep");
}


void DisplayFaceStatistics::renderToStep (
    ViewNumber::Enum viewNumber, size_t timeStep)
{
    G3D::Rect2D viewRect = GetGLWidget ().GetViewRect ();
    glPushMatrix ();
    GetGLWidget ().ModelViewTransform (viewNumber, timeStep);
    glViewport (0, 0, viewRect.width (), viewRect.height ());
    clearColorBufferMinMax (viewRect, m_step);
    m_step->bind ();
    ClearColorStencilBuffers (Qt::black, 0);
    m_storeShaderProgram.Bind ();
    const Foam& foam = GetGLWidget ().GetFoamAlongTime ().GetFoam (timeStep);
    const Foam::Bodies& bodies = foam.GetBodies ();
    writeFacesValues (viewNumber, bodies);
    m_storeShaderProgram.release ();
    m_step->release ();
    glPopMatrix ();
}

void DisplayFaceStatistics::addStepToCurrent (const G3D::Rect2D& viewRect)
{
    m_current->bind ();
    m_addShaderProgram.Bind ();

    // bind previous texture
    glActiveTexture (TextureEnum (m_addShaderProgram.GetPreviousTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_previous->texture ());

    // bind step texture
    glActiveTexture (TextureEnum (m_addShaderProgram.GetStepTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_step->texture ());
    // set the active texture to texture 0
    glActiveTexture (GL_TEXTURE0);

    RenderFromFbo (
	G3D::Rect2D::xywh (0, 0, viewRect.width (), viewRect.height ()), 
	*m_step);
    m_addShaderProgram.release ();
    m_current->release ();
}


void DisplayFaceStatistics::removeStepFromCurrent (const G3D::Rect2D& viewRect)
{
    m_current->bind ();
    m_removeShaderProgram.Bind ();

    // bind previous texture
    glActiveTexture (TextureEnum (m_removeShaderProgram.GetPreviousTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_previous->texture ());

    // bind step texture
    glActiveTexture (TextureEnum (m_removeShaderProgram.GetStepTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_step->texture ());
    // set the active texture to texture 0
    glActiveTexture (GL_TEXTURE0);

    RenderFromFbo (
	G3D::Rect2D::xywh (0, 0, viewRect.width (), viewRect.height ()),
	*m_step);
    m_removeShaderProgram.release ();
    m_current->release ();
}



void DisplayFaceStatistics::copyCurrentToPrevious ()
{
    QSize size = m_current->size ();
    QRect rect (QPoint (0, 0), size);
    QGLFramebufferObject::blitFramebuffer (
	m_previous.get (), rect, m_current.get (), rect);
}

// Based on OpenGL FAQ, 9.090 How do I draw a full-screen quad?
void DisplayFaceStatistics::clearColorBufferMinMax (
    const G3D::Rect2D& viewRect,
    const boost::scoped_ptr<QGLFramebufferObject>& fbo)
{
    fbo->bind ();
    m_initShaderProgram.Bind ();
    glPushAttrib (GL_VIEWPORT_BIT);
    glViewport (0, 0, viewRect.width (), viewRect.height ());
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

void DisplayFaceStatistics::Display (
    ViewNumber::Enum viewNumber, StatisticsType::Enum displayType)
{
    if (m_current.get () != 0)
    {
	pair<double, double> minMax = getStatisticsMinMax (viewNumber);
	const G3D::Rect2D viewRect = GetGLWidget ().GetViewRect (viewNumber);
	display (viewRect, minMax.first, minMax.second, displayType, *m_current);
    }
}

void DisplayFaceStatistics::DisplayAndRotate (
    ViewNumber::Enum viewNumber,
    StatisticsType::Enum displayType, 
    G3D::Vector2 rotationCenter, float angleDegrees)
{
    if (m_current.get () != 0)
    {
	pair<double, double> minMax = getStatisticsMinMax (viewNumber);
	const G3D::Rect2D viewRect = GetGLWidget ().GetViewRect (viewNumber);
	displayAndRotate (
	    viewRect, minMax.first, minMax.second, displayType, *m_current,
	    rotationCenter, angleDegrees);
    }
}



void DisplayFaceStatistics::save (const G3D::Rect2D& viewRect,
    QGLFramebufferObject& fbo, const char* postfix, size_t timeStep,
    GLfloat minValue, GLfloat maxValue, StatisticsType::Enum displayType)
{
    // render to the debug buffer
    m_debug->bind ();
    display (G3D::Rect2D::xywh (0, 0, viewRect.width (), viewRect.height ()), 
	     minValue, maxValue, displayType, fbo);
    m_debug->release ();
    ostringstream ostr;
    ostr << "images/" 
	 << setfill ('0') << setw (4) << timeStep << postfix << ".png";
    m_debug->toImage ().save (ostr.str ().c_str ());    
}

void DisplayFaceStatistics::writeFacesValues (
    ViewNumber::Enum viewNumber, const Foam::Bodies& bodies)
{
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT |
		  GL_ENABLE_BIT | GL_TEXTURE_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    
    glEnable (GL_STENCIL_TEST);
    glDisable (GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_1D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture (
	GL_TEXTURE_1D, 
	GetGLWidget ().GetViewSettings (viewNumber).GetColorBarTexture ());
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<
	      DisplayFaceBodyPropertyColor<
	      SetterValueVertexAttribute>, SetterValueVertexAttribute> (
		  GetGLWidget (), 
		  GetGLWidget ().GetViewSettings (viewNumber).
		  GetBodySelector (), 
		  SetterValueVertexAttribute (
		      GetGLWidget (), viewNumber, &m_storeShaderProgram,
		      m_storeShaderProgram.GetVValueIndex ()),
		  DisplayElement::INVISIBLE_CONTEXT));
    glPopAttrib ();
}

void DisplayFaceStatistics::glActiveTexture (GLenum texture) const
{
    const_cast<GLWidget&>(GetGLWidget ()).glActiveTexture (texture);
}

pair<double, double> DisplayFaceStatistics::getStatisticsMinMax (
    ViewNumber::Enum view) const
{
    double minValue, maxValue;
    if (GetGLWidget ().GetViewSettings (view).GetStatisticsType () == 
	StatisticsType::COUNT)
    {
	minValue = 0;
	maxValue = GetGLWidget ().GetFoamAlongTime ().GetTimeSteps ();
    }
    else
    {
	minValue = GetGLWidget ().GetFoamAlongTime ().GetMin (
	    GetGLWidget ().GetViewSettings (view).GetBodyProperty ());
	maxValue = GetGLWidget ().GetFoamAlongTime ().GetMax (
	    GetGLWidget ().GetViewSettings (view).GetBodyProperty ());
    }
    return pair<double, double> (minValue, maxValue);
}
