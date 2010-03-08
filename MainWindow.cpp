/**
 * @file   MainWindow.h
 * @author Dan R. Lipsa
 *
 * Contains definitions for the UI class
 */
#include "GLWidget.h"
#include "MainWindow.h"
#include "SystemDifferences.h"
#include "DebugStream.h"
#include "DataFiles.h"


MainWindow::MainWindow(DataFiles& dataFiles) : 
    m_play (false), PLAY_TEXT (">"), PAUSE_TEXT("||"),
    m_timer (new QTimer(this))
{
    setupUi (this);
    m_dataSlider->setMinimum (0);
    m_dataSlider->setMaximum (dataFiles.GetData ().size () - 1);
    m_dataSlider->setSingleStep (1);
    m_dataSlider->setPageStep (10);
    m_glWidget->SetDataFiles (&dataFiles);
    updateStatus ();
    if (! dataFiles.GetData()[0]->IsTorus ())
	m_periodicModelGroupBox->hide ();

    QObject::connect(m_timer.get (), SIGNAL(timeout()),
                     this, SLOT(IncrementSlider ()));
}


void MainWindow::InteractionModeRotate ()
{
    m_interactionModeComboBox->setCurrentIndex (0);
}

void MainWindow::InteractionModeScale ()
{
    int index = m_interactionModeComboBox->currentIndex ();
    index = (index == 1) ? 2 : 1;
    m_interactionModeComboBox->setCurrentIndex (index);
}

void MainWindow::InteractionModeTranslate ()
{
    m_interactionModeComboBox->setCurrentIndex (3);
}


void MainWindow::ViewVertices (bool checked)
{
    if (checked)
	m_widthGroupBox->setEnabled (true);
}

void MainWindow::ViewEdges (bool checked)
{
    if (checked)
	m_widthGroupBox->setEnabled (true);
}

void MainWindow::ViewFaces (bool checked)
{
    if (checked)
	m_widthGroupBox->setEnabled (false);
}

void MainWindow::ViewBodies (bool checked)
{
    if (checked)
	m_widthGroupBox->setEnabled (false);
}

void MainWindow::ViewCenterPaths (bool checked)
{
    if (checked)
	m_widthGroupBox->setEnabled (false);
}


void MainWindow::TogglePlay ()
{
    if (m_play)
    {
        m_timer->stop ();
        m_toolButtonPlay->setText (PLAY_TEXT);
	updateButtons ();
    }
    else
    {
        m_timer->start ();
        m_toolButtonPlay->setText (PAUSE_TEXT);
        enableBegin (false);
        enableEnd (false);
    }
    m_play = ! m_play;
}

void MainWindow::BeginSlider ()
{
    m_dataSlider->setValue (m_dataSlider->minimum ());
    updateButtons ();
    updateStatus ();
}

void MainWindow::EndSlider ()
{
    m_dataSlider->setValue (m_dataSlider->maximum ());
    updateButtons ();
    updateStatus ();
}

void MainWindow::IncrementSlider ()
{
    int value = m_dataSlider->value ();
    if (value < m_dataSlider->maximum ())
        m_dataSlider->setValue (value + 1);
    else
        TogglePlay ();
}

void MainWindow::DataSliderValueChanged (int)
{
    updateButtons ();
    updateStatus ();
}


void MainWindow::updateButtons ()
{
    enableBegin (true);
    enableEnd (true);
    enablePlay (true);
}

void MainWindow::updateStatus ()
{
    vector<Data*>& data = m_glWidget->GetDataFiles ().GetData ();
    Data& currentData = m_glWidget->GetCurrentData ();
    QString oldString = m_status->text ();
    ostringstream ostr, bubble;
    ostr << "Time step: " 
	 << (m_glWidget->GetCurrentDataIndex () + 1) << " of "
	 << data.size () 
	 << ", Bubbles: " << currentData.GetBodies ().size ();
    if (m_glWidget->GetDisplayedBody () != m_glWidget->DISPLAY_ALL)
	bubble << ", Bubble: " << m_glWidget->GetDisplayedBody () << ends;
    ostr << bubble.str () << ends;
    QString newString (ostr.str().c_str ());
    if (oldString != newString)
	m_status->setText (newString);
}


void MainWindow::enableBegin (bool enable)
{
    if (enable && 
        m_dataSlider->value () > m_dataSlider->minimum ())
        m_toolButtonBegin->setDisabled (false);
    else
        m_toolButtonBegin->setDisabled (true);
}

void MainWindow::enableEnd (bool enable)
{
    if (enable && 
        m_dataSlider->value () < m_dataSlider->maximum ())
        m_toolButtonEnd->setDisabled (false);
    else
        m_toolButtonEnd->setDisabled (true);
}

void MainWindow::enablePlay (bool enable)
{
    if (enable && 
        m_dataSlider->value () < m_dataSlider->maximum ())
        m_toolButtonPlay->setDisabled (false);
    else
        m_toolButtonPlay->setDisabled (true);
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
            m_glWidget->IncrementDisplayedBody ();
	    updateStatus ();
	    break;
	case Qt::ShiftModifier:
	    m_glWidget->IncrementDisplayedFace ();
	    break;
	case Qt::ControlModifier:
	    //m_glWidget->IncrementDisplayedEdge ();
	    break;
	}
        break;
    case Qt::Key_PageDown:
	switch (modifiers)
	{
	case Qt::NoModifier:
            m_glWidget->DecrementDisplayedBody ();
	    updateStatus ();
	    break;
	case Qt::ShiftModifier:
	    m_glWidget->DecrementDisplayedFace ();
	    break;
	case Qt::ControlModifier:
	    //m_glWidget->DecrementDisplayedEdge ();
	    break;
	}
	break;
    case Qt::Key_Space:
    {
        string s = G3D::getOpenGLState (false);
        cdbg << s;
	break;
    }

    case Qt::Key_R:
	InteractionModeRotate ();
	cdbg << "rotate" << endl;
	break;
    case Qt::Key_S:
	InteractionModeScale ();
	cdbg << "scale" << endl;
	break;
    case Qt::Key_T:
	InteractionModeTranslate ();
	cdbg << "translate" << endl;
	break;
    }
}
