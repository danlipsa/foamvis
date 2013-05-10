/**
 * @file   WidgetVtk.cpp
 * @author Dan R. Lipsa
 * @date 13 June 2012
 * 
 * Definitions for the widget for displaying foam bubbles using Vtk
 */

#include "PipelineAverage3D.h"
#include "Body.h"
#include "ColorBarModel.h"
#include "Debug.h"
#include "Foam.h"
#include "FoamvisInteractorStyle.h"
#include "OpenGLUtils.h"
#include "RegularGridAverage.h"
#include "Settings.h"
#include "Simulation.h"
#include "AttributeAverages3D.h"
#include "ViewSettings.h"
#include "WidgetVtk.h"


// Private Classes/Functions
// ======================================================================

class RenderWindowPaintEnd : public vtkCommand
{
public:
    RenderWindowPaintEnd (WidgetVtk* widgetVtk) :
	m_widgetVtk (widgetVtk)
    {
    }
    void Execute (vtkObject *caller, unsigned long eventId, void *callData);
private:
    WidgetVtk* m_widgetVtk;
};


void RenderWindowPaintEnd::Execute (
    vtkObject *caller, unsigned long eventId, void *callData)
{
    (void) caller;(void)callData;(void)eventId;
    m_widgetVtk->ToViewTransform ();
    m_widgetVtk->SendPaintEnd ();
}

using G3D::Rect2D;

Rect2D toRatio (const Rect2D& barRect, const Rect2D& viewRect)
{
    return G3D::Rect2D::xywh (
        (barRect.x0 () - viewRect.x0 ())/ viewRect.width (),
        (barRect.y0 () - viewRect.y0 ())/ viewRect.height (),
        barRect.width () / viewRect.width (),
        barRect.height () / viewRect.height ());
}


// Methods WidgetVtk
// ======================================================================

WidgetVtk::WidgetVtk (QWidget* parent) :
    QVTKWidget (parent),
    WidgetBase (this, &WidgetBase::IsVtkView, &WidgetBase::GetVtkCount),
    m_fontSize (10)
{
    setVisible (false);
    createActions ();
}

void WidgetVtk::createActions ()
{
    MAKE_COMMON_CONNECTIONS;
}


// ======================================================================
// PipelineBase

void WidgetVtk::UpdateFocus ()
{
    WidgetBase::ForAllViews (
	boost::bind (&WidgetVtk::updateViewFocus, this, _1));
    update ();
}
void WidgetVtk::updateViewFocus (ViewNumber::Enum viewNumber)
{
    bool focus = (GetViewNumber () == viewNumber) && 
	GetSettings ().IsViewFocusShown ();
    PipelineBase& pipeline = *m_pipeline[viewNumber];
    pipeline.UpdateFocus (focus);
}

void WidgetVtk::FromViewTransform (ViewNumber::Enum viewNumber)
{
    m_pipeline[viewNumber]->FromViewTransform (viewNumber, *this);
    update ();
}

void WidgetVtk::ToViewTransform (ViewNumber::Enum viewNumber)
{
    // This may be called by RenderWindowPaintEnd and may arrive after the 
    // view was switched to a Gl view
    if (IsVtkView (viewNumber))
    {
        PipelineBase& pipeline = *m_pipeline[viewNumber];
        pipeline.ToViewTransform (viewNumber, this);
    }
}

void WidgetVtk::RemoveViews ()
{
    vtkSmartPointer<vtkRenderWindow> renderWindow = GetRenderWindow ();
    WidgetBase::ForAllViews (boost::bind (&WidgetVtk::removeView, this, _1));
    setVisible (false);
}
void WidgetVtk::removeView (ViewNumber::Enum viewNumber)
{
    if (m_pipeline[viewNumber])
    {
        vtkSmartPointer<vtkRenderWindow> renderWindow = GetRenderWindow ();
        renderWindow->RemoveRenderer (m_pipeline[viewNumber]->GetRenderer ());
        m_pipeline[viewNumber].reset ();
    }
}


// ======================================================================
// PipelineAverage3D
void WidgetVtk::SendPaintEnd ()
{
    Q_EMIT PaintEnd ();
}

void WidgetVtk::Init (boost::shared_ptr<Settings> settings,
                      boost::shared_ptr<const SimulationGroup> simulationGroup)
{
    WidgetBase::Init (settings, simulationGroup, 0);
    // interactor style
    VTK_CREATE (FoamvisInteractorStyle, interactorStyle);
    interactorStyle->SetSettings (GetSettingsPtr ());
    
    QVTKInteractor *interactor = GetInteractor();
    interactor->SetInteractorStyle (interactorStyle);
    
    for (size_t i = 0; i < m_average.size (); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	m_average[i].reset (new AttributeAverages3D (
				viewNumber, settings, simulationGroup));
    }
}

