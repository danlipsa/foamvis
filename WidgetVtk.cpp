/**
 * @file   WidgetVtk.cpp
 * @author Dan R. Lipsa
 * @date 13 June 2012
 * 
 * Definitions for the widget for displaying foam bubbles using Vtk
 */

#include "PipelineAverage3d.h"
#include "Body.h"
#include "DebugStream.h"
#include "Foam.h"
#include "FoamvisInteractorStyle.h"
#include "OpenGLUtils.h"
#include "RegularGridAverage.h"
#include "Settings.h"
#include "Simulation.h"
#include "ViewSettings.h"
#include "WidgetVtk.h"
#include "vtkConeSource.h"

//#define __LOG__(code) code
#define __LOG__(code)

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
    m_widgetVtk->VtkToView ();
    m_widgetVtk->SendPaintEnd ();
}


// Methods WidgetVtk
// ======================================================================

WidgetVtk::WidgetVtk (QWidget* parent) :
    QVTKWidget (parent),
    WidgetBase (this, &Settings::IsVtkView, &Settings::GetVtkCount),
    m_fontSize (10)
{
    setVisible (false);

    initCopy (m_actionCopyTransformation, m_signalMapperCopyTransformation);
    connect (m_signalMapperCopyTransformation.get (),
	     SIGNAL (mapped (int)),
	     this,
	     SLOT (CopyTransformFrom (int)));
}

// ======================================================================
// PipelineBase
void WidgetVtk::UpdateColorTransferFunction (
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction, 
    const char* name)
{
    m_pipeline[GetSettings ()->GetViewNumber ()]->UpdateColorTransferFunction (
	colorTransferFunction, name);
    update ();
}

void WidgetVtk::UpdateFocus ()
{
    ForAllViews (
	boost::bind (&WidgetVtk::updateViewFocus, this, _1));
    update ();
}
void WidgetVtk::updateViewFocus (ViewNumber::Enum viewNumber)
{
    bool focus = (GetSettings ()->GetViewNumber () == viewNumber) && 
	GetSettings ()->IsViewFocusShown ();
    PipelineBase& pipeline = *m_pipeline[viewNumber];
    pipeline.UpdateFocus (focus);
}

void WidgetVtk::ViewToVtk (ViewNumber::Enum viewNumber)
{
    const ViewSettings& vs = GetSettings ()->GetViewSettings (viewNumber);
    const Foam& foam = m_average[viewNumber]->GetFoam ();
    const Simulation& simulation = m_average[viewNumber]->GetSimulation ();
    PipelineBase& pipeline = *m_pipeline[viewNumber];
    pipeline.ViewToVtk (vs, simulation.GetBoundingBox ().center (), foam);
}

void WidgetVtk::VtkToView (ViewNumber::Enum viewNumber)
{
    // This may be called by RenderWindowPaintEnd and may arrive after the 
    // view was switched to a Gl view
    if (GetSettings ()->IsVtkView (viewNumber))
    {
        ViewSettings& vs = GetSettings ()->GetViewSettings (viewNumber);
        const Foam& foam = m_average[viewNumber]->GetFoam ();
        PipelineBase& pipeline = *m_pipeline[viewNumber];
        pipeline.VtkToView (vs, foam);
    }
}

