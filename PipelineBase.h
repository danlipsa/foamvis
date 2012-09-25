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
class ViewSettings;
class Foam;


class PipelineBase
{
public:
    PipelineBase (size_t fontSize, PipelineType::Enum type);

    vtkSmartPointer<vtkRenderer> GetRenderer () const
    {
        return m_renderer;
    }
    virtual void UpdateColorTransferFunction (
        vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction, 
        const char * name);
    void PositionScalarBar (G3D::Rect2D position);
    void UpdateViewTitle (
        const char* title, const G3D::Vector2& position);
    void UpdateFocus (bool focus);
    void ViewToVtk (const ViewSettings& vs, 
                    G3D::Vector3 simulationCenter, const Foam& foam);
    void VtkToView (ViewSettings& vs, const Foam& foam);
    PipelineType::Enum GetType () const
    {
        return m_type;
    }


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
    PipelineType::Enum m_type;
};


#endif //__PIPELINE_BASE_H__

// Local Variables:
// mode: c++
// End:
