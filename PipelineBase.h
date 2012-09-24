/**
 * @file   PipelineBase.h
 * @author Dan R. Lipsa
 * @date 4 Sept 2012
 * 
 * VTK pipeline for displaying 3D average
 * 
 */
#ifndef __PIPELINE_BASE_H__
#define __PIPELINE_BASE_H__

#include "Enums.h"

class PipelineBase
{
public:
    PipelineBase (size_t fontSize);

    vtkSmartPointer<vtkRenderer> GetRenderer () const
    {
        return m_renderer;
    }
    void UpdateScalarBar (
        vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction, 
        const char * name);
    void PositionScalarBar (G3D::Rect2D position);
    void UpdateViewTitle (
        const char* title, const G3D::Vector2& position);
    void UpdateFocus (bool focus);

protected:
    void createRenderer ();
    void createScalarBar ();
    void createViewTitle (size_t fontSize);
    void createFocusRect ();

private:
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkScalarBarActor> m_scalarBar;
    vtkSmartPointer<vtkActor2D> m_viewTitleActor;
    vtkSmartPointer<vtkActor2D> m_focusRectActor;
};


#endif //__PIPELINE_BASE_H__

// Local Variables:
// mode: c++
// End:
