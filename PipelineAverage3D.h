/**
 * @file   PipelineAverage3D.h
 * @author Dan R. Lipsa
 * @date 4 Sept 2012
 * 
 * VTK pipeline for displaying 3D average
 * 
 */
#ifndef __PIPELINE_AVERAGE_3D_H__
#define __PIPELINE_AVERAGE_3D_H__

#include "Enums.h"
#include "PipelineBase.h"

class RegularGridAverage;

class PipelineAverage3D : public PipelineBase
{
public:
    PipelineAverage3D (
        size_t objects, size_t constraintSurfaces, size_t fontSize);

    virtual void UpdateColorTransferFunction (
        vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction,
        const char* name);

    void UpdateThreshold (QwtDoubleInterval interval);
    void UpdateOpacity (float contextAlpha);
    void UpdateAverage (boost::shared_ptr<RegularGridAverage> average);
    void UpdateViewTitle (
        bool titleShown, const G3D::Vector2& postion,
        const string& simulationName, const string& viewTitle);

private:
    vtkSmartPointer<vtkActor> m_averageActor;
    vtkSmartPointer<vtkThreshold> m_threshold;
    vector<vtkSmartPointer<vtkActor> > m_constraintSurface;
    vector<vtkSmartPointer<vtkActor> > m_object;
};


#endif //__PIPELINE_AVERAGE_3D_H__

// Local Variables:
// mode: c++
// End:
