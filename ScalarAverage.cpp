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
#include "ScalarAverage.h"
#include "ScalarDisplay.h"
#include "ShaderProgram.h"
#include "Simulation.h"
#include "Utils.h"


// Private classes/functions
// ======================================================================


// ScalarAverage Methods
// ======================================================================

template<typename PropertySetter>
boost::shared_ptr<ScalarDisplay> 
ScalarAverageTemplate<PropertySetter>::m_displayShaderProgram;

template<typename PropertySetter>
void ScalarAverageTemplate<PropertySetter>::InitShaders ()
{
    cdbg << "==== ScalarAverageTemplate ====" << endl;
    ImageBasedAverage<PropertySetter>::m_initShaderProgram.reset (
	new ShaderProgram (0, RESOURCE("ScalarInit.frag")));
    ImageBasedAverage<PropertySetter>::m_storeShaderProgram.reset (
	new StoreShaderProgram (
	    RESOURCE("ScalarStore.vert"), RESOURCE("ScalarStore.frag")));
    ImageBasedAverage<PropertySetter>::m_addShaderProgram.reset (
	new AddShaderProgram (RESOURCE("ScalarAdd.frag")));
    ImageBasedAverage<PropertySetter>::m_removeShaderProgram.reset (
	new AddShaderProgram (RESOURCE("ScalarRemove.frag")));
    ScalarAverageTemplate<PropertySetter>::m_displayShaderProgram.reset (
	new ScalarDisplay (RESOURCE("ScalarDisplay.frag")));
}

template<typename PropertySetter>
void ScalarAverageTemplate<PropertySetter>::rotateAndDisplay (
    GLfloat minValue, GLfloat maxValue,
    StatisticsType::Enum displayType, 
    typename ImageBasedAverage<PropertySetter>::FbosCountFbos srcFbo,
    ViewingVolumeOperation::Enum enclose,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    m_displayShaderProgram->Bind (minValue, maxValue, displayType);
    // activate texture unit 1 - scalar average
    this->glActiveTexture (
	TextureEnum (m_displayShaderProgram->GetScalarAverageTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, srcFbo.second->texture ());
    this->GetWidgetGl ().ActivateViewShader (
	this->GetViewNumber (), enclose,
	rotationCenter, angleDegrees);
    // activate texture unit 0
    this->glActiveTexture (GL_TEXTURE0);
    m_displayShaderProgram->release ();
}

template<typename PropertySetter>
vtkSmartPointer<vtkImageData> ScalarAverageTemplate<PropertySetter>::getData (
    const G3D::Rect2D& oc, BodyScalar::Enum property) const
{
    G3D::Rect2D windowCoord = rectInside (gluProject (oc));
    G3D::Rect2D objectCoord = gluUnProject (
        windowCoord, GluUnProjectZOperation::SET0);

    vtkSmartPointer<vtkFloatArray> scalar = 
        ImageBasedAverage<PropertySetter>::getData (
            this->m_fbos.m_current, windowCoord, GL_RED);
    scalar->SetName (BodyScalar::ToString (property));
    
    vtkSmartPointer<vtkFloatArray> count = 
        ImageBasedAverage<PropertySetter>::getData (
            this->m_countFbos.m_current, windowCoord, GL_GREEN);
    // scalar / count
    for (vtkIdType i = 0; i < scalar->GetNumberOfTuples (); ++i)
    {
        float c = count->GetComponent (i, 0);
        if (c != 0)
            scalar->SetComponent (i, 0, scalar->GetComponent (i, 0) / c);
    }
    int extent[6] = {0, windowCoord.width () - 1,
                     0, windowCoord.height () -1,
                     0, 0};
    vtkSmartPointer<vtkImageData> scalarImage = CreateRegularGridNoAttributes (
        G3D::AABox (G3D::Vector3 (objectCoord.x0y0 (), 0),
                    G3D::Vector3 (objectCoord.x1y1 (), 0)), extent);
    scalarImage->GetPointData ()->AddArray (scalar);
    return scalarImage;
}

template<typename PropertySetter>
void ScalarAverageTemplate<PropertySetter>::CacheData (
    AverageCache* averageCache,
    const G3D::Rect2D& objectCoord, BodyScalar::Enum property) const
{
    vtkSmartPointer<vtkImageData> data = getData (objectCoord, property);
    averageCache->SetScalar (property, data);
}



// Template instantiations
//======================================================================
/// @cond
template class ScalarAverageTemplate<SetterVertexAttribute>;
template class ScalarAverageTemplate<SetterNop>;
/// @endcond
