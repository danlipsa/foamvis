/**s
 * @file   GLWidget.cpp
 * @author Dan R. Lipsa
 *
 * Definitions for the widget for displaying foam bubbles using OpenGL
 */


#include "Body.h"
#include "BodyAlongTime.h"
#include "FoamAlongTime.h"
#include "Debug.h"
#include "DebugStream.h"
#include "DisplayVertexFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "DisplayFaceFunctors.h"
#include "DisplayBodyFunctors.h"
#include "ElementUtils.h"
#include "GLWidget.h"


// Private Classes
// ======================================================================

/**
 * Stores information about various OpenGL characteristics of the graphic card
 */
struct OpenGLParam
{
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

template<typename T>
class identity 
{
public:
    identity (T value)
    {
	m_value = value;
    }
    T operator() ()
    {
	return m_value;
    }
private:
    T m_value;
};


// Private Functions
// ======================================================================


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
    cdbg << "OpenGL" << endl
         << "Vendor: " << glGetString (GL_VENDOR) << endl
         << "Renderer: " << glGetString (GL_RENDERER) << endl
         << "Version: " << glGetString (GL_VERSION) << endl
	//<< "Extensions: " << glGetString (GL_EXTENSIONS) << endl
         << "Stereo support: " << static_cast<bool>(stereoSupport) << endl
         << "Double buffer support: " 
	 << static_cast<bool>(doubleBufferSupport) << endl;
    for_each (info.begin (), info.end (), mem_fun_ref(&OpenGLParam::get));
    for_each (info.begin (), info.end (), mem_fun_ref(&OpenGLParam::print));
}


void displayOriginalDomainFaces (G3D::Vector3 first,
				 G3D::Vector3 second,
				 G3D::Vector3 third)
{
    G3D::Vector3 origin(0, 0, 0);
    G3D::Vector3 sum = first + second;
    for (int i = 0; i < 2; i++)
    {
	glBegin (GL_POLYGON);
	glVertex (origin);
	glVertex (first);
	glVertex (sum);
	glVertex (second);
	glEnd ();
	origin += third;
	first += third;
	second += third;
	sum += third;
    }
}

// Static Fields
// ======================================================================

const size_t GLWidget::DISPLAY_ALL(numeric_limits<size_t>::max());
const size_t GLWidget::QUADRIC_SLICES (20);
const size_t GLWidget::QUADRIC_STACKS (20);


const float GLWidget::OBJECTS_WIDTH[] = {0.0, 1.0, 3.0, 5.0, 7.0};


// Methods
// ======================================================================

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent), 
      m_viewType (VIEW_TYPE_COUNT),
      m_torusOriginalDomainDisplay (false),
      m_torusOriginalDomainClipped (false),
      m_interactionMode (InteractionMode::ROTATE),
      m_object (0),
      m_foamAlongTime (0), m_timeStep (0),
      m_displayedBodyIndex (DISPLAY_ALL), m_displayedFaceIndex (DISPLAY_ALL),
      m_displayedEdgeIndex (DISPLAY_ALL),
      m_physicalVertexSize (1), m_physicalEdgeWidth (1),
      m_physicalVertexColor (Qt::blue), m_physicalEdgeColor (Qt::blue),
      m_tessellationVertexSize (1), m_tessellationEdgeWidth (1),
      m_normalVertexSize (2), m_normalEdgeWidth (1),
      m_tessellationVertexColor (Qt::green), m_tessellationEdgeColor (Qt::green),
      m_centerPathColor (Qt::red),
      m_edgesTorusTubes (false),
      m_facesTorusTubes (false),
      m_edgesBodyCenter (false)
{
    cdbg << "---------- GLWidget constructor ----------\n";
    const int DOMAIN_INCREMENT_COLOR[] = {100, 0, 200};
    const int POSSIBILITIES = 3; //domain increment can be *, - or +
    using G3D::Vector3int16;
    for (int i = 0;
	 i < POSSIBILITIES * POSSIBILITIES * POSSIBILITIES; i++)
    {
	G3D::Vector3int16 di = Edge::IntToLocation (i);
	QColor color (
	    DOMAIN_INCREMENT_COLOR[di.x + 1],
	    DOMAIN_INCREMENT_COLOR[di.y + 1],
	    DOMAIN_INCREMENT_COLOR[di.z + 1]);
	m_endTranslationColor[di] = color;
    }
    m_endTranslationColor[Vector3int16(0,0,0)] = QColor(0,0,0);
    m_endTranslationColor[Vector3int16(0,0,0)] = QColor(0,0,0);
    m_quadric = gluNewQuadric ();
    gluQuadricCallback (m_quadric, GLU_ERROR,
			reinterpret_cast<void (*)()>(&quadricErrorCallback));
    initViewTypeDisplay ();
}





void GLWidget::initViewTypeDisplay ()
{
    boost::array<ViewTypeDisplay, VIEW_TYPE_COUNT> vtd = 
	{{
	{&GLWidget::displayListVerticesNormal, identity<Lighting>(NO_LIGHTING)},
	{&GLWidget::displayListVerticesPhysical, 
	 identity<Lighting> (NO_LIGHTING)},
	{&GLWidget::displayListVerticesTorus, identity<Lighting> (NO_LIGHTING)},
	
	{&GLWidget::displayListEdgesNormal, identity<Lighting> (NO_LIGHTING)},
	{&GLWidget::displayListEdgesPhysical, identity<Lighting> (NO_LIGHTING)},
	{&GLWidget::displayListEdgesTorus, 
	 bl::if_then_else_return (bl::bind (&GLWidget::edgesTorusTubes, this), 
				  LIGHTING, NO_LIGHTING)},
	
	{&GLWidget::displayListFacesNormal, identity<Lighting> (NO_LIGHTING)},
	{&GLWidget::displayListFacesLighting, identity<Lighting> (LIGHTING)},
	{&GLWidget::displayListFacesTorus, 
	 bl::if_then_else_return (bl::bind (&GLWidget::facesTorusTubes, this),
				  LIGHTING, NO_LIGHTING)},
	
	{&GLWidget::displayListCenterPaths, identity<Lighting> (LIGHTING)},
	}};
    copy (vtd.begin (), vtd.end (), VIEW_TYPE_DISPLAY.begin ());
}

void GLWidget::SetFoamAlongTime (FoamAlongTime* dataAlongTime) 
{
    cdbg << "SetFoamAlongTime\n";
    m_foamAlongTime = dataAlongTime;
    Face* f = 
	GetCurrentFoam ().GetBody (0)->GetFace (0);
    Edge* e = f->GetEdge (0);
    float length = (*e->GetEnd () - *e->GetBegin ()).length ();

    m_edgeRadius = length / 20;
    m_arrowBaseRadius = 5 * m_edgeRadius;
    m_arrowHeight = 10 * m_edgeRadius;
}



GLWidget::~GLWidget()
{
    makeCurrent();
    glDeleteLists(m_object, 1);
    m_object = 0;
    gluDeleteQuadric (m_quadric);
    m_quadric = 0;
}

void GLWidget::view (bool checked, ViewType view)
{
    if (checked)
    {
        m_viewType = view;
	if ((VIEW_TYPE_DISPLAY[view].m_lighting) () == LIGHTING)
	    enableLighting ();
	else
	    disableLighting ();
	UpdateDisplay ();
    }
}


QSize GLWidget::minimumSizeHint() 
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() 
{
    return QSize(512, 512);
}

