/**
 * @file   MainWindow.h
 * @author Dan R. Lipsa
 *
 * Contains definitions for the UI class
 */
#include "Data.h"
#include "DataFiles.h"
#include "DebugStream.h"
#include "GLWidget.h"
#include "MainWindow.h"
#include "ProcessBodyTorus.h"
#include "SystemDifferences.h"


MainWindow::MainWindow(DataFiles& dataFiles) : 
    m_play (false), PLAY_TEXT (">"), PAUSE_TEXT("||"),
    m_timer (new QTimer(this)), m_processBodyTorus (0), 
    m_currentBody (0),
    m_saveMovie(false), m_currentFrame(0)
{
    setupUi (this);
    sliderData->setMinimum (0);
    sliderData->setMaximum (dataFiles.GetData ().size () - 1);
    sliderData->setSingleStep (1);
    sliderData->setPageStep (10);
    widgetGl->SetDataFiles (&dataFiles);
    updateStatus ();
    if (dataFiles.GetData()[0]->IsTorus ())
    {
	radioButtonVerticesPhysical->setEnabled (false);
	radioButtonEdgesPhysical->setEnabled (false);
	radioButtonCenterPath->setEnabled (false);
    }
    else
    {
	radioButtonVerticesTorus->setEnabled (false);
	radioButtonEdgesTorus->setEnabled (false);
	radioButtonFacesTorus->setEnabled (false);
	checkBoxTorusOriginalDomain->setEnabled (false);
    }
    if (dataFiles.GetData ()[0]->GetSpaceDimension () == 2)
    {
	radioButtonEdgesNormal->toggle ();
	tabWidget->setCurrentWidget (edges);
	radioButtonCenterPath->setEnabled (false);
    }
    else
    {
	radioButtonFacesNormal->toggle ();
	tabWidget->setCurrentWidget (faces);
    }
    
    // 100 ms
    m_timer->setInterval (100);
    QObject::connect(m_timer.get (), SIGNAL(timeout()),
                     this, SLOT(IncrementSlider ()));
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
    enableBegin (true);
    enableEnd (true);
    enablePlay (true);
}

void MainWindow::updateStatus ()
{
    vector<Data*>& data = widgetGl->GetDataFiles ().GetData ();
    Data& currentData = widgetGl->GetCurrentData ();
    QString oldString = labelStatus->text ();
    ostringstream ostr;
    ostr << "Time step: " 
	 << (widgetGl->GetCurrentDataIndex () + 1) << " of "
	 << data.size () 
	 << ", Bubbles: " << currentData.GetBodies ().size ();
    if (widgetGl->GetDisplayedBody () != widgetGl->DISPLAY_ALL)
	ostr << ", Bubble: " << widgetGl->GetDisplayedBody ();
    if (widgetGl->GetDisplayedFace () != widgetGl->DISPLAY_ALL)
	ostr << ", Face: " << widgetGl->GetDisplayedFace ();
    ostr << ends;
    QString newString (ostr.str().c_str ());
    if (oldString != newString)
	labelStatus->setText (newString);
}


void MainWindow::enableBegin (bool enable)
{
    if (enable && 
        sliderData->value () > sliderData->minimum ())
        toolButtonBegin->setDisabled (false);
    else
        toolButtonBegin->setDisabled (true);
}

void MainWindow::enableEnd (bool enable)
{
    if (enable && 
        sliderData->value () < sliderData->maximum ())
        toolButtonEnd->setDisabled (false);
    else
        toolButtonEnd->setDisabled (true);
}

void MainWindow::enablePlay (bool enable)
{
    if (enable && 
        sliderData->value () < sliderData->maximum ())
        toolButtonPlay->setDisabled (false);
    else
        toolButtonPlay->setDisabled (true);
}


void MainWindow::keyPressEvent (QKeyEvent* event)
{
    Qt::KeyboardModifiers modifiers = event->modifiers ();
    switch (event->key ())
    {
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
	case Qt::ControlModifier:
	    //widgetGl->IncrementDisplayedEdge ();
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
	case Qt::ControlModifier:
	    //widgetGl->DecrementDisplayedEdge ();
	    break;
	}
	break;
    case Qt::Key_Space:
    {
	try
	{
	    Body* b = widgetGl->GetCurrentData ().GetBody (m_currentBody);
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
			widgetGl->GetCurrentData ().GetBodies ().size ();
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

void MainWindow::TogglePlay ()
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
        enableBegin (false);
        enableEnd (false);
    }
    m_play = ! m_play;
}

void MainWindow::BeginSlider ()
{
    sliderData->setValue (sliderData->minimum ());
    updateButtons ();
    updateStatus ();
}

void MainWindow::EndSlider ()
{
    sliderData->setValue (sliderData->maximum ());
    updateButtons ();
    updateStatus ();
}

void MainWindow::IncrementSlider ()
{
    int value = sliderData->value ();
    if (value < sliderData->maximum ())
        sliderData->setValue (value + 1);
    else
        TogglePlay ();
}

void MainWindow::DataSliderValueChanged (int value)
{
    widgetGl->DataSliderValueChanged (value);
    widgetGl->UpdateDisplay ();
    updateButtons ();
    updateStatus ();
    if (m_saveMovie)
    {
        ostringstream file;
        file << "movie/frame" << setfill ('0') << setw (4) <<
	    m_currentFrame << ".jpg" << ends;
        QImage snapshot = QPixmap::grabWidget (widgetDisplay).toImage ();
	string f = file.str ();
	if (! snapshot.save (f.c_str ()))
	    cdbg << "Error saving " << f << endl;
	m_currentFrame++;
    }
}


void MainWindow::ViewPhysicalVertices (bool checked)
{
    widgetGl->ViewPhysicalVertices (checked);
    if (checked)
	stackedWidgetVertices->setCurrentWidget (pageVerticesPhysical);
    else
	stackedWidgetVertices->setCurrentWidget (pageVerticesEmpty);
}

void MainWindow::ViewRawEdges (bool checked)
{
    widgetGl->ViewRawEdges (checked);
    if (checked)
	stackedWidgetEdges->setCurrentWidget (pageEdgesTorus);
    else
	stackedWidgetEdges->setCurrentWidget (pageEdgesEmpty);
}

void MainWindow::ViewRawFaces (bool checked)
{
    widgetGl->ViewRawFaces (checked);
    if (checked)
	stackedWidgetFaces->setCurrentWidget (pageFacesTorus);
    else
	stackedWidgetFaces->setCurrentWidget (pageFacesEmpty);
}


void MainWindow::ViewPhysicalEdges (bool checked)
{
    widgetGl->ViewPhysicalEdges (checked);
    if (checked)
	stackedWidgetEdges->setCurrentWidget (pageEdgesPhysical);
    else
	stackedWidgetEdges->setCurrentWidget (pageEdgesEmpty);
}

void MainWindow::SaveMovie (bool checked)
{
    m_saveMovie = checked;
    if (checked)
	m_currentFrame = 0;
    DataSliderValueChanged (sliderData->value ());
}
