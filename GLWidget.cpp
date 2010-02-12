/**
 * @file   GLWidget.cpp
 * @author Dan R. Lipsa
 *
 * Definitions for the widget for displaying foam bubbles using OpenGL
 */
#include "GLWidget.h"
#include "DataFiles.h"
#include "DebugStream.h"

/**
 * Displays the first vertex in an edge
 * @param e the edge
 */
inline void displayFirstVertex (const OrientedEdge* e)
{
    const Vertex* p = e->GetBegin ();
    glVertex3f(p->x, p->y, p->z);
}
/**
 * Displays all face vertices on the OpenGL canvas
 * @param f the face to be displayed
 */
void displayFaceVertices (const OrientedFace* f)
{
    const vector<OrientedEdge*>& v = f->GetFace()->GetOrientedEdges ();
    if (f->IsReversed ())
        for_each (v.rbegin (), v.rend (), displayFirstVertex);
    else
        for_each (v.begin (), v.end (), displayFirstVertex);
}
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
};
/**
 * Functor that displays a vertex
 */
class displayVertex : public unary_function<const Vertex*, void>
{
public:
    /**
     * Constructor
     * @param widget Where should be the vertex displayed
     */
    displayVertex (GLWidget& widget) : 
	m_widget (widget) 
    {
    }
    /**
     * Functor that displays a vertex
     * @param v the vertex to be displayed
     */
    void operator() (const Vertex* v)
    {
	float pointSize = (v->IsPhysical ()) ? 
	    m_widget.GetPhysicalObjectsWidth () :
	    m_widget.GetTessellationObjectsWidth ();
	const Body* body;
	unsigned int db = m_widget.GetDisplayedBody ();
	if (pointSize != 0.0 &&
	    (db == numeric_limits<unsigned int>::max() ||
	     ((body = m_widget.GetCurrentData ().GetBody (db)) != 0 &&
	      body->HasVertex (v))))
	{
	    glPointSize (pointSize);
	    m_widget.qglColor (
		v->IsPhysical () ? 
		m_widget.GetPhysicalObjectsColor () : 
		m_widget.GetTessellationObjectsColor () );
	    glBegin(GL_POINTS);
	    glVertex3f(v->x, v->y, v->z);
	    glEnd();
	}
    }
protected:
    /**
     * Where should be the vertex displayed
     */
    GLWidget& m_widget;
};


/**
 * Functor that displays an edge
 */
class displayEdge : public unary_function<const Edge*, void>
{
public:
    /**
     * Constructor
     * @param widget Where should be the edge displayed
     */
    displayEdge (GLWidget& widget) : 
	m_widget (widget) 
    {
    }
    /**
     * Functor that displays an edge
     * @param e the edge to be displayed
     */
    void operator() (const Edge* e)
    {
	float edgeSize = (e->IsPhysical ()) ? 
	    m_widget.GetPhysicalObjectsWidth () :
	    m_widget.GetTessellationObjectsWidth ();
	const Body* body;
	unsigned int db = m_widget.GetDisplayedBody ();
	if (edgeSize != 0.0 &&
	    (db == m_widget.DISPLAY_ALL ||
	     ((body = m_widget.GetCurrentData ().GetBody (db)) != 0 &&
	      body->HasEdge (e))))
	{
	    Vertex* begin = e->GetBegin ();
	    Vertex* end = e->GetEnd ();
	    glLineWidth (edgeSize);
	    m_widget.qglColor (
		e->IsPhysical () ? 
		m_widget.GetPhysicalObjectsColor () : 
		m_widget.GetTessellationObjectsColor () );
	    glBegin(GL_LINES);
	    glVertex3f(begin->x, begin->y, begin->z);
	    glVertex3f(end->x, end->y, end->z);
	    glEnd();
	}
    }
protected:
    /**
     * Where should be the vertex displayed
     */
    GLWidget& m_widget;
};



/**
 * Functor that displays a face
 */
class displayFace : public unary_function<const OrientedFace*, void>
{
public:
    /**
     * Constructor
     * @param widget Where should be the face displayed
     */
    displayFace (GLWidget& widget) : 
        m_widget (widget), m_count(0) {}
    /**
     * Functor that displays a face
     * @param f the face to be displayed
     */
    virtual void operator() (const OrientedFace* f)
    {
        if (m_count <= m_widget.GetDisplayedFace ())
        {
            displayFaceVertices (f);
            if (m_count == m_widget.GetDisplayedFace ())
                cdbg << "face " << m_count << ": " << *f << endl;
        }
        m_count++;
    }
protected:
    /**
     * Where should be the face displayed
     */
    GLWidget& m_widget;
    /**
     * Used to display fewer faces (for DEBUG purposes)
     */
    unsigned int m_count;
};


/**
 * Functor that displays a face using the color specified in the DMP file
 */
class displayFaceWithColor : public displayFace
{
public:
    /**
     * Constructor
     * @param widget where is the face displayed
     */
    displayFaceWithColor (GLWidget& widget) : 
        displayFace (widget) {}

