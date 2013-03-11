/**
 * @file   PipelineBase.h
 * @author Dan R. Lipsa
 * @date 24 Sept 2012
 * 
 * VTK pipeline
 * 
 */
#ifndef __PIPELINE_BASE_H__
#define __PIPELINE_BASE_H__

#include "Enums.h"
class Base;
class ViewSettings;
class Foam;

/**
 * Provides basic functionality for a view display
 */
class PipelineBase
{
public:
    PipelineBase (size_t fontSize, PipelineType::Enum type);

    vtkSmartPointer<vtkRenderer> GetRenderer () const
    {
        return m_renderer;
    }
    virtual void UpdateColorBarModel (
        vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction, 
        const char * name);
    void PositionScalarBar (G3D::Rect2D position);
    void UpdateViewTitle (
        const char* title, const G3D::Vector2& position);
    void UpdateFocus (bool focus);
    void FromViewTransform (ViewNumber::Enum viewNumber, const Base& base);
    void ToViewTransform (ViewNumber::Enum viewNumber, Base* base) const;
    PipelineType::Enum GetType () const
    {
        return m_type;
    }


protected:
    void createRenderer ();
    void createScalarBarActor ();
    void createViewTitleActor (size_t fontSize);
    void createFocusRectActor ();

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
