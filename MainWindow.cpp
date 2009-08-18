#include <G3D/G3DAll.h>
#include <QtGui>
#include "GLWidget.h"
#include "MainWindow.h"
#include "SystemDifferences.h"

MainWindow::MainWindow(Data* data)
{
    setupUi (this);
    m_glWidget->SetData (data);
}

void MainWindow::keyPressEvent (QKeyEvent* event)
{
    switch (event->key ())
    {
    case Qt::Key_PageUp:
        m_glWidget->IncrementDisplayedBody ();
        break;
    case Qt::Key_PageDown:
        m_glWidget->DecrementDisplayedBody ();
        break;
    case Qt::Key_Space:
        string s = G3D::getOpenGLState (false);
        OutputDebugStringA (s.c_str ());
    }
}
