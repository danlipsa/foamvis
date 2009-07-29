#include <math.h>
#include <algorithm>
#include <QtGui>
#include <QtOpenGL>
#include <G3D/Vector3.h>
#include "GLWidget.h"
#include "Point.h"
#include "Data.h"

using namespace std;
using namespace G3D;

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent)
{
    m_data = 0;
    m_objectVertices = 0;
    m_objectEdges = 0;
    m_objectFaces = 0;
    m_objectBodies = 0;
    m_xRot = 0;
    m_yRot = 0;
    m_zRot = 0;
}

GLWidget::~GLWidget()
{
    makeCurrent();
    glDeleteLists(m_objectVertices, 1);
    glDeleteLists(m_objectEdges, 1);
    glDeleteLists(m_objectFaces, 1);
    glDeleteLists(m_objectBodies, 1);
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
    normalizeAngle(&angle);
    cerr << "SetXRotation: " << angle << endl;
    if (angle != m_xRot) {
        m_xRot = angle;
        emit XRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::SetYRotation(int angle)
{
    normalizeAngle(&angle);
    cerr << "SetXRotation: " << angle << endl;
    if (angle != m_yRot) {
        m_yRot = angle;
        emit YRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::SetZRotation(int angle)
{
    normalizeAngle(&angle);
    cerr << "SetXRotation: " << angle << endl;
    if (angle != m_zRot) {
        m_zRot = angle;
        emit ZRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::initializeGL()
{
    m_objectVertices = makeVertices();
    m_objectEdges = makeEdges ();
    m_objectFaces = makeFaces ();
    m_objectBodies = makeBodies ();

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    //glEnable(GL_CULL_FACE);

    GLfloat mat_specular_front[] = { 1.0, 0.0, 0.0, 1.0 };
    GLfloat mat_shininess_front[] = { 50.0 };

    GLfloat mat_specular_back[] = { 0.0, 1.0, 0.0, 1.0 };
    GLfloat mat_shininess_back[] = { 50.0 };


    GLfloat light_position[] = { 2.0, 2.0, 2.0, 0.0 };
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel (GL_SMOOTH);

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular_front);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess_front);
    glMaterialfv(GL_BACK, GL_SPECULAR, mat_specular_back);
    glMaterialfv(GL_BACK, GL_SHININESS, mat_shininess_back);


    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.5, 1.5, -1.5, 1.5, -1.5, 1.5);

    glMatrixMode (GL_MODELVIEW);
    glLoadMatrixf (m_data->GetViewMatrix ());
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glRotated(m_xRot, 1.0, 0.0, 0.0);
    glRotated(m_yRot, 0.0, 1.0, 0.0);
    glRotated(m_zRot, 0.0, 0.0, 1.0);
    glCallList(m_objectBodies);
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

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();
    // scale this with the size of the window
    

    if (event->buttons() & Qt::LeftButton) {
        SetXRotation(m_xRot + 8 * dy);
        SetYRotation(m_yRot + 8 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        SetXRotation(m_xRot + 8 * dy);
        SetZRotation(m_zRot + 8 * dx);
    }
    m_lastPos = event->pos();
}

inline void displayVertex (const Point* p)
{
    if (p != 0)
	glVertex3f(p->GetX (), p->GetY (), p->GetZ ());
}

inline void displayFirstVertex (const OrientedEdge* e)
{
    const Point* p = e->GetBegin ();
    glVertex3f(p->GetX (), p->GetY (), p->GetZ ());
}


void displayEdge (const Edge* e)
{
    if (e != 0)
    {
	const Point* p = e->GetBegin ();
	glVertex3f(p->GetX (), p->GetY (), p->GetZ ());
	p = e->GetEnd ();
	glVertex3f(p->GetX (), p->GetY (), p->GetZ ());
    }
}

struct displayFace : public unary_function<void, const Face*>
{
    displayFace (bool reversed) : m_reversed (reversed) {}

    void operator() (const Face* f)
    {
	if (f != 0)
	{
	    const vector<OrientedEdge*> v = f->GetOrientedEdges ();
	    // specify the normal vector
	    const Point* begin = v[0]->GetBegin ();
	    const Point* end = v[0]->GetEnd ();
	    if (m_reversed)
		swap (begin, end);
	    Vector3 first(end->GetX () - begin->GetX (),
			  end->GetY () - begin->GetY (),
			  end->GetZ () - begin->GetZ ());
	    begin = v[1]->GetBegin ();
	    end = v[1]->GetEnd ();
	    if (m_reversed)
		swap (begin, end);
	    Vector3 second(end->GetX () - begin->GetX (),
			   end->GetY () - begin->GetY (),
			   end->GetZ () - begin->GetZ ());
	    Vector3 normal (first.cross(second).unit ());
	    glNormal3f (normal.x, normal.y, normal.z);

	    // specify the vertices for the triangle
	    for_each (v.begin (), v.end (), displayFirstVertex);
	}
    }
private:
    bool m_reversed;
};

void displayOrientedFace (OrientedFace* of)
{
    return displayFace (of->IsReversed ()) (of->GetFace());
}


void displayBody (Body* b)
{
    const vector<OrientedFace*> v = b->GetOrientedFaces ();
    for_each (v.begin (), v.end (), displayOrientedFace);
}

GLuint GLWidget::makeVertices ()
{
    GLuint list = glGenLists(1);
    const vector<Point*>& points = m_data->GetPoints ();
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
    const vector<Edge*>& edges = m_data->GetEdges ();
    glNewList(list, GL_COMPILE);
    glBegin(GL_LINES);
    for_each (edges.begin (), edges.end (), displayEdge);
    glEnd();
    glEndList();
    return list;
}

GLuint GLWidget::makeFaces ()
{
    GLuint list = glGenLists(1);
    const vector<Face*>& faces = m_data->GetFaces ();
    glNewList(list, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for_each (faces.begin (), faces.end (), displayFace (false));
    glEnd();
    glEndList();
    return list;
}

GLuint GLWidget::makeBodies ()
{
    GLuint list = glGenLists(1);
    const vector<Body*>& bodies = m_data->GetBodies ();
    glNewList(list, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for_each (bodies.begin (), bodies.end (), displayBody);
    glEnd();
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