void GLWidget::enableLighting ()
{
    const G3D::Vector3& max = m_foamAlongTime->GetAABox ().high ();
    GLfloat lightPosition[] = { 2*max.x, 2*max.y, 2*max.z, 0.0 };
    GLfloat lightAmbient[] = {1.0, 1.0, 1.0, 1.0};

    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glLoadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glPopMatrix ();
    
    glShadeModel (GL_SMOOTH);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

    glEnable (GL_COLOR_MATERIAL);
    glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void GLWidget::calculateViewingVolume ()
{
    using G3D::Vector3;
    const Vector3& low = m_foamAlongTime->GetAABox ().low ();
    const Vector3& high = m_foamAlongTime->GetAABox ().high ();
    float border = ((high - low) / 8).max ();
    float min = low.min () - border;
    float max = high.max () + border;
    m_viewingVolume.set (Vector3 (min, min, min),
			 Vector3 (max, max, max));
    cdbg << "Viewing volume: " << m_viewingVolume << endl;
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
    cdbg << "initializeGL" << endl;
    qglClearColor (QColor(Qt::white));

    printOpenGLInfo ();

    m_object = displayList (m_viewType);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode (GL_MODELVIEW);
    glLoadMatrix (m_transform);

    glCallList (m_object);
    detectOpenGLError ();
}



void GLWidget::resizeGL(int width, int height)
{
    using G3D::Rect2D;using G3D::Vector2;
    cdbg << "resizeGL" << endl;
    calculateViewingVolume ();
    Vector2 viewportStart = m_viewport.x0y0 ();
    float ratio = (m_viewingVolume.high ().x - m_viewingVolume.low ().x) / 
	(m_viewingVolume.high ().y - m_viewingVolume.low ().y);
    if ((static_cast<float>(width) / height) > ratio)
    {
	int newWidth = ratio * height;
	m_viewport = Rect2D::xywh ((width - newWidth) / 2, 0,
				   newWidth, height);
    }
    else
    {
	int newHeight = 1 / ratio * width;
	m_viewport = Rect2D::xywh (0, (height - newHeight) / 2,
				   width, newHeight);
    }
    glViewport (m_viewport.x0 (), m_viewport.y0 (), 
		m_viewport.width (), m_viewport.height ());
    project ();
}

void GLWidget::setRotation (int axis, float angleRadians)
{
    using G3D::Matrix3;using G3D::Vector3;
    Vector3 axes[3] = {
	Vector3::unitX (), Vector3::unitY (), Vector3::unitZ ()
    };
    m_transform.rotation = 
	Matrix3::fromAxisAngle (axes[axis], angleRadians) * m_transform.rotation;
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
}

void scaleAABox (G3D::AABox* aabox, float change)
{
    using G3D::Vector3;
    Vector3 center = aabox->center ();
    Vector3 newLow = aabox->low () * change + center * (1 - change);
    Vector3 newHigh = aabox->high () * change + center * (1 - change);
    aabox->set (newLow, newHigh);
}

void scaleRect2D (G3D::Rect2D* aabox, float change)
{
    using G3D::Vector2;
    Vector2 center = aabox->center ();
    Vector2 newLow = aabox->x0y0 () * change + center * (1 - change);
    Vector2 newHigh = aabox->x1y1 () * change + center * (1 - change);
    *aabox = G3D::Rect2D::xyxy ( newLow, newHigh);
}

float GLWidget::ratioFromCenter (const QPoint& p)
{
    using G3D::Vector2;
    Vector2 center = m_viewport.center ();
    Vector2 lastPos (m_lastPos.x (), m_lastPos.y());
    Vector2 currentPos (p.x (), p.y ());
    float ratio = 
	(currentPos - center).length () / 
	(lastPos - center).length ();
    return ratio;
}


void GLWidget::rotate (const QPoint& position)
{
    int dx = position.x() - m_lastPos.x();
    int dy = position.y() - m_lastPos.y();

    // scale this with the size of the window
    int side = std::min (m_viewport.width (), m_viewport.height ());
    float dxRadians = static_cast<float>(dx) * (M_PI / 2) / side;
    float dyRadians = static_cast<float>(dy) * (M_PI / 2) / side;
    setRotation (0, dyRadians);
    setRotation (1, dxRadians);
}

void GLWidget::translateViewport (const QPoint& position)
{
    int dx = position.x() - m_lastPos.x();
    int dy = position.y() - m_lastPos.y();
    m_viewport = G3D::Rect2D::xywh (m_viewport.x0 () + dx,
				    m_viewport.y0 () - dy,
				    m_viewport.width (),
				    m_viewport.height ());
    glViewport (m_viewport.x0 (), m_viewport.y0 (), 
		m_viewport.width (), m_viewport.height ());
}


void GLWidget::scale (const QPoint& position)
{
    float ratio = 1 / ratioFromCenter (position);
    scaleAABox (&m_viewingVolume, ratio);
    project ();
}

void GLWidget::scaleViewport (const QPoint& position)
{
    float ratio = ratioFromCenter (position);
    scaleRect2D (&m_viewport, ratio);
    glViewport (m_viewport.x0 (), m_viewport.y0 (), 
		m_viewport.width (), m_viewport.height ());
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    switch (m_interactionMode)
    {
    case InteractionMode::ROTATE:
	rotate (event->pos ());
	break;
    case InteractionMode::TRANSLATE_VIEWPORT:
	translateViewport (event->pos ());
	break;
    case InteractionMode::SCALE:
	scale (event->pos ());
	break;
    case InteractionMode::SCALE_VIEWPORT:
	scaleViewport (event->pos ());
    default:
	break;
    }
    updateGL ();
    m_lastPos = event->pos();
}


void GLWidget::displayFacesContour (vector<Body*>& bodies)
{
    qglColor (QColor(Qt::black));
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    for_each (bodies.begin (), bodies.end (),
              DisplayBody<
	      DisplayFace<
	      DisplaySameEdges> > (*this));
}

void GLWidget::displayFacesOffset (vector<Body*>& bodies)
{
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1, 1);
    for_each (bodies.begin (), bodies.end (),
              DisplayBody<DisplayFaceWithColor> (*this));
    glDisable (GL_POLYGON_OFFSET_FILL);
}

