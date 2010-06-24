/**
 * @file   MainWindow.h
 * @author Dan R. Lipsa
 *
 * Contains definitions for the UI class
 */
#include "Foam.h"
#include "FoamAlongTime.h"
#include "DebugStream.h"
#include "GLWidget.h"
#include "MainWindow.h"
#include "ProcessBodyTorus.h"
#include "SystemDifferences.h"


MainWindow::MainWindow(FoamAlongTime& dataAlongTime) : 
    m_play (false), PLAY_TEXT (">"), PAUSE_TEXT("||"),
    m_timer (new QTimer(this)), m_processBodyTorus (0), 
    m_currentBody (0),
    m_saveMovie(false), m_currentFrame(0)
{
    // for anti-aliased lines
    QGLFormat fmt;
    fmt.setAlpha(true);
    QGLFormat::setDefaultFormat(fmt);    

    setupUi (this);
    sliderData->setMinimum (0);
    sliderData->setMaximum (dataAlongTime.GetFoamsSize () - 1);
    sliderData->setSingleStep (1);
    sliderData->setPageStep (10);
    widgetGl->SetFoamAlongTime (&dataAlongTime);
    updateStatus ();
    m_currentTranslatedBody = widgetGl->GetCurrentFoam ().GetBodies ().begin ();

    if (dataAlongTime.GetFoamsSize () == 1)
    {
        toolButtonBegin->setDisabled (true);
	toolButtonEnd->setDisabled (true);
	toolButtonPlay->setDisabled (true);
    }

    if (! dataAlongTime.GetFoam (0)->IsTorus ())
    {
	radioButtonVerticesTorus->setEnabled (false);
	radioButtonEdgesTorus->setEnabled (false);
	radioButtonFacesTorus->setEnabled (false);
	groupBoxTorusOriginalDomain->setEnabled (false);
    }
    if (dataAlongTime.GetFoam (0)->GetSpaceDimension () == 2)
    {
	radioButtonEdgesNormal->toggle ();
	tabWidget->setCurrentWidget (edges);
    }
    else
    {
	radioButtonFacesNormal->toggle ();
	tabWidget->setCurrentWidget (faces);
    }
    
    // 100 ms
    m_timer->setInterval (100);
    QObject::connect(m_timer.get (), SIGNAL(timeout()),
                     this, SLOT(TimeoutTimer ()));
}


void MainWindow::InteractionModeRotate ()
{
    comboBoxInteractionMode->setCurrentIndex (InteractionMode::ROTATE);
}

void MainWindow::InteractionModeScale ()
{
    int index = comboBoxInteractionMode->currentIndex ();
    index = (index == InteractionMode::SCALE) ? 
	InteractionMode::SCALE_VIEWPORT : InteractionMode::SCALE;
    comboBoxInteractionMode->setCurrentIndex (index);
}

void MainWindow::InteractionModeTranslate ()
{
    comboBoxInteractionMode->setCurrentIndex (
	InteractionMode::TRANSLATE_VIEWPORT);
}

void MainWindow::updateButtons ()
{
    enableBegin ();
    enableEnd ();
    enablePlay ();
}

void MainWindow::updateStatus ()
{
    Foam& currentFoam = widgetGl->GetCurrentFoam ();
    QString oldString = labelStatus->text ();
    ostringstream ostr;
    ostr << "Time step: " 
	 << (widgetGl->GetTimeStep () + 1)<< " of " 
	 << widgetGl->GetFoamAlongTime ().GetFoamsSize ()
	 << ", Bubbles: " << currentFoam.GetBodies ().size ();
    if (widgetGl->GetDisplayedBodyIndex () != GLWidget::DISPLAY_ALL)
	ostr << ", Bubble: " << (widgetGl->GetDisplayedBodyId () + 1);
    if (widgetGl->GetDisplayedFaceIndex () != GLWidget::DISPLAY_ALL)
	ostr << ", Face: " << (widgetGl->GetDisplayedFaceId () + 1);
    if (widgetGl->GetDisplayedEdgeIndex () != GLWidget::DISPLAY_ALL)
	ostr << ", Edge: " << (widgetGl->GetDisplayedEdgeId () + 1);
    ostr << ends;
    QString newString (ostr.str().c_str ());
    if (oldString != newString)
	labelStatus->setText (newString);
}


