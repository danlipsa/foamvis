/**
 * @file   ScalarAverage.h
 * @author Dan R. Lipsa
 * @date  25 Jul. 2011
 *
 * Implementation for the ScalarAverage class 
 *
 */

#include "AverageCacheT1KDEVelocity.h"
#include "AverageShaders.h"
#include "Debug.h"
#include "OpenGLUtils.h"
#include "ShaderProgram.h"
#include "Simulation.h"
#include "Utils.h"
#include "VectorAverage.h"
#include "ViewSettings.h"
#include "WidgetGl.h"

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

VectorAverage::VectorAverage (ViewNumber::Enum viewNumber, 
			      const WidgetGl& widgetGl) :

    TensorAverageTemplate<SetterVelocity> (
	viewNumber, widgetGl, AverageType::VECTOR,
	&WidgetGl::GetVelocitySizeInitialRatio,
	&ViewSettings::GetVelocityInverseClampMaxRatio,
	&ViewSettings::GetVelocityLineWidth, this->m_fbos, 2)
{
}

vtkSmartPointer<vtkImageData> VectorAverage::getData () const
{
    G3D::Rect2D windowCoord = GetWindowCoord ();
    G3D::Rect2D objectCoord = gluUnProject (
        windowCoord, GluUnProjectZOperation::SET0);
    BodyAttribute::Enum attribute = BodyAttribute::VELOCITY;

    // read (x, y, count) from opengl
    vtkSmartPointer<vtkFloatArray> velocity = 
        ImageBasedAverage<SetterVelocity>::getData (
            this->m_fbos.m_current, windowCoord, GL_RGB);
    velocity->SetName (BodyAttribute::ToString (attribute));    

    // vector / count
    for (vtkIdType i = 0; i < velocity->GetNumberOfTuples (); ++i)
    {
        float c = velocity->GetComponent (i, m_countIndex);
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
        velocity->SetComponent (i, m_countIndex, 0);
    }
    int extent[6] = {0, windowCoord.width () - 1,
                     0, windowCoord.height () -1,
                     0, 0};
    __LOG__ (save (velocity, windowCoord, 3, 
                   GetWidgetGl ().GetInterval (GetAverageType ()).maxValue ()););
    vtkSmartPointer<vtkImageData> image = CreateRegularGrid (
        G3D::AABox (G3D::Vector3 (objectCoord.x0y0 (), 0),
                    G3D::Vector3 (objectCoord.x1y1 (), 0)), extent);
    image->GetPointData ()->SetVectors (velocity);
    image->GetPointData ()->SetActiveAttribute (
	BodyAttribute::ToString (attribute), 
        BodyAttribute::GetType (attribute));
    return image;
}

void VectorAverage::CacheData (
    boost::shared_ptr<AverageCacheT1KDEVelocity> averageCache) const
{
    vtkSmartPointer<vtkImageData> data = getData ();
    averageCache->SetVelocity (data);
}
