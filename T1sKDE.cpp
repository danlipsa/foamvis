/**
 * @file   T1sKDE.h
 * @author Dan R. Lipsa
 * @date  31 Aug. 2011
 *
 * Implementation for the T1sKDE class 
 */

#include "AverageShaders.h"
#include "Debug.h"
#include "Simulation.h"
#include "WidgetGl.h"
#include "OpenGLUtils.h"
#include "ScalarDisplay.h"
#include "Settings.h"
#include "ShaderProgram.h"
#include "T1sKDE.h"
#include "Utils.h"
#include "ViewSettings.h"

// Private classes/functions
// ======================================================================

class GaussianInitShaderProgram : public ShaderProgram
{
public:
    GaussianInitShaderProgram (const char* frag);
    void Bind (float kernelSigma);
protected:
    float m_sigmaLocation;
};

GaussianInitShaderProgram::GaussianInitShaderProgram (const char* frag) :
    ShaderProgram (0, frag)
{
    m_sigmaLocation = uniformLocation("u_sigma");
    RuntimeAssert (m_sigmaLocation != -1, "Invalid location: u_sigma");
}

void GaussianInitShaderProgram::Bind (float sigma)
{
    ShaderProgram::Bind ();
    setUniformValue (m_sigmaLocation, sigma);
}


class GaussianStoreShaderProgram : public ShaderProgram
{
public:
    GaussianStoreShaderProgram (const char* frag);
    GLint GetGaussianTexUnit ()
    {
	return 1;
    }
    void Bind ();
protected:
    int m_gaussianTexUnitLocation;
};

GaussianStoreShaderProgram::GaussianStoreShaderProgram (const char* frag) :
    ShaderProgram (0, frag)
{
    m_gaussianTexUnitLocation = uniformLocation("u_gaussianTexUnit");
}

void GaussianStoreShaderProgram::Bind ()
{
    ShaderProgram::Bind ();
    setUniformValue (m_gaussianTexUnitLocation, GetGaussianTexUnit ());
}


// T1sKDE Methods and static fields
// ======================================================================

boost::shared_ptr<
    GaussianInitShaderProgram> T1sKDE::m_gaussianInitShaderProgram;
boost::shared_ptr<GaussianStoreShaderProgram
		  > T1sKDE::m_gaussianStoreShaderProgram;


void T1sKDE::InitShaders ()
{
    cdbg << "==== T1sKDE ====" << endl;
    m_initShaderProgram.reset (
	new ShaderProgram (0, RESOURCE("T1sKDEInit.frag")));
    m_addShaderProgram.reset (
	new AddShaderProgram (RESOURCE("ScalarAdd.frag")));
    m_removeShaderProgram.reset (
	new AddShaderProgram (RESOURCE("ScalarRemove.frag")));
    m_displayShaderProgram.reset (
	new ScalarDisplay (RESOURCE("ScalarDisplay.frag")));
    m_gaussianInitShaderProgram.reset (new GaussianInitShaderProgram
				       (RESOURCE ("GaussianInit.frag")));
    m_gaussianStoreShaderProgram.reset (new GaussianStoreShaderProgram
				       (RESOURCE ("GaussianStore.frag")));
}

const float s_kernelSigmaInBubbleDiameters = 3;

T1sKDE::T1sKDE (ViewNumber::Enum viewNumber, const WidgetGl& widgetGl) :
    ScalarAverageTemplate<SetterNop> (viewNumber, widgetGl, 
				      "t1sKDE", QColor (0, 255, 0, 0)),
    m_kernelSigma (s_kernelSigmaInBubbleDiameters * 
                   GetWidgetGl ().GetBubbleDiameter (viewNumber)),
    m_kernelTextureShown (false)
{
}

size_t T1sKDE::GetKernelTextureSize () const
{
    return m_kernelSigma / GetOnePixelInObjectSpace (GetSimulation ().Is2D ());
}


void T1sKDE::AverageInit ()
{
    WarnOnOpenGLError ("a - T1sKDE::AverageInit");
    ScalarAverageTemplate<SetterNop>::AverageInit ();
    initKernel ();
    WarnOnOpenGLError ("b - T1sKDE::AverageInit");
}


// Interactive Visualization of Streaming Data with Kernel Density Estimation
// Ove Daae Lampe and Helwig Hauser
// h: bandwidth is equal with standard deviation
void T1sKDE::initKernel ()
{
    float kernelTextureSize = GetKernelTextureSize ();
    QSize size (kernelTextureSize, kernelTextureSize);
    m_kernel.reset (
	new QGLFramebufferObject (
	    size, QGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, 
	    GL_RGBA32F));
    RuntimeAssert (m_kernel->isValid (), 
		   string ("Framebuffer initialization failed:") + GetId ());
    m_kernel->bind ();    
    m_gaussianInitShaderProgram->Bind (m_kernelSigma);
    ActivateShader (
	G3D::Rect2D (G3D::Vector2 (kernelTextureSize, kernelTextureSize)));
    m_gaussianInitShaderProgram->release ();
    m_kernel->release ();
}

void T1sKDE::SetKernelSigmaInBubbleDiameters (float kernelSigmaInBubbleDiameters)
{
    m_kernelSigma = kernelSigmaInBubbleDiameters * 
        GetWidgetGl ().GetBubbleDiameter (GetViewNumber ());
    initKernel ();
}

float T1sKDE::GetKernelSigmaInBubbleDiameters () const
{
    return m_kernelSigma / GetWidgetGl ().GetBubbleDiameter (GetViewNumber ());
}

void T1sKDE::writeStepValues (ViewNumber::Enum viewNumber, size_t timeStep, 
			      size_t subStep)
{
    WarnOnOpenGLError ("a - T1sKDE::writeStepValues");
    // activate texture unit 1
    glActiveTexture (
	TextureEnum (m_gaussianStoreShaderProgram->GetGaussianTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_kernel->texture ());
    m_gaussianStoreShaderProgram->Bind ();
    GetWidgetGl ().DisplayT1Quad (viewNumber, timeStep, subStep);
    m_gaussianStoreShaderProgram->release ();
    // activate texture unit 0
    glActiveTexture (GL_TEXTURE0);    
    WarnOnOpenGLError ("b - T1sKDE::writeStepValues");
}

void T1sKDE::DisplayTextureSize (ViewNumber::Enum viewNumber, size_t timeStep, 
				 size_t subStep) const
{
    glPushAttrib (GL_CURRENT_BIT | GL_POLYGON_BIT);
    glColor (GetSettings ().GetHighlightColor (
                 viewNumber, HighlightNumber::H0));
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    GetWidgetGl ().DisplayT1Quad (viewNumber, timeStep, subStep);
    glPopAttrib ();
}

size_t T1sKDE::getStepSize (size_t timeStep) const
{
    ViewSettings& vs = GetSettings ().GetViewSettings (GetViewNumber ());
    return GetSimulation ().GetT1 (timeStep, 
				    vs.T1sShiftLower ()).size ();
}

float T1sKDE::GetPeakHeight () const
{
    //return 1 / (m_kernelSigma * m_kernelSigma * 2 * M_PI);
    return 1;
}

void T1sKDE::CacheData (boost::shared_ptr<AverageCache> averageCache) const
{
    vtkSmartPointer<vtkImageData> data = getData (GetId ());
    averageCache->SetT1KDE (data);
}
