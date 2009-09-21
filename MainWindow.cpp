#include <G3D/G3DAll.h>
#include <QtGui>
#include <QTimer>
#include "GLWidget.h"
#include "MainWindow.h"
#include "SystemDifferences.h"
#include "DebugStream.h"
using namespace std;

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

    QObject::connect(m_toolButtonPlayPause, SIGNAL(clicked()), 
                     this, SLOT(TooglePlay()));
    QObject::connect(m_toolButtonBegin, SIGNAL(clicked()),
                     this, SLOT(BeginSlider ()));
    QObject::connect(m_toolButtonEnd, SIGNAL(clicked()), 
                     this, SLOT(EndSlider ()));
    QObject::connect(m_timer, SIGNAL(timeout()),
                     this, SLOT(IncrementSlider ()));
}

void MainWindow::TooglePlay ()
{
    if (m_play)
    {
        m_timer->stop ();
        m_toolButtonPlayPause->setText (PLAY_TEXT);
        enableBegin (true);
        enableEnd (true);
    }
    else
    {
        m_timer->start ();
        m_toolButtonPlayPause->setText (PAUSE_TEXT);
        enableBegin (false);
        enableEnd (false);
    }
    m_play = ! m_play;
}

void MainWindow::BeginSlider ()
{
    m_dataSlider->setValue (m_dataSlider->minimum ());
    enableBegin (true);
    enableEnd (true);
}

void MainWindow::EndSlider ()
{
    m_dataSlider->setValue (m_dataSlider->maximum ());
    enableBegin (true);
    enableEnd (true);
}

void MainWindow::IncrementSlider ()
{
    int value = m_dataSlider->value ();
    if (value < m_dataSlider->maximum ())
        m_dataSlider->setValue (value + 1);
    else
        TooglePlay ();
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
