#include <QtGui>

#include "GLWidget.h"
#include "MainWindow.h"

MainWindow::MainWindow(Data* data)
{
    setupUi (this);
    m_glWidget->SetData (data);
}