void GLWidget::displayOriginalDomain ()
{
    const OOBox& periods = GetCurrentFoam().GetPeriods ();
    if (m_torusOriginalDomainDisplay)
    {
	glPushAttrib (GL_POLYGON_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
	glLineWidth (1.0);
	qglColor (QColor (Qt::black));
	glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

	displayOriginalDomainFaces (periods[0], periods[1], periods[2]);
	displayOriginalDomainFaces (periods[1], periods[2], periods[0]);
	displayOriginalDomainFaces (periods[2], periods[0], periods[1]);
	glPopAttrib ();
    }
}


GLuint GLWidget::displayListVerticesNormal ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);

    glPushAttrib (GL_CURRENT_BIT | GL_POINT_BIT);
    qglColor (QColor (Qt::black));
    glPointSize (m_normalVertexSize);

    Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
    glBegin(GL_POINTS);
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<
	      DisplayFace<
	      DisplayEdges<
	      DisplayEdgeVertices> > > (*this));
    glEnd ();
    glPopAttrib ();
    displayOriginalDomain ();
    glEndList();
    return list;
}

GLuint GLWidget::displayListVerticesPhysical ()
{
    Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glPushAttrib (GL_POINT_BIT | GL_CURRENT_BIT);
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<
	      DisplayFace<
	      DisplayEdges<
	      DisplayPhysicalVertex> > > (*this));
    glPopAttrib ();

    displayOriginalDomain ();
    glEndList();
    return list;
}

GLuint GLWidget::displayListVerticesTorus ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);

    glPushAttrib (GL_POINT_BIT | GL_CURRENT_BIT);
    glPointSize (m_normalVertexSize);
    qglColor (QColor (Qt::black));
    glBegin (GL_POINTS);
    Foam::Vertices& vertices = GetCurrentFoam ().GetVertices ();
    for_each (vertices.begin (), vertices.end (), DisplayOriginalVertex());
    glEnd ();
    glPopAttrib ();

    displayOriginalDomain ();
    glEndList();
    return list;
}


GLuint GLWidget::displayListEdges (
    boost::function<void (Edge*)> displayEdge,
    boost::function<bool (Edge*)> shouldDisplayEdge)
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);
    glLineWidth (m_normalEdgeWidth);
    BOOST_FOREACH (Edge* e, GetCurrentFoam ().GetEdges ())
	if (shouldDisplayEdge (e))
	    displayEdge (e);
    glPopAttrib ();
    displayOriginalDomain ();
    displayCenterOfBodies ();
    glEndList();
    return list;
}


GLuint GLWidget::displayListEdgesNormal ()
{
    //GCC BUG: if  you pass 'this' instead  of '*this', you  don't get any
    //compilation error but you get strange runtime error.
    using boost::mem_fn;
    return m_torusOriginalDomainClipped ?
	displayListEdges (DisplayEdgeTorusClipped (*this),
			  mem_fn(&Edge::IsClipped)) :
	displayListEdges (DisplayEdgeWithColor (*this),
			  mem_fn(&Edge::ShouldDisplay));
}

GLuint GLWidget::displayListEdgesPhysical ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);
    Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<
	      DisplayFace<
	      DisplayEdges<
	      DisplayEdgeTessellationOrPhysical> > >(*this));
    glPopAttrib ();
    displayOriginalDomain ();
    displayCenterOfBodies ();
    glEndList();
    return list;
}

