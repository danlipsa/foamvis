#include <math.h>
#include <algorithm>
#include <stdexcept>

#include <QtGui>
#include <QtOpenGL>

#include <G3D/Vector3.h>
#include "GLWidget.h"
#include "Vertex.h"
#include "Data.h"
using namespace std;
using namespace G3D;

inline void deleteObject (GLuint object)
{
    glDeleteLists(object, 1);
}

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent), m_viewType (BODIES), 
      m_object(VIEW_TYPE_NUMBER, 0), m_rotation (3, 0),
      m_accumulator (3, 0), m_data(0)
{}

GLWidget::~GLWidget()
{
    makeCurrent();
    for_each (m_object.begin (), m_object.end (), deleteObject);
}

void GLWidget::ViewVertices (bool checked)
{
    if (checked)
	m_viewType = VERTICES;
    initLightFlat ();
    updateGL ();
}

void GLWidget::ViewEdges (bool checked)
{
    if (checked)
	m_viewType = EDGES;
    initLightFlat ();
    updateGL ();
}

void GLWidget::ViewFaces (bool checked)
{
    if (checked)
	m_viewType = FACES;
    initLightFlat ();
    updateGL ();
}

void GLWidget::ViewBodies (bool checked)
{
    if (checked)
	m_viewType = BODIES;
    initLightBodies ();
    updateGL ();
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(400, 400);
}


void GLWidget::SetXRotation(int angle)
{
    setRotationSlot (0, angle);
}

void GLWidget::SetYRotation(int angle)
{
    setRotationSlot (1, angle);
}

void GLWidget::SetZRotation(int angle)
{
    setRotationSlot (2, angle);
}

void GLWidget::setRotationSlot (int axis, int angle)
{
    normalizeAngle(&angle);
    int rotation = getRotation (axis);
    if (angle != rotation) {
        setRotation (axis, angle);
        emitRotationChanged (axis, angle);
        updateGL();
    }
}

void GLWidget::emitRotationChanged (int axis, int angle)
{
    switch (axis)
    {
    case 0:
	emit XRotationChanged (angle);
	return;
    case 1:
	emit YRotationChanged (angle);
	return;
    case 2:
	emit ZRotationChanged (angle);
	return;
    default:
	throw domain_error ("Invalid axis: " + axis);
    }
}

void GLWidget::initLightBodies ()
{
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    //glEnable(GL_CULL_FACE);

    GLfloat mat_specular_front[] = { 1.0, 0.0, 0.0, 1.0 };
    GLfloat mat_shininess_front[] = { 50.0 };

    GLfloat mat_specular_back[] = { 0.0, 1.0, 0.0, 1.0 };
    GLfloat mat_shininess_back[] = { 50.0 };


    GLfloat light_position[] = { 2.0, 2.0, 2.0, 0.0 };
    glShadeModel (GL_SMOOTH);

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular_front);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess_front);
    glMaterialfv(GL_BACK, GL_SPECULAR, mat_specular_back);
    glMaterialfv(GL_BACK, GL_SHININESS, mat_shininess_back);


    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glLoadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glPopMatrix ();
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
}

void GLWidget::initLightFlat ()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
}

void GLWidget::initializeGL()
{
    m_object[VERTICES] = displayVertices();
    m_object[EDGES] = displayEdges ();
    m_object[FACES] = displayFaces ();
    m_object[BODIES] = displayBodies ();
    qglClearColor (QColor(Qt::cyan));        
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.5, 1.5, -1.5, 1.5, -1.5, 1.5);

    glMatrixMode (GL_MODELVIEW);
    glLoadMatrixf (m_data->GetViewMatrix ());

    switch (m_viewType)
    {
    case VERTICES:
    case EDGES:
    case FACES:
	initLightFlat ();
	break;
    case BODIES:
	GLWidget::initLightBodies ();
	break;
    default:
	throw domain_error (
	    "ViewType enum has an invalid value: " + m_viewType);
    }
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glRotated(getRotation (0), 1.0, 0.0, 0.0);
    glRotated(getRotation (1), 0.0, 1.0, 0.0);
    glRotated(getRotation (2), 0.0, 0.0, 1.0);
    qglColor (QColor(Qt::black));
    glCallList(m_object[m_viewType]);
}

void GLWidget::resizeGL(int width, int height)
{
    int side = std::min (width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    setAccumulator (0, 0);
    if (event->buttons() & Qt::LeftButton) {
	setAccumulator (1, 0);
    } else if (event->buttons() & Qt::RightButton) {
	setAccumulator (2, 0);
    }
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();
    // scale this with the size of the window
    QSize size = this->size ();
    int side = std::min (size.width (), size.height ());
    float dxDegrees = static_cast<float>(dx) * 90 / side;
    float dyDegrees = static_cast<float>(dy) * 90 / side;

    if (event->buttons() & Qt::LeftButton) {
        accumulate (0, dyDegrees);
        accumulate (1, dxDegrees);
    } else if (event->buttons() & Qt::RightButton) {
        accumulate (0, dyDegrees);
        accumulate (2, dxDegrees);
    }
    m_lastPos = event->pos();
}

void GLWidget::accumulate (int axis, float value)
{
    float accumulator = getAccumulator (axis) + value;
    if (accumulator >= 0)
    {
	int intPart = static_cast<int>(floorf(accumulator));
	if (intPart > 0)
	{
	    setRotationSlot (axis, getRotation (axis) + intPart);
	    accumulator -= intPart;
	}
    }
    else
    {
	accumulator = - accumulator;
	int intPart = static_cast<int>(floorf(accumulator));
	if (intPart > 0)
	{
	    setRotationSlot (axis, getRotation (axis) - intPart);
	    accumulator -= intPart;
	}
	accumulator = - accumulator;
    }
    setAccumulator (axis, accumulator);
}

inline void displayVertex (const Vertex* p)
{
    if (p != 0)
	glVertex3f(p->GetX (), p->GetY (), p->GetZ ());
}

inline void displayFirstVertex (const OrientedEdge* e)
{
    const Vertex* p = e->GetBegin ();
    glVertex3f(p->GetX (), p->GetY (), p->GetZ ());
}


void displayEdge (const Edge* e)
{
    if (e != 0)
    {
	const Vertex* p = e->GetBegin ();
	glVertex3f(p->GetX (), p->GetY (), p->GetZ ());
	p = e->GetEnd ();
	glVertex3f(p->GetX (), p->GetY (), p->GetZ ());
    }
}

void displayOrientedFace (const OrientedFace* f)
{
    const vector<OrientedEdge*> v = f->GetFace()->GetOrientedEdges ();
    // specify the normal vector
    const Vertex* begin = v[0]->GetBegin ();
    const Vertex* end = v[0]->GetEnd ();
    if (f->IsReversed ())
	swap (begin, end);
    Vector3 first(end->GetX () - begin->GetX (),
		  end->GetY () - begin->GetY (),
		  end->GetZ () - begin->GetZ ());
    begin = v[1]->GetBegin ();
    end = v[1]->GetEnd ();
    if (f->IsReversed ())
	swap (begin, end);
    Vector3 second(end->GetX () - begin->GetX (),
		   end->GetY () - begin->GetY (),
		   end->GetZ () - begin->GetZ ());
    Vector3 normal (first.cross(second).unit ());
    glNormal3f (normal.x, normal.y, normal.z);

    // specify the vertices for the triangle
    for_each (v.begin (), v.end (), displayFirstVertex);
}

struct displayFaceWithContur : public unary_function<const Face*, void>
{
    displayFaceWithContur (GLWidget& widget) : 
	m_widget (widget) {}

    void operator() (const Face* f)
    {
	m_widget.qglColor (QColor(f->GetColor ()));
	const vector<OrientedEdge*> v = f->GetOrientedEdges ();
	for_each (v.begin (), v.end (), displayFirstVertex);
    }
private:
    GLWidget& m_widget;
};

void displayBody (Body* b)
{
    const vector<OrientedFace*> v = b->GetOrientedFaces ();
    for_each (v.begin (), v.end (), displayOrientedFace);
}

GLuint GLWidget::displayVertices ()
{
    GLuint list = glGenLists(1);
    const vector<Vertex*>& points = m_data->GetPoints ();
    glNewList(list, GL_COMPILE);
    glBegin(GL_POINTS);
    for_each (points.begin (), points.end (), displayVertex);
    glEnd();
    glEndList();
    return list;
}

GLuint GLWidget::displayEdges ()
{
    GLuint list = glGenLists(1);
    const vector<Edge*>& edges = m_data->GetEdges ();
    glNewList(list, GL_COMPILE);
    glBegin(GL_LINES);
    for_each (edges.begin (), edges.end (), displayEdge);
    glEnd();
    glEndList();
    return list;
}

GLuint GLWidget::displayFaces ()
{
    GLuint list = glGenLists(1);
    const vector<Face*>& faces = m_data->GetFaces ();
    QColor black(Qt::black);
    glNewList(list, GL_COMPILE);
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_TRIANGLES);
    for_each (faces.begin (), faces.end (), 
	      displayFaceWithContur (*this));
    glEnd ();
    glEndList();
    return list;
}

GLuint GLWidget::displayBodies ()
{
    GLuint list = glGenLists(1);
    const vector<Body*>& bodies = m_data->GetBodies ();
    glNewList(list, GL_COMPILE);
    glBegin (GL_TRIANGLES);
    for_each (bodies.begin (), bodies.end (), displayBody);
    glEnd ();
    glEndList();
    return list;
}


void GLWidget::normalizeAngle(int *angle)
{
    while (*angle < 0)
        *angle += 360;
    while (*angle > 360)
        *angle -= 360;
}
