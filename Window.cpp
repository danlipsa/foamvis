#include <QtGui>

#include "GLWidget.h"
#include "Window.h"

Window::Window(Data* data)
{
    setupUi (this);
    m_glWidget->SetData (data);
}

