/**
 * @file   T1KDE2D.h
 * @author Dan R. Lipsa
 * @date  31 Aug. 2011
 *
 * Implementation for the T1KDE2D class 
 */

#include "AverageShaders.h"
#include "Debug.h"
#include "Simulation.h"
#include "WidgetGl.h"
#include "OpenGLUtils.h"
#include "ScalarDisplay.h"
#include "Settings.h"
#include "ShaderProgram.h"
#include "T1KDE2D.h"
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


// T1KDE2D Methods and static fields
// ======================================================================

boost::shared_ptr<
    GaussianInitShaderProgram> T1KDE2D::m_gaussianInitShaderProgram;
boost::shared_ptr<GaussianStoreShaderProgram
		  > T1KDE2D::m_gaussianStoreShaderProgram;


void T1KDE2D::InitShaders ()
{
    cdbg << "==== T1KDE2D ====" << endl;
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

T1KDE2D::T1KDE2D (ViewNumber::Enum viewNumber, const WidgetGl& widgetGl) :
    ScalarAverage2DTemplate<SetterNop> (viewNumber, widgetGl, 
                                        "t1sKDE", QColor (0, 255, 0, 0))
{
}

float T1KDE2D::getKernelSigma () const
{
    const ViewSettings& vs = GetViewSettings ();
    return vs.GetT1KDESigmaInBubbleDiameter () * 
        GetBubbleDiameter (GetViewNumber ());
}

size_t T1KDE2D::GetKernelTextureSize () const
{
    const ViewSettings& vs = GetViewSettings ();
    // WARNING: has to be the same as in GaussianInit.frag
    const float STDDEV_COUNT = 5.0;
    // we want our texture to cover 1 sigma
    return STDDEV_COUNT * getKernelSigma () / vs.GetOnePixelInObjectSpace ();
}


void T1KDE2D::AverageInit ()
{
    WarnOnOpenGLError ("a - T1KDE2D::AverageInit");
    ScalarAverage2DTemplate<SetterNop>::AverageInit ();
    InitKernel ();
    WarnOnOpenGLError ("b - T1KDE2D::AverageInit");
}


// Interactive Visualization of Streaming Data with Kernel Density Estimation
// Ove Daae Lampe and Helwig Hauser
// h: bandwidth is equal with standard deviation
void T1KDE2D::InitKernel ()
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
    m_gaussianInitShaderProgram->Bind (getKernelSigma ());
    ActivateShader (
	G3D::Rect2D (G3D::Vector2 (kernelTextureSize, kernelTextureSize)));
    m_gaussianInitShaderProgram->release ();
    m_kernel->release ();
}

void T1KDE2D::writeStepValues (ViewNumber::Enum viewNumber, size_t timeStep, 
			      size_t subStep)
{
    // activate texture unit 1
    glActiveTexture (
	TextureEnum (m_gaussianStoreShaderProgram->GetGaussianTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, m_kernel->texture ());
    m_gaussianStoreShaderProgram->Bind ();
    GetWidgetGl ().DisplayT1Quad (viewNumber, timeStep, subStep);
    m_gaussianStoreShaderProgram->release ();
    // activate texture unit 0
    glActiveTexture (GL_TEXTURE0);    
}

void T1KDE2D::DisplayTextureSize (ViewNumber::Enum viewNumber, size_t timeStep, 
				 size_t subStep) const
{
    glPushAttrib (GL_CURRENT_BIT | GL_POLYGON_BIT);
    glColor (GetSettings ().GetHighlightColor (
                 viewNumber, HighlightNumber::H0));
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    GetWidgetGl ().DisplayT1Quad (viewNumber, timeStep, subStep);
    glPopAttrib ();
}

size_t T1KDE2D::getStepSize (size_t timeStep) const
{
    ViewSettings& vs = GetSettings ().GetViewSettings (GetViewNumber ());
    return GetSimulation ().GetT1 (timeStep, 
				    vs.T1sShiftLower ()).size ();
}

void T1KDE2D::CacheData (boost::shared_ptr<AverageCache> averageCache) const
{
    vtkSmartPointer<vtkImageData> data = getData (GetId ());
    averageCache->SetT1KDE (data);
}
