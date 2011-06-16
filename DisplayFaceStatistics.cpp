/**
 * @file   DisplayFaceStatistics.h
 * @author Dan R. Lipsa
 * @date  24 Oct. 2010
 *
 * Implementation for the DisplayFaceStatistics class 
 *
 * @todo: Fix average value for not available values. Now not
 * available values are stored as the lowest value.
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
    clear (viewRect);
}

void DisplayFaceStatistics::clear (const G3D::Rect2D& viewRect)
{
    m_step->bind ();ClearColorStencilBuffers (Qt::black, 0);
    m_step->release ();

    m_new->bind ();ClearColorBuffer (Qt::black);
    m_new->release ();
    clearColorBufferMinMax (viewRect, m_old);
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
    addStepToNew (viewRect);
    //save (viewRect, *m_new, "new", timeStep,
    //minMax.first, minMax.second, StatisticsType::AVERAGE);
    copyNewToOld ();
    //save (viewRect, *m_old, "old", timeStep, 
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
    removeStepFromNew (viewRect);
    //save (viewRect, *m_new, "new_", timeStep,
    //minMax.first, minMax.second, StatisticsType::AVERAGE);
    copyNewToOld ();
    //save (viewRect, *m_old, "old_", timeStep, 
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

void DisplayFaceStatistics::addStepToNew (const G3D::Rect2D& viewRect)
{
    m_new->bind ();
    m_addShaderProgram.Bind ();

    // bind old texture
    glActiveTexture (TextureEnum (m_addShaderProgram.GetOldTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_old->texture ());

    // bind step texture
    glActiveTexture (TextureEnum (m_addShaderProgram.GetStepTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_step->texture ());
    // set the active texture to texture 0
    glActiveTexture (GL_TEXTURE0);

    RenderFromFbo (
	G3D::Rect2D::xywh (0, 0, viewRect.width (), viewRect.height ()), 
	*m_step);
    m_addShaderProgram.release ();
    m_new->release ();
}


void DisplayFaceStatistics::removeStepFromNew (const G3D::Rect2D& viewRect)
{
    m_new->bind ();
    m_removeShaderProgram.Bind ();

    // bind old texture
    glActiveTexture (TextureEnum (m_removeShaderProgram.GetOldTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_old->texture ());

    // bind step texture
    glActiveTexture (TextureEnum (m_removeShaderProgram.GetStepTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_step->texture ());
    // set the active texture to texture 0
    glActiveTexture (GL_TEXTURE0);

    RenderFromFbo (
	G3D::Rect2D::xywh (0, 0, viewRect.width (), viewRect.height ()),
	*m_step);
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
    if (m_new.get () != 0)
    {
	pair<double, double> minMax = getStatisticsMinMax (viewNumber);
	const G3D::Rect2D viewRect = GetGLWidget ().GetViewRect (viewNumber);
	display (viewRect, minMax.first, minMax.second, displayType, *m_new);
    }
}

void DisplayFaceStatistics::DisplayAndRotate (
    ViewNumber::Enum viewNumber,
    StatisticsType::Enum displayType, 
    G3D::Vector2 rotationCenter, float angleDegrees)
{
    if (m_new.get () != 0)
    {
	pair<double, double> minMax = getStatisticsMinMax (viewNumber);
	const G3D::Rect2D viewRect = GetGLWidget ().GetViewRect (viewNumber);
	displayAndRotate (
	    viewRect, minMax.first, minMax.second, displayType, *m_new,
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
		  GetGLWidget (), GetGLWidget ().GetBodySelector (), 
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
