/**
 * @file   ScalarAverage.h
 * @author Dan R. Lipsa
 * @date  25 Jul. 2011
 *
 * Implementation for the ScalarAverage class 
 *
 */

#include "AverageShaders.h"
#include "DebugStream.h"
#include "WidgetGl.h"
#include "OpenGLUtils.h"
#include "ShaderProgram.h"
#include "Simulation.h"
#include "VectorAverage.h"
#include "Utils.h"
#include "ViewSettings.h"

VectorAverage::VectorAverage (ViewNumber::Enum viewNumber, 
			      const WidgetGl& widgetGl, 
			      FramebufferObjects& countFbos) :
    TensorAverageTemplate<SetterVelocity> (
	viewNumber, widgetGl,
	&WidgetGl::GetVelocitySizeInitialRatio,
	&ViewSettings::GetVelocityClampingRatio,
	&ViewSettings::GetVelocityLineWidth,
	countFbos)
{
}

void VectorAverage::InitShaders ()
{
    cdbg << "==== VectorAverage ====" << endl;
    m_initShaderProgram.reset (
	new ShaderProgram (0, RESOURCE("TensorInit.frag")));
    m_storeShaderProgram.reset (
	new StoreShaderProgram (
	    RESOURCE("VectorStore.vert"), RESOURCE("TensorStore.frag")));
    m_addShaderProgram.reset (
	new AddShaderProgram (RESOURCE("TensorAdd.frag")));
    m_removeShaderProgram.reset (
	new AddShaderProgram (RESOURCE("TensorRemove.frag")));
    m_displayShaderProgram.reset (
	new TensorDisplay (RESOURCE("TensorDisplay.vert"),
			   RESOURCE("VectorDisplay.frag")));
}

vtkSmartPointer<vtkImageData> VectorAverage::getData (
    const G3D::Rect2D& oc) const
{
    G3D::Rect2D windowCoord = rectInside (gluProject (oc));
    G3D::Rect2D objectCoord = gluUnProject (
        windowCoord, GluUnProjectZOperation::SET0);
    BodyAttribute::Enum attribute = BodyAttribute::VELOCITY;

    // opengl gets 3 componens from the hardware
    vtkSmartPointer<vtkFloatArray> velocity = 
        ImageBasedAverage<SetterVelocity>::getData (
            this->m_fbos.m_current, windowCoord, GL_RGB);
    velocity->SetName (BodyAttribute::ToString (attribute));    
    vtkSmartPointer<vtkFloatArray> count = 
        ImageBasedAverage<SetterVelocity>::getData (
            this->m_countFbos.m_current, windowCoord, GL_GREEN);

    // vector / count
    for (vtkIdType i = 0; i < velocity->GetNumberOfTuples (); ++i)
    {
        float c = count->GetComponent (i, 0);
        if (c != 0)
        {
            velocity->SetComponent (i, 0, 
                                    velocity->GetComponent (i, 0) / c);
            velocity->SetComponent (i, 1, 
                                    velocity->GetComponent (i, 1) / c);
        }
        else
        {
            velocity->SetComponent (i, 0, 0);
            velocity->SetComponent (i, 1, 0);
        }
        velocity->SetComponent (i, 2, 0);
    }
    int extent[6] = {0, windowCoord.width () - 1,
                     0, windowCoord.height () -1,
                     0, 0};
    vtkSmartPointer<vtkImageData> image = CreateRegularGridNoAttributes (
        G3D::AABox (G3D::Vector3 (objectCoord.x0y0 (), 0),
                    G3D::Vector3 (objectCoord.x1y1 (), 0)), extent);
    image->GetPointData ()->SetVectors (velocity);
    image->GetPointData ()->SetActiveAttribute (
	BodyAttribute::ToString (attribute), BodyAttribute::GetType (attribute));
    return image;
}

void VectorAverage::CacheData (
    AverageCache* averageCache, const G3D::Rect2D& objectCoord) const
{
    vtkSmartPointer<vtkImageData> data = getData (objectCoord);
    averageCache->SetVelocity (data);
}