    /**
     * Functor that displays a colored face
     * @param f face to be displayed
     */
    virtual void operator() (const OrientedFace* f)
    {
        if (m_count <= m_widget.GetDisplayedFace ())
        {
            glColor4fv (Color::GetValue(f->GetFace ()->GetColor ()));
            displayFaceVertices (f);
        }
        m_count++;
    }
};

/**
 * Displays a face and specifies the normal to the face. Used for lighting.
 */
class displayFaceWithNormal : public displayFace
{
public:
    /**
     * Constructor
     * @param widget where to display the face
     */
    displayFaceWithNormal (GLWidget& widget) : 
        displayFace (widget) {}
    /**
     * Functor used to display a face together to the normal
     * @param f face to be displayed
     */
    virtual void operator() (const OrientedFace* f)
    {
        if (m_count <= m_widget.GetDisplayedFace ())
        {

	    // specify the normal vector
	    const Vertex* begin = f->GetBegin (0);
	    const Vertex* end = f->GetEnd (0);
	    Vector3 first(end->x - begin->x,
			  end->y - begin->y,
			  end->z - begin->z);
	    begin = f->GetBegin (1);
	    end = f->GetEnd (1);
	    Vector3 second(end->x - begin->x,
			   end->y - begin->y,
			   end->z - begin->z);
	    Vector3 normal (first.cross(second).unit ());
	    glNormal3f (normal.x, normal.y, normal.z);

	    // specify the vertices
	    displayFaceVertices (f);
	}
	m_count++;
    }
};

/**
 * Functor used to display a body
 */
class displayBody
{
public:
    /**
     * Constructor
     * @param widget where to display the body
     */
    displayBody (GLWidget& widget) : m_widget (widget)
    {}
    /**
     * Functor used to display a body
     * @param b the body to be displayed
     */
    void operator () (const Body* b)
    {
        unsigned int displayedBody = m_widget.GetDisplayedBody ();
        if ( displayedBody == m_widget.DISPLAY_ALL ||
             b->GetOriginalIndex () == displayedBody)
        {
	    display (b);
        }
    }
    /**
     * Returns the widget where we display
     */
    GLWidget& GetWidget () {return m_widget;}
    
protected:
    /**
     * Displays the body
     * @param b the body
     */
    virtual void display (const Body* b) = 0;
private:
    /**
     * Where to display the body
     */
    GLWidget& m_widget;
};


/**
 * Functor that displays the center of a bubble
 */
class displayBodyCenter : public displayBody
{
public:
    /**
     * Constructor
     * @param widget where to display the center of the bubble
     */
    displayBodyCenter (GLWidget& widget) : displayBody (widget) {}
protected:
    /**
     * Displays the center of a body (bubble)
     * @param b body to display the center of
     */
    virtual void display (const Body* b)
    {
	G3D::Vector3 v = b->GetCenter ();
	glVertex3f(v.x, v.y, v.z);
    }
};


/**
 * Displays a body going through all its faces
 */
