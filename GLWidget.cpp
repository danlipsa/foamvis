/**
 * @file   GLWidget.cpp
 * @author Dan R. Lipsa
 *
 * Definitions for the widget for displaying foam bubbles using OpenGL
 */
#include "GLWidget.h"
#include "DataFiles.h"
#include "DebugStream.h"
#include "DisplayVertexFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "DisplayFaceFunctors.h"
#include "DisplayBodyFunctors.h"

/**
 * Stores information about various OpenGL characteristics of the graphic card
 */
struct OpenGLParam
{
    /**
     * What OpenGL characteristic
     */
    GLenum m_what;
    /**
     * Where to store information about that characteristic
     */
    GLint* m_where;
    /**
     * Name of the OpenGL characteristic
     */
    const char* m_name;

    /**
     * Reads an OpenGLParam from OpenGL
     */
    void get ()
    {
	glGetIntegerv (m_what, m_where);
    }

    /**
     * Prints an OpenGLParam
     */
    void print ()
    {
	cdbg << m_name << ": " << *m_where << endl;
    }
};

/**
 * Dealocates the space occupied by  an old OpenGL object and stores a
 * newObject
 *
 * @param object address where the  old object is stored and where the
 * new object will be stored
 * @param newObject the new object that will be stored
 */
inline void setObject (GLuint* object, GLuint newObject)
{
    glDeleteLists(*object, 1);
    *object = newObject;
}

/**
 * Check the OpenGL  error code and prints a message  to cdbg if there
 * is an error
 */
void detectOpenGLError ()
{
    GLenum errCode;
    if ((errCode = glGetError()) != GL_NO_ERROR)
        cdbg << "OpenGL Error: " << gluErrorString(errCode) << endl;
}

/**
 * Prints information  about the OpenGL  implementation (hardware) the
 * program runs on.
 */
void printOpenGLInfo ()
{
    GLboolean stereoSupport;
    GLboolean doubleBufferSupport;
    GLint auxBuffers;
    GLint redBits, greenBits, blueBits, alphaBits;
    GLint redBitsAccum, greenBitsAccum, blueBitsAccum, alphaBitsAccum;
    GLint indexBits;
    GLint depthBits;
    GLint stencilBits;
    boost::array<OpenGLParam, 12> info = {{
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
    }};
    glGetBooleanv (GL_STEREO, &stereoSupport);
    glGetBooleanv (GL_DOUBLEBUFFER, &doubleBufferSupport);
    for_each (info.begin (), info.end (), mem_fun_ref(&OpenGLParam::get));
    cdbg << "OpenGL" << endl
         << "Vendor: " << glGetString (GL_VENDOR) << endl
         << "Renderer: " << glGetString (GL_RENDERER) << endl
         << "Version: " << glGetString (GL_VERSION) << endl
         << "Extensions: " << glGetString (GL_EXTENSIONS) << endl
         << "Stereo support: " << static_cast<bool>(stereoSupport) << endl
         << "Double buffer support: " 
         << static_cast<bool>(doubleBufferSupport) << endl;
    for_each (info.begin (), info.end (), mem_fun_ref(&OpenGLParam::print));
}

 float GLWidget::OBJECTS_WIDTH[] = {0.0, 1.0, 3.0, 5.0, 7.0};

 unsigned int GLWidget::DISPLAY_ALL(numeric_limits<unsigned int>::max());

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent), 
      m_viewType (BODIES),
      m_object(0),
      m_dataFiles(0), m_dataIndex (0),
      m_displayedBody(DISPLAY_ALL), m_displayedFace(DISPLAY_ALL),
      m_saveMovie(false), m_currentFrame(0),
      m_physicalObjectsWidth (1), 
      m_physicalObjectsColor (Qt::blue),
      m_tessellationObjectsWidth (1),
      m_tessellationObjectsColor (Qt::green),
      m_centerPathColor (Qt::red)
{
}

GLWidget::~GLWidget()
{
    makeCurrent();
    glDeleteLists(m_object, 1);
}



// Slots
// =====

void GLWidget::ViewVertices (bool checked)
{
    if (checked)
    {
        m_viewType = VERTICES;
        setObject (&m_object, displayVertices ());
	initLightFlat ();
	updateGL ();
    }
}

