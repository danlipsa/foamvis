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
#include "GLWidget.h"
#include "Debug.h"
#include "DisplayFaceAverage.h"
#include "DisplayBlend.h"
#include "DisplayBodyFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "DisplayFaceFunctors.h"
#include "DisplayVertexFunctors.h"
#include "OpenGLInfo.h"
#include "OpenGLUtils.h"
#include "Utils.h"

// Private Classes
// ======================================================================

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


// Static Fields
// ======================================================================

const size_t GLWidget::DISPLAY_ALL(numeric_limits<size_t>::max());
const size_t GLWidget::QUADRIC_SLICES (20);
const size_t GLWidget::QUADRIC_STACKS (20);


// Methods
// ======================================================================

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent), 
      m_viewType (ViewType::COUNT),
      m_torusOriginalDomainDisplay (false),
      m_torusOriginalDomainClipped (false),
      m_interactionMode (InteractionMode::ROTATE),
      m_foamAlongTime (0), m_timeStep (0),
      m_displayedBodyIndex (DISPLAY_ALL), m_displayedFaceIndex (DISPLAY_ALL),
      m_displayedEdgeIndex (DISPLAY_ALL),
      m_contextAlpha (0.05),
      m_rotationMatrixModel (G3D::Matrix3::identity ()),
      m_scalingFactorModel (1),
      m_lighting (false),
      m_rotationMatrixLight (G3D::Matrix3::identity ()),
      m_showLightPosition (false),
      m_angleOfView (0),
      m_edgesTubes (false),
      m_facesShowEdges (true),
      m_edgesBodyCenter (false),
      m_edgesTessellation (true),
      m_centerPathBodyShown (false),
      m_boundingBox (false),
      m_centerPathColor (BodyProperty::NONE),
      m_facesColor (BodyProperty::NONE),
      m_notAvailableCenterPathColor (Qt::black),
      m_notAvailableFaceColor (Qt::white),
      m_bodySelector (new CycleSelector (*this)),
      m_useColorMap (false),
      m_colorBarModel (new ColorBarModel ()),
      m_colorBarTexture (0),
      m_srcAlphaBlend (1),
      m_playMovie (false)
{
    makeCurrent ();
    m_displayBlend.reset (new DisplayBlend (*this));
    m_displayFaceAverage.reset (new DisplayFaceAverage (*this));
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
    initQuadrics ();
    initViewTypeDisplay ();    
    createActions ();
}

void GLWidget::initQuadrics ()
{
    m_quadric = gluNewQuadric ();
    gluQuadricCallback (m_quadric, GLU_ERROR,
			reinterpret_cast<void (*)()>(&quadricErrorCallback));
    gluQuadricDrawStyle (m_quadric, GLU_FILL);
    gluQuadricNormals (m_quadric, GLU_SMOOTH);
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
    // WARNING: This has to be in the same order as ViewType::Enum
    boost::array<ViewTypeDisplay, ViewType::COUNT> vtd = 
	{{	
	{&GLWidget::displayEdgesNormal, identity<Lighting> (NO_LIGHTING)},
	
	{&GLWidget::displayEdgesTorus, 
	 bl::if_then_else_return (
	     bl::bind (&GLWidget::edgeLighting, this), 
	     LIGHTING, NO_LIGHTING)},

	{&GLWidget::displayFacesTorus, 
	 bl::if_then_else_return (bl::bind (&GLWidget::edgeLighting, this),
				  LIGHTING, NO_LIGHTING)},
	

	{&GLWidget::displayFacesNormal, 
	 bl::if_then_else_return (bl::bind (&GLWidget::hasLighting, this),
				  LIGHTING, NO_LIGHTING)},
	
	{&GLWidget::displayFacesAverage, identity<Lighting> (NO_LIGHTING)},
	
	
	{&GLWidget::displayCenterPathsWithBodies, 
	 bl::if_then_else_return (bl::bind (&GLWidget::edgeLighting, this),
				  LIGHTING, NO_LIGHTING)},

	}};
    copy (vtd.begin (), vtd.end (), m_viewTypeDisplay.begin ());
}

void GLWidget::SetFoamAlongTime (FoamAlongTime* dataAlongTime) 
{
    m_foamAlongTime = dataAlongTime;
    setEdgeRadius (0);

    calculateCameraDistance ();
    initialLightPosition ();
}

bool GLWidget::edgeLighting () const
{
    return m_edgesTubes && m_lighting;
}


void GLWidget::setEdgeRadius (int sliderValue, int maxValue)
{
    boost::shared_ptr<Edge> e = GetCurrentFoam ().GetStandardEdge ();
    double length = (*e->GetEnd () - *e->GetBegin ()).length ();

    double r = length / 20;
    double R = 4 * r;

    m_edgesTubes = (sliderValue != 0);
    m_edgeRadius = (R - r) * sliderValue / maxValue + r;
    m_arrowBaseRadius = 5 * m_edgeRadius;
    m_arrowHeight = 10 * m_edgeRadius;    
}

GLWidget::~GLWidget()
{
    makeCurrent();
    gluDeleteQuadric (m_quadric);
    m_quadric = 0;
}

void GLWidget::view (bool checked, ViewType::Enum view)
{
    if (checked)
    {
	makeCurrent ();
        m_viewType = view;
	if ((m_viewTypeDisplay[view].m_lighting) () == LIGHTING)
	    glEnable (GL_LIGHTING);
	else
	    glDisable (GL_LIGHTING);
	updateGL ();
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

void GLWidget::initialLightPosition ()
{
    m_lightPosition =  GetFoamAlongTime ().GetBoundingBox ().high ();
    m_directionalLight = true;
}

void GLWidget::setLightPosition ()
{
    if (! m_lighting)
	return;

    // light position
    glPushMatrix ();
    {
	glLoadIdentity ();
	glTranslate (- m_cameraDistance * G3D::Vector3::unitZ ());
	glMultMatrix (m_rotationMatrixLight);
	GLfloat lightPosition[] = {m_lightPosition.x, m_lightPosition.y,
				   m_lightPosition.z, ! m_directionalLight};
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	if (m_showLightPosition)
	{
	    glPointSize (4);
	    glBegin (GL_POINTS);
	    glVertex (m_lightPosition);
	    glEnd ();
	}
    }
    glPopMatrix ();
}


void GLWidget::initLighting ()
{    
    // light colors
    GLfloat lightAmbient[] = {0, 0, 0, 1.0};     // default (0, 0, 0, 1)
    GLfloat lightDiffuse[] = {1.0, 1.0, 1.0, 1.0};  // default (1, 1, 1, 1)
    GLfloat lightSpecular[] = {1.0, 1.0, 1.0, 1.0}; // default (1, 1, 1, 1)

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    // material colors: ambient and diffuse colors are set using glColor
    GLfloat materialSpecular[] = {1.0, 1.0, 1.0, 1.0}; //(0, 0, 0, 1)
    GLfloat materialShininess[] = {50.0};              // 0
    GLfloat materialEmission[] = {0.0, 0.0, 0.0, 1.0}; //(0, 0, 0, 1)
    glEnable (GL_COLOR_MATERIAL);
    glColorMaterial (GL_FRONT, GL_AMBIENT_AND_DIFFUSE); 
    //GLfloat materialAmbient[] = {.2, .2, .2, 1.0};     //(.2, .2, .2, 1.0)
    //GLfloat materialDiffuse[] = {.8, .8, .8, 1.0};     //(.8, .8, .8, 1.0)
    //glMaterialfv (GL_FRONT, GL_AMBIENT, materialAmbient);  
    //glMaterialfv (GL_FRONT, GL_DIFFUSE, materialDiffuse);

    glMaterialfv (GL_FRONT, GL_SPECULAR, materialSpecular);  
    glMaterialfv (GL_FRONT, GL_SHININESS, materialShininess);
    glMaterialfv (GL_FRONT, GL_EMISSION, materialEmission);

    glLightModeli (GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
    glShadeModel (GL_SMOOTH);
    glEnable(GL_LIGHT0);
}

G3D::AABox GLWidget::calculateCenteredViewingVolume () const
{
    using G3D::Vector3;
    G3D::AABox boundingBox = GetFoamAlongTime ().GetBoundingBox ();
    EncloseRotation (&boundingBox);
    Vector3 center = boundingBox.center ();
    return G3D::AABox (boundingBox.low () - center, 
		       boundingBox.high () - center);
}

void GLWidget::ModelViewTransformNoRotation () const
{
    glLoadIdentity ();
    glTranslate (- m_cameraDistance * G3D::Vector3::unitZ () - 
		 GetFoamAlongTime ().GetBoundingBox ().center ());
}

void GLWidget::modelViewTransform () const
{
    glLoadIdentity ();
    glTranslate (- m_cameraDistance * G3D::Vector3::unitZ ());    
    glMultMatrix (m_rotationMatrixModel);
    if (GetCurrentFoam ().GetDimension () == 3)
	rotateSurfaceEvolverCompatible ();
    glTranslate (-GetFoamAlongTime ().GetBoundingBox ().center ());
}

void GLWidget::projectionTransform () const
{
    G3D::AABox centeredViewingVolume = calculateCenteredViewingVolume ();
    G3D::Vector3 translation (m_cameraDistance * G3D::Vector3::unitZ ());
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

void GLWidget::ViewportTransform (
    int width, int height, double scale,
    G3D::Rect2D* viewport) const
{
    G3D::Rect2D vv2dScreen;
    G3D::Rect2D windowWorld;
    viewingVolumeCalculations (width, height, &vv2dScreen, &windowWorld);
    if (GetCurrentFoam ().GetDimension () == 2)
    {
	const double ADJUST = 99.0/100.0;
	G3D::Rect2D bb2dScreen;
	double change;
	boundingBoxCalculations (
	    width, height, windowWorld, &bb2dScreen, &change);
	scale *= (change * ADJUST);
    }
    Scale (&vv2dScreen, scale);
    if (viewport != 0)
	*viewport = vv2dScreen;
    glViewport (vv2dScreen);

/*
    G3D::AABox bb = GetFoamAlongTime ().GetBoundingBox ();
    G3D::Vector3 low = bb.low ();
    G3D::Vector3 high = bb.high ();
    // second method to calculate bb2dScreen2 which seems more precise.
    G3D::Rect2D bb2dScreen2 = G3D::Rect2D::xyxy (
	gluProject (low).xy (),
	gluProject (G3D::Vector3 (high.x, high.y, low.z)).xy ());
    cdbg << "gluProject = " << bb2dScreen2 << endl
	 << "bb2dScreen = " << bb2dScreen << endl << endl;
*/
    //return QSize (bb2dScreen.width (), bb2dScreen.height ());
}


void GLWidget::viewingVolumeCalculations (
    int width, int height,
    G3D::Rect2D* vv2dScreen, G3D::Rect2D* windowWorld) const
{
    G3D::AABox vv = calculateCenteredViewingVolume ();
    G3D::Rect2D vv2d = G3D::Rect2D::xyxy (vv.low ().xy (), vv.high ().xy ());
    double windowRatio = static_cast<double>(width) / height;
    double vvratio = vv2d.width () / vv2d.height ();
    if (windowRatio > vvratio)
    {
	double newWidth = vvratio * height;
	*vv2dScreen = G3D::Rect2D::xywh ((width - newWidth) / 2, 0,
					     newWidth, height);
	*windowWorld = G3D::Rect2D::xywh (0, 0,
	    vv2d.height () * windowRatio, vv2d.height ());
    }
    else
    {
	double newHeight = width / vvratio;
	*vv2dScreen = G3D::Rect2D::xywh (0, (height - newHeight) / 2,
				   width, newHeight);
	*windowWorld = G3D::Rect2D::xywh (0, 0,
	    vv2d.width (), vv2d.width () / windowRatio);
    }
    //cdbg << "vv2d=" << vv2d << "windowWorld=" << *windowWorld << endl;
}

void GLWidget::boundingBoxCalculations (
    int width, int height, 
    const G3D::Rect2D& windowWorld, G3D::Rect2D* bb2dScreen,
    double* change) const
{
    G3D::AABox bb = GetFoamAlongTime ().GetBoundingBox ();
    //cdbg << "bb=" << bb << endl;
    G3D::Rect2D bb2d = G3D::Rect2D::xyxy (bb.low ().xy (), bb.high ().xy ());
    double bbratio = bb2d.width () / bb2d.height ();
    double windowRatio = static_cast<double>(width) / height;
    if (windowRatio > bbratio)
    {
	*change = windowWorld.height () / bb2d.height ();
	double newWidth = bbratio * height;
	*bb2dScreen = G3D::Rect2D::xywh ((width - newWidth) / 2, 0,
					newWidth, height);
    }
    else
    {
	*change = windowWorld.width () / bb2d.width ();
	double newHeight = width / bbratio;
	*bb2dScreen = G3D::Rect2D::xywh (
	    0, (height - newHeight) / 2, width, newHeight);
    }
}

void GLWidget::rotateSurfaceEvolverCompatible () const
{
    /**
     *  y        z
     *    x ->     y
     * z        x
     */
    const static G3D::Matrix3 evolverAxes (0, 1, 0,  0, 0, 1,  1, 0, 0); 
    glMultMatrix (evolverAxes);
    glMultMatrix (GetCurrentFoam ().GetViewMatrix ().
		  approxCoordinateFrame ().rotation);
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
    m_rotationMatrixModel = G3D::Matrix3::identity ();
    m_rotationMatrixLight = G3D::Matrix3::identity ();
    m_scalingFactorModel = 1;
    initialLightPosition ();
    makeCurrent ();
    ViewportTransform (width (), height (), m_scalingFactorModel, &m_viewport);
    updateGL ();
}

void GLWidget::SelectAll ()
{
    m_displayedBodyIndex = DISPLAY_ALL;
    m_displayedFaceIndex = DISPLAY_ALL;
    m_displayedEdgeIndex = DISPLAY_ALL;
    updateGL ();
}

void GLWidget::DeselectAll ()
{
}

void GLWidget::Info ()
{
    string message = (AreAllBodiesDisplayed ()) ? 
	getFoamsInfo () : 
	GetDisplayedBody ()->ToString ();
    QMessageBox msgBox (this);
    msgBox.setText(message.c_str ());
    msgBox.exec();
}


string GLWidget::getFoamsInfo () const
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
    return ostr.str ();
}


// Uses antialiased points and lines
// See OpenGL Programming Guide, 7th edition, Chapter 6: Blending,
// Antialiasing, Fog and Polygon Offset page 293
void GLWidget::initializeGL()
{
    initializeGLFunctions ();
    glClearColor (Qt::white);    
    glEnable(GL_DEPTH_TEST);
    projectionTransform ();
    initializeTextures ();
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_displayFaceAverage->InitShaders ();
    initLighting ();
}

void GLWidget::paintGL ()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setLightPosition ();
    modelViewTransform ();
    display ();
    //displayTextureColorMap ();
    displayAxes ();
    detectOpenGLError ();
}

void GLWidget::resizeGL(int width, int height)
{
    if (width == 0 && height == 0)
	return;
    ViewportTransform (width, height, m_scalingFactorModel, &m_viewport);
    QSize size = QSize (width, height);
    if (m_srcAlphaBlend < 1)
	m_displayBlend->Init (size);
}

void GLWidget::RenderFromFbo (QGLFramebufferObject& fbo) const
{

    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, fbo.texture ());
    glPushAttrib (GL_VIEWPORT_BIT);
    glViewport (0, 0, width (), height ());

    glPushMatrix ();
    {
	glLoadIdentity ();
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	{
	    glLoadIdentity ();
	    glBegin (GL_QUADS);
	    glTexCoord2i (0, 0);glVertex3i (-1, -1, -1);
	    glTexCoord2i (1, 0);glVertex3i (1, -1, -1);
	    glTexCoord2i (1, 1);glVertex3i (1, 1, -1);
	    glTexCoord2i (0, 1);glVertex3i (-1, 1, -1);
	    glEnd ();
	}
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
    }
    glPopMatrix ();
    glPopAttrib ();
    glDisable (GL_TEXTURE_2D);


/*
    using G3D::Vector3;
    G3D::AABox bb = GetFoamAlongTime ().GetBoundingBox ();
    Vector3 low = bb.low ();
    Vector3 high = bb.high ();
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, fbo.texture ());
    glPolygonMode (GL_FRONT, GL_FILL);
    glBegin (GL_QUADS);
    glTexCoord2i (0, 0);glVertex (low);
    glTexCoord2i (1, 0);glVertex (Vector3 (high.x, low.y, low.z));
    glTexCoord2i (1, 1);glVertex (Vector3 (high.x, high.y, low.z));
    glTexCoord2i (0, 1);glVertex (Vector3 (low.x, high.y, low.z));
    glEnd ();	    
    glDisable (GL_TEXTURE_2D);
*/
}

void GLWidget::setRotation (int axis, double angleRadians, G3D::Matrix3* rotate)
{
    using G3D::Matrix3;using G3D::Vector3;
    Vector3 axes[3] = {
	Vector3::unitX (), Vector3::unitY (), Vector3::unitZ ()
    };
    *rotate = Matrix3::fromAxisAngle (axes[axis], angleRadians) * (*rotate);
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

void GLWidget::rotate (const QPoint& position, G3D::Matrix3* rotate)
{
    int dx = position.x() - m_lastPos.x();
    int dy = position.y() - m_lastPos.y();

    // scale this with the size of the window
    int side = std::min (m_viewport.width (), m_viewport.height ());
    double dxRadians = static_cast<double>(dx) * (M_PI / 2) / side;
    double dyRadians = static_cast<double>(dy) * (M_PI / 2) / side;
    setRotation (0, dyRadians, rotate);
    setRotation (1, dxRadians, rotate);
}

void GLWidget::translateViewport (const QPoint& position)
{
    int dx = position.x() - m_lastPos.x();
    int dy = position.y() - m_lastPos.y();
    m_viewport = G3D::Rect2D::xywh (m_viewport.x0 () + dx,
				    m_viewport.y0 () - dy,
				    m_viewport.width (),
				    m_viewport.height ());
    glViewport (m_viewport);
}

void GLWidget::translateLight (const QPoint& position)
{
    G3D::AABox vv = calculateCenteredViewingVolume ();
    G3D::Vector2 oldPosition = G3D::Vector2 (m_lastPos.x (), m_lastPos.y ());
    G3D::Vector2 newPosition = G3D::Vector2 (position.x (), position.y ());
    G3D::Vector2 viewportCenter = 
	(m_viewport.x1y1 () + m_viewport.x0y0 ()) / 2;
    float screenChange = 
	((newPosition - viewportCenter).length () - 
	 (oldPosition - viewportCenter).length ());
    float ratio = screenChange / 
	(m_viewport.x1y1 () - m_viewport.x0y0 ()).length ();

    m_lightPosition += (ratio * m_lightPosition);
}


void GLWidget::scaleViewport (const QPoint& position)
{
    double ratio = ratioFromCenter (position);
    Scale (&m_viewport, ratio);
    glViewport (m_viewport);
    m_scalingFactorModel = m_scalingFactorModel * ratio;
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    switch (m_interactionMode)
    {
    case InteractionMode::ROTATE:
	rotate (event->pos (), &m_rotationMatrixModel);
	break;
    case InteractionMode::TRANSLATE:
	translateViewport (event->pos ());
	break;
    case InteractionMode::SCALE:
	scaleViewport (event->pos ());
    case InteractionMode::ROTATE_LIGHT:
	rotate (event->pos (), &m_rotationMatrixLight);
	break;
    case InteractionMode::TRANSLATE_LIGHT:
	translateLight (event->pos ());
	break;

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
    glColor (Qt::black);
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

    displayOpositeFaces (G3D::Vector3::zero (), 
			 oobox[0], oobox[1], oobox[2]);
    displayOpositeFaces (G3D::Vector3::zero (),
			 oobox[1], oobox[2], oobox[0]);
    displayOpositeFaces (G3D::Vector3::zero (), 
			 oobox[2], oobox[0], oobox[1]);
    glPopAttrib ();
}


void GLWidget::displayBoundingBox () const
{
    if (m_boundingBox)
	displayBox (GetFoamAlongTime ().GetBoundingBox (), Qt::black, GL_LINE);
}

void GLWidget::displayAxes () const
{
    using G3D::Vector3;
    const G3D::AABox& aabb = GetFoamAlongTime ().GetBoundingBox ();
    Vector3 origin = aabb.low ();
    Vector3 diagonal = aabb.high () - origin;
    Vector3 first = origin + diagonal.x * Vector3::unitX ();
    Vector3 second = origin + diagonal.y * Vector3::unitY ();
    Vector3 third = origin + diagonal.z * Vector3::unitZ ();
    glBegin (GL_LINES);

    glColor (Qt::red);
    glVertex (origin);
    glVertex (first);

    glColor (Qt::green);
    glVertex (origin);
    glVertex (second);

    glColor (Qt::blue);
    glVertex (origin);
    glVertex (third);
    glEnd ();
}

void GLWidget::displayBox (const G3D::AABox& aabb, 
			   const QColor& color, GLenum polygonMode) const
{
    using G3D::Vector3;
    glPushAttrib (GL_POLYGON_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glLineWidth (1.0);
    glColor (color);
    glPolygonMode (GL_FRONT_AND_BACK, polygonMode);
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
void GLWidget::displayEdges () const
{
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);

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
    displayBoundingBox ();
}

template<typename displayEdge>
void GLWidget::displayStandaloneEdges (bool useZPos, double zPos) const
{
    const Foam::Edges& standaloneEdges = 
	GetCurrentFoam ().GetStandaloneEdges ();
    BOOST_FOREACH (boost::shared_ptr<Edge> edge, standaloneEdges)
	displayEdge (*this, DisplayElement::FOCUS, useZPos, zPos) (edge);
}

void GLWidget::displayEdgesNormal () const
{
    m_torusOriginalDomainClipped ?
	displayEdges <DisplayEdgeTorusClipped> () :
	displayEdges <DisplayEdgeWithColor<> >();
}

void GLWidget::displayEdgesTorus () const
{
    if (m_edgesTubes)
	displayEdgesTorusTubes ();
    else
	displayEdgesTorusLines ();
}

void GLWidget::displayFacesTorus () const
{
    if (m_edgesTubes)
	displayFacesTorusTubes ();
    else
	displayFacesTorusLines ();
}


void GLWidget::displayEdgesTorusTubes () const
{
    glPushAttrib (GL_POLYGON_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    EdgeSet edgeSet;
    GetCurrentFoam ().GetEdgeSet (&edgeSet);
    for_each (
	edgeSet.begin (), edgeSet.end (),
	DisplayEdgeTorus<DisplayEdgeTube, DisplayArrowTube, false>(*this));
    glPopAttrib ();

    displayOriginalDomain ();
}

void GLWidget::displayEdgesTorusLines () const
{
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);

    EdgeSet edgeSet;
    GetCurrentFoam ().GetEdgeSet (&edgeSet);
    for_each (edgeSet.begin (), edgeSet.end (),
	      DisplayEdgeTorus<DisplayEdge, DisplayArrow, false> (*this));
    glPopAttrib ();

    displayOriginalDomain ();
}


void GLWidget::displayCenterOfBodies (bool useZPos) const
{
    if ((m_viewType == ViewType::EDGES && m_edgesBodyCenter) ||
	m_viewType == ViewType::CENTER_PATHS)
    {
	double zPos = (m_viewType == ViewType::CENTER_PATHS) ? 
	    GetTimeStep () * GetTimeDisplacement () : 0;
	glPushAttrib (GL_POINT_BIT | GL_CURRENT_BIT);
	glPointSize (4.0);
	glColor (Qt::red);
	const Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
	for_each (bodies.begin (), bodies.end (), 
		  DisplayBodyCenter (*this, *m_bodySelector, useZPos, zPos));
	glPopAttrib ();
    }
}

void GLWidget::displayFacesNormal () const
{
    const Foam& foam = GetCurrentFoam ();
    const Foam::Bodies& bodies = foam.GetBodies ();
    if (foam.IsQuadratic ())
    {
	if (m_facesShowEdges)
	    displayFacesContour<DisplaySameEdges> (bodies);
	displayFacesInterior<DisplaySameEdges> (bodies);
	displayStandaloneFaces<DisplaySameEdges> ();
    }
    else
    {
	if (m_facesShowEdges)
	    displayFacesContour<DisplaySameTriangles> (bodies);
	displayFacesInterior<DisplaySameTriangles> (bodies);
	displayStandaloneFaces<DisplaySameTriangles> ();
    }

    displayStandaloneEdges< DisplayEdgeWithColor<> > ();
    displayOriginalDomain ();
    displayBoundingBox ();
}

void GLWidget::displayFacesAverage () const
{
    const FoamAlongTime& foamAlongTime = GetFoamAlongTime ();
    m_displayFaceAverage->Display (foamAlongTime.GetMin (GetFacesColor ()),
				   foamAlongTime.GetMax (GetFacesColor ()));
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
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1, 1);
    glEnable(GL_TEXTURE_1D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture (GL_TEXTURE_1D, GetColorBarTexture ());
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<DisplayFaceWithColor<displaySameEdges> > (
		  *this, *m_bodySelector, 
		  DisplayElement::TRANSPARENT_CONTEXT, m_facesColor));
    glPopAttrib ();
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

void GLWidget::displayFacesTorusTubes () const
{
    glPushAttrib (GL_POLYGON_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    FaceSet faceSet;
    GetCurrentFoam ().GetFaceSet (&faceSet);
    for_each (faceSet.begin (), faceSet.end (),
	      DisplayFace<
	      DisplayEdges<
	      DisplayEdgeTorus<DisplayEdgeTube, DisplayArrowTube, true> > > (
		  *this));
    glPopAttrib ();
    displayOriginalDomain ();
}


void GLWidget::displayFacesTorusLines () const
{
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
}

void GLWidget::displayCenterPathsWithBodies () const
{
    glLineWidth (1.0);
    displayCenterPaths ();
    if (CenterPathBodyShown ())
    {
	const Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
	double zPos = GetTimeStep () * GetTimeDisplacement ();
	for_each (bodies.begin (), bodies.end (),
		  DisplayBody<DisplayFace<
		  DisplayEdges<DisplayEdgeWithColor<
		  DisplayElement::DONT_DISPLAY_TESSELLATION> > > > (
		      *this, *m_bodySelector, 
		      DisplayElement::INVISIBLE_CONTEXT, 
		      BodyProperty::NONE, true, zPos));
	displayCenterOfBodies (true);
    }
    displayOriginalDomain ();
    displayBoundingBox ();
    displayStandaloneEdges< DisplayEdgeWithColor<> > (true, 0);
    if (GetTimeDisplacement () != 0)
    {
	
	displayStandaloneEdges< DisplayEdgeWithColor<> > (
	    true,
	    (GetFoamAlongTime ().GetTimeSteps () - 1)*GetTimeDisplacement ());	
    }
}

void GLWidget::displayCenterPaths () const
{
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT);
    glEnable(GL_TEXTURE_1D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture (GL_TEXTURE_1D, GetColorBarTexture ());
    const BodiesAlongTime::BodyMap& bats = GetBodiesAlongTime ().GetBodyMap ();
    
    for_each (bats.begin (), bats.end (),
	      DisplayCenterPath<> (
		  *this, m_centerPathColor, *m_bodySelector, 
		  GetCurrentFoam ().GetDimension () == 2 ? true : false,
		  m_timeDisplacement));
    glPopAttrib ();
}

void GLWidget::display () const
{
    if (m_srcAlphaBlend < 1)
	m_displayBlend->Display ();
    else
	DisplayViewType ();
}

void GLWidget::DisplayViewType () const
{
    (this->*(m_viewTypeDisplay[m_viewType].m_display)) ();
}

bool GLWidget::IsDisplayedBody (size_t bodyId) const
{
    return (AreAllBodiesDisplayed () || GetDisplayedBodyId () == bodyId);
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
	m_viewType != ViewType::EDGES_TORUS &&
	m_viewType != ViewType::FACES_TORUS;
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
	m_viewType != ViewType::FACES;
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
	updateGL ();
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
	updateGL ();
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
	updateGL ();
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
	updateGL ();
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
	updateGL ();
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
	updateGL ();
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



// Slots
// ======================================================================
void GLWidget::ToggledEnableLighting (bool checked)
{
    makeCurrent ();
    m_lighting = checked;
    if ((m_viewTypeDisplay[m_viewType].m_lighting) () == LIGHTING)
    {
	
	glEnable (GL_LIGHTING);
    }
    else
    {
	glDisable (GL_LIGHTING);
    }
    updateGL ();
}

void GLWidget::ToggledEnableDirectionalLight (bool checked)
{
    m_directionalLight = checked;
    updateGL ();
}

void GLWidget::ToggledShowLightPosition (bool checked)
{
    m_showLightPosition = checked;
    updateGL ();
}


void GLWidget::ToggledShowBoundingBox (bool checked)
{
    m_boundingBox = checked;
    updateGL ();
}

void GLWidget::ToggledCenterPathShowBody (bool checked)
{
    m_centerPathBodyShown = checked;
    updateGL ();
}

void GLWidget::ToggledEdgesNormal (bool checked)
{
    view (checked, ViewType::EDGES);
}

void GLWidget::ToggledEdgesTorus (bool checked)
{
    view (checked, ViewType::EDGES_TORUS);
}

void GLWidget::ToggledEdgesBodyCenter (bool checked)
{
    m_edgesBodyCenter = checked;
    updateGL ();
}

void GLWidget::ToggledFacesShowEdges (bool checked)
{
    m_facesShowEdges = checked;
    updateGL ();
}

void GLWidget::ToggledFacesNormal (bool checked)
{
    view (checked, ViewType::FACES);
}

void GLWidget::ToggledFaceEdgesTorus (bool checked)
{
    view (checked, ViewType::FACES_TORUS);
}


void GLWidget::calculateFacesAverage ()
{
    makeCurrent ();
    const FoamAlongTime& foamAlongTime = GetFoamAlongTime ();
    m_displayFaceAverage->Init (QSize (width (), height ()));
    m_displayFaceAverage->Calculate (
	GetFacesColor (), foamAlongTime.GetMin (GetFacesColor ()),
	foamAlongTime.GetMax (GetFacesColor ()));
}

void GLWidget::ToggledFacesAverage (bool checked)
{
    if (checked)
	calculateFacesAverage ();
    view (checked, ViewType::FACES_AVERAGE);
}


void GLWidget::ToggledEdgesTessellation (bool checked)
{
    m_edgesTessellation = checked;
    updateGL ();
}


void GLWidget::ToggledShowTorusOriginalDomain (bool checked)
{
    m_torusOriginalDomainDisplay = checked;
    updateGL ();
}

void GLWidget::ToggledTorusOriginalDomainClipped (bool checked)
{
    m_torusOriginalDomainClipped = checked;
    updateGL ();
}

void GLWidget::ToggledCenterPath (bool checked)
{
    view (checked, ViewType::CENTER_PATHS);
}


void GLWidget::CurrentIndexChangedInteractionMode (int index)
{
    m_interactionMode = static_cast<InteractionMode::Enum>(index);
}

void GLWidget::BodyPropertyChanged (
    boost::shared_ptr<ColorBarModel> colorBarModel,
    BodyProperty::Enum bodyProperty, ViewType::Enum viewType)
{
    RuntimeAssert (
	viewType == ViewType::FACES || 
	viewType == ViewType::CENTER_PATHS ||
	viewType == ViewType::FACES_AVERAGE,
	"Invalid view type: ", viewType);
    switch (viewType)
    {
    case ViewType::FACES:
	m_facesColor = bodyProperty;
	m_useColorMap = (m_facesColor != BodyProperty::NONE);
	break;
    case ViewType::FACES_AVERAGE:
	m_facesColor = bodyProperty;
	m_useColorMap = (m_facesColor != BodyProperty::NONE);
	calculateFacesAverage ();
	break;
    case ViewType::CENTER_PATHS:
	m_centerPathColor = bodyProperty;
	m_useColorMap = (m_centerPathColor != BodyProperty::NONE);
	break;
    default:
	RuntimeAssert (false, "Invalid value in switch: ", viewType);
    }
    if (m_useColorMap)
	ColorBarModelChanged (colorBarModel);
    else
	updateGL ();
}


void GLWidget::ColorBarModelChanged (
    boost::shared_ptr<ColorBarModel> colorBarModel)
{
    m_colorBarModel = colorBarModel;    
    const QImage image = colorBarModel->GetImage ();
    image.save ("colorbar.jpg");
    makeCurrent ();
    glTexImage1D (GL_TEXTURE_1D, 0, GL_RGBA, image.width (), 
		  0, GL_BGRA, GL_UNSIGNED_BYTE, image.scanLine (0));
    updateGL ();
}


void GLWidget::ValueChangedSliderTimeSteps (int timeStep)
{
    m_timeStep = timeStep;
    makeCurrent ();
    updateGL ();
    if (m_srcAlphaBlend < 1)
	m_displayBlend->Step ( m_timeStep != 0, timeStep);
    updateGL ();
}

void GLWidget::ValueChangedBlend (int index)
{
    QSlider* slider = static_cast<QSlider*> (sender ());
    size_t maximum = slider->maximum ();
    if (m_srcAlphaBlend == 1 && index != 0)
    {
	QSize size (width (), height ());
	m_displayBlend->Init (size);
    }
    else if (m_srcAlphaBlend < 1 && index == 0)
	m_displayBlend->Release ();
    // m_srcAlphaBlend is between 1 and 0.5
    m_srcAlphaBlend = 1 - static_cast<double>(index) / (2 * maximum);
    updateGL ();
}

void GLWidget::ValueChangedTimeDisplacement (int timeDisplacement)
{
    QSlider* slider = static_cast<QSlider*> (sender ());
    size_t maximum = slider->maximum ();
    G3D::AABox bb = GetFoamAlongTime ().GetBoundingBox ();
    m_timeDisplacement = 
	(bb.high () - bb.low ()).z * timeDisplacement / 
	GetFoamAlongTime ().GetTimeSteps () / maximum;
    updateGL ();
}

void GLWidget::ValueChangedEdgesRadius (int sliderValue)
{
    makeCurrent ();
    QSlider* slider = static_cast<QSlider*> (sender ());
    size_t maximum = slider->maximum ();
    setEdgeRadius (sliderValue, maximum);
    ToggledEnableLighting (m_lighting);
    updateGL ();
}

void GLWidget::ValueChangedAngleOfView (int angleOfView)
{
    makeCurrent ();
    m_angleOfView = angleOfView;
    calculateCameraDistance ();
    projectionTransform ();
    updateGL ();
}


void GLWidget::ShowOpenGLInfo ()
{
    ostringstream ostr;
    printOpenGLInfo (ostr);
    boost::scoped_ptr<OpenGLInfo> openGLInfo (
	new OpenGLInfo (this, ostr.str ()));
    openGLInfo->exec ();
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

double GLWidget::TexCoord (double value) const
{
    if (m_useColorMap)
	return m_colorBarModel->TexCoord (value);
    else
	return 0;
}

void GLWidget::initializeTextures ()
{
    glGenTextures (1, &m_colorBarTexture);
    glBindTexture (GL_TEXTURE_1D, m_colorBarTexture);
    
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_useColorMap = false;
}

void GLWidget::displayTextureColorMap () const
{
    glPushAttrib (GL_CURRENT_BIT | GL_VIEWPORT_BIT);
    glPushMatrix ();
    {
	// modelview
	glLoadIdentity ();
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	{
	    glLoadIdentity ();
	    glOrtho (0, width (), 0, height (), -1, 1);

	    glViewport (0, 0, width (), height ());
	    
	    glEnable(GL_TEXTURE_1D);
	    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	    glBindTexture (GL_TEXTURE_1D, m_colorBarTexture);

	    glBegin (GL_QUADS);
	    glTexCoord1f(0);glVertex2s (0, 0);
	    glTexCoord1f(1);glVertex2s (256, 0);
	    glTexCoord1f(1);glVertex2s (256, 16);
	    glTexCoord1f(0);glVertex2s (0, 16);
	    glEnd ();
	    glDisable (GL_TEXTURE_1D);
	}
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
    }
    glPopMatrix ();    
    glPopAttrib ();
}

QColor GLWidget::GetCenterPathContextColor () const
{
    QColor returnColor (Qt::black);
    returnColor.setAlphaF (GetContextAlpha ());
    return returnColor;
}

void GLWidget::SetPlayMovie (bool playMovie)
{
    m_playMovie = playMovie;
}