GLuint GLWidget::displayListEdgesTorusTubes ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glPushAttrib (GL_POLYGON_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    gluQuadricDrawStyle (m_quadric, GLU_FILL);
    gluQuadricNormals (m_quadric, GLU_SMOOTH);

    Foam::Edges& edges = GetCurrentFoam ().GetEdges ();
    for_each (edges.begin (), edges.end (),
	      DisplayEdgeTorus<DisplayEdgeTube, DisplayArrowTube, false>(*this));
    glPopAttrib ();

    displayOriginalDomain ();
    glEndList();
    return list;
}

GLuint GLWidget::displayListEdgesTorusLines ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);

    Foam::Edges& edges = GetCurrentFoam ().GetEdges ();
    for_each (edges.begin (), edges.end (),
	      DisplayEdgeTorus<DisplayEdge, DisplayArrow, false> (*this));
    glPopAttrib ();

    displayOriginalDomain ();
    glEndList();
    return list;
}


void GLWidget::displayCenterOfBodies ()
{
    if ((m_viewType == EDGES && m_edgesBodyCenter) ||
	m_viewType == CENTER_PATHS)
    {
	glPushAttrib (GL_POINT_BIT | GL_CURRENT_BIT);
	glPointSize (4.0);
	qglColor (QColor (Qt::red));
	glBegin(GL_POINTS);
	Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
	for_each (bodies.begin (),bodies.end (), DisplayBodyCenter (*this));
	glEnd ();
	glPopAttrib ();
    }
}

GLuint GLWidget::displayListFacesNormal ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);

    Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
    displayFacesContour (bodies);
    displayFacesOffset (bodies);

    displayOriginalDomain ();
    glEndList();
    return list;
}

GLuint GLWidget::displayListFacesLighting ()
{
    GLuint list = glGenLists(1);
    Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
    glNewList(list, GL_COMPILE);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    for_each (bodies.begin (), bodies.end (),
              DisplayBody<DisplayFaceWithNormal>(*this));
    glEndList();
    return list;
}

GLuint GLWidget::displayListFacesTorusTubes ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glPushAttrib (GL_POLYGON_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glLineWidth (3.0);
    gluQuadricDrawStyle (m_quadric, GLU_FILL);
    gluQuadricNormals (m_quadric, GLU_SMOOTH);
    const Foam::Faces& faces = GetCurrentFoam ().GetFaces ();
    for_each (faces.begin (), faces.end (),
	      DisplayFace<
	      DisplayEdges<
	      DisplayEdgeTorus<
	      DisplayEdgeTube, DisplayArrowTube, true> > > (*this) );
    glPopAttrib ();

    displayOriginalDomain ();
    glEndList();
    return list;
}


GLuint GLWidget::displayListFacesTorusLines ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);

    const Foam::Faces& faces = GetCurrentFoam ().GetFaces ();
    for_each (faces.begin (), faces.end (),
	      DisplayFace<
	      DisplayEdges<
	      DisplayEdgeTorus<
	      DisplayEdge, DisplayArrow, true> > > (*this) );
    glPopAttrib ();

    displayOriginalDomain ();
    glEndList();
    return list;
}

GLuint GLWidget::displayListCenterPaths ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    qglColor (QColor (Qt::black));
    BodiesAlongTime::BodyMap& bats = GetBodiesAlongTime ().GetBodyMap ();
    DisplayCenterPath dcp(*this);
    if (GetDisplayedBodyIndex () == DISPLAY_ALL)
	for_each (bats.begin (), bats.end (), dcp);
    else
	dcp (GetDisplayedBodyId ());
    displayCenterOfBodies ();

    glEndList();
    return list;
}

GLuint GLWidget::displayList (ViewType type)
{
    RuntimeAssert (type < VIEW_TYPE_COUNT, 
		   "ViewType enum has an invalid value: ", m_viewType);
    return (this->*(VIEW_TYPE_DISPLAY[type].m_displayList)) ();
}