void WidgetVtk::RemoveViews ()
{
    vtkSmartPointer<vtkRenderWindow> renderWindow = GetRenderWindow ();
    ForAllViews (boost::bind (&WidgetVtk::removeView, this, _1));
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
// PipelineAverage3d
void WidgetVtk::SendPaintEnd ()
{
    Q_EMIT PaintEnd ();
}

void WidgetVtk::CreateAverage3d (boost::shared_ptr<Settings> settings,
                                 const SimulationGroup& simulationGroup)    
{
    SetSettings (settings);
    // interactor style
    VTK_CREATE (FoamvisInteractorStyle, interactorStyle);
    interactorStyle->SetInteractionModeQuery (
        boost::bind (&Settings::GetInteractionMode, GetSettings ()));
    
    QVTKInteractor *interactor = GetInteractor();
    interactor->SetInteractorStyle (interactorStyle);
    
    for (size_t i = 0; i < m_average.size (); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	m_average[i].reset (new RegularGridAverage (
				viewNumber,
				*settings, simulationGroup));
    }
}

void WidgetVtk::CreatePipelineAverage3d (
    size_t objects, size_t constraintSurfaces, size_t fontSize)
{
    m_fontSize = fontSize + 4;
    vtkRenderWindow* renWin = GetRenderWindow ();

    for (size_t i = 0; i < ViewNumber::COUNT; ++i)
    {
	m_pipelineAverage3d[i].reset (
            new PipelineAverage3d (
                objects, constraintSurfaces, fontSize));
    }

    vtkSmartPointer<RenderWindowPaintEnd> sendPaint (
        new RenderWindowPaintEnd (this));
    renWin->AddObserver (vtkCommand::EndEvent, sendPaint);
}

void WidgetVtk::UpdateAverage3dThreshold (QwtDoubleInterval interval)
{
    ViewNumber::Enum viewNumber = GetSettings ()->GetViewNumber ();
    if (GetPipelineType (viewNumber) == PipelineType::AVERAGE_3D)
    {
        m_pipelineAverage3d[viewNumber]->UpdateThreshold (interval);
        update ();
    }
}


void WidgetVtk::UpdateAverage3dOpacity ()
{
    ForAllPipelines (PipelineType::AVERAGE_3D, 
                     boost::bind (&WidgetVtk::updateViewOpacity, this, _1));
    update ();
}
void WidgetVtk::updateViewOpacity (ViewNumber::Enum viewNumber)
{
    m_pipelineAverage3d[viewNumber]->UpdateOpacity (
        GetSettings ()->GetContextAlpha ());
}


void WidgetVtk::AddAverage3dView (
    ViewNumber::Enum viewNumber,
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction,
    QwtDoubleInterval interval)
{
    vtkSmartPointer<vtkRenderWindow> renderWindow = GetRenderWindow ();
    boost::shared_ptr<RegularGridAverage> average = m_average[viewNumber];
    const ViewSettings& vs = GetSettings ()->GetViewSettings (viewNumber);
    const char* scalarName = FaceScalar::ToString (vs.GetBodyOrFaceScalar ());
    const Simulation& simulation = m_average[viewNumber]->GetSimulation ();
    const Foam& foam = m_average[viewNumber]->GetFoam ();
    m_pipeline[viewNumber] = m_pipelineAverage3d[viewNumber];
    PipelineAverage3d& pipeline = *m_pipelineAverage3d[viewNumber];
    G3D::AABox vv = CalculateViewingVolume (viewNumber);
    average->AverageInitStep ();
    int direction = 0;
    pipeline.UpdateAverage (average, direction);
    pipeline.ViewToVtk (vs, simulation.GetBoundingBox ().center (), foam);
    pipeline.UpdateOpacity (GetSettings ()->GetContextAlpha ());
    pipeline.UpdateThreshold (interval);
    pipeline.UpdateColorTransferFunction (colorTransferFunction, scalarName);
    pipeline.UpdateFocus (GetSettings ()->GetViewNumber () == viewNumber);

    G3D::Rect2D viewRect = GetNormalizedViewRect (viewNumber);
    G3D::Vector2 position = G3D::Vector2 (
        viewRect.center ().x, viewRect.y1 () * 0.99);
    renderWindow->AddRenderer(pipeline.GetRenderer ());
    pipeline.GetRenderer ()->SetViewport (viewRect.x0 (), viewRect.y0 (),
				      viewRect.x1 (), viewRect.y1 ());
    pipeline.UpdateViewTitle (GetSettings ()->IsTitleShown (), 
                              position, average, viewNumber);
    resizeViewEvent (viewNumber);
    setVisible (true);
}

void WidgetVtk::UpdateViewAverage3d (
    ViewNumber::Enum viewNumber,
    const boost::array<int, ViewNumber::COUNT>& direction)
{
    PipelineAverage3d& pipeline = *m_pipelineAverage3d[viewNumber];
    boost::shared_ptr<RegularGridAverage> average = m_average[viewNumber];
    pipeline.UpdateAverage (average, direction[viewNumber]);
    updateViewTitle (viewNumber);
}


void WidgetVtk::UpdateAverage3dTitle ()
{
    ForAllPipelines (PipelineType::AVERAGE_3D,
	boost::bind (&WidgetVtk::updateViewTitle, this, _1));
    update ();
}
void WidgetVtk::updateViewTitle (ViewNumber::Enum viewNumber)
{
    bool titleShown = GetSettings ()->IsTitleShown ();
    PipelineAverage3d& pipeline = *m_pipelineAverage3d[viewNumber];
    G3D::Rect2D viewRect = GetNormalizedViewRect (viewNumber);
    G3D::Vector2 position = G3D::Vector2 (
        viewRect.center ().x, 
        viewRect.y1 () * .98);
    boost::shared_ptr<RegularGridAverage> average = m_average[viewNumber];
    pipeline.UpdateViewTitle (titleShown, position, average, viewNumber);
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

const Simulation& WidgetVtk::GetSimulation (ViewNumber::Enum viewNumber) const
{
    return m_average[viewNumber]->GetSimulation ();
}

void WidgetVtk::CopyTransformFrom (int fromViewNumber)
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    GetViewSettings (viewNumber).CopyTransformation (
	GetViewSettings (ViewNumber::Enum (fromViewNumber)));
    ViewToVtk (viewNumber);
}

void WidgetVtk::ForAllPipelines (
    PipelineType::Enum type, boost::function <void (ViewNumber::Enum)> f)
{
    for (int i = 0; i < GetSettings ()->GetViewCount (); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::FromSizeT (i);
	if (GetSettings ()->IsVtkView (viewNumber) &&
            GetPipelineType (viewNumber) == type)
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

void WidgetVtk::contextMenuEvent (QContextMenuEvent *event)
{
    QVTKWidget::contextMenuEvent (event);
    QMenu menu (this);
    addCopyMenu (&menu, "Copy Transformation", &m_actionCopyTransformation[0]);
    menu.exec (event->globalPos());
}

void WidgetVtk::resizeEvent (QResizeEvent * event)
{
    QVTKWidget::resizeEvent (event);
    ForAllViews (boost::bind (&WidgetVtk::resizeViewEvent, this, _1));
}
void WidgetVtk::resizeViewEvent (ViewNumber::Enum viewNumber)
{
    G3D::Rect2D viewRect = GetViewRect (viewNumber);
    G3D::Rect2D viewColorBarRect = Settings::GetViewColorBarRect (viewRect);
    G3D::Rect2D position = G3D::Rect2D::xywh (
        (viewColorBarRect.x0 () - viewRect.x0 ())/ viewRect.width (),
        (viewColorBarRect.y0 () - viewRect.y0 ())/ viewRect.height (),
        viewColorBarRect.width () / viewRect.width () * 5,
        viewColorBarRect.height () / viewRect.height () * 1.2);	
    m_pipeline[viewNumber]->PositionScalarBar (position);
    m_pipeline[viewNumber]->GetRenderer ()->ResetCamera ();
}

