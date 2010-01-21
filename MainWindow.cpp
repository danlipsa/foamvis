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
#include "Data.h"


MainWindow::MainWindow(vector<Data*>& data) : 
    m_play (false), PLAY_TEXT (">"), PAUSE_TEXT("||")
{
    setupUi (this);
    m_timer = new QTimer (this);
    m_dataSlider->setMinimum (0);
    m_dataSlider->setMaximum (data.size () - 1);
    m_dataSlider->setSingleStep (1);
    m_dataSlider->setPageStep (10);
    m_glWidget->SetData (data);
    updateStatus ();


    QObject::connect(m_timer, SIGNAL(timeout()),
                     this, SLOT(IncrementSlider ()));
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
    cdbg << "Toogle play" << endl;
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

void MainWindow::SliderValueChanged (int)
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
    vector<Data*>& data = m_glWidget->GetData ();
    Data& currentData = m_glWidget->GetCurrentData ();
    QString oldString = m_status->text ();
    ostringstream ostr;
    ostr << "Time steps: " << data.size () 
	 << ", Bubbles: " << currentData.GetBodies ().size () << ends;
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
        if (modifiers == Qt::ShiftModifier)
            m_glWidget->IncrementDisplayedFace ();
        else
            m_glWidget->IncrementDisplayedBody ();
        break;
    case Qt::Key_PageDown:
        if (modifiers == Qt::ShiftModifier)
            m_glWidget->DecrementDisplayedFace ();
        else
            m_glWidget->DecrementDisplayedBody ();
        break;
    case Qt::Key_Space:
        string s = G3D::getOpenGLState (false);
        cdbg << s;
    }
}