void GLWidget::IncrementDisplayedBody ()
{
    if (m_viewType == VERTICES_TORUS || m_viewType == EDGES_TORUS)
	return;
    ++m_displayedBodyIndex;
    m_displayedFaceIndex = DISPLAY_ALL;
    if (m_displayedBodyIndex == 
	GetFoamAlongTime ().GetFoam (0)->GetBodies ().size ())
        m_displayedBodyIndex = DISPLAY_ALL;
    UpdateDisplay ();
    cdbg << "displayed body: " << m_displayedBodyIndex << endl;
}


void GLWidget::IncrementDisplayedFace ()
{
    ++m_displayedFaceIndex;
    if (m_viewType == FACES_TORUS)
    {
	if (m_displayedFaceIndex == 	GetCurrentFoam ().GetFaces ().size ())
	    m_displayedFaceIndex = DISPLAY_ALL;
    }
    if (m_displayedBodyIndex != DISPLAY_ALL)
    {
        Body& body = *GetCurrentFoam ().GetBodies ()[m_displayedBodyIndex];
        if (m_displayedFaceIndex == body.GetOrientedFaces ().size ())
            m_displayedFaceIndex = DISPLAY_ALL;
    }
    UpdateDisplay ();
}

void GLWidget::IncrementDisplayedEdge ()
{
    if (m_displayedBodyIndex != DISPLAY_ALL && m_displayedFaceIndex != DISPLAY_ALL)
    {
	++m_displayedEdgeIndex;
	Face& face = *GetDisplayedFace ();
	if (m_displayedEdgeIndex == face.GetOrientedEdges ().size ())
	    m_displayedEdgeIndex = DISPLAY_ALL;
	UpdateDisplay ();
    }
}

void GLWidget::DecrementDisplayedBody ()
{
    if (m_viewType == VERTICES_TORUS || m_viewType == EDGES_TORUS)
	return;
    if (m_displayedBodyIndex == DISPLAY_ALL)
        m_displayedBodyIndex = 
	    GetFoamAlongTime ().GetFoam (0)->GetBodies ().size ();
    --m_displayedBodyIndex;
    m_displayedFaceIndex = DISPLAY_ALL;
    UpdateDisplay ();
    cdbg << "displayed body: " << m_displayedBodyIndex << endl;
}

void GLWidget::DecrementDisplayedFace ()
{
    if (m_viewType == FACES_TORUS)
    {
	if (m_displayedFaceIndex == DISPLAY_ALL)
	    m_displayedFaceIndex = GetCurrentFoam ().GetFaces ().size ();
    }
    if (m_displayedBodyIndex != DISPLAY_ALL)
    {
        Body& body = *GetCurrentFoam ().GetBodies ()[m_displayedBodyIndex];
        if (m_displayedFaceIndex == DISPLAY_ALL)
            m_displayedFaceIndex = body.GetOrientedFaces ().size ();
    }
    --m_displayedFaceIndex;
    UpdateDisplay ();
}

void GLWidget::DecrementDisplayedEdge ()
{
    if (m_displayedBodyIndex != DISPLAY_ALL && m_displayedFaceIndex != DISPLAY_ALL)
    {
	Face& face = *GetDisplayedFace ();
	if (m_displayedEdgeIndex == DISPLAY_ALL)
	    m_displayedEdgeIndex = face.GetOrientedEdges ().size ();
	--m_displayedEdgeIndex;
	UpdateDisplay ();
    }
}



Foam& GLWidget::GetCurrentFoam () const
{
    return *m_foamAlongTime->GetFoam (m_timeStep);
}

const QColor& GLWidget::GetEndTranslationColor (
    const G3D::Vector3int16& di) const
{
    EndLocationColor::const_iterator it = m_endTranslationColor.find (di);
    RuntimeAssert (it != m_endTranslationColor.end (),
		   "Invalid domain increment ", di);
    return (*it).second;
}



// Slots and slot like methods
// ======================================================================

void GLWidget::ToggledVerticesNormal (bool checked)
{
    view (checked, VERTICES);
}

void GLWidget::ToggledVerticesPhysical (bool checked)
{
    view (checked, VERTICES_PHYSICAL);
}

void GLWidget::ToggledVerticesTorus (bool checked)
{
    view (checked, VERTICES_TORUS);
}




void GLWidget::ToggledEdgesNormal (bool checked)
{
    view (checked, EDGES);
}

void GLWidget::ToggledEdgesPhysical (bool checked)
{
    view (checked, EDGES_PHYSICAL);
}

void GLWidget::ToggledEdgesTorus (bool checked)
{
    view (checked, EDGES_TORUS);
}

void GLWidget::ToggledEdgesTorusTubes (bool checked)
{
    m_edgesTorusTubes = checked;
    ToggledEdgesTorus (true);
}

void GLWidget::ToggledEdgesBodyCenter (bool checked)
{
    m_edgesBodyCenter = checked;
    UpdateDisplay ();
}

void GLWidget::ToggledFacesNormal (bool checked)
{
    view (checked, FACES);
}

void GLWidget::ToggledFacesTorus (bool checked)
{
    view (checked, FACES_TORUS);
}

void GLWidget::ToggledFacesTorusTubes (bool checked)
{
    m_facesTorusTubes = checked;
    ToggledFacesTorus (true);
}



void GLWidget::ToggledTorusOriginalDomainDisplay (bool checked)
{
    m_torusOriginalDomainDisplay = checked;
    UpdateDisplay ();
}

void GLWidget::ToggledTorusOriginalDomainClipped (bool checked)
{
    m_torusOriginalDomainClipped = checked;
    UpdateDisplay ();
}

void GLWidget::ToggledBodies (bool checked)
{
    view (checked, FACES_LIGHTING);
}

void GLWidget::ToggledCenterPath (bool checked)
{
    view (checked, CENTER_PATHS);
}


void GLWidget::currentIndexChangedInteractionMode (int index)
{
    m_interactionMode = static_cast<InteractionMode::Name>(index);
}

void GLWidget::ValueChangedSliderData (int newIndex)
{
    m_timeStep = newIndex;
    UpdateDisplay ();
}


void GLWidget::ValueChangedVerticesPhysical (int value)
{
    m_physicalVertexSize = value;
    UpdateDisplay ();
}

void GLWidget::ValueChangedEdgesPhysical (int value)
{
    m_physicalEdgeWidth = value;
    UpdateDisplay ();
}

void GLWidget::ValueChangedVerticesTessellation (int value)
{
    m_tessellationVertexSize = value;
    UpdateDisplay ();
}

void GLWidget::ValueChangedEdgesTessellation (int value)
{
    m_tessellationEdgeWidth = value;
    UpdateDisplay ();
}

BodiesAlongTime& GLWidget::GetBodiesAlongTime ()
{
    return GetFoamAlongTime ().GetBodiesAlongTime ();
}

BodyAlongTime& GLWidget::GetBodyAlongTime (size_t id)
{
    return GetBodiesAlongTime ().GetOneBody (id);
}

Body* GLWidget::GetDisplayedBody () const
{
    size_t i = GetDisplayedBodyIndex ();
    const Foam& data = GetCurrentFoam ();
    return data.GetBody (i);
}

size_t GLWidget::GetDisplayedBodyId () const
{
    return GetDisplayedBody ()->GetId ();
}

size_t GLWidget::GetDisplayedFaceId () const
{
    return GetDisplayedFace ()->GetId ();
}

Face* GLWidget::GetDisplayedFace () const
{
    size_t i = GetDisplayedFaceIndex ();
    if  (m_viewType == FACES_TORUS)
	return GetCurrentFoam ().GetFaces ()[i];
    else if (m_displayedBodyIndex != DISPLAY_ALL)
    {
	Body& body = *GetDisplayedBody ();
	return body.GetFace (i);
    }
    RuntimeAssert (false, "There is no displayed face");
    return 0;
}

Edge* GLWidget::GetDisplayedEdge () const
{
    if (m_displayedBodyIndex != DISPLAY_ALL && 
	m_displayedFaceIndex != DISPLAY_ALL)
    {
	Face* face = GetDisplayedFace ();
	return face->GetEdge (m_displayedEdgeIndex);
    }
    RuntimeAssert (false, "There is no displayed edge");
    return 0;
}

size_t GLWidget::GetDisplayedEdgeId () const
{
    return GetDisplayedEdge ()->GetId ();
}


// Static Methods
//======================================================================
void GLWidget::disableLighting ()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
}

void GLWidget::quadricErrorCallback (GLenum errorCode)
{
    const GLubyte* message = gluErrorString (errorCode);
    cdbg << "Quadric error: " << message << endl;
}