void WidgetVtk::Average3dCreatePipeline (
    size_t objects, size_t constraintSurfaces, size_t fontSize)
{
    m_fontSize = fontSize + 4;
    vtkRenderWindow* renWin = GetRenderWindow ();

    for (size_t i = 0; i < ViewNumber::COUNT; ++i)
    {
        bool hasForce = GetFoam ().GetForces ().size () == objects;
	m_pipelineAverage3d[i].reset (
            new PipelineAverage3D (
                objects, hasForce,
                constraintSurfaces, fontSize));
    }

    vtkSmartPointer<RenderWindowPaintEnd> sendPaint (
        new RenderWindowPaintEnd (this));
    renWin->AddObserver (vtkCommand::EndEvent, sendPaint);
}

void WidgetVtk::UpdateThresholdScalar (QwtDoubleInterval interval)
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    if (GetPipelineType (viewNumber) == PipelineType::AVERAGE_3D)
    {
        const ViewSettings& vs = GetViewSettings (viewNumber);
        m_pipelineAverage3d[viewNumber]->UpdateThresholdScalar (
            interval, 
            vs.GetBodyOrOtherScalar () );
        update ();
    }
}


void WidgetVtk::FromView (ViewNumber::Enum viewNumber)
{
    if (! IsVtkView (viewNumber))
        return;
    if (GetPipelineType (viewNumber) == PipelineType::AVERAGE_3D)
    {
        m_pipelineAverage3d[viewNumber]->FromView (viewNumber, *this);
        resizeViewEvent (viewNumber);
        update ();
    }
}


void WidgetVtk::UpdateAverage3dTitle ()
{
    ForAllViews (PipelineType::AVERAGE_3D,
	boost::bind (&WidgetVtk::updateViewTitle, this, _1));
    update ();
}
void WidgetVtk::updateViewTitle (ViewNumber::Enum viewNumber)
{
    bool titleShown = GetSettings ().IsTitleShown ();
    const ViewSettings& vs = GetViewSettings (viewNumber);
    PipelineAverage3D& pipeline = *m_pipelineAverage3d[viewNumber];
    const Simulation& simulation = m_average[viewNumber]->GetSimulation ();
    G3D::Rect2D viewRect = GetNormalizedViewRect (viewNumber);
    G3D::Vector2 position = G3D::Vector2 (
        viewRect.center ().x, viewRect.y1 () * .98);
    pipeline.UpdateViewTitle (
        titleShown, position, simulation.GetName (), vs.GetTitle (viewNumber));
}


void WidgetVtk::UpdateAverageForce ()
{
    ViewNumber::Enum viewNumber = GetViewNumber ();    
    if (! IsVtkView (viewNumber))
        return;
    PipelineAverage3D& pipeline = *m_pipelineAverage3d[viewNumber];
    pipeline.UpdateAverageForce (
        *m_average[viewNumber]->GetForceAverage ());
    update ();
}

void WidgetVtk::UpdateAverageVelocity ()
{
    ViewNumber::Enum viewNumber = GetViewNumber ();    
    if (! IsVtkView (viewNumber))
        return;
    PipelineAverage3D& pipeline = *m_pipelineAverage3d[viewNumber];
    pipeline.FromView (viewNumber, *this);    
    pipeline.UpdateAverageVelocity (
        m_average[viewNumber]->GetVelocityAverage ());
    update ();
}

