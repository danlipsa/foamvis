#include <QtGui>
#include <QtOpenGL>

#include <math.h>
#include <algorithm>
#include "GLWidget.h"
#include "Point.h"
#include "Data.h"

using namespace std;
//! [0]
GLWidget::GLWidget(Data& data, QWidget *parent)
    : QGLWidget(parent), m_data(data)
{
    m_object = 0;
    m_xRot = 0;
    m_yRot = 0;
    m_zRot = 0;
}
//! [0]

//! [1]
GLWidget::~GLWidget()
{
    makeCurrent();
    glDeleteLists(m_object, 1);
}
//! [1]

//! [2]
QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}
//! [2]

//! [3]
QSize GLWidget::sizeHint() const
//! [3] //! [4]
{
    return QSize(400, 400);
}
//! [4]

//! [5]
void GLWidget::setXRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != m_xRot) {
        m_xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}
//! [5]

void GLWidget::setYRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != m_yRot) {
        m_yRot = angle;
        emit yRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setZRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != m_zRot) {
        m_zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}

//! [6]
void GLWidget::initializeGL()
{
    m_object = makeObject();
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}
//! [6]

//! [7]
void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslated(0.0, 0.0, -10.0);
    glRotated(m_xRot / 16.0, 1.0, 0.0, 0.0);
    glRotated(m_yRot / 16.0, 0.0, 1.0, 0.0);
    glRotated(m_zRot / 16.0, 0.0, 0.0, 1.0);
    glCallList(m_object);
}
//! [7]

//! [8]
void GLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, +0.5, +0.5, -0.5, 4.0, 15.0);
    glMatrixMode(GL_MODELVIEW);
}
//! [8]

//! [9]
void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
}
//! [9]

//! [10]
void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(m_xRot + 8 * dy);
        setYRotation(m_yRot + 8 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(m_xRot + 8 * dy);
        setZRotation(m_zRot + 8 * dx);
    }
    m_lastPos = event->pos();
}
//! [10]

inline void sendPointToGL (Point* p)
{glVertex3d(p->GetX (), p->GetY (), p->GetZ ());}


GLuint GLWidget::makeObject()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    vector<Point*> points = m_data.GetPoints ();
    glBegin(GL_POINTS);
    for_each (points.begin (), points.end (), sendPointToGL);
    glEnd();

    glEndList();
    return list;
}

void GLWidget::normalizeAngle(int *angle)
{
    while (*angle < 0)
        *angle += 360 * 16;
    while (*angle > 360 * 16)
        *angle -= 360 * 16;
}
