#include <QtGui>

#include "GLWidget.h"
#include "Window.h"

Window::Window(Data& data)
{
    m_glWidget = new GLWidget (data);

    xSlider = createSlider();
    ySlider = createSlider();
    zSlider = createSlider();

    connect(xSlider, SIGNAL(valueChanged(int)), 
	    m_glWidget, SLOT(setXRotation(int)));
    connect(m_glWidget, SIGNAL(xRotationChanged(int)), 
	    xSlider, SLOT(setValue(int)));

    connect(ySlider, SIGNAL(valueChanged(int)), 
	    m_glWidget, SLOT(setYRotation(int)));
    connect(m_glWidget, SIGNAL(yRotationChanged(int)), 
	    ySlider, SLOT(setValue(int)));

    connect(zSlider, SIGNAL(valueChanged(int)),
	    m_glWidget, SLOT(setZRotation(int)));
    connect(m_glWidget, SIGNAL(zRotationChanged(int)),
	    zSlider, SLOT(setValue(int)));

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_glWidget);
    mainLayout->addWidget(xSlider);
    mainLayout->addWidget(ySlider);
    mainLayout->addWidget(zSlider);
    setLayout(mainLayout);

    xSlider->setValue(15 * 16);
    ySlider->setValue(345 * 16);
    zSlider->setValue(0 * 16);
    setWindowTitle(tr("Hello GL"));
}

QSlider *Window::createSlider()
{
    QSlider *slider = new QSlider(Qt::Vertical);
    slider->setRange(0, 360 * 16);
    slider->setSingleStep(16);
    slider->setPageStep(15 * 16);
    slider->setTickInterval(15 * 16);
    slider->setTickPosition(QSlider::TicksRight);
    return slider;
}