void GLWidget::ViewEdges (bool checked)
{
    if (checked)
    {
        m_viewType = EDGES;
        setObject (&m_object, displayEdges ());
	initLightFlat ();
	updateGL ();
    }
}

void GLWidget::ViewFaces (bool checked)
{
    if (checked)
    {
        m_viewType = FACES;
        setObject (&m_object, displayFaces ());
	initLightFlat ();
	updateGL ();
    }
}

void GLWidget::ViewRawVertices (bool checked)
{
    if (checked)
    {
        m_viewType = RAW_VERTICES;
        setObject (&m_object, displayRawVertices ());
	initLightFlat ();
	updateGL ();
    }
}

void GLWidget::ViewRawEdges (bool checked)
{
    if (checked)
    {
        m_viewType = RAW_EDGES;
        setObject (&m_object, displayRawEdges ());
	initLightFlat ();
	updateGL ();
    }
}

void GLWidget::ViewRawFaces (bool checked)
{
    if (checked)
    {
        m_viewType = RAW_FACES;
        setObject (&m_object, displayRawFaces ());
	initLightFlat ();
	updateGL ();
    }
}



void GLWidget::ViewBodies (bool checked)
{
    if (checked)
    {
        m_viewType = BODIES;
        setObject (&m_object, displayBodies ());
	initLightBodies ();
	updateGL ();
    }
}

void GLWidget::ViewCenterPaths (bool checked)
{
    if (checked)
    {
        m_viewType = CENTER_PATHS;
        setObject (&m_object, displayCenterPaths ());
	initLightFlat ();
	updateGL ();
    }
}


void GLWidget::InteractionModeChanged (int index)
{
    m_interactionMode = static_cast<InteractionMode>(index);
}

void GLWidget::DataSliderValueChanged (int newIndex)
{
    m_dataIndex = newIndex;
    setObject (&m_object, display(m_viewType));
    updateGL ();
}

void GLWidget::SaveMovie (bool checked)
{
    m_saveMovie = checked;
    updateGL ();
}

void GLWidget::PhysicalObjectsWidthChanged (int value)
{
    m_physicalObjectsWidth = value;
    setObject (&m_object, display(m_viewType));
    updateGL ();
}

void GLWidget::TessellationObjectsWidthChanged (int value)
{
    m_tessellationObjectsWidth = value;
    setObject (&m_object, display(m_viewType));
    updateGL ();
}


// End Slots
// =========

QSize GLWidget::minimumSizeHint() 
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() 
{
    return QSize(512, 512);
}

