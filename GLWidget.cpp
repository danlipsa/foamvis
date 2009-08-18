#include <math.h>
#include <limits.h>

#include <G3D/G3DAll.h>
#include <algorithm>
#include <stdexcept>
#include <QtGui>
#include <QtOpenGL>
#include "GLWidget.h"
#include "Vertex.h"
#include "Data.h"
using namespace std;

inline void deleteObject (GLuint object)
{
    glDeleteLists(object, 1);
}

void detectOpenGLError ()
{
    GLenum errCode;
    if ((errCode = glGetError()) != GL_NO_ERROR)
    {
        ostringstream ostr;
        ostr << "OpenGL Error: " << gluErrorString(errCode) << endl << ends;
        OutputDebugStringA (ostr.str ().c_str ());
    }
}

struct OpenGLParam
{
    GLenum m_what;
    GLint* m_where;
    char* m_name;
};
inline void storeOpenGLParam (OpenGLParam& param)
{
    glGetIntegerv (param.m_what, param.m_where);
}

class printOpenGLParam : public unary_function<OpenGLParam&, void>
{
public:
        printOpenGLParam (ostringstream& ostr) : m_ostr (ostr) {}

    void operator() (OpenGLParam& param)
    {
        m_ostr << param.m_name << ": " << *param.m_where << endl;
    }
private:
    ostringstream& m_ostr;
};


void printOpenGLInfo ()
{
    ostringstream ostr;
    GLboolean stereoSupport;
    GLboolean doubleBufferSupport;
    GLint auxBuffers;
    GLint redBits, greenBits, blueBits, alphaBits;
    GLint redBitsAccum, greenBitsAccum, blueBitsAccum, alphaBitsAccum;
    GLint indexBits;
    GLint depthBits;
    GLint stencilBits;
    OpenGLParam info[] = {
        {GL_AUX_BUFFERS, &auxBuffers, "AUX_BUFFERS"},
        {GL_RED_BITS, &redBits, "RED_BITS"},
        {GL_GREEN_BITS, &greenBits, "GREEN_BITS"},
        {GL_BLUE_BITS, &blueBits, "BLUE_BITS"},
        {GL_ALPHA_BITS, &alphaBits, "ALPHA_BITS"},
        {GL_ACCUM_RED_BITS, &redBitsAccum, "ACCUM_RED_BITS"},
        {GL_ACCUM_GREEN_BITS, &greenBitsAccum, "ACCUM_GREEN_BITS"},
        {GL_ACCUM_BLUE_BITS, &blueBitsAccum, "ACCUM_BLUE_BITS"},
        {GL_ACCUM_ALPHA_BITS, &alphaBitsAccum, "ACCUM_ALPHA_BITS"},
        {GL_INDEX_BITS, &indexBits, "INDEX_BITS"},
        {GL_DEPTH_BITS, &depthBits, "DEPTH_BITS"},
        {GL_STENCIL_BITS, &stencilBits, "STENCIL_BITS"},
    };
    glGetBooleanv (GL_STEREO, &stereoSupport);
    glGetBooleanv (GL_DOUBLEBUFFER, &doubleBufferSupport);
    for_each (info, info + sizeof (info) / sizeof (info[0]), storeOpenGLParam);
    ostr << "OpenGL" << endl
         << "Vendor: " << glGetString (GL_VENDOR) << endl
         << "Renderer: " << glGetString (GL_RENDERER) << endl
         << "Version: " << glGetString (GL_VERSION) << endl
         << "Extensions: " << glGetString (GL_EXTENSIONS) << endl
         << "Stereo support: " << static_cast<bool>(stereoSupport) << endl
         << "Double buffer support: " 
         << static_cast<bool>(doubleBufferSupport) << endl;
    for_each (info, info + sizeof (info) / sizeof (info[0]),
              printOpenGLParam (ostr));
    ostr << ends;
    OutputDebugStringA (ostr.str ().c_str ());
}

inline void displayFirstVertex (const OrientedEdge* e)
{
    const Vertex* p = e->GetBegin ();
    glVertex3f(p->GetX (), p->GetY (), p->GetZ ());
}

void displayFaceVertices (const OrientedFace* f)
{
    const vector<OrientedEdge*>& v = f->GetFace()->GetOrientedEdges ();
    if (f->IsReversed ())
        for_each (v.rbegin (), v.rend (), displayFirstVertex);
    else
        for_each (v.begin (), v.end (), displayFirstVertex);
}

class displayFace : public unary_function<const OrientedFace*, void>
{
public:
    displayFace (GLWidget& widget) : 
        m_widget (widget) {}

    void operator() (const OrientedFace* f)
    {
        displayFaceVertices (f);
    }
private:
    GLWidget& m_widget;
};


class displayFaceWithColor : public unary_function<const OrientedFace*, void>
{
public:
    displayFaceWithColor (GLWidget& widget) : 
        m_widget (widget) {}

    void operator() (const OrientedFace* f)
    {
        glColor4fv (Color::GetValue(f->GetFace ()->GetColor ()));
        displayFaceVertices (f);
    }
private:
    GLWidget& m_widget;
};


class displayFaceWithNormal : public unary_function<const OrientedFace*, void>
{
public:
    displayFaceWithNormal (GLWidget& widget) : 
        m_widget (widget) {}

    void operator() (const OrientedFace* f)
    {
        // specify the normal vector
        const Vertex* begin = f->GetBegin (0);
        const Vertex* end = f->GetEnd (0);
        Vector3 first(end->GetX () - begin->GetX (),
                      end->GetY () - begin->GetY (),
                      end->GetZ () - begin->GetZ ());
        begin = f->GetBegin (1);
        end = f->GetEnd (1);
        Vector3 second(end->GetX () - begin->GetX (),
                       end->GetY () - begin->GetY (),
                       end->GetZ () - begin->GetZ ());
        Vector3 normal (first.cross(second).unit ());
        glNormal3f (normal.x, normal.y, normal.z);

        // specify the vertices
        displayFaceVertices (f);
    }
private:
    GLWidget& m_widget;
};


template <class displayFace>
class displayBody : public unary_function<const Body*, void>
{
public:
    displayBody (GLWidget& widget) : m_widget (widget), m_count(0)
    {}

    void operator () (const Body* b)
    {
        unsigned int displayedBody = m_widget.GetDisplayedBody ();
        if ( displayedBody == UINT_MAX ||
             m_count == displayedBody)
        {
            const vector<OrientedFace*> v = b->GetOrientedFaces ();
            for_each (v.begin (), v.end (), displayFace(m_widget));
        }
        m_count++;
    }
private:
    GLWidget& m_widget;
    unsigned int m_count;
};

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent), m_viewType (BODIES), 
      m_object(VIEW_TYPE_NUMBER, 0),
      m_data(0), m_displayedBody(UINT_MAX)
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

void GLWidget::initLightBodies ()
{
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    //glEnable(GL_CULL_FACE);

    GLfloat mat_specular_front[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess_front[] = { 50.0 };

    GLfloat mat_specular_back[] = { 1.0, 0.0, 0.0, 1.0 };
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
    //glEnable(GL_CULL_FACE);
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
        const float* background = Color::GetValue (Color::COLOR_WHITE);
    glClearColor (background[0], background[1], background[2], background[3]);        
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
    printOpenGLInfo ();
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    qglColor (QColor(Qt::black));
    glCallList (m_object[m_viewType]);
    detectOpenGLError ();
}

void GLWidget::resizeGL(int width, int height)
{
    int side = std::min (width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
}

void GLWidget::setRotation (int axis, float angle)
{
    using G3D::Matrix4;
    const float axes[3][3] = {{1,0,0}, {0,1,0}, {0,0,1}};
	makeCurrent ();
	Matrix4 modelView;
    glGetMatrix (GL_MODELVIEW_MATRIX, modelView);
	Matrix4& columnOrderMatrix = modelView.transpose ();
    glLoadIdentity ();
    glRotatef (angle, axes[axis][0], axes[axis][1], axes[axis][2]);
    glMultMatrixf (columnOrderMatrix);
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
    QSize size = this->size ();
    int side = std::min (size.width (), size.height ());
    float dxDegrees = static_cast<float>(dx) * 90 / side;
    float dyDegrees = static_cast<float>(dy) * 90 / side;

    if (event->buttons() & Qt::LeftButton) {
        setRotation (0, dyDegrees);
        setRotation (1, dxDegrees);
    } else if (event->buttons() & Qt::RightButton) {
        setRotation (0, dyDegrees);
        setRotation (2, dxDegrees);
    }
    updateGL ();
    m_lastPos = event->pos();
}

GLuint GLWidget::displayEV (ViewType type)
{
    const vector<Body*>& bodies = m_data->GetBodies ();
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    if (type == VERTICES)
    {
        glPolygonMode (GL_FRONT_AND_BACK, GL_POINT);
        glPointSize (2);
    }
    else
        glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_TRIANGLES);
    for_each (bodies.begin (), bodies.end (),
              displayBody<displayFaceWithColor> (*this));
    glEnd();
    glEndList();
    return list;
}

void GLWidget::displayFacesContour (const vector<Body*>& bodies)
{
    qglColor (QColor(Qt::black));
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    glBegin (GL_TRIANGLES);
    for_each (bodies.begin (), bodies.end (),
              displayBody<displayFace> (*this));
    glEnd ();
}

void GLWidget::displayFacesOffset (const vector<Body*>& bodies)
{
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1, 1);
    glBegin (GL_TRIANGLES);
    for_each (bodies.begin (), bodies.end (),
              displayBody<displayFaceWithColor> (*this));
    glEnd ();
    glDisable (GL_POLYGON_OFFSET_FILL);
}

GLuint GLWidget::displayFaces ()
{
    GLuint list = glGenLists(1);
    const vector<Body*>& bodies = m_data->GetBodies ();
    glNewList(list, GL_COMPILE);
    displayFacesContour (bodies);
    displayFacesOffset (bodies);
    glEndList();
    return list;
}

GLuint GLWidget::displayBodies ()
{
    GLuint list = glGenLists(1);
    const vector<Body*>& bodies = m_data->GetBodies ();
    glNewList(list, GL_COMPILE);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glBegin (GL_TRIANGLES);
    for_each (bodies.begin (), bodies.end (),
              displayBody<displayFaceWithNormal>(*this));
    glEnd ();
    glEndList();
    return list;
}

unsigned int GLWidget::GetDisplayedBody ()
{
    return m_displayedBody;
}

void GLWidget::IncrementDisplayedBody ()
{
    m_displayedBody++;
    if (m_displayedBody == m_data->GetBodies ().size ())
        m_displayedBody = UINT_MAX;
    m_object[FACES] = displayFaces ();
    updateGL ();
    ostringstream ostr;
    ostr << "displayed body: " << m_displayedBody << endl;
    OutputDebugStringA (ostr.str (). c_str ());
}

void GLWidget::DecrementDisplayedBody ()
{
    if (m_displayedBody == UINT_MAX)
        m_displayedBody = m_data->GetBodies ().size ();
    m_displayedBody--;
    m_object[FACES] = displayFaces ();
    updateGL ();
    ostringstream ostr;
    ostr << "displayed body: " << m_displayedBody << endl;
    OutputDebugStringA (ostr.str (). c_str ());
}