void MainWindow::enableBegin ()
{
    if (sliderData->value () > sliderData->minimum ())
        toolButtonBegin->setDisabled (false);
}

void MainWindow::enableEnd ()
{
    if (sliderData->value () < sliderData->maximum ())
        toolButtonEnd->setDisabled (false);
}

void MainWindow::enablePlay ()
{
    if (sliderData->value () < sliderData->maximum ())
        toolButtonPlay->setDisabled (false);
}


void MainWindow::keyPressEvent (QKeyEvent* event)
{
    Qt::KeyboardModifiers modifiers = event->modifiers ();
    switch (event->key ())
    {
    case Qt::Key_Up:
	widgetGl->IncrementDisplayedEdge ();
	updateStatus ();
	break;
    case Qt::Key_Down:
	widgetGl->DecrementDisplayedEdge ();
	updateStatus ();
	break;
    case Qt::Key_PageUp:
	switch (modifiers)
	{
	case Qt::NoModifier:
            widgetGl->IncrementDisplayedBody ();
	    updateStatus ();
	    break;
	case Qt::ShiftModifier:
	    widgetGl->IncrementDisplayedFace ();
	    updateStatus ();
	    break;
	}
        break;
    case Qt::Key_PageDown:
	switch (modifiers)
	{
	case Qt::NoModifier:
            widgetGl->DecrementDisplayedBody ();
	    updateStatus ();
	    break;
	case Qt::ShiftModifier:
	    widgetGl->DecrementDisplayedFace ();
	    updateStatus ();
	    break;
	}
	break;
    case Qt::Key_Space:
    {
	Foam& currentFoam = widgetGl->GetCurrentFoam ();
	VertexSet vertexSet;
	EdgeSet edgeSet;
	FaceSet faceSet;
	currentFoam.GetVertexSet (&vertexSet);
	currentFoam.GetEdgeSet (&edgeSet);
	currentFoam.GetFaceSet (&faceSet);
	m_currentTranslatedBody = currentFoam.BodyInsideOriginalDomainStep (
	    m_currentTranslatedBody, &vertexSet, &edgeSet, &faceSet);
	if (m_currentTranslatedBody == currentFoam.GetBodies ().end ())
	{
	    cdbg << "End body translation" << endl;
	}
	widgetGl->UpdateDisplay ();
	break;
    }

    case Qt::Key_A:
    {
	try
	{
	    Foam& currentFoam = widgetGl->GetCurrentFoam ();
	    boost::shared_ptr<Body>  b = currentFoam.GetBody (m_currentBody);
	    if (m_processBodyTorus == 0)
	    {
		m_processBodyTorus = new ProcessBodyTorus (
		    currentFoam, b);
		m_processBodyTorus->Initialize ();
	    }
	    else
	    {
		VertexSet vertexSet;
		EdgeSet edgeSet;
		FaceSet faceSet;
		currentFoam.GetVertexSet (&vertexSet);
		currentFoam.GetEdgeSet (&edgeSet);
		currentFoam.GetFaceSet (&faceSet);
		if (! m_processBodyTorus->Step (&vertexSet, &edgeSet, &faceSet))
		{
		    m_processBodyTorus = 0;
		    cdbg << "End process torus" << endl;
		    m_currentBody = (m_currentBody + 1) % 
			widgetGl->GetCurrentFoam ().GetBodies ().size ();
		}
	    }
	    widgetGl->UpdateDisplay ();
	}
	catch (const exception& e)
	{
	    cdbg << "Exception: " << e.what () << endl;
	}
	break;
    }

    case Qt::Key_R:
	InteractionModeRotate ();
	break;
    case Qt::Key_S:
	InteractionModeScale ();
	break;
    case Qt::Key_T:
	InteractionModeTranslate ();
	break;
    case Qt::Key_P:
	cdbg << "OpenGL State:" << endl;
	cdbg << G3D::getOpenGLState (false) << endl;
	break;
    }
}


