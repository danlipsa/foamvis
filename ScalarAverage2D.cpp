/**
 * @file   ScalarAverage2D.h
 * @author Dan R. Lipsa
 * @date  25 Jul. 2011
 *
 * Implementation for the ScalarAverage2D class 
 *
 */

#include "AverageShaders.h"
#include "Debug.h"
#include "WidgetGl.h"
#include "OpenGLUtils.h"
#include "ScalarAverage2D.h"
#include "ScalarDisplay.h"
#include "ShaderProgram.h"
#include "Simulation.h"
#include "Utils.h"


// Private classes/functions
// ======================================================================


// ScalarAverage2D Methods
// ======================================================================

template<typename PropertySetter>
boost::shared_ptr<ScalarDisplay> 
ScalarAverage2DTemplate<PropertySetter>::m_displayShaderProgram;

template<typename PropertySetter>
void ScalarAverage2DTemplate<PropertySetter>::InitShaders ()
{
    cdbg << "==== ScalarAverage2DTemplate ====" << endl;
    ImageBasedAverage<PropertySetter>::m_initShaderProgram.reset (
	new ShaderProgram (0, RESOURCE("ScalarInit.frag")));
    ImageBasedAverage<PropertySetter>::m_storeShaderProgram.reset (
	new StoreShaderProgram (
	    RESOURCE("ScalarStore.vert"), RESOURCE("ScalarStore.frag")));
    ImageBasedAverage<PropertySetter>::m_addShaderProgram.reset (
	new AddShaderProgram (RESOURCE("ScalarAdd.frag")));
    ImageBasedAverage<PropertySetter>::m_removeShaderProgram.reset (
	new AddShaderProgram (RESOURCE("ScalarRemove.frag")));
    ScalarAverage2DTemplate<PropertySetter>::m_displayShaderProgram.reset (
	new ScalarDisplay (RESOURCE("ScalarDisplay.frag")));
}

template<typename PropertySetter>
ScalarAverage2DTemplate<PropertySetter>::ScalarAverage2DTemplate (
    ViewNumber::Enum viewNumber, 
    const WidgetGl& widgetGl, string id, 
    QColor stepClearColor) :

    ImageBasedAverage<PropertySetter> (
        viewNumber, widgetGl, id, stepClearColor, this->m_fbos, 1)
{
}


template<typename PropertySetter>
void ScalarAverage2DTemplate<PropertySetter>::rotateAndDisplay (
    GLfloat minValue, GLfloat maxValue,
    StatisticsType::Enum displayType, CountType::Enum countType,
    typename ImageBasedAverage<PropertySetter>::FbosCountFbos srcFbo,
    ViewingVolumeOperation::Enum enclose,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    size_t timeWindow = this->GetCurrentTimeWindow ();
    timeWindow = (timeWindow == 0 ? 1 : timeWindow);
    m_displayShaderProgram->Bind (
        minValue, maxValue, displayType, countType, timeWindow);
    // activate texture unit 1 - scalar average
    this->glActiveTexture (
	TextureEnum (m_displayShaderProgram->GetScalarAverageTexUnit ()));
    glBindTexture (GL_TEXTURE_2D, srcFbo.m_countFbos->texture ());
    this->GetWidgetGl ().ActivateViewShader (
	this->GetViewNumber (), enclose,
	rotationCenter, angleDegrees);
    // activate texture unit 0
    this->glActiveTexture (GL_TEXTURE0);
    m_displayShaderProgram->release ();
}

template<typename PropertySetter>
vtkSmartPointer<vtkImageData> ScalarAverage2DTemplate<PropertySetter>::getData (
    const char* name) const
{
    G3D::Rect2D windowCoord = this->GetWindowCoord ();
    G3D::Rect2D objectCoord = gluUnProject (
        windowCoord, GluUnProjectZOperation::SET0);
    vtkSmartPointer<vtkFloatArray> scalar = 
        ImageBasedAverage<PropertySetter>::getData (
            this->m_fbos.m_current, windowCoord, GL_RED);
    scalar->SetName (name);
    
    vtkSmartPointer<vtkFloatArray> count = 
        ImageBasedAverage<PropertySetter>::getData (
            this->m_countFbos.m_current, windowCoord, GL_GREEN);
    // scalar / count
    for (vtkIdType i = 0; i < scalar->GetNumberOfTuples (); ++i)
    {
        float c = count->GetComponent (i, 0);
        if (c != 0)
            scalar->SetComponent (i, 0, scalar->GetComponent (i, 0) / c);
        else
            scalar->SetComponent (i, 0, 0);
    }
    int extent[6] = {0, static_cast<int>(windowCoord.width ()) - 1,
                     0, static_cast<int>(windowCoord.height ()) -1,
                     0, 0};
    vtkSmartPointer<vtkImageData> image = CreateRegularGrid (
        G3D::AABox (G3D::Vector3 (objectCoord.x0y0 (), 0),
                    G3D::Vector3 (objectCoord.x1y1 (), 0)), extent);
    image->GetPointData ()->SetScalars (scalar);
    image->GetPointData ()->SetActiveAttribute (
        name, vtkDataSetAttributes::SCALARS);
    return image;
}



// Template instantiations
//======================================================================
/// @cond
template class ScalarAverage2DTemplate<SetterVertexAttribute>;
template class ScalarAverage2DTemplate<SetterNop>;
/// @endcond
