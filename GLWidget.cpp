/**
 * @file   GLWidget.cpp
 * @author Dan R. Lipsa
 *
 * Definitions for the widget for displaying foam bubbles using OpenGL
 */


#include "Body.h"
#include "BodyAlongTime.h"
#include "BodySelector.h"
#include "ColorBarModel.h"
#include "FoamAlongTime.h"
#include "Debug.h"
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
class OpenGLFeature
{
public:
    enum Type
    {
	BOOLEAN,
	INTEGER,
	STRING,
	SEPARATOR
    };

    OpenGLFeature (GLenum what, Type type, const char* name) :
	m_what (what), m_type (type), m_name (name)
    {
    }

    OpenGLFeature (const char* name) :
	m_what (0), m_type (SEPARATOR), m_name (name)
    {
    }


    /**
     * Reads an OpenGLFeature from OpenGL
     */
    string get () const
    {
	ostringstream ostr;
	switch (m_type)
	{
	case INTEGER:
	{
	    GLint where;
	    glGetIntegerv (m_what, &where);
	    ostr << where;
	    return ostr.str ();
	}
	
	case BOOLEAN:
	{
	    GLboolean where;
	    glGetBooleanv (m_what, &where);
	    ostr << (where ? "true" : "false");
	    return ostr.str ();
	}
	
	case STRING:
	    ostr << glGetString (m_what);
	    return ostr.str ();
	
	case SEPARATOR:
	    return "";
	
	default:
	{
	    ThrowException ("Invalid storage type for OpenGLFeature");
	    return 0;
	}
	}
    }