// Slots
// ======================================================================

void MainWindow::ClickedPlay ()
{
    if (m_play)
    {
        m_timer->stop ();
        toolButtonPlay->setText (PLAY_TEXT);
	updateButtons ();
    }
    else
    {
        m_timer->start ();
        toolButtonPlay->setText (PAUSE_TEXT);
        toolButtonBegin->setDisabled (true);
        toolButtonEnd->setDisabled (true);
    }
    m_play = ! m_play;
}

void MainWindow::ClickedBegin ()
{
    sliderData->setValue (sliderData->minimum ());
    updateButtons ();
    updateStatus ();
}

void MainWindow::ClickedEnd ()
{
    sliderData->setValue (sliderData->maximum ());
    updateButtons ();
    updateStatus ();
}

void MainWindow::TimeoutTimer ()
{
    int value = sliderData->value ();
    if (value < sliderData->maximum ())
        sliderData->setValue (value + 1);
    else
        ClickedPlay ();
}

void MainWindow::ValueChangedSliderData (int value)
{
    widgetGl->ValueChangedSliderData (value);
    widgetGl->UpdateDisplay ();
    updateButtons ();
    updateStatus ();
    if (m_saveMovie)
    {
        ostringstream file;
        file << "movie/frame" << setfill ('0') << setw (4) <<
	    m_currentFrame << ".jpg" << ends;
	cdbg << "Taking snapshot ...";
        QImage snapshot = 
	    QPixmap::grabWindow (widgetDisplay->winId ()).toImage ();
	string f = file.str ();
	cdbg << "Saving " << f << " ..." << endl;
	if (! snapshot.save (f.c_str ()))
	    cdbg << "Error saving " << f << endl;
	m_currentFrame++;
    }
}


void MainWindow::ToggledVerticesPhysical (bool checked)
{
    widgetGl->ToggledVerticesPhysical (checked);
    if (checked)
	stackedWidgetVertices->setCurrentWidget (pageVerticesPhysical);
    else
	stackedWidgetVertices->setCurrentWidget (pageVerticesEmpty);
}

void MainWindow::ToggledEdgesNormal (bool checked)
{
    widgetGl->ToggledEdgesNormal (checked);
    if (checked)
	stackedWidgetEdges->setCurrentWidget (pageEdgesNormal);
    else
	stackedWidgetEdges->setCurrentWidget (pageEdgesEmpty);
}


void MainWindow::ToggledEdgesTorus (bool checked)
{
    widgetGl->ToggledEdgesTorus (checked);
    if (checked)
	stackedWidgetEdges->setCurrentWidget (pageEdgesTorus);
    else
	stackedWidgetEdges->setCurrentWidget (pageEdgesEmpty);
}

void MainWindow::ToggledFacesTorus (bool checked)
{
    widgetGl->ToggledFacesTorus (checked);
    if (checked)
	stackedWidgetFaces->setCurrentWidget (pageFacesTorus);
    else
	stackedWidgetFaces->setCurrentWidget (pageFacesEmpty);
}


void MainWindow::ToggledEdgesPhysical (bool checked)
{
    widgetGl->ToggledEdgesPhysical (checked);
    if (checked)
	stackedWidgetEdges->setCurrentWidget (pageEdgesPhysical);
    else
	stackedWidgetEdges->setCurrentWidget (pageEdgesEmpty);
}

void MainWindow::ToggledSaveMovie (bool checked)
{
    m_saveMovie = checked;
    if (checked)
	m_currentFrame = 0;
    ValueChangedSliderData (sliderData->value ());
}
