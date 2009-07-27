#include <QtGui>
#include <QtOpenGL>

#include <math.h>
#include <algorithm>
#include "GLWidget.h"
#include "Point.h"
#include "Data.h"

using namespace std;

GLWidget::GLWidget(Data& data, QWidget *parent)
    : QGLWidget(parent), m_data(data)
{
    m_objectVertices = 0;
    m_objectEdges = 0;
    m_objectFacets = 0;
    m_xRot = 0;
    m_yRot = 0;
    m_zRot = 0;
}

GLWidget::~GLWidget()
{
    makeCurrent();
    glDeleteLists(m_objectVertices, 1);
    glDeleteLists(m_objectEdges, 1);
    glDeleteLists(m_objectFacets, 1);
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(400, 400);
}

void GLWidget::setXRotation(int angle)
{
    normalizeAngle(&angle);
    if (angle != m_xRot) {
        m_xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}

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

void GLWidget::initializeGL()
{
    m_objectVertices = makeVertices();
    m_objectEdges = makeEdges ();
    m_objectFacets = makeFacets ();
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.5, 1.5, -1.5, 1.5, -1.5, 1.5);

    glMatrixMode (GL_MODELVIEW);
    glLoadMatrixf (m_data.GetViewMatrix ());
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glRotated(m_xRot / 16.0, 1.0, 0.0, 0.0);
    glRotated(m_yRot / 16.0, 0.0, 1.0, 0.0);
    glRotated(m_zRot / 16.0, 0.0, 0.0, 1.0);
    glCallList(m_objectFacets);
}

void GLWidget::resizeGL(int width, int height)
{
    int side = min (width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
}

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

inline void displayVertex (const Point* p)
{
    if (p != 0)
	glVertex3f(p->GetX (), p->GetY (), p->GetZ ());
}

void displayEdge (Edge* e)
{
    if (e != 0)
    {
	const Point* p = e->GetBegin ();
	glVertex3f(p->GetX (), p->GetY (), p->GetZ ());
	p = e->GetEnd ();
	glVertex3f(p->GetX (), p->GetY (), p->GetZ ());
    }
}

void displayFacet (Facet* f)
{
    if (f != 0)
    {
    }
}


GLuint GLWidget::makeVertices ()
{
    GLuint list = glGenLists(1);
    const vector<Point*>& points = m_data.GetPoints ();
    glNewList(list, GL_COMPILE);
    glBegin(GL_POINTS);
    for_each (points.begin (), points.end (), displayVertex);
    glEnd();
    glEndList();
    return list;
}

GLuint GLWidget::makeEdges ()
{
    GLuint list = glGenLists(1);
    const vector<Edge*>& edges = m_data.GetEdges ();
    glNewList(list, GL_COMPILE);
    glBegin(GL_LINES);
    for_each (edges.begin (), edges.end (), displayEdge);
    glEnd();
    glEndList();
    return list;
}

GLuint GLWidget::makeFacets ()
{
    GLuint list = glGenLists(1);
    const vector<Facet*>& facets = m_data.GetFacets ();
    glNewList(list, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for_each (facets.begin (), facets.end (), displayFacet);
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