    void print () const
    {
	cdbg << m_name;
	if (m_what != 0)
	    cdbg << ": " << get ();
	cdbg << endl;
    }

private:
    /**
     * What OpenGL characteristic
     */
    GLenum m_what;
    /**
     * Data type of the required information
     */
    Type m_type;
    /**
     * Name of the OpenGL characteristic
     */
    string m_name;
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
void detectOpenGLError (string message = "")
{
    GLenum errCode;
    if ((errCode = glGetError()) != GL_NO_ERROR)
        cdbg << "OpenGL Error " << message.c_str () << ":"
		    << gluErrorString(errCode);
}

/**
 * Prints information  about the OpenGL  implementation (hardware) the
 * program runs on.
 */
void printOpenGLInfo ()
{
    boost::array<OpenGLFeature, 22> info = {{
	OpenGLFeature (GL_VENDOR, OpenGLFeature::STRING, "GL_VENDOR"),
	OpenGLFeature (GL_RENDERER, OpenGLFeature::STRING, "GL_RENDERER"),
	OpenGLFeature (GL_VERSION, OpenGLFeature::STRING, "GL_VERSION"),
	
	OpenGLFeature ("--- Texture ---"),
	OpenGLFeature (GL_MAX_TEXTURE_SIZE, OpenGLFeature::INTEGER,
		       "GL_MAX_TEXTURE_SIZE"),
	OpenGLFeature (GL_MAX_COLOR_ATTACHMENTS_EXT, OpenGLFeature::INTEGER,
		       "GL_MAX_COLOR_ATTACHMENTS_EXT"),
	OpenGLFeature ("--- Vertex Shader ---"),
	OpenGLFeature (GL_MAX_VERTEX_ATTRIBS, OpenGLFeature::INTEGER,
		       "GL_MAX_VERTEX_ATTRIBS"),

	
	OpenGLFeature (GL_AUX_BUFFERS, OpenGLFeature::INTEGER, "AUX_BUFFERS"),
        OpenGLFeature (GL_RED_BITS, OpenGLFeature::INTEGER, "RED_BITS"),
        OpenGLFeature (GL_GREEN_BITS, OpenGLFeature::INTEGER, "GREEN_BITS"),
        OpenGLFeature (GL_BLUE_BITS, OpenGLFeature::INTEGER, "BLUE_BITS"),
        OpenGLFeature (GL_ALPHA_BITS, OpenGLFeature::INTEGER, "ALPHA_BITS"),
        OpenGLFeature (GL_ACCUM_RED_BITS, OpenGLFeature::INTEGER, 
		       "ACCUM_RED_BITS"),
        OpenGLFeature (GL_ACCUM_GREEN_BITS, OpenGLFeature::INTEGER, 
		       "ACCUM_GREEN_BITS"),
        OpenGLFeature (GL_ACCUM_BLUE_BITS, OpenGLFeature::INTEGER, 
		       "ACCUM_BLUE_BITS"),
        OpenGLFeature (GL_ACCUM_ALPHA_BITS, OpenGLFeature::INTEGER, 
		       "ACCUM_ALPHA_BITS"),
        OpenGLFeature (GL_INDEX_BITS, OpenGLFeature::INTEGER, "INDEX_BITS"),
        OpenGLFeature (GL_DEPTH_BITS, OpenGLFeature::INTEGER, "DEPTH_BITS"),
        OpenGLFeature (GL_STENCIL_BITS, OpenGLFeature::INTEGER, "STENCIL_BITS"),
	
	OpenGLFeature (GL_STEREO, OpenGLFeature::BOOLEAN, "GL_STEREO"),
	OpenGLFeature (GL_DOUBLEBUFFER, OpenGLFeature::BOOLEAN,
		       "GL_DOUBLEBUFFER")
	}};
    for_each (info.begin (), info.end (),
	      boost::bind (&OpenGLFeature::print, _1));
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
      m_contextAlpha (0.05),
      m_angleOfView (0),
      m_edgesTorusTubes (false),
      m_facesTorusTubes (false),
      m_edgesBodyCenter (false),
      m_edgesTessellation (true),
      m_centerPathDisplayBody (false),
      m_boundingBox (false),
      m_centerPathColor (BodyProperty::NONE),
      m_facesColor (BodyProperty::NONE),
      m_notAvailableCenterPathColor (Qt::black),
      m_notAvailableFaceColor (Qt::white),
      m_bodySelector (new CycleSelector (*this)),
      m_useColorMap (false),
      m_colorBarModel (new ColorBarModel ())
{
    const int DOMAIN_INCREMENT_COLOR[] = {100, 0, 200};
    const int POSSIBILITIES = 3; //domain increment can be *, - or +
    using G3D::Vector3int16;
    m_rotate = G3D::Matrix3::identity ();
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
    createActions ();
}


void GLWidget::createActions ()
{
    m_actionResetTransformation = boost::make_shared<QAction> (
	tr("&Reset Transformation"), this);
    m_actionResetTransformation->setShortcut(
	QKeySequence (tr ("Shift+R")));
    m_actionResetTransformation->setStatusTip(tr("Reset Transformation"));
    connect(m_actionResetTransformation.get (), SIGNAL(triggered()),
	    this, SLOT(ResetTransformation ()));    
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
    boost::shared_ptr<Edge> e = GetCurrentFoam ().GetStandardEdge ();
    double length = (*e->GetEnd () - *e->GetBegin ()).length ();

    m_edgeRadius = length / 20;
    m_arrowBaseRadius = 5 * m_edgeRadius;
    m_arrowHeight = 10 * m_edgeRadius;
    calculateCameraDistance ();
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
	UpdateDisplayList ();
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
    const G3D::Vector3& max = GetFoamAlongTime ().GetAABox ().high ();
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

G3D::AABox GLWidget::calculateCenteredViewingVolume () const
{
    G3D::AABox aaBox = GetFoamAlongTime ().GetAABox ();
    EncloseRotation (&aaBox);
    G3D::Vector3 center = aaBox.center ();
    return G3D::AABox (aaBox.low () - center, aaBox.high () - center);
}

void GLWidget::viewingTransformation () const
{
    glLoadIdentity ();
    glTranslate (- m_cameraDistance * G3D::Vector3::unitZ ());    
}

void GLWidget::modelingTransformation () const
{
    /**
     *  y        z
     *    x ->     y
     * z        x
     */
    const static G3D::Matrix3 evolverAxes (0, 1, 0,  0, 0, 1,  1, 0, 0); 
    glMultMatrix (m_rotate);
    if (GetCurrentFoam ().GetSpaceDimension () == 3)
    {
	glMultMatrix (evolverAxes);
	glMultMatrix (GetCurrentFoam ().GetViewMatrix ().
		      approxCoordinateFrame ().rotation);
    }
    glTranslate (-GetFoamAlongTime ().GetAABox ().center ());
    glCallList (m_object);
}


void GLWidget::projectionTransformation () const
{
    using G3D::Vector3;
    G3D::AABox centeredViewingVolume = calculateCenteredViewingVolume ();
    Vector3 translation (m_cameraDistance * G3D::Vector3::unitZ ());
    G3D::AABox viewingVolume (
	centeredViewingVolume.low () - translation,
	centeredViewingVolume.high () - translation);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    if (m_angleOfView == 0)
    {
	glOrtho (viewingVolume.low ().x, viewingVolume.high ().x,
		 viewingVolume.low ().y, viewingVolume.high ().y, 
		 -viewingVolume.high ().z, -viewingVolume.low ().z);
    }
    else
    {
	glFrustum (viewingVolume.low ().x, viewingVolume.high ().x,
		   viewingVolume.low ().y, viewingVolume.high ().y, 
		   -viewingVolume.high ().z, -viewingVolume.low ().z);
    }
    glMatrixMode (GL_MODELVIEW);
}


void GLWidget::calculateCameraDistance ()
{
    G3D::AABox centeredViewingVolume = calculateCenteredViewingVolume ();
    G3D::Vector3 diagonal = 
	centeredViewingVolume.high () - centeredViewingVolume.low ();
    if (m_angleOfView == 0)
	m_cameraDistance = diagonal.z;
    else
	m_cameraDistance = diagonal.y / 2 / 
	    tan (m_angleOfView * M_PI / 360) + diagonal.z / 2;
}


void GLWidget::ResetTransformation ()
{
    m_rotate = G3D::Matrix3::identity ();
    resizeGL (width (), height ());
    UpdateDisplayList ();
}

void GLWidget::ChangePalette ()
{
    
}

void GLWidget::SelectAll ()
{
    m_displayedBodyIndex = DISPLAY_ALL;
    m_displayedFaceIndex = DISPLAY_ALL;
    m_displayedEdgeIndex = DISPLAY_ALL;
    UpdateDisplayList ();
}

void GLWidget::DeselectAll ()
{
}

void GLWidget::Info ()
{

    const Foam& foam = *GetFoamAlongTime ().GetFoam (0);
    size_t timeSteps = GetFoamAlongTime ().GetTimeSteps ();

    VertexSet vertexSet;
    EdgeSet edgeSet;
    FaceSet faceSet;
    foam.GetVertexSet (&vertexSet);
    foam.GetEdgeSet (&edgeSet);
    foam.GetFaceSet (&faceSet);

    ostringstream ostr;
    ostr << 
	"<table border>"
	"<tr><th>Bodies</th><td>" << foam.GetBodies ().size () << "</td></tr>"
	"<tr><th>Faces</th><td>" << faceSet.size () << "</td></tr>"
	"<tr><th>Edges</th><td>" << edgeSet.size () << "</td></tr>"
	"<tr><th>Vertices</th><td>" << vertexSet.size () << "</td></tr>"
	"<tr><th>Time steps</th><td>" << timeSteps << "</td></tr>"
	"</table>" << endl;

    QMessageBox msgBox (this);
    msgBox.setText(ostr.str().c_str ());
    msgBox.exec();
}


// Uses antialiased points and lines
// See OpenGL Programming Guide, 7th edition, Chapter 6: Blending,
// Antialiasing, Fog and Polygon Offset page 293
void GLWidget::initializeGL()
{
    glClearColor (1., 1., 1., 0.);
    
    printOpenGLInfo ();
    GLWidget::disableLighting ();
    m_object = displayList (m_viewType);
    glEnable(GL_DEPTH_TEST);

    // for anti-aliased lines and points
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_LINE_SMOOTH);
    glEnable (GL_POINT_SMOOTH);
    projectionTransformation ();

}

void GLWidget::paintGL()
{
    using G3D::Vector3;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    viewingTransformation ();
    modelingTransformation ();
    detectOpenGLError ();
}



void GLWidget::resizeGL(int width, int height)
{
    using G3D::Rect2D;using G3D::Vector2;
    Vector2 viewportStart = m_viewport.x0y0 ();
    double ratio = 1;
    if ((static_cast<double>(width) / height) > ratio)
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
}

void GLWidget::setRotation (int axis, double angleRadians)
{
    using G3D::Matrix3;using G3D::Vector3;
    Vector3 axes[3] = {
	Vector3::unitX (), Vector3::unitY (), Vector3::unitZ ()
    };
    m_rotate = Matrix3::fromAxisAngle (axes[axis], angleRadians) * m_rotate;
}

double GLWidget::ratioFromCenter (const QPoint& p)
{
    using G3D::Vector2;
    Vector2 center = m_viewport.center ();
    Vector2 lastPos (m_lastPos.x (), m_lastPos.y());
    Vector2 currentPos (p.x (), p.y ());
    double ratio = 
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
    double dxRadians = static_cast<double>(dx) * (M_PI / 2) / side;
    double dyRadians = static_cast<double>(dy) * (M_PI / 2) / side;
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

void GLWidget::scaleViewport (const QPoint& position)
{
    double ratio = ratioFromCenter (position);
    Scale (&m_viewport, ratio);
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
    case InteractionMode::TRANSLATE:
	translateViewport (event->pos ());
	break;
    case InteractionMode::SCALE:
	scaleViewport (event->pos ());
    default:
	break;
    }
    updateGL ();
    m_lastPos = event->pos();
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
}


void GLWidget::displayOriginalDomain () const
{
    if (m_torusOriginalDomainDisplay)
	displayBox (GetCurrentFoam().GetOriginalDomain ());
}

void GLWidget::displayBox (const OOBox& oobox) const
{
    glPushAttrib (GL_POLYGON_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glLineWidth (1.0);
    qglColor (QColor (Qt::black));
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

    displayOpositeFaces (G3D::Vector3::zero (), 
			 oobox[0], oobox[1], oobox[2]);
    displayOpositeFaces (G3D::Vector3::zero (),
			 oobox[1], oobox[2], oobox[0]);
    displayOpositeFaces (G3D::Vector3::zero (), 
			 oobox[2], oobox[0], oobox[1]);
    glPopAttrib ();
}


void GLWidget::displayAABox () const
{
    if (m_boundingBox)
	displayBox (GetCurrentFoam ().GetAABox ());
}

void GLWidget::displayAxes () const
{
    using G3D::Vector3;
    const G3D::AABox& aabb = GetCurrentFoam ().GetAABox ();
    Vector3 origin = aabb.low ();
    Vector3 diagonal = aabb.high () - origin;
    Vector3 first = origin + diagonal.x * Vector3::unitX ();
    Vector3 second = origin + diagonal.y * Vector3::unitY ();
    Vector3 third = origin + diagonal.z * Vector3::unitZ ();
    glBegin (GL_LINES);

    qglColor (QColor (Qt::red));
    glVertex (origin);
    glVertex (first);

    qglColor (QColor (Qt::green));
    glVertex (origin);
    glVertex (second);

    qglColor (QColor (Qt::blue));
    glVertex (origin);
    glVertex (third);
    glEnd ();
}


void GLWidget::displayBox (const G3D::AABox& aabb) const
{
    using G3D::Vector3;
    glPushAttrib (GL_POLYGON_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glLineWidth (1.0);
    qglColor (QColor (Qt::black));
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    Vector3 diagonal = aabb.high () - aabb.low ();
    Vector3 first = diagonal.x * Vector3::unitX ();
    Vector3 second = diagonal.y * Vector3::unitY ();
    Vector3 third = diagonal.z * Vector3::unitZ ();
    
    displayOpositeFaces (aabb.low (), first, second, third);
    displayOpositeFaces (aabb.low (), second, third, first);
    displayOpositeFaces (aabb.low (), third, first, second);
    glPopAttrib ();
}


template<typename displayEdge>
GLuint GLWidget::displayListEdges () const
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
	      displayEdge> > > (*this, *m_bodySelector));
    displayStandaloneEdges<displayEdge> ();

    glPopAttrib ();
    displayOriginalDomain ();
    displayCenterOfBodies ();
    displayAABox ();
    glEndList();
    return list;
}

template<typename displayEdge>
void GLWidget::displayStandaloneEdges () const
{
    const Foam::Edges& standaloneEdges = 
	GetCurrentFoam ().GetStandaloneEdges ();
    BOOST_FOREACH (boost::shared_ptr<Edge> edge, standaloneEdges)
	displayEdge (*this, DisplayElement::FOCUS) (edge);
}


GLuint GLWidget::displayListEdgesNormal () const
{
    return m_torusOriginalDomainClipped ?
	displayListEdges <DisplayEdgeTorusClipped> () :
	displayListEdges <DisplayEdgeWithColor<> >();
}

GLuint GLWidget::displayListEdgesTorusTubes () const
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

GLuint GLWidget::displayListEdgesTorusLines () const
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


void GLWidget::displayCenterOfBodies () const
{
    if ((m_viewType == EDGES && m_edgesBodyCenter) ||
	m_viewType == CENTER_PATHS)
    {
	glPushAttrib (GL_POINT_BIT | GL_CURRENT_BIT);
	glPointSize (4.0);
	qglColor (QColor (Qt::red));
	const Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
	for_each (bodies.begin (), bodies.end (), 
		  DisplayBodyCenter (*this, *m_bodySelector));
	glPopAttrib ();
    }
}

GLuint GLWidget::displayListFacesNormal () const
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);