void GLWidget::initLightBodies ()
{
    const G3D::Vector3& max = m_dataFiles->GetAABox ().high ();
    GLfloat light_position[] = { max.x, max.y, max.z, 0.0 };
    glShadeModel (GL_SMOOTH);

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

void GLWidget::calculateViewingVolume ()
{
    using G3D::Vector3;
    const Vector3& min = m_dataFiles->GetAABox ().low ();
    const Vector3& max = m_dataFiles->GetAABox ().high ();
    float border = ((max - min) / 10).max ();
    m_viewingVolume.set (
	Vector3 (min.x - border, min.y - border, min.z - border),
	Vector3 (max.x + border, max.y + border, max.z + border));
}

void GLWidget::project ()
{
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho(m_viewingVolume.low ().x, m_viewingVolume.high ().x,
	    m_viewingVolume.low ().y, m_viewingVolume.high ().y, 
	    m_viewingVolume.low ().z, m_viewingVolume.high ().z);
}

void GLWidget::initializeGL()
{
    using G3D::Vector3;
    m_object = display (m_viewType);
    float* background = Color::GetValue (Color::WHITE);
    glClearColor (background[0], background[1],
		  background[2], background[3]);        
    calculateViewingVolume ();
    project ();

    glMatrixMode (GL_MODELVIEW);
    //glLoadMatrixf (GetCurrentData ().GetViewMatrix ());

    switch (m_viewType)
    {
    case VERTICES:
    case EDGES:
    case FACES:
    case CENTER_PATHS:
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
    glCallList (m_object);
    detectOpenGLError ();
    if (m_saveMovie)
    {
        ostringstream file;
        file << "movie/frame" << setfill ('0') << setw (4) <<
	    m_currentFrame << ".jpg" << ends;
        QImage snapshot = grabFrameBuffer ();
	string f = file.str ();
	if (! snapshot.save (f.c_str ()))
	    cdbg << "Error saving " << f << endl;
	m_currentFrame++;
    }
}



void GLWidget::resizeGL(int width, int height)
{
    using G3D::Rect2D;
    float ratio = (m_viewingVolume.high ().x - m_viewingVolume.low ().x) / 
	(m_viewingVolume.high ().y - m_viewingVolume.low ().y);
    if ((static_cast<float>(width) / height) > ratio)
    {
	int newWidth = ratio * height;
	m_viewport = Rect2D::xywh ( (width - newWidth) / 2, 0,
				    newWidth, height);
    }
    else
    {
	int newHeight = 1 / ratio * width;
	m_viewport = Rect2D::xywh (0, (height - newHeight) / 2,
				   width, newHeight);
    }
    glViewport (m_viewportStart.x + m_viewport.x0 (),
		m_viewportStart.y + m_viewport.y0 (), 
		m_viewport.width (), m_viewport.height ());
}

void GLWidget::setRotation (int axis, float angle)
{
    using G3D::Matrix4;
    float axes[3][3] = {{1,0,0}, {0,1,0}, {0,0,1}};
    makeCurrent ();
    glMatrixMode (GL_MODELVIEW);
    Matrix4 modelView;
    glGetMatrix (GL_MODELVIEW_MATRIX, modelView);
    const Matrix4& columnOrderMatrix = modelView.transpose ();
    glLoadIdentity ();
    glRotatef (angle, axes[axis][0], axes[axis][1], axes[axis][2]);
    glMultMatrixf (columnOrderMatrix);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
}

void scaleAABox (G3D::AABox* aabox, float change)
{
    using G3D::Vector3;
    Vector3 center = aabox->center ();
    Vector3 newLow = aabox->low ()*change + center * (1 - change);
    Vector3 newHigh = aabox->high ()*change + center * (1 - change);
    aabox->set (newLow, newHigh);
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    using G3D::Vector2;
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();
    switch (m_interactionMode)
    {
    case ROTATE:
    {
	// scale this with the size of the window
	int side = std::min (m_viewport.width (), m_viewport.height ());
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
	break;
    }
    case TRANSLATE:
    {
	m_viewportStart.x += dx;
	m_viewportStart.y -= dy;
	resizeGL (width (), height ());
	updateGL ();	
	break;
    }
    case SCALE:
    {
	Vector2 center = m_viewport.center () + m_viewportStart;
	Vector2 lastPos (m_lastPos.x (), m_lastPos.y());
	Vector2 currentPos (event->x (), event->y ());
	float change = (currentPos - center).length () - 
	    (lastPos - center).length ();
	change /= std::min(m_viewport.width (), m_viewport.height ());
	change = powf (2, - change);
	scaleAABox (&m_viewingVolume, change);
	project ();
	updateGL ();
	break;
    }
    default:
	break;
    }
    m_lastPos = event->pos();
}


void GLWidget::displayFacesContour (vector<Body*>& bodies)
{
    qglColor (QColor(Qt::black));
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    for_each (bodies.begin (), bodies.end (),
              displayBodyWithFace (*this,
				   displayFace<displaySameEdges>(*this)));
}

void GLWidget::displayFacesOffset (vector<Body*>& bodies)
{
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1, 1);
    for_each (bodies.begin (), bodies.end (),
              displayBodyWithFace (*this, displayFaceWithColor(*this)));
    glDisable (GL_POLYGON_OFFSET_FILL);
}

GLuint GLWidget::displayVertices ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    vector<Body*>& bodies = GetCurrentData ().GetBodies ();
    for_each (bodies.begin (), bodies.end (),
	      displayBodyWithFace (
		  *this, displayFace<displayDifferentVertices>(*this)));
    glPointSize (1.0);
    glEndList();
    return list;
}

void displayOriginalVertex (Vertex* v)
{
    if (! v->IsDuplicate ())
    {
	glVertex3f(v->x, v->y, v->z);	
    }
}

