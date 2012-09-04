/**
 * @file   WidgetGl.cpp
 * @author Dan R. Lipsa
 * @date 13 June 2012
 * 
 * Definitions for the widget for displaying foam bubbles using Vtk
 */

#include "Average3dPipeline.h"
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

class SendPaintEnd : public vtkCommand
{
public:
    SendPaintEnd (WidgetVtk* widgetVtk) :
	m_widgetVtk (widgetVtk)
    {
    }
    void Execute (vtkObject *caller, unsigned long eventId, void *callData);
private:
    WidgetVtk* m_widgetVtk;
};


void SendPaintEnd::Execute (
    vtkObject *caller, unsigned long eventId, void *callData)
{
    (void) caller;(void)callData;
    if (eventId == vtkCommand::EndEvent)
	Q_EMIT m_widgetVtk->PaintEnd ();
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
	     SLOT (CopyTransformationFrom (int)));
}

void WidgetVtk::CreateAverage (boost::shared_ptr<Settings> settings,
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

void WidgetVtk::CreateAverage3dPipeline (
    size_t objects, size_t constraintSurfaces, size_t fontSize)
{
    m_fontSize = fontSize + 4;
    vtkRenderWindow* renWin = GetRenderWindow ();

    for (size_t i = 0; i < ViewNumber::COUNT; ++i)
    {
	m_pipeline[i].reset (new Average3dPipeline (
                                 objects, constraintSurfaces, fontSize));
    }

    vtkSmartPointer<SendPaintEnd> sendPaint (new SendPaintEnd (this));
    renWin->AddObserver (vtkCommand::EndEvent, sendPaint);
}

void WidgetVtk::UpdateThreshold (QwtDoubleInterval interval)
{
    m_pipeline[ GetSettings ()->GetViewNumber ()]->UpdateThreshold (interval);
    update ();
}

void WidgetVtk::UpdateColorTransferFunction (
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction, 
    const char* name)
{
    m_pipeline[GetSettings ()->GetViewNumber ()]->UpdateColorTransferFunction (
	colorTransferFunction, name);
    update ();
}

void WidgetVtk::UpdateOpacity ()
{
    for (int i = 0; i < GetSettings ()->GetViewCount (); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::FromSizeT (i);
	m_pipeline[viewNumber]->UpdateOpacity (
	    GetSettings ()->GetContextAlpha ());
    }
    update ();
}


void WidgetVtk::RemoveViews ()
{
    vtkSmartPointer<vtkRenderWindow> renderWindow = GetRenderWindow ();
    for (size_t i = 0; i < m_pipeline.size (); ++i)
	renderWindow->RemoveRenderer (m_pipeline[i]->GetRenderer ());
    setVisible (false);
}

void WidgetVtk::AddView (
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
    Average3dPipeline& pipeline = *m_pipeline[viewNumber];
    G3D::AABox vv = CalculateViewingVolume (viewNumber);
    G3D::AABox bb = simulation.GetBoundingBox ();
    average->AverageInitStep ();
    int direction = 0;
    pipeline.UpdateAverage (average, direction);
    pipeline.CopyTransformationFromView (vs, bb, foam);
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
    pipeline.UpdateTitle (GetSettings ()->IsTitleShown (), 
                          position, average, viewNumber);
    resizeEvent (0);
    pipeline.GetRenderer ()->ResetCamera ();
    setVisible (true);
}

void WidgetVtk::UpdateTransformation (ViewNumber::Enum viewNumber)
{
    const ViewSettings& vs = GetSettings ()->GetViewSettings (viewNumber);
    const Simulation& simulation = m_average[viewNumber]->GetSimulation ();
    G3D::AABox bb = simulation.GetBoundingBox ();
    const Foam& foam = m_average[viewNumber]->GetFoam ();
    Average3dPipeline& pipeline = *m_pipeline[viewNumber];
    pipeline.CopyTransformationFromView (vs, bb, foam);
}



void WidgetVtk::UpdateAverage (
    const boost::array<int, ViewNumber::COUNT>& direction)
{
    ForAllViews (
	boost::bind (&WidgetVtk::UpdateViewAverage, this, _1, direction));
}
void WidgetVtk::UpdateViewAverage (
    ViewNumber::Enum viewNumber,
    const boost::array<int, ViewNumber::COUNT>& direction)
{
    Average3dPipeline& pipeline = *m_pipeline[viewNumber];
    boost::shared_ptr<RegularGridAverage> average = m_average[viewNumber];
    pipeline.UpdateAverage (average, direction[viewNumber]);
    updateViewTitle (viewNumber);
}


void WidgetVtk::UpdateTitle ()
{
    ForAllViews (
	boost::bind (&WidgetVtk::updateViewTitle, this, _1));
    update ();
}
void WidgetVtk::updateViewTitle (ViewNumber::Enum viewNumber)
{
    bool titleShown = GetSettings ()->IsTitleShown ();
    Average3dPipeline& pipeline = *m_pipeline[viewNumber];
    G3D::Rect2D viewRect = GetNormalizedViewRect (viewNumber);
    G3D::Vector2 position = G3D::Vector2 (
        viewRect.center ().x, 
        viewRect.y1 () * .98);
    boost::shared_ptr<RegularGridAverage> average = m_average[viewNumber];
    pipeline.UpdateTitle (titleShown, position, average, viewNumber);
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
    Average3dPipeline& pipeline = *m_pipeline[viewNumber];
    pipeline.UpdateFocus (focus);
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
    (void) event;
    for (int i = 0; i < GetSettings ()->GetViewCount (); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::FromSizeT (i);
	if (GetSettings ()->IsVtkView (viewNumber))
	{
	    G3D::Rect2D viewRect = GetViewRect (viewNumber);
	    G3D::Rect2D viewColorBarRect = 
		Settings::GetViewColorBarRect (viewRect);
	    G3D::Rect2D position = G3D::Rect2D::xywh (
		(viewColorBarRect.x0 () - viewRect.x0 ())/ viewRect.width (),
		(viewColorBarRect.y0 () - viewRect.y0 ())/ viewRect.height (),
		viewColorBarRect.width () / viewRect.width () * 5,
		viewColorBarRect.height () / viewRect.height () * 1.2);	
	    m_pipeline[viewNumber]->PositionScalarBar (position);
	}
    }
}

const Simulation& WidgetVtk::GetSimulation (ViewNumber::Enum viewNumber) const
{
    return m_average[viewNumber]->GetSimulation ();
}

void WidgetVtk::CopyTransformationFrom (int fromViewNumber)
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    GetViewSettings (viewNumber).CopyTransformation (
	GetViewSettings (ViewNumber::Enum (fromViewNumber)));
    UpdateTransformation (viewNumber);
}