template <typename displayFunction>
class displayBodyWithFace : public displayBody
{
public:
    /**
     * Constructor
     * @param widget where to display the body
     */
    displayBodyWithFace (GLWidget& widget) : 
	displayBody (widget)
    {}
protected:
    /**
     * Displays a body going through all its faces
     * @param b the body to be displayed
     */
    virtual void display (const Body* b)
    {
	const vector<OrientedFace*> v = b->GetOrientedFaces ();
	for_each (v.begin (), v.end (), displayFunction(GetWidget ()));
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
 * Stores an OpenGLParam
 */
inline void storeOpenGLParam (OpenGLParam& param)
{
    glGetIntegerv (param.m_what, param.m_where);
}

/**
 * Prints an OpenGLParam
 */
void printOpenGLParam (OpenGLParam& param)
{
    cdbg << param.m_name << ": " << *param.m_where << endl;
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
    for_each (info, 
              info + sizeof (info) / sizeof (info[0]), storeOpenGLParam);
    cdbg << "OpenGL" << endl
         << "Vendor: " << glGetString (GL_VENDOR) << endl
         << "Renderer: " << glGetString (GL_RENDERER) << endl
         << "Version: " << glGetString (GL_VERSION) << endl
         << "Extensions: " << glGetString (GL_EXTENSIONS) << endl
         << "Stereo support: " << static_cast<bool>(stereoSupport) << endl
         << "Double buffer support: " 
         << static_cast<bool>(doubleBufferSupport) << endl;
    for_each (info, info + sizeof (info) / sizeof (info[0]),
              printOpenGLParam);
}

const float GLWidget::OBJECTS_WIDTH[] = {0.0, 1.0, 3.0, 5.0, 7.0};

const unsigned int GLWidget::DISPLAY_ALL(numeric_limits<unsigned int>::max());

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

QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(512, 512);
}

void GLWidget::initLightBodies ()
{
    using namespace G3D;
    const Vector3& max = m_dataFiles->GetAABox ().high ();
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


void GLWidget::initializeGL()
{
    using namespace G3D;
    m_object = display (m_viewType);
    const float* background = Color::GetValue (Color::WHITE);
    glClearColor (background[0], background[1],
                  background[2], background[3]);        
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    //glOrtho(-1.5, 1.5, -1.5, 1.5, -1.5, 1.5);
    const Vector3& min = m_dataFiles->GetAABox ().low ();
    const Vector3& max = m_dataFiles->GetAABox ().high ();
    float INCREASE = 1.5;
    glOrtho(INCREASE * min.x, INCREASE * max.x,
	    INCREASE * min.y, INCREASE * max.y, 
	    INCREASE * min.z, INCREASE * max.z);
    
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
    const Matrix4& columnOrderMatrix = modelView.transpose ();
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


void GLWidget::displayFacesContour (const vector<Body*>& bodies)
{
    qglColor (QColor(Qt::black));
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    glBegin (GL_TRIANGLES);
    for_each (bodies.begin (), bodies.end (),
              displayBodyWithFace<displayFace> (*this));
    glEnd ();
}

void GLWidget::displayFacesOffset (const vector<Body*>& bodies)
{
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1, 1);
    glBegin (GL_TRIANGLES);
    for_each (bodies.begin (), bodies.end (),
              displayBodyWithFace<displayFaceWithColor> (*this));
    glEnd ();
    glDisable (GL_POLYGON_OFFSET_FILL);
}

GLuint GLWidget::displayVertices ()
{
    const vector<Vertex*>& vertices = GetCurrentData ().GetVertices ();
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);

    for_each (vertices.begin (), vertices.end (), displayVertex (*this));
    glPointSize (1.0);

    glEndList();
    return list;
}

GLuint GLWidget::displayEdges ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);

    const vector<Edge*>& edges = GetCurrentData ().GetEdges ();
    for_each (edges.begin (), edges.end (), displayEdge (*this));

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
    const vector<Body*>& bodies = GetCurrentData ().GetBodies ();
    for_each (bodies.begin (),bodies.end (), displayBodyCenter (*this));
    glEnd ();
}

/**
 * Functor that displays a body center given the index of the body
 */
class displayBodyCenterFromData : public displayBodyCenter
{
public:
    /**
     * Constructor
     * @param widget where to display the body center
     * @param index what body to display
     */
    displayBodyCenterFromData (GLWidget& widget, unsigned int index) :
	displayBodyCenter (widget), m_index (index) {}
    /**
     * Functor that displays a body center
     * @param data Data object that constains the body
     */
    void operator () (Data* data)
    {
	const Body* body = data->GetBody (m_index);
	if (body != 0)
	    display (body);
    }
private:
    /**
     * What body to display
     */
    int m_index;
};


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
	pair<unsigned int, const Body*> p) {operator() (p.first);}

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
    const map<unsigned int, const Body*>& originalIndexBodyMap = 
	GetDataFiles ().GetData ()[0]->GetOriginalIndexBodyMap ();
    if (GetDisplayedBody () == DISPLAY_ALL)
	for_each (originalIndexBodyMap.begin (), originalIndexBodyMap.end (),
		  displayCenterPath (*this));
    else
    {
	map<unsigned int, const Body*>::const_iterator it = 
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

    const vector<Body*>& bodies = GetCurrentData ().GetBodies ();
    displayFacesContour (bodies);
    displayFacesOffset (bodies);

    glEndList();
    return list;
}

GLuint GLWidget::displayBodies ()
{
    GLuint list = glGenLists(1);
    const vector<Body*>& bodies = GetCurrentData ().GetBodies ();
    glNewList(list, GL_COMPILE);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glBegin (GL_TRIANGLES);
    for_each (bodies.begin (), bodies.end (),
              displayBodyWithFace<displayFaceWithNormal>(*this));
    glEnd ();
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
    default:
        throw domain_error (
            "ViewType enum has an invalid value: " + m_viewType);
    }
}

void GLWidget::IncrementDisplayedFace ()
{
    if (m_displayedBody != DISPLAY_ALL)
    {
        const Body& body = *GetCurrentData ().GetBodies ()[m_displayedBody];
        m_displayedFace++;
        if (m_displayedFace == body.GetOrientedFaces ().size ())
            m_displayedFace = DISPLAY_ALL;
        setObject (&m_object, displayFaces ());
        updateGL ();
    }
}

void GLWidget::DecrementDisplayedFace ()
{
    if (m_displayedBody != DISPLAY_ALL)
    {
        const Body& body = *GetCurrentData ().GetBodies ()[m_displayedBody];
        if (m_displayedFace == DISPLAY_ALL)
            m_displayedFace = body.GetOrientedFaces ().size ();
        m_displayedFace--;
        setObject (&m_object, displayFaces ());
        updateGL ();
    }
}

void GLWidget::IncrementDisplayedBody ()
{
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