void displayOriginalDomainFaces (G3D::Vector3 first,
				 G3D::Vector3 second,
				 const G3D::Vector3& third)
{
    G3D::Vector3 origin(0, 0, 0);
    G3D::Vector3 sum = first + second;
    for (int i = 0; i < 2; i++)
    {
	glBegin (GL_POLYGON);
	glVertex3f (origin.x, origin.y, origin.z);
	glVertex3f (first.x, first.y, first.z);
	glVertex3f (sum.x, sum.y, sum.z);
	glVertex3f (second.x, second.y, second.z);
	glEnd ();
	origin += third;
	first += third;
	second += third;
	sum += third;
    }
}


void displayOriginalDomain (const G3D::Vector3* periods)
{
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth (1.0);
    displayOriginalDomainFaces (periods[0], periods[1], periods[2]);
    displayOriginalDomainFaces (periods[1], periods[2], periods[0]);
    displayOriginalDomainFaces (periods[2], periods[0], periods[1]);
}


GLuint GLWidget::displayRawVertices ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glPointSize (3.0);
    glBegin (GL_POINTS);
    vector<Vertex*>& vertices = GetCurrentData ().GetVertices ();
    for_each (vertices.begin (), vertices.end (), displayOriginalVertex);
    glEnd ();
    glPointSize (1.0);

    displayOriginalDomain (GetCurrentData().GetPeriods ());
    glEndList();
    return list;
}

void displayOriginalEdge (Edge* edge)
{
    if (! edge->IsDuplicate ())
    {
	const Vertex& b = *(edge->GetBegin ());
	const Vertex& e = *(edge->GetEnd ());
	glVertex3f (b.x, b.y, b.z);
	glVertex3f (e.x, e.y, e.z);
    }
}

GLuint GLWidget::displayRawEdges ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    qglColor (QColor (Qt::black));
    glLineWidth (3.0);
    glBegin (GL_LINES);
    vector<Edge*>& edges = GetCurrentData ().GetEdges ();
    for_each (edges.begin (), edges.end (), displayOriginalEdge);
    glEnd ();
    glLineWidth (1.0);

    displayOriginalDomain (GetCurrentData().GetPeriods ());
    glEndList();
    return list;
}

GLuint GLWidget::displayRawFaces ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    qglColor (QColor (Qt::black));
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth (3.0);
    vector<Face*>& faces = GetCurrentData ().GetFaces ();
    for_each (faces.begin (), faces.end (), displayFace<> (*this) );
    
    glLineWidth (1.0);

    displayOriginalDomain (GetCurrentData().GetPeriods ());
    glEndList();
    return list;
}



GLuint GLWidget::displayEdges ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    vector<Body*>& bodies = GetCurrentData ().GetBodies ();
    for_each (bodies.begin (), bodies.end (),
	      displayBodyWithFace(
		  *this, displayFace<displayDifferentEdges>(*this)));

    if (! GetCurrentData ().IsTorus ())
	displayCenterOfBodies ();

    glLineWidth (1.0);
    glEndList();
    return list;
}

void GLWidget::displayCenterOfBodies ()
{
    glPointSize (4.0);
    qglColor (QColor (Qt::red));
    glBegin(GL_POINTS);
     vector<Body*>& bodies = GetCurrentData ().GetBodies ();
    for_each (bodies.begin (),bodies.end (), displayBodyCenter (*this));
    glEnd ();
}



/**
 * Displays the center path for a certain body
 */
class displayCenterPath
{
public:
    /**
     * Constructor
     * @param widget where to display the center path
     */
    displayCenterPath (GLWidget& widget) : m_widget (widget) {}
    /**
     * Displays the center path for a certain body
     * @param index what body to display the center path for
     */
    void operator () (unsigned int index)
    {
	glBegin(GL_LINE_STRIP);
	vector<Data*>& data = m_widget.GetDataFiles ().GetData ();
	for_each (data.begin (), data.end (), 
		  displayBodyCenterFromData (m_widget, index));
	glEnd ();
    }
    /**
     * Helper function which calls operator () (unsigned int index).
     * @param p a pair original index body pointer
     */
    inline void operator () (
	pair<unsigned int,  Body*> p) {operator() (p.first);}

private:
    /**
     * Where to display the center path
     */
    GLWidget& m_widget;
};


