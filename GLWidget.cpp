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
#include "Utils.h"
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
    G3D::Vector3 origin;
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
      m_normalVertexSize (3), m_normalEdgeWidth (1),
      m_contextAlpha (0.03),
      m_centerPathColor (Qt::red),
      m_edgesTorusTubes (false),
      m_facesTorusTubes (false),
      m_edgesBodyCenter (false),
      m_edgesTessellation (false),
      m_centerPathDisplayBody (false)
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
	{&GLWidget::displayListEdgesNormal, identity<Lighting> (NO_LIGHTING)},
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
    m_foamAlongTime = dataAlongTime;
    boost::shared_ptr<Face>  f = 
	GetCurrentFoam ().GetBody (0)->GetFace (0);
    boost::shared_ptr<Edge>  e = f->GetEdge (0);
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

// Uses antialiased points and lines
// See OpenGL Programming Guide, 7th edition, Chapter 6: Blending,
// Antialiasing, Fog and Polygon Offset page 293
void GLWidget::initializeGL()
{
    cdbg << "initializeGL" << endl;
    glClearColor (1., 1., 1., 0.);
    
    printOpenGLInfo ();
    GLWidget::disableLighting ();
    m_object = displayList (m_viewType);
    glEnable(GL_DEPTH_TEST);

    // for anti-aliased lines and points
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_LINE_SMOOTH);
    glEnable (GL_POINT_SMOOTH);
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
	Matrix3::fromAxisAngle (axes[axis], 
				angleRadians) * m_transform.rotation;
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

void GLWidget::displayOriginalDomain ()
{
    const OOBox& periods = GetCurrentFoam().GetOriginalDomain ();
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

template<typename displayEdge>
GLuint GLWidget::displayListEdges ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);
    glLineWidth (m_normalEdgeWidth);

    const Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<
	      DisplayFace<
	      DisplayEdges<
	      displayEdge> > >(*this));
    displayStandaloneEdges ();

    glPopAttrib ();
    displayOriginalDomain ();
    displayCenterOfBodies ();
    glEndList();
    return list;
}

void GLWidget::displayStandaloneEdges () const
{
    const Foam::Edges& standaloneEdges = GetCurrentFoam ().GetStandaloneEdges ();
    BOOST_FOREACH (boost::shared_ptr<Edge> edge, standaloneEdges)
	DisplayEdgeWithColor<> (*this, DisplayElement::FOCUS) (*edge);
}


GLuint GLWidget::displayListEdgesNormal ()
{
    return m_torusOriginalDomainClipped ?
	displayListEdges <DisplayEdgeTorusClipped> () :
	displayListEdges <DisplayEdgeWithColor<> >();
}

GLuint GLWidget::displayListEdgesTorusTubes ()
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glPushAttrib (GL_POLYGON_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    gluQuadricDrawStyle (m_quadric, GLU_FILL);
    gluQuadricNormals (m_quadric, GLU_SMOOTH);

    EdgeSet edgeSet;
    GetCurrentFoam ().GetEdgeSet (&edgeSet);
    for_each (edgeSet.begin (), edgeSet.end (),
	      DisplayEdgeTorus<DisplayEdgeTube, 
	      DisplayArrowTube, false>(*this));
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

    EdgeSet edgeSet;
    GetCurrentFoam ().GetEdgeSet (&edgeSet);
    for_each (edgeSet.begin (), edgeSet.end (),
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
	Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
	for_each (bodies.begin (), bodies.end (), DisplayBodyCenter (*this));
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

    displayStandaloneEdges ();
    displayOriginalDomain ();
    glEndList();
    return list;
}

void GLWidget::displayFacesContour (vector<boost::shared_ptr<Body> >& bodies)
{
    glColor (G3D::Color4 (Color::GetValue(Color::BLACK), 
			  GetContextAlpha ()));
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    for_each (bodies.begin (), bodies.end (),
              DisplayBody<
	      DisplayFace<
	      DisplaySameEdges> > (*this));
}

// See OpenGL Programming Guide, 7th edition, Chapter 6: Blending,
// Antialiasing, Fog and Polygon Offset page 293
void GLWidget::displayFacesOffset (vector<boost::shared_ptr<Body> >& bodies)
{
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1, 1);
    for_each (bodies.begin (), bodies.end (),
              DisplayBody<
	      DisplayFaceWithColor> (*this));
    glDisable (GL_POLYGON_OFFSET_FILL);
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
    FaceSet faceSet;
    GetCurrentFoam ().GetFaceSet (&faceSet);
    for_each (faceSet.begin (), faceSet.end (),
	      DisplayFace<
	      DisplayEdges<
	      DisplayEdgeTorus<DisplayEdgeTube, DisplayArrowTube, true> > > (
		  *this));
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

    FaceSet faceSet;
    GetCurrentFoam ().GetFaceSet (&faceSet);
    for_each (faceSet.begin (), faceSet.end (),
	      DisplayFace<
	      DisplayEdges<
	      DisplayEdgeTorus<DisplayEdge, DisplayArrow, true> > > (
		  *this, DisplayElement::FOCUS) );
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
    glLineWidth (1.0);

    BodiesAlongTime::BodyMap& bats = GetBodiesAlongTime ().GetBodyMap ();
    DisplayCenterPath displayCenterPath(*this);
    if (GetDisplayedBodyIndex () == DISPLAY_ALL)
	for_each (bats.begin (), bats.end (), displayCenterPath);
    else
	displayCenterPath (GetDisplayedBodyId ());

    if (IsCenterPathDisplayBody ())
    {
	Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
	for_each (bodies.begin (), bodies.end (),
		  DisplayBody<DisplayFace<
		  DisplayEdges<DisplayEdgeWithColor<
		  DisplayElement::DONT_DISPLAY_TESSELLATION> > > > (
		      *this, DisplayElement::INVISIBLE_CONTEXT));
    }

    displayCenterOfBodies ();
    displayOriginalDomain ();
    displayStandaloneEdges ();

    glEndList();
    return list;
}

GLuint GLWidget::displayList (ViewType type)
{
    RuntimeAssert (type < VIEW_TYPE_COUNT, 
		   "ViewType enum has an invalid value: ", m_viewType);
    return (this->*(VIEW_TYPE_DISPLAY[type].m_displayList)) ();
}

bool GLWidget::IsDisplayedBody (size_t bodyId) const
{
    return 
	(GetDisplayedBodyIndex () == DISPLAY_ALL ||
	 GetDisplayedBodyId () == bodyId);
}

bool GLWidget::IsDisplayedBody (const boost::shared_ptr<Body>  body) const
{
    return IsDisplayedBody (body->GetId ());
}

bool GLWidget::IsDisplayedFace (size_t faceI) const
{
    size_t faceIndex = GetDisplayedFaceIndex ();
    return (faceIndex == DISPLAY_ALL || faceIndex == faceI);
}

bool GLWidget::IsDisplayedEdge (size_t oeI) const
{
    size_t edgeIndex = GetDisplayedEdgeIndex ();
    return edgeIndex == DISPLAY_ALL || edgeIndex == oeI;
}


bool GLWidget::DoesSelectBody () const
{
    return 
	m_viewType != EDGES_TORUS &&
	m_viewType != FACES_TORUS;
}

bool GLWidget::DoesSelectFace () const
{
    return 
	m_displayedBodyIndex != DISPLAY_ALL;
}

bool GLWidget::DoesSelectEdge () const
{
    return 
	m_displayedFaceIndex != DISPLAY_ALL &&
	m_viewType != FACES;
}


void GLWidget::IncrementDisplayedBody ()
{
    if (DoesSelectBody ())
    {
	++m_displayedBodyIndex;
	m_displayedFaceIndex = DISPLAY_ALL;
	if (m_displayedBodyIndex == 
	    GetFoamAlongTime ().GetFoam (0)->GetBodies ().size ())
	    m_displayedBodyIndex = DISPLAY_ALL;
	UpdateDisplay ();
    }
}


void GLWidget::IncrementDisplayedFace ()
{
    if (DoesSelectFace ())
    {
	++m_displayedFaceIndex;
        Body& body = *GetCurrentFoam ().GetBodies ()[m_displayedBodyIndex];
        if (m_displayedFaceIndex == body.GetOrientedFaces ().size ())
            m_displayedFaceIndex = DISPLAY_ALL;
	UpdateDisplay ();
    }
}

void GLWidget::IncrementDisplayedEdge ()
{
    if (DoesSelectEdge ())
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
    if (DoesSelectBody ())
    {
	if (m_displayedBodyIndex == DISPLAY_ALL)
	    m_displayedBodyIndex = 
		GetFoamAlongTime ().GetFoam (0)->GetBodies ().size ();
	--m_displayedBodyIndex;
	m_displayedFaceIndex = DISPLAY_ALL;
	UpdateDisplay ();
    }
}

void GLWidget::DecrementDisplayedFace ()
{
    if (DoesSelectFace ())
    {
        Body& body = *GetCurrentFoam ().GetBodies ()[m_displayedBodyIndex];
        if (m_displayedFaceIndex == DISPLAY_ALL)
            m_displayedFaceIndex = body.GetOrientedFaces ().size ();
	--m_displayedFaceIndex;
	UpdateDisplay ();
    }
}

void GLWidget::DecrementDisplayedEdge ()
{
    if (DoesSelectEdge ())
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


void GLWidget::ToggledCenterPathDisplayBody (bool checked)
{
    m_centerPathDisplayBody = checked;
    UpdateDisplay ();
}

void GLWidget::ToggledEdgesNormal (bool checked)
{
    view (checked, EDGES);
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

void GLWidget::ToggledEdgesTessellation (bool checked)
{
    m_edgesTessellation = checked;
    UpdateDisplay ();
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


BodiesAlongTime& GLWidget::GetBodiesAlongTime ()
{
    return GetFoamAlongTime ().GetBodiesAlongTime ();
}

BodyAlongTime& GLWidget::GetBodyAlongTime (size_t id)
{
    return GetBodiesAlongTime ().GetOneBody (id);
}

boost::shared_ptr<Body>  GLWidget::GetDisplayedBody () const
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

boost::shared_ptr<Face> GLWidget::GetDisplayedFace () const
{
    size_t i = GetDisplayedFaceIndex ();
    if (m_displayedBodyIndex != DISPLAY_ALL)
    {
	Body& body = *GetDisplayedBody ();
	return body.GetFace (i);
    }
    RuntimeAssert (false, "There is no displayed face");
    return boost::shared_ptr<Face>();
}

boost::shared_ptr<Edge>  GLWidget::GetDisplayedEdge () const
{
    if (m_displayedBodyIndex != DISPLAY_ALL && 
	m_displayedFaceIndex != DISPLAY_ALL)
    {
	boost::shared_ptr<Face>  face = GetDisplayedFace ();
	return face->GetEdge (m_displayedEdgeIndex);
    }
    RuntimeAssert (false, "There is no displayed edge");
    return boost::shared_ptr<Edge>();
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
}

void GLWidget::quadricErrorCallback (GLenum errorCode)
{
    const GLubyte* message = gluErrorString (errorCode);
    cdbg << "Quadric error: " << message << endl;
}
