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
        size_t objects, bool hasForce,
        size_t constraintSurfaces, size_t fontSize);

    virtual void UpdateColorMapScalar (const ColorBarModel& colorMapScalar);
    virtual void UpdateColorMapVelocity (const ColorBarModel& colorMapVelocity);
    void UpdateThresholdScalar (QwtDoubleInterval interval, 
                                size_t scalar);
    void UpdateAverageScalar (const RegularGridAverage& average);
    void UpdateAverageForce (const ForceAverage& forceAverage);
    void UpdateAverageVelocity (const RegularGridAverage& velocity);
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
    void createOutlineSimulationActor ();
    void createOutlineTorusActor ();

    void updateAlpha (
        float alpha, vector<vtkSmartPointer<vtkActor> >& actors);
    template <typename Iterator>
    void setPolyActors (Iterator begin, Iterator end);
    void updateForce (size_t objectIndex, ForceType::Enum forceType,
                      G3D::Vector3 force, G3D::Vector3 position, bool shown);
    void fromViewVelocityGlyph (ViewNumber::Enum viewNumber, const Base& base);
    void fromViewScalar (ViewNumber::Enum viewNumber, const Base& base);
    void updateContourColor ();

    
private:
    // scalar average
    vtkSmartPointer<vtkActor> m_scalarAverageActor;
    vtkSmartPointer<vtkThreshold> m_scalarThreshold;
    vtkSmartPointer<vtkContourFilter> m_scalarContour;

    // constraint surfaces
    vector<vtkSmartPointer<vtkActor> > m_constraintSurface;
    // objects
    vector<vtkSmartPointer<vtkActor> > m_object;
    // for each object, 3 forces acting on it
    vector<boost::array<vtkSmartPointer<vtkActor>, 3> > m_forceActor;
    // velocity glyphs
    vtkSmartPointer<vtkPointSource> m_velocityGlyphSeeds;
    vtkSmartPointer<vtkThreshold> m_velocityGlyphThresholdOutsideCylinder;
    vtkSmartPointer<vtkThresholdPoints> m_velocityGlyphThresholdOutsideBB;
    vtkSmartPointer<vtkThresholdPoints> m_velocityGlyphThresholdNorm;
    vtkSmartPointer<vtkGlyph3D> m_velocityGlyph;
    vtkSmartPointer<vtkActor> m_velocityGlyphActor;
    // outlines
    vtkSmartPointer<vtkOutlineFilter> m_outlineSimulation;
    vtkSmartPointer<vtkActor> m_outlineSimulationActor;
    vtkSmartPointer<vtkOutlineSource> m_outlineTorus;
    vtkSmartPointer<vtkActor> m_outlineTorusActor;
};


#endif //__PIPELINE_AVERAGE_3D_H__

// Local Variables:
// mode: c++
// End:
