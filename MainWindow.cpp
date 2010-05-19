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
    setupUi (this);
    sliderData->setMinimum (0);
    sliderData->setMaximum (dataAlongTime.GetFoam ().size () - 1);
    sliderData->setSingleStep (1);
    sliderData->setPageStep (10);
    widgetGl->SetFoamAlongTime (&dataAlongTime);
    updateStatus ();
    if (dataAlongTime.GetFoam ().size () == 1)
    {
        toolButtonBegin->setDisabled (true);
	toolButtonEnd->setDisabled (true);
	toolButtonPlay->setDisabled (true);
    }
    if (dataAlongTime.GetFoam()[0]->IsTorus ())
    {
	radioButtonVerticesPhysical->setEnabled (false);
	radioButtonEdgesPhysical->setEnabled (false);
    }
    else
    {
	radioButtonVerticesTorus->setEnabled (false);
	radioButtonEdgesTorus->setEnabled (false);
	radioButtonFacesTorus->setEnabled (false);
	groupBoxTorusOriginalDomain->setEnabled (false);
    }
    if (dataAlongTime.GetFoam ()[0]->GetSpaceDimension () == 2)
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
    FoamAlongTime::Foams& data = widgetGl->GetFoamAlongTime ().GetFoam ();
    Foam& currentFoam = widgetGl->GetCurrentFoam ();
    QString oldString = labelStatus->text ();
    ostringstream ostr;
    ostr << "Time step: " 
	 << (widgetGl->GetTimeStep () + 1)<< " of " << data.size () 
	 << ", Bubbles: " << currentFoam.GetBodies ().size ();
    if (widgetGl->GetDisplayedBodyIndex () != GLWidget::DISPLAY_ALL)
	ostr << ", Bubble: " << (widgetGl->GetDisplayedBodyOriginalIndex () + 1);
    if (widgetGl->GetDisplayedFaceIndex () != GLWidget::DISPLAY_ALL)
	ostr << ", Face: " << (widgetGl->GetDisplayedFaceOriginalIndex () + 1);
    if (widgetGl->GetDisplayedEdgeIndex () != GLWidget::DISPLAY_ALL)
	ostr << ", Edge: " << (widgetGl->GetDisplayedEdgeOriginalIndex () + 1);
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
	try
	{
	    Body* b = widgetGl->GetCurrentFoam ().GetBody (m_currentBody);
	    if (m_processBodyTorus == 0)
	    {
		m_processBodyTorus = new ProcessBodyTorus (b);
		m_processBodyTorus->Initialize ();
	    }
	    else
		if (! m_processBodyTorus->Step ())
		{
		    m_processBodyTorus = 0;
		    cdbg << "End process torus" << endl;
		    m_currentBody = (m_currentBody + 1) % 
			widgetGl->GetCurrentFoam ().GetBodies ().size ();
		}
	    widgetGl->UpdateDisplay ();
	}
	catch (exception& e)
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