    const Foam& foam = GetCurrentFoam ();
    const Foam::Bodies& bodies = foam.GetBodies ();
    if (foam.IsQuadratic ())
    {
	displayFacesContour<DisplaySameEdges> (bodies);
	displayFacesInterior<DisplaySameEdges> (bodies);
	displayStandaloneFaces<DisplaySameEdges> ();
    }
    else
    {
	displayFacesContour<DisplaySameTriangles> (bodies);
	displayFacesInterior<DisplaySameTriangles> (bodies);
	displayStandaloneFaces<DisplaySameTriangles> ();
    }

    displayStandaloneEdges< DisplayEdgeWithColor<> > ();
    displayOriginalDomain ();
    displayAABox ();
    //displayAxes ();
    glEndList();
    return list;
}


template<typename displaySameEdges>
void GLWidget::displayStandaloneFaces () const
{
    const Foam::Faces& faces = GetCurrentFoam ().GetStandaloneFaces ();
    displayFacesContour<displaySameEdges> (faces);
    displayFacesInterior<displaySameEdges> (faces);
}

template<typename displaySameEdges>
void GLWidget::displayFacesContour (const Foam::Faces& faces) const
{
    glColor (G3D::Color4 (Color::GetValue(Color::BLACK), 
			  GetContextAlpha ()));
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    for_each (faces.begin (), faces.end (),
	      DisplayFace<displaySameEdges> (*this));
}

template<typename displaySameEdges>
void GLWidget::displayFacesContour (const Foam::Bodies& bodies) const
{
    glColor (G3D::Color4 (Color::GetValue(Color::BLACK), 
			  GetContextAlpha ()));
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody< DisplayFace<displaySameEdges> > (
		  *this, *m_bodySelector));
}

// See OpenGL Programming Guide, 7th edition, Chapter 6: Blending,
// Antialiasing, Fog and Polygon Offset page 293
template<typename displaySameEdges>
void GLWidget::displayFacesInterior (const Foam::Bodies& bodies) const
{
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1, 1);
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<DisplayFaceWithColor<displaySameEdges> > (
		  *this, *m_bodySelector, 
		  DisplayElement::TRANSPARENT_CONTEXT, m_facesColor));
    glDisable (GL_POLYGON_OFFSET_FILL);
}

template<typename displaySameEdges>
void GLWidget::displayFacesInterior (const Foam::Faces& faces) const
{
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1, 1);
    for_each (faces.begin (), faces.end (), 
	      DisplayFaceWithColor<displaySameEdges> (*this));
    glDisable (GL_POLYGON_OFFSET_FILL);
}


GLuint GLWidget::displayListFacesLighting () const
{
    GLuint list = glGenLists(1);
    const Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
    glNewList(list, GL_COMPILE);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    for_each (bodies.begin (), bodies.end (),
              DisplayBody<DisplayFaceWithNormal>(*this, *m_bodySelector));
    glEndList();
    return list;
}

GLuint GLWidget::displayListFacesTorusTubes () const
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


GLuint GLWidget::displayListFacesTorusLines () const
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

GLuint GLWidget::displayListCenterPaths () const
{
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glLineWidth (1.0);
    displayCenterPaths ();
    if (IsCenterPathDisplayBody ())
    {
	const Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
	for_each (bodies.begin (), bodies.end (),
		  DisplayBody<DisplayFace<
		  DisplayEdges<DisplayEdgeWithColor<
		  DisplayElement::DONT_DISPLAY_TESSELLATION> > > > (
		      *this, *m_bodySelector, 
		      DisplayElement::INVISIBLE_CONTEXT));
	displayCenterOfBodies ();
    }
    displayOriginalDomain ();
    displayAABox ();
    displayStandaloneEdges< DisplayEdgeWithColor<> > ();
    glEndList();
    return list;
}

void GLWidget::displayCenterPaths () const
{
    glPushAttrib (GL_CURRENT_BIT);
    const BodiesAlongTime::BodyMap& bats = GetBodiesAlongTime ().GetBodyMap ();
    for_each (bats.begin (), bats.end (),
	      DisplayCenterPath (*this, m_centerPathColor, *m_bodySelector));
    glPopAttrib ();
}

GLuint GLWidget::displayList (ViewType type)
{
    RuntimeAssert (type < VIEW_TYPE_COUNT, 
		   "ViewType enum has an invalid value: ", m_viewType);
    return (this->*(VIEW_TYPE_DISPLAY[type].m_displayList)) ();
}

bool GLWidget::IsDisplayedBody (size_t bodyId) const
{
    return (IsDisplayedAllBodies () || GetDisplayedBodyId () == bodyId);
}

bool GLWidget::IsDisplayedBody (const boost::shared_ptr<Body> body) const
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


bool GLWidget::doesSelectBody () const
{
    return 
	m_viewType != EDGES_TORUS &&
	m_viewType != FACES_TORUS;
}

bool GLWidget::doesSelectFace () const
{
    return 
	m_displayedBodyIndex != DISPLAY_ALL;
}

bool GLWidget::doesSelectEdge () const
{
    return 
	m_displayedFaceIndex != DISPLAY_ALL &&
	m_viewType != FACES;
}




void GLWidget::IncrementDisplayedBody ()
{
    if (doesSelectBody ())
    {
	++m_displayedBodyIndex;
	m_displayedFaceIndex = DISPLAY_ALL;
	if (m_displayedBodyIndex == 
	    GetFoamAlongTime ().GetFoam (0)->GetBodies ().size ())
	    m_displayedBodyIndex = DISPLAY_ALL;
	UpdateDisplayList ();
    }
}


void GLWidget::IncrementDisplayedFace ()
{
    if (doesSelectFace ())
    {
	++m_displayedFaceIndex;
        Body& body = *GetCurrentFoam ().GetBodies ()[m_displayedBodyIndex];
        if (m_displayedFaceIndex == body.GetOrientedFaces ().size ())
            m_displayedFaceIndex = DISPLAY_ALL;
	UpdateDisplayList ();
    }
}

void GLWidget::IncrementDisplayedEdge ()
{
    if (doesSelectEdge ())
    {
	++m_displayedEdgeIndex;
	Face& face = *GetDisplayedFace ();
	if (m_displayedEdgeIndex == face.GetOrientedEdges ().size ())
	    m_displayedEdgeIndex = DISPLAY_ALL;
	UpdateDisplayList ();
    }
}

void GLWidget::DecrementDisplayedBody ()
{
    if (doesSelectBody ())
    {
	if (m_displayedBodyIndex == DISPLAY_ALL)
	    m_displayedBodyIndex = 
		GetFoamAlongTime ().GetFoam (0)->GetBodies ().size ();
	--m_displayedBodyIndex;
	m_displayedFaceIndex = DISPLAY_ALL;
	UpdateDisplayList ();
    }
}

void GLWidget::DecrementDisplayedFace ()
{
    if (doesSelectFace ())
    {
        Body& body = *GetCurrentFoam ().GetBodies ()[m_displayedBodyIndex];
        if (m_displayedFaceIndex == DISPLAY_ALL)
            m_displayedFaceIndex = body.GetOrientedFaces ().size ();
	--m_displayedFaceIndex;
	UpdateDisplayList ();
    }
}

void GLWidget::DecrementDisplayedEdge ()
{
    if (doesSelectEdge ())
    {
	Face& face = *GetDisplayedFace ();
	if (m_displayedEdgeIndex == DISPLAY_ALL)
	    m_displayedEdgeIndex = face.GetOrientedEdges ().size ();
	--m_displayedEdgeIndex;
	UpdateDisplayList ();
    }
}



const Foam& GLWidget::GetCurrentFoam () const
{
    return *GetFoamAlongTime ().GetFoam (m_timeStep);
}

Foam& GLWidget::GetCurrentFoam ()
{
    return *GetFoamAlongTime ().GetFoam (m_timeStep);
}


const QColor& GLWidget::GetEndTranslationColor (
    const G3D::Vector3int16& di) const
{
    EndLocationColor::const_iterator it = m_endTranslationColor.find (di);
    RuntimeAssert (it != m_endTranslationColor.end (),
		   "Invalid domain increment ", di);
    return (*it).second;
}


const BodiesAlongTime& GLWidget::GetBodiesAlongTime () const
{
    return GetFoamAlongTime ().GetBodiesAlongTime ();
}

const BodyAlongTime& GLWidget::GetBodyAlongTime (size_t id) const
{
    return GetBodiesAlongTime ().GetBodyAlongTime (id);
}

boost::shared_ptr<Body> GLWidget::GetDisplayedBody () const
{
    return GetBodyAlongTime (GetDisplayedBodyId ()).GetBody (GetTimeStep ());
}

size_t GLWidget::GetDisplayedBodyId () const
{
    return GetFoamAlongTime ().GetFoam (0)->GetBody (
	m_displayedBodyIndex)->GetId ();
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

boost::shared_ptr<Edge> GLWidget::GetDisplayedEdge () const
{
    if (m_displayedBodyIndex != DISPLAY_ALL && 
	m_displayedFaceIndex != DISPLAY_ALL)
    {
	boost::shared_ptr<Face> face = GetDisplayedFace ();
	return face->GetEdge (m_displayedEdgeIndex);
    }
    RuntimeAssert (false, "There is no displayed edge");
    return boost::shared_ptr<Edge>();
}

size_t GLWidget::GetDisplayedEdgeId () const
{
    return GetDisplayedEdge ()->GetId ();
}


void GLWidget::UpdateDisplayList ()
{
    glDeleteLists(m_object, 1);
    m_object = displayList (m_viewType);
    updateGL ();
}



// Slots and slot like methods
// ======================================================================

void GLWidget::ToggledBoundingBox (bool checked)
{
    m_boundingBox = checked;
    UpdateDisplayList ();
}

void GLWidget::ToggledCenterPathDisplayBody (bool checked)
{
    m_centerPathDisplayBody = checked;
    UpdateDisplayList ();
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
    UpdateDisplayList ();
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
    UpdateDisplayList ();
}


void GLWidget::ToggledTorusOriginalDomainDisplay (bool checked)
{
    m_torusOriginalDomainDisplay = checked;
    UpdateDisplayList ();
}

void GLWidget::ToggledTorusOriginalDomainClipped (bool checked)
{
    m_torusOriginalDomainClipped = checked;
    UpdateDisplayList ();
}

void GLWidget::ToggledBodies (bool checked)
{
    view (checked, FACES_LIGHTING);
}

void GLWidget::ToggledCenterPath (bool checked)
{
    view (checked, CENTER_PATHS);
}


void GLWidget::CurrentIndexChangedInteractionMode (int index)
{
    m_interactionMode = static_cast<InteractionMode::Enum>(index);
}

void GLWidget::ValueChangedSliderTimeSteps (int timeStep)
{
    m_timeStep = timeStep;
    UpdateDisplayList ();
}

void GLWidget::ValueChangedAngleOfView (int newIndex)
{
    makeCurrent ();
    m_angleOfView = newIndex;
    calculateCameraDistance ();
    projectionTransformation ();
    updateGL ();
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
    qWarning () << "Quadric error:" << message;
}

void GLWidget::displayOpositeFaces (G3D::Vector3 origin,
				    G3D::Vector3 faceFirst,
				    G3D::Vector3 faceSecond,
				    G3D::Vector3 translation)
{
    G3D::Vector3 faceOrigin;
    G3D::Vector3 faceSum = faceFirst + faceSecond;
    G3D::Vector3 translations[] = {origin, translation};
    for (int i = 0; i < 2; i++)
    {
	faceOrigin += translations[i];
	faceFirst += translations[i];
	faceSecond += translations[i];
	faceSum += translations[i];

	glBegin (GL_POLYGON);
	glVertex (faceOrigin);
	glVertex (faceFirst);
	glVertex (faceSum);
	glVertex (faceSecond);
	glEnd ();
    }
}

void GLWidget::CurrentIndexChangedCenterPathColor (int value)
{
    RuntimeAssert (value < BodyProperty::COUNT,
		   "Invalid BodyProperty: ", value);
    m_centerPathColor = BodyProperty::FromSizeT(value);
}

void GLWidget::CurrentIndexChangedFacesColor (int value)
{
    RuntimeAssert (value < BodyProperty::COUNT,
		   "Invalid BodyProperty: ", value);
    m_facesColor = BodyProperty::FromSizeT(value);
}


void GLWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu (this);
    menu.addAction (m_actionResetTransformation.get ());
    menu.addAction (m_actionSelectAll.get ());
    menu.addAction (m_actionDeselectAll.get ());
    menu.addAction (m_actionInfo.get ());
    menu.exec (event->globalPos());
}


void GLWidget::SetActionSelectAll (
    boost::shared_ptr<QAction> actionSelectAll)
{
    m_actionSelectAll = actionSelectAll;
    connect(m_actionSelectAll.get (), SIGNAL(triggered()),
	    this, SLOT(SelectAll ()));    
}

void GLWidget::SetActionInfo (boost::shared_ptr<QAction> actionInfo)
{
    m_actionInfo = actionInfo;
    connect(m_actionInfo.get (), SIGNAL(triggered()),
	    this, SLOT(Info ()));    
}


void GLWidget::SetActionDeselectAll (
    boost::shared_ptr<QAction> actionDeselectAll)
{
    m_actionDeselectAll = actionDeselectAll;
    connect(m_actionDeselectAll.get (), SIGNAL(triggered()),
	    this, SLOT(DeselectAll ()));
}

QColor GLWidget::MapScalar (double value) const
{
    if (m_useColorMap)
	return m_colorBarModel->MapScalar (value);
    else
	return Qt::black;
}

void GLWidget::ColorBarModelChanged (ColorBarModel* colorBarModel)
{
    *m_colorBarModel = *colorBarModel;
    UpdateDisplayList ();
}