GLuint GLWidget::displayCenterPaths ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    qglColor (QColor (Qt::black));
     map<unsigned int, Body*>& originalIndexBodyMap = 
	GetDataFiles ().GetData ()[0]->GetOriginalIndexBodyMap ();
    if (GetDisplayedBody () == DISPLAY_ALL)
	for_each (originalIndexBodyMap.begin (), originalIndexBodyMap.end (),
		  displayCenterPath (*this));
    else
    {
	map<unsigned int, Body*>::const_iterator it = 
	    originalIndexBodyMap.find (GetDisplayedBody());
	displayCenterPath (*this) (*it);
    }
    displayCenterOfBodies ();

    glEndList();
    return list;
}

GLuint GLWidget::displayFaces ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);

    vector<Body*>& bodies = GetCurrentData ().GetBodies ();
    displayFacesContour (bodies);
    displayFacesOffset (bodies);

    glEndList();
    return list;
}

GLuint GLWidget::displayBodies ()
{
    GLuint list = glGenLists(1);
    vector<Body*>& bodies = GetCurrentData ().GetBodies ();
    glNewList(list, GL_COMPILE);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    for_each (bodies.begin (), bodies.end (),
              displayBodyWithFace(*this, displayFaceWithNormal(*this)));
    glEndList();
    return list;
}

GLuint GLWidget::display (ViewType type)
{
    
    switch (type)
    {
    case VERTICES:
        return displayVertices ();
    case EDGES:
        return displayEdges ();
    case FACES:
        return displayFaces ();
    case BODIES:
        return displayBodies ();
    case CENTER_PATHS:
	return displayCenterPaths ();
    case RAW_VERTICES:
	return displayRawVertices ();
    case RAW_EDGES:
	return displayRawEdges ();
    case RAW_FACES:
	return displayRawFaces ();
    default:
        throw domain_error (
            "ViewType enum has an invalid value: " + m_viewType);
    }
}

void GLWidget::IncrementDisplayedFace ()
{
    m_displayedFace++;
    if (m_viewType == RAW_FACES)
    {
	if (m_displayedFace == 	GetCurrentData ().GetFaces ().size ())
	    m_displayedFace = DISPLAY_ALL;
    }
    if (m_displayedBody != DISPLAY_ALL)
    {
        Body& body = *GetCurrentData ().GetBodies ()[m_displayedBody];
        if (m_displayedFace == body.GetOrientedFaces ().size ())
            m_displayedFace = DISPLAY_ALL;
    }
    setObject (&m_object, display(m_viewType));
    updateGL ();
}

void GLWidget::DecrementDisplayedFace ()
{
    if (m_viewType == RAW_FACES)
    {
	if (m_displayedFace == DISPLAY_ALL)
	    m_displayedFace = GetCurrentData ().GetFaces ().size ();
    }
    if (m_displayedBody != DISPLAY_ALL)
    {
        Body& body = *GetCurrentData ().GetBodies ()[m_displayedBody];
        if (m_displayedFace == DISPLAY_ALL)
            m_displayedFace = body.GetOrientedFaces ().size ();
    }
    m_displayedFace--;
    setObject (&m_object, display(m_viewType));
    updateGL ();

}

void GLWidget::IncrementDisplayedBody ()
{
    if (m_viewType == RAW_VERTICES || m_viewType == RAW_EDGES ||
	m_viewType == RAW_FACES)
	return;
    m_displayedBody++;
    m_displayedFace = DISPLAY_ALL;
    if (m_displayedBody == GetDataFiles ().GetData ()[0]->GetBodies ().size ())
        m_displayedBody = DISPLAY_ALL;
    setObject (&m_object, display(m_viewType));
    updateGL ();
    cdbg << "displayed body: " << m_displayedBody << endl;
}

void GLWidget::DecrementDisplayedBody ()
{
    if (m_viewType == RAW_VERTICES || m_viewType == RAW_EDGES ||
	m_viewType == RAW_FACES)
	return;
    if (m_displayedBody == DISPLAY_ALL)
        m_displayedBody = GetDataFiles ().GetData ()[0]->GetBodies ().size ();
    m_displayedBody--;
    m_displayedFace = DISPLAY_ALL;
    setObject (&m_object, display(m_viewType));
    updateGL ();
    cdbg << "displayed body: " << m_displayedBody << endl;
}


Data& GLWidget::GetCurrentData ()
{
    return *m_dataFiles->GetData ()[m_dataIndex];
}
