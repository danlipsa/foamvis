/**
 * @file   Average3dPipeline.h
 * @author Dan R. Lipsa
 * @date 4 Sept 2012
 * 
 * VTK pipeline for displaying 3D average
 * 
 */
#ifndef __AVERAGE_PIPELINE_3D_H__
#define __AVERAGE_PIPELINE_3D_H__

#include "Enums.h"

class ViewSettings;
class Foam;
class RegularGridAverage;

class Average3dPipeline
{
public:
    Average3dPipeline (
        size_t objects, size_t constraintSurfaces, size_t fontSize);
    void UpdateThreshold (QwtDoubleInterval interval);
    void UpdateColorTransferFunction (
        vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction,
        const char* name);
    void PositionScalarBar (G3D::Rect2D position);
    void UpdateOpacity (float contextAlpha);
    void ViewToVtk (const ViewSettings& vs, 
                    G3D::Vector3 simulationCenter, const Foam& foam);
    void VtkToView (ViewSettings& vs, const Foam& foam);

    void UpdateAverage (
        boost::shared_ptr<RegularGridAverage> average, int direction);
    void UpdateViewTitle (
        bool titleShown, const G3D::Vector2& postion,
        boost::shared_ptr<RegularGridAverage> average, 
        ViewNumber::Enum viewNumber);
    void UpdateFocus (bool focus);
    vtkSmartPointer<vtkRenderer> GetRenderer () const
    {
        return m_renderer;
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

    vtkSmartPointer<vtkActor> m_averageActor;
    vtkSmartPointer<vtkThreshold> m_threshold;
    vector<vtkSmartPointer<vtkActor> > m_constraintSurface;
    vector<vtkSmartPointer<vtkActor> > m_object;
};


#endif //__AVERAGE_PIPELINE_3D_H__

// Local Variables:
// mode: c++
// End:
