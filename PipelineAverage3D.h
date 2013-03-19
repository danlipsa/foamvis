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

class ColorBarModel;
class ForceAverage;
class RegularGridAverage;
class ViewSettings;

class PipelineAverage3D : public PipelineBase
{
public:
    PipelineAverage3D (
        size_t objects, size_t constraintSurfaces, size_t fontSize);

    virtual void UpdateScalarColorBarModel (
        const ColorBarModel& colorBarModel, const char* name);
    virtual void UpdateVelocityColorBarModel (
        const ColorBarModel& colorBarModel);
    void UpdateThreshold (QwtDoubleInterval interval);
    void UpdateScalarAverage (const RegularGridAverage& average);
    void UpdateForceAverage (const ForceAverage& forceAverage);
    void UpdateVelocityAverage (const RegularGridAverage& velocity);
    void UpdateViewTitle (
        bool titleShown, const G3D::Vector2& postion,
        const string& simulationName, const string& viewTitle);
    virtual void FromView (ViewNumber::Enum viewNumber, const Base& base);

private:
    void createScalarAverageActor ();
    void createObjectActor (size_t objectCount);
    void createForceActor (size_t objectCount);
    void createConstraintSurfaceActor (size_t constraintSurfaceCount);
    void createVelocityGlyphActor ();

    void updateAlpha (
        float alpha, vector<vtkSmartPointer<vtkActor> >& actors);
    template <typename Iterator>
    void setPolyActors (Iterator begin, Iterator end);
    void updateForce (size_t objectIndex, ForceType::Enum forceType,
                      G3D::Vector3 force, G3D::Vector3 position, bool shown);
    void fromViewVelocityGlyph (ViewNumber::Enum viewNumber, const Base& base);

    
private:
    // scalar average
    vtkSmartPointer<vtkActor> m_scalarAverageActor;
    vtkSmartPointer<vtkThreshold> m_threshold;
    // constraint surfaces
    vector<vtkSmartPointer<vtkActor> > m_constraintSurface;
    // objects
    vector<vtkSmartPointer<vtkActor> > m_object;
    // for each object, 3 forces acting on it
    vector<boost::array<vtkSmartPointer<vtkActor>, 3> > m_forceActor;
    // velocity glyphs
    vtkSmartPointer<vtkPointSource> m_velocityGlyphSeeds;
    vtkSmartPointer<vtkProbeFilter> m_velocityGlyphProbe;
    vtkSmartPointer<vtkGlyph3D> m_velocityGlyph;
    vtkSmartPointer<vtkActor> m_velocityGlyphActor;
};


#endif //__PIPELINE_AVERAGE_3D_H__

// Local Variables:
// mode: c++
// End:
