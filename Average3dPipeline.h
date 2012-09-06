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

struct Average3dPipeline
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
    void UpdateTitle (
        bool titleShown, const G3D::Vector2& postion,
        boost::shared_ptr<RegularGridAverage> average, 
        ViewNumber::Enum viewNumber);
    void UpdateFocus (bool focus);
    vtkSmartPointer<vtkRenderer> GetRenderer () const
    {
        return m_renderer;
    }

private:
    vtkSmartPointer<vtkScalarBarActor> m_scalarBar;
    vtkSmartPointer<vtkActor> m_averageActor;
    vtkSmartPointer<vtkThreshold> m_threshold;
    vtkSmartPointer<vtkActor2D> m_textActor;
    vector<vtkSmartPointer<vtkActor> > m_constraintSurface;
    vector<vtkSmartPointer<vtkActor> > m_object;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkActor2D> m_focusActor;
};


#endif //__AVERAGE_PIPELINE_3D_H__

// Local Variables:
// mode: c++
// End:
