#include <G3D/G3DAll.h>
#include <QtGui>
#include "GLWidget.h"
#include "MainWindow.h"
#include "SystemDifferences.h"
#include "DebugStream.h"
using namespace std;

MainWindow::MainWindow(vector<Data*>& data)
{
    setupUi (this);
    m_dataSlider->setMinimum (0);
    m_dataSlider->setMaximum (data.size () - 1);
    m_dataSlider->setSingleStep (1);
    m_dataSlider->setPageStep (10);
    m_glWidget->SetData (data);
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