void WidgetVtk::UpdateView (
    ViewNumber::Enum viewNumber,
    const ColorBarModel& scalarColorMap, QwtDoubleInterval scalarInterval,
    const ColorBarModel& velocityColorMap)
{
    boost::shared_ptr<RegularGridAverage> scalarAverage = 
        m_average[viewNumber]->GetScalarAverage ();
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const Simulation& simulation = GetSimulation (viewNumber);
    PipelineAverage3D& pipeline = *m_pipelineAverage3d[viewNumber];
    G3D::AABox vv = CalculateViewingVolume (viewNumber, simulation);


    m_pipeline[viewNumber] = m_pipelineAverage3d[viewNumber];
    scalarAverage->SetBodyAttribute (vs.GetBodyOrOtherScalar ());
    m_average[viewNumber]->AverageInitStep (vs.GetTimeWindow ());
    m_average[viewNumber]->ComputeAverage ();

    pipelineUpdateScalar (viewNumber, scalarColorMap, scalarInterval);
    pipeline.UpdateAverageForce (*m_average[viewNumber]->GetForceAverage ());
    pipeline.UpdateT1 (getT1Vtk (viewNumber));
    pipelineUpdateVelocity (viewNumber, velocityColorMap);    
    // other
    pipeline.FromView (viewNumber, *this);
    pipeline.UpdateFocus (GetViewNumber () == viewNumber);
    pipeline.FromViewTransform (viewNumber, *this);

    G3D::Rect2D viewRect = GetNormalizedViewRect (viewNumber);
    G3D::Vector2 position = G3D::Vector2 (
        viewRect.center ().x, viewRect.y1 () * 0.99);
    vtkSmartPointer<vtkRenderWindow> renderWindow = GetRenderWindow ();
    renderWindow->AddRenderer(pipeline.GetRenderer ());
    pipeline.GetRenderer ()->SetViewport (viewRect.x0 (), viewRect.y0 (),
                                          viewRect.x1 (), viewRect.y1 ());
    pipeline.UpdateViewTitle (
        GetSettings ().IsTitleShown (), 
        position, simulation.GetName (), vs.GetTitle (viewNumber));
    resizeViewEvent (viewNumber);
    setVisible (true);
    m_pipeline[viewNumber]->GetRenderer ()->ResetCamera ();
    update ();
}


void WidgetVtk::pipelineUpdateScalar (
    ViewNumber::Enum viewNumber,
    const ColorBarModel& scalarColorMap, QwtDoubleInterval interval)
{
    PipelineAverage3D& pipeline = *m_pipelineAverage3d[viewNumber];
    const ViewSettings& vs = GetViewSettings (viewNumber);
    pipeline.UpdateAverageScalar (
        *m_average[viewNumber]->GetBodyOrOtherScalarAverage ());
    pipeline.UpdateThresholdScalar (interval, vs.GetBodyOrOtherScalar ());
    pipeline.UpdateColorMapScalar (scalarColorMap);
}

void WidgetVtk::pipelineUpdateVelocity (
    ViewNumber::Enum viewNumber, const ColorBarModel& velocityColorMap)
{
    PipelineAverage3D& pipeline = *m_pipelineAverage3d[viewNumber];
    pipeline.UpdateAverageVelocity (
        m_average[viewNumber]->GetVelocityAverage ());
    pipeline.UpdateColorMapVelocity (velocityColorMap);
}

void WidgetVtk::UpdateAverage (ViewNumber::Enum viewNumber, int direction)
{
    PipelineAverage3D& pipeline = *m_pipelineAverage3d[viewNumber];
    const ViewSettings& vs = GetViewSettings (viewNumber);
    boost::shared_ptr<RegularGridAverage> scalarAverage = 
        m_average[viewNumber]->GetScalarAverage ();
    m_average[viewNumber]->AverageStep (direction, vs.GetTimeWindow ());
    m_average[viewNumber]->ComputeAverage ();
    pipeline.UpdateAverageScalar (
        *m_average[viewNumber]->GetBodyOrOtherScalarAverage ());
    pipeline.UpdateAverageForce (*m_average[viewNumber]->GetForceAverage ());
    pipeline.UpdateAverageVelocity (
        m_average[viewNumber]->GetVelocityAverage ());
    pipeline.UpdateT1 (getT1Vtk (viewNumber));
    updateViewTitle (viewNumber);
}

vtkSmartPointer<vtkPolyData> WidgetVtk::getT1Vtk (ViewNumber::Enum viewNumber)
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const Simulation& simulation = GetSimulation (viewNumber);
    if (vs.IsT1Shown ())
    {
        if (vs.IsT1AllTimeSteps ())
        {
            for (size_t i = 0; i < simulation.GetTimeSteps (); ++i)
                ;
        }
        else
        {
            return getT1Vtk (viewNumber, GetTime (viewNumber));
        }
    }
}

vtkSmartPointer<vtkPolyData> WidgetVtk::getT1Vtk (
    ViewNumber::Enum viewNumber, size_t time)
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const Simulation& simulation = GetSimulation (viewNumber);
    vtkSmartPointer<vtkPolyData> t1Vtk = 
        simulation.GetT1Vtk (time, vs.T1sShiftLower ());
    if (! vs.IsAverageAround ())
        return t1Vtk;
    boost::shared_ptr<RegularGridAverage> scalarAverage = 
        m_average[viewNumber]->GetScalarAverage ();
    G3D::Vector3 t = scalarAverage->GetTranslation (time);
    VTK_CREATE (vtkTransform, transform);
    transform->Translate (t.x, t.y, t.z);
    VTK_CREATE (vtkTransformPolyDataFilter, transformFilter);
    transformFilter->SetTransform (transform);
    transformFilter->SetInputData (t1Vtk);
    transformFilter->Update ();
    return vtkPolyData::SafeDownCast (transformFilter->GetOutput ());
}


G3D::Rect2D WidgetVtk::GetNormalizedViewRect (ViewNumber::Enum viewNumber) const
{
    float w = width ();
    float h = height ();
    G3D::Rect2D vr = GetViewRect (viewNumber);
    G3D::Rect2D viewRect = G3D::Rect2D::xyxy (vr.x0 () / w, vr.y0 () / h,
					      vr.x1 () / w, vr.y1 () / h);    
    return viewRect;
}

void WidgetVtk::ForAllViews (
    PipelineType::Enum type, boost::function <void (ViewNumber::Enum)> f)
{
    for (size_t i = 0; i < GetViewCount (); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::FromSizeT (i);
	if (IsVtkView (viewNumber) && GetPipelineType (viewNumber) == type)
	    f (viewNumber);
    }
}

PipelineType::Enum WidgetVtk::GetPipelineType (ViewNumber::Enum viewNumber)
{
    if (m_pipeline[viewNumber] != 0)
        return m_pipeline[viewNumber]->GetType ();
    else
        return PipelineType::COUNT;
}


// Overrides
////////////
void WidgetVtk::mousePressEvent (QMouseEvent *event)
{
    QVTKWidget::mousePressEvent (event);
    G3D::Vector2 p = QtToOpenGl (event->pos (), height ());
    setView (p);
}

void WidgetVtk::contextMenuEventView (QMenu* menu) const
{
    {
        QMenu* menuCopy = menu->addMenu ("Copy");
        addCopyMenu (menuCopy, "Transform", &m_actionCopyTransform[0]);
        addCopyMenu (menuCopy, "Force ratio", &m_actionCopyForceRatio[0]);
        addCopyCompatibleMenu (menuCopy, "Selection", &m_actionCopySelection[0], 
                               &WidgetBase::IsSelectionCopyCompatible);
    }
    {
	QMenu* menuReset = menu->addMenu ("Reset transform");
	menuReset->addAction (m_actionResetTransformAll.get ());
	menuReset->addAction (m_actionResetTransformFocus.get ());
    }
}

void WidgetVtk::contextMenuEvent (QContextMenuEvent *event)
{
    QVTKWidget::contextMenuEvent (event);
    WidgetBase::contextMenuEvent (event);
}

void WidgetVtk::resizeEvent (QResizeEvent * event)
{
    QVTKWidget::resizeEvent (event);
    WidgetBase::ForAllViews (
        boost::bind (&WidgetVtk::resizeViewEvent, this, _1));
}
void WidgetVtk::resizeViewEvent (ViewNumber::Enum viewNumber)
{
    G3D::Rect2D viewRect = GetViewRect (viewNumber);
    const Settings& settings = GetSettings ();
    const ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::Rect2D scalarBarRect = 
        settings.GetColorMapScalarRectWithLabels (viewNumber, viewRect);
    m_pipeline[viewNumber]->PositionScalarBar (
        toRatio (scalarBarRect, viewRect));
    if (vs.IsVelocityShown ())
    {
        G3D::Rect2D vectorBarRect =
            settings.GetColorMapVelocityRectWithLabels (viewNumber, viewRect);
        m_pipeline[viewNumber]->PositionVectorBar (
            toRatio (vectorBarRect, viewRect));
    }
}


// slots
// ===========================================================================
void WidgetVtk::CopySelectionFrom (int fromViewNumber)
{
    ViewNumber::Enum toViewNumber = GetViewNumber ();
    GetViewSettings (toViewNumber).CopySelection (
	GetViewSettings (ViewNumber::Enum (fromViewNumber)));
    update ();
}

void WidgetVtk::ResetTransformAll ()
{
    ResetTransformFocus ();
}

void WidgetVtk::ResetTransformFocus ()
{
    WidgetBase::ResetTransformFocus ();
    FromViewTransform ();
}

void WidgetVtk::CopyTransformFromSlot (int vn)
{
    ViewNumber::Enum viewNumber = ViewNumber::Enum (vn);
    CopyTransformFrom (viewNumber);
    FromViewTransform (viewNumber);
}

void WidgetVtk::CopyForceRatioFromSlot (int vn)
{
    ViewNumber::Enum viewNumber = ViewNumber::Enum (vn);
    CopyForceRatioFrom (viewNumber);
    FromViewTransform (viewNumber);
}
