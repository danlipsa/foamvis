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
#include "Debug.h"
#include "DebugStream.h"
#include "DisplayFaceStatistics.h"
#include "DisplayBodyFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "DisplayFaceFunctors.h"
#include "DisplayVertexFunctors.h"
#include "Edge.h"
#include "Face.h"
#include "Foam.h"
#include "FoamAlongTime.h"
#include "GLWidget.h"
#include "OpenGLInfo.h"
#include "OpenGLUtils.h"
#include "SelectBodiesById.h"
#include "Utils.h"
#include "Vertex.h"

// Private Functions
// ======================================================================

G3D::AABox AdjustXOverYRatio (const G3D::AABox& box, double xOverY)
{
    G3D::Vector3 center = box.center ();
    G3D::Vector3 low, high;
    if (xOverY >= 1)
    {
	double extentX = xOverY * box.extent ().y;
	low = box.low ();
	low.x = center.x - extentX / 2;
	high = box.high ();
	high.x = center.x + extentX / 2;
    }
    else
    {
	double extentY = box.extent ().x / xOverY;
	low = box.low ();
	low.y = center.y - extentY / 2;
	high = box.high ();
	high.y = center.y + extentY / 2;
    }
    G3D::AABox result = G3D::AABox (low, high);
    return result;
}

boost::shared_ptr<IdBodySelector> idBodySelectorComplement (
    const Foam& foam, const vector<size_t> bodyIds)
{
    Foam::Bodies bodies = foam.GetBodies ();
    vector<size_t> allBodyIds (bodies.size ());
    transform (bodies.begin (), bodies.end (), allBodyIds.begin (),
	       boost::bind (&Body::GetId, _1));
    boost::shared_ptr<IdBodySelector> idBodySelector =
	boost::make_shared<IdBodySelector> (allBodyIds);
    idBodySelector->SetDifference (bodyIds);
    return idBodySelector;
}

void initialize (boost::array<GLfloat, 4>& colors,
		 const boost::array<GLfloat, 4>& values)
{
    copy (values.begin (), values.end (), colors.begin ());
}

template <typename T>
void display (const char* name, const T& what)
{
    ostream_iterator<GLfloat> out (cdbg, " ");
    cdbg << name;
    copy (what.begin (), what.end (), out);
    cdbg << endl;
}


// Private Classes
// ======================================================================


// Static Fields
// ======================================================================

const size_t GLWidget::DISPLAY_ALL(numeric_limits<size_t>::max());
const size_t GLWidget::NONE(numeric_limits<size_t>::max());
// quadrics
const size_t GLWidget::QUADRIC_SLICES = 8;
const size_t GLWidget::QUADRIC_STACKS = 1;
// alpha
const double GLWidget::MIN_CONTEXT_ALPHA = 0.05;
const double GLWidget::MAX_CONTEXT_ALPHA = 0.5;

const double GLWidget::ENCLOSE_ROTATION_RATIO = 1;

// Methods
// ======================================================================

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent),
      m_viewType (ViewType::COUNT),
      m_torusOriginalDomainDisplay (false),
      m_torusOriginalDomainClipped (false),
      m_interactionMode (InteractionMode::ROTATE),
      m_statisticsType (StatisticsType::AVERAGE),
      m_foamAlongTime (0), m_timeStep (0),
      m_selectedBodyIndex (DISPLAY_ALL), m_selectedFaceIndex (DISPLAY_ALL),
      m_selectedEdgeIndex (DISPLAY_ALL),
      m_stationaryBodyId (NONE),
      m_stationaryBodyTimeStep (0),
      m_contextAlpha (MIN_CONTEXT_ALPHA),
      m_rotationModel (G3D::Matrix3::identity ()),
      m_scaleRatio (1),
      m_translation (G3D::Vector3::zero ()),
      m_lightingEnabled (false),
      m_selectedLight (LightPosition::TOP_RIGHT),
      m_lightEnabled (0),
      m_lightPositionShown (0),
      m_angleOfView (0),
      m_edgeRadiusMultiplier (0),
      m_edgesTubes (false),
      m_facesShowEdges (true),
      m_edgesBodyCenter (false),
      m_edgesTessellation (true),
      m_centerPathBodyShown (false),
      m_contextHidden (false),
      m_boundingBoxShown (false),
      m_bodiesBoundingBoxesShown (false),
      m_axesShown (false),
      m_textureColorBarShown (false),
      m_bodyProperty (BodyProperty::NONE),
      m_bodySelector (AllBodySelector::Get ()),
      m_colorBarTexture (0),
      m_timeDisplacement (0.0),
      m_playMovie (false),
      m_selectBodiesById (new SelectBodiesById (this)),
      m_contextView (false),
      m_hideContent(false),
      m_tubeCenterPathUsed (true),
      m_listCenterPaths (0),
      m_t1sShown (false)
{
    makeCurrent ();
    m_displayFaceStatistics.reset (new DisplayFaceStatistics (*this));
    initEndTranslationColor ();
    initQuadrics ();
    initViewTypeDisplay ();
    createActions ();
    setInitialLightParameters ();
    // default (0, 0, 0, 1)
    boost::array<GLfloat,4> lightAmbient = {{0, 0, 0, 1.0}};
    // default (1, 1, 1, 1)
    boost::array<GLfloat,4> lightDiffuse = {{1.0, 1.0, 1.0, 1.0}};
    // default (1, 1, 1, 1)
    boost::array<GLfloat,4> lightSpecular = {{1.0, 1.0, 1.0, 1.0}};
    for_each (m_lightAmbient.begin (), m_lightAmbient.end (),
	      boost::bind (initialize, _1, lightAmbient));
    for_each (m_lightDiffuse.begin (), m_lightDiffuse.end (),
	      boost::bind (initialize, _1, lightDiffuse));
    for_each (m_lightSpecular.begin (), m_lightSpecular.end (),
	      boost::bind (initialize, _1, lightSpecular));
}

void GLWidget::initEndTranslationColor ()
{
    const int DOMAIN_INCREMENT_COLOR[] = {255, 0, 255};
    for (size_t i = 0;
	 i < Edge::DOMAIN_INCREMENT_POSSIBILITIES *
	     Edge::DOMAIN_INCREMENT_POSSIBILITIES *
	     Edge::DOMAIN_INCREMENT_POSSIBILITIES; i++)
    {
	G3D::Vector3int16 di = Edge::IntToLocation (i);
	QColor color (
	    DOMAIN_INCREMENT_COLOR[di.x + 1],
	    DOMAIN_INCREMENT_COLOR[di.y + 1],
	    DOMAIN_INCREMENT_COLOR[di.z + 1]);
	m_endTranslationColor[di] = color;
    }
    m_endTranslationColor[G3D::Vector3int16(0,0,0)] = QColor(0,0,0);
}


void GLWidget::initQuadrics ()
{
    m_quadric = gluNewQuadric ();
    gluQuadricCallback (m_quadric, GLU_ERROR,
			reinterpret_cast<void (*)()>(&quadricErrorCallback));
    gluQuadricDrawStyle (m_quadric, GLU_FILL);
    gluQuadricNormals (m_quadric, GLU_SMOOTH);
    gluQuadricOrientation (m_quadric, GLU_OUTSIDE);
}


void GLWidget::createActions ()
{
    m_actionSelectAll = boost::make_shared<QAction> (tr("&Select all"), this);
    m_actionSelectAll->setStatusTip(tr("Select all"));
    connect(m_actionSelectAll.get (), SIGNAL(triggered()),
	    this, SLOT(SelectAll ()));

    m_actionDeselectAll = boost::make_shared<QAction> (
	tr("&Deselect all"), this);
    m_actionDeselectAll->setStatusTip(tr("Deselect all"));
    connect(m_actionDeselectAll.get (), SIGNAL(triggered()),
	    this, SLOT(DeselectAll ()));

    m_actionResetTransformation = boost::make_shared<QAction> (
	tr("&Reset transformation"), this);
    m_actionResetTransformation->setShortcut(
	QKeySequence (tr ("Ctrl+R")));
    m_actionResetTransformation->setStatusTip(tr("Reset transformation"));
    connect(m_actionResetTransformation.get (), SIGNAL(triggered()),
	    this, SLOT(ResetTransformation ()));


    m_actionResetSelectedLightPosition = boost::make_shared<QAction> (
	tr("&Reset selected light"), this);
    m_actionResetSelectedLightPosition->setShortcut(
	QKeySequence (tr ("Ctrl+L")));
    m_actionResetSelectedLightPosition->setStatusTip(
	tr("Reset selected light"));
    connect(m_actionResetSelectedLightPosition.get (), SIGNAL(triggered()),
	    this, SLOT(ResetSelectedLightPosition ()));


    m_actionSelectBodiesById = boost::make_shared<QAction> (
	tr("&Select bodies by id"), this);
    m_actionSelectBodiesById->setStatusTip(tr("Select bodies by id"));
    connect(m_actionSelectBodiesById.get (), SIGNAL(triggered()),
	    this, SLOT(SelectBodiesByIdList ()));

    m_actionStationarySet = boost::make_shared<QAction> (
	tr("&Set stationary"), this);
    m_actionStationarySet->setStatusTip(tr("Set stationary"));
    connect(m_actionStationarySet.get (), SIGNAL(triggered()),
	    this, SLOT(StationarySet ()));

    m_actionStationaryReset = boost::make_shared<QAction> (
	tr("&Reset stationary"), this);
    m_actionStationaryReset->setStatusTip(tr("Reset stationary"));
    connect(m_actionStationaryReset.get (), SIGNAL(triggered()),
	    this, SLOT(StationaryReset ()));

    m_actionStationaryContextAdd = boost::make_shared<QAction> (
	tr("&Add context"), this);
    m_actionStationaryContextAdd->setStatusTip(tr("Add context"));
    connect(m_actionStationaryContextAdd.get (), SIGNAL(triggered()),
	    this, SLOT(StationaryContextAdd ()));

    m_actionInfoFocus = boost::make_shared<QAction> (tr("&Focus"), this);
    m_actionInfoFocus->setStatusTip(tr("Info focus"));
    connect(m_actionInfoFocus.get (), SIGNAL(triggered()), this, 
	    SLOT(InfoFocus ()));

    m_actionInfoFoam = boost::make_shared<QAction> (tr("&Foam"), this);
    m_actionInfoFoam->setStatusTip(tr("Info foam"));
    connect(m_actionInfoFoam.get (), SIGNAL(triggered()), this, 
	    SLOT(InfoFoam ()));

    m_actionInfoOpenGL = boost::make_shared<QAction> (
	tr("&OpenGL"), this);
    m_actionInfoOpenGL->setStatusTip(tr("Info OpenGL"));
    connect(m_actionInfoOpenGL.get (), SIGNAL(triggered()),
	    this, SLOT(InfoOpenGL ()));
}

void GLWidget::initViewTypeDisplay ()
{
    // WARNING: This has to be in the same order as ViewType::Enum
    boost::array<ViewTypeDisplay, ViewType::COUNT> vtd =
	{{&GLWidget::displayEdgesNormal,
	  &GLWidget::displayEdgesTorus,
	  &GLWidget::displayFacesTorus,
	  &GLWidget::displayFacesNormal,
	  &GLWidget::displayFacesStatistics,
	  &GLWidget::displayCenterPathsWithBodies,
	 }};
    copy (vtd.begin (), vtd.end (), m_viewTypeDisplay.begin ());
}

void GLWidget::SetFoamAlongTime (FoamAlongTime* foamAlongTime)
{
    m_foamAlongTime = foamAlongTime;
    calculateCameraDistance ();
    m_axesOrder =
	(foamAlongTime->GetDimension () == 2) ?
	AxesOrder::TWO_D :
	AxesOrder::THREE_D;
    Foam::Bodies bodies = foamAlongTime->GetFoam (0)->GetBodies ();
    if (bodies.size () != 0)
    {
	size_t maxIndex = bodies.size () - 1;
	m_selectBodiesById->SetMinBodyId (bodies[0]->GetId ());
	m_selectBodiesById->SetMaxBodyId (bodies[maxIndex]->GetId ());
	m_selectBodiesById->UpdateLabelMinMax ();
    }
    m_displayFaceStatistics->SetHistoryCount (foamAlongTime->GetTimeSteps ());
}


double GLWidget::getMinimumEdgeRadius () const
{
    G3D::Vector3 objectOrigin = gluUnProject (G3D::Vector2::zero (),
					      GluUnProjectZOperation::SET0);
    G3D::Vector3 objectOne = gluUnProject (G3D::Vector2::unitX (),
					   GluUnProjectZOperation::SET0);
    return (objectOne - objectOrigin).length () * m_scaleRatio;
}

void GLWidget::setEdgeRadius ()
{
    calculateEdgeRadius (m_edgeRadiusMultiplier,
	&m_edgeRadius, &m_arrowBaseRadius,
			 &m_arrowHeight, &m_edgesTubes);
}

void GLWidget::calculateEdgeRadius (
    double edgeRadiusMultiplier,
    double* edgeRadius, double* arrowBaseRadius, 
    double* arrowHeight, bool* edgesTubes) const
{
    double r = getMinimumEdgeRadius ();
    double R = 3 * r;

    if (edgesTubes != 0)
	*edgesTubes = (edgeRadiusMultiplier != 0.0);
    *edgeRadius = (R - r) * edgeRadiusMultiplier + r;
    *arrowBaseRadius = 4 * (*edgeRadius);
    *arrowHeight = 11 * (*edgeRadius);
}

GLWidget::~GLWidget()
{
    makeCurrent();
    gluDeleteQuadric (m_quadric);
    m_quadric = 0;
}

void GLWidget::changeView (bool checked, ViewType::Enum view)
{
    if (checked)
    {
        m_viewType = view;
	compile ();
	update ();
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

void GLWidget::setInitialLightParameters ()
{
    for (size_t i = 0; i < LightPosition::COUNT; ++i)
    {
	setInitialLightPosition (LightPosition::Enum(i));
	m_directionalLightEnabled[i] = true;
    }
}

void GLWidget::setInitialLightPosition (LightPosition::Enum i)
{
    m_lightPositionRatio[i] = 1;
    m_rotationLight[i] = G3D::Matrix3::identity ();
}

G3D::Vector3 GLWidget::getInitialLightPosition (
    LightPosition::Enum lightPosition) const
{
    G3D::AABox bb = calculateCenteredViewingVolume (
	static_cast<double> (width ()) / height ());
    G3D::Vector3 high = bb.high (), low = bb.low ();
    G3D::Vector3 nearRectangle[] = {
	G3D::Vector3 (high.x, high.y, high.z),
	G3D::Vector3 (low.x, high.y, high.z),
	G3D::Vector3 (low.x, low.y, high.z),
	G3D::Vector3 (high.x, low.y, high.z),
    };
    return nearRectangle[lightPosition];
}

void GLWidget::positionLights ()
{
    for (size_t i = 0; i < LightPosition::COUNT; ++i)
	positionLight (LightPosition::Enum (i));
}

void GLWidget::positionLight (LightPosition::Enum i)
{
    if (m_lightEnabled[i])
    {
	makeCurrent ();
	G3D::Vector3 lp = getInitialLightPosition (
	    LightPosition::Enum (i)) * m_lightPositionRatio[i];
	glPushMatrix ();
	glLoadMatrix (G3D::CoordinateFrame (m_rotationLight[i]));
	if (m_directionalLightEnabled[i])
	{
	    glLightf(GL_LIGHT0 + i, GL_SPOT_CUTOFF, 180);
	    boost::array<GLfloat, 4> lightDirection = {{lp.x, lp.y, lp.z, 0}};
	    glLightfv(GL_LIGHT0 + i, GL_POSITION, &lightDirection[0]);
	}
	else
	{
	    glLightf(GL_LIGHT0 + i, GL_SPOT_CUTOFF, 15);
	    boost::array<GLfloat, 3> lightDirection = {{-lp.x, -lp.y, -lp.z}};
	    glLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, &lightDirection[0]);
	    glPushMatrix ();
	    glLoadIdentity ();
	    glTranslatef (0, 0, - m_cameraDistance);
	    glMultMatrix (m_rotationLight[i]);
	    GLfloat lightPosition[] = {lp.x, lp.y, lp.z, 1};
	    glLightfv(GL_LIGHT0 + i, GL_POSITION, lightPosition);
	    glPopMatrix ();
	}
	glPopMatrix ();
    }
}

void GLWidget::showLightPositions () const
{
    for (size_t i = 0; i < LightPosition::COUNT; ++i)
	showLightPosition (LightPosition::Enum (i));
}

void GLWidget::showLightPosition (LightPosition::Enum i) const
{
    if (m_lightPositionShown[i])
    {
	const double sqrt3 = 1.7321;
	glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT);    
	glPushMatrix ();
	glLoadIdentity ();
	glTranslatef (0, 0, - m_cameraDistance);
	glMultMatrix (m_rotationLight[i]);
	G3D::Vector3 lp = getInitialLightPosition (
	    LightPosition::Enum (i)) / sqrt3;
	glColor (m_lightEnabled[i] ? Qt::red : Qt::gray);
	if (isLightingEnabled ())
	    glDisable (GL_LIGHTING);
	DisplayOrientedEdge () (lp, G3D::Vector3::zero ());
	glPopMatrix ();
	glPopAttrib ();
    }
}

void GLWidget::translateLight (const QPoint& position)
{
    G3D::AABox vv = calculateCenteredViewingVolume (
	static_cast<double> (width ()) / height ());
    G3D::Vector2 oldPosition = G3D::Vector2 (m_lastPos.x (), m_lastPos.y ());
    G3D::Vector2 newPosition = G3D::Vector2 (position.x (), position.y ());
    G3D::Vector2 viewportCenter =
	(m_viewport.x1y1 () + m_viewport.x0y0 ()) / 2;
    float screenChange =
	((newPosition - viewportCenter).length () -
	 (oldPosition - viewportCenter).length ());
    float ratio = screenChange /
	(m_viewport.x1y1 () - m_viewport.x0y0 ()).length ();

    m_lightPositionRatio[m_selectedLight] = 
	(1 + ratio) * m_lightPositionRatio[m_selectedLight];
}


void GLWidget::initializeLighting ()
{
    // material colors: ambient and diffuse colors are set using glColor
    GLfloat materialSpecular[] = {1.0, 1.0, 1.0, 1.0}; //(0, 0, 0, 1)
    GLfloat materialShininess[] = {50.0};              // 0
    GLfloat materialEmission[] = {0.0, 0.0, 0.0, 1.0}; //(0, 0, 0, 1)
    glColorMaterial (GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    //GLfloat materialAmbient[] = {.2, .2, .2, 1.0};     //(.2, .2, .2, 1.0)
    //GLfloat materialDiffuse[] = {.8, .8, .8, 1.0};     //(.8, .8, .8, 1.0)
    //glMaterialfv (GL_FRONT, GL_AMBIENT, materialAmbient);
    //glMaterialfv (GL_FRONT, GL_DIFFUSE, materialDiffuse);

    glMaterialfv (GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialfv (GL_FRONT, GL_SHININESS, materialShininess);
    glMaterialfv (GL_FRONT, GL_EMISSION, materialEmission);
    // See OpenGL FAQ 21.040 
    //Lighting and texture mapping work pretty well, but why don't I see 
    // specular highlighting?
    glLightModeli (GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
    // See OpenGL FAQ 18.090 
    // Why is the lighting incorrect after I scale my scene to change its size?
    glEnable(GL_RESCALE_NORMAL);
    glShadeModel (GL_SMOOTH);
    glEnable (GL_COLOR_MATERIAL);
}

G3D::AABox GLWidget::calculateCenteredViewingVolume (
    double xOverY) const
{
    G3D::AABox boundingBox = AdjustXOverYRatio (
	EncloseRotation (GetFoamAlongTime ().GetBoundingBox (), 
			 ENCLOSE_ROTATION_RATIO), xOverY);
    G3D::Vector3 center = boundingBox.center ();
    return boundingBox - center;
}

/**
 * @todo: make sure context view works for 3D
 */
void GLWidget::translateAndScale (
    double scaleRatio,
    const G3D::Vector3& translation, bool contextView) const
{
    glScale (scaleRatio);
    // if 2D, the back plane stays in the same place
    if (GetFoamAlongTime ().GetDimension () == 2 && 
	! IsTimeDisplacementUsed ())
    {
	G3D::AABox boundingBox = GetFoamAlongTime ().GetBoundingBox ();
	float zTranslation = boundingBox.center ().z - boundingBox.low ().z;
	zTranslation = zTranslation - zTranslation / scaleRatio;
	glTranslatef (0, 0, zTranslation);
    }

    glTranslate (contextView ? (translation / scaleRatio) : translation);
}

void GLWidget::ModelViewTransform (size_t timeStep) const
{
    glLoadIdentity ();
    // viewing transform
    glTranslatef (0, 0, - m_cameraDistance);
    
    // modeling transform
    if (! m_contextView)
	translateAndScale (m_scaleRatio, m_translation, m_contextView);
    glMultMatrix (m_rotationModel);
    switch (m_axesOrder)
    {
    case AxesOrder::TWO_D_TIME_DISPLACEMENT:
	rotate2DTimeDisplacement ();
	break;
    case AxesOrder::TWO_D_ROTATE_RIGHT90:
	rotate2DRight90 ();
	break;
    case AxesOrder::THREE_D:
	rotate3D ();
	break;
    default:
	break;
    }
    translateFoamStationaryBody (timeStep);
    glTranslate (- GetFoamAlongTime ().GetBoundingBox ().center ());
}

void GLWidget::translateFoamStationaryBody (size_t timeStep) const
{
    if (m_stationaryBodyId != NONE)
    {
	G3D::Vector3 translation = 
	    GetFoamAlongTime ().GetFoam (m_stationaryBodyTimeStep)->
	    GetBody (m_stationaryBodyId)->GetCenter () -
	    
	    GetFoamAlongTime ().GetFoam (timeStep)->
	    GetBody (m_stationaryBodyId)->GetCenter ();
	glTranslate (translation);
    }
}

G3D::AABox GLWidget::calculateViewingVolume (double xOverY) const
{
    G3D::AABox centeredViewingVolume = calculateCenteredViewingVolume (xOverY);
    G3D::Vector3 translation (m_cameraDistance * G3D::Vector3::unitZ ());
    G3D::AABox result = centeredViewingVolume - translation;
    return result;
}


void GLWidget::projectionTransform ()
{
    makeCurrent ();
    double xOverY = double (width ()) / height ();
    G3D::AABox viewingVolume = calculateViewingVolume (xOverY);
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
    glLoadIdentity ();
    
}

void GLWidget::ViewportTransform ()
{
    G3D::Rect2D screenWorld;
    viewingVolumeCalculations (width (), height (), &m_viewport, &screenWorld);
    glViewport (m_viewport);
}


void GLWidget::viewingVolumeCalculations (
    int width, int height,
    G3D::Rect2D* vv2dScreen, G3D::Rect2D* screenWorld) const
{
    G3D::AABox vv = calculateCenteredViewingVolume (
	static_cast<double> (width) / height);
    G3D::Rect2D vv2d = G3D::Rect2D::xyxy (vv.low ().xy (), vv.high ().xy ());
    double windowRatio = static_cast<double>(width) / height;
    double vvratio = vv2d.width () / vv2d.height ();
    if (windowRatio > vvratio)
    {
	double newWidth = vvratio * height;
	*vv2dScreen = G3D::Rect2D::xywh ((width - newWidth) / 2, 0,
					     newWidth, height);
	*screenWorld = G3D::Rect2D::xywh (0, 0,
	    vv2d.height () * windowRatio, vv2d.height ());
    }
    else
    {
	double newHeight = width / vvratio;
	*vv2dScreen = G3D::Rect2D::xywh (0, (height - newHeight) / 2,
				   width, newHeight);
	*screenWorld = G3D::Rect2D::xywh (0, 0,
	    vv2d.width (), vv2d.width () / windowRatio);
    }
    //cdbg << "vv2d=" << vv2d << "screenWorld=" << *screenWorld << endl;
}

void GLWidget::boundingBoxCalculations (
    int width, int height,
    const G3D::Rect2D& screenWorld, G3D::Rect2D* bb2dScreen,
    double* change) const
{
    G3D::AABox bb = GetFoamAlongTime ().GetBoundingBox ();
    //cdbg << "bb=" << bb << endl;
    G3D::Rect2D bb2d = G3D::Rect2D::xyxy (bb.low ().xy (), bb.high ().xy ());
    double bbratio = bb2d.width () / bb2d.height ();
    double windowRatio = static_cast<double>(width) / height;
    if (windowRatio > bbratio)
    {
	*change = screenWorld.height () / bb2d.height ();
	double newWidth = bbratio * height;
	*bb2dScreen = G3D::Rect2D::xywh ((width - newWidth) / 2, 0,
					newWidth, height);
    }
    else
    {
	*change = screenWorld.width () / bb2d.width ();
	double newHeight = width / bbratio;
	*bb2dScreen = G3D::Rect2D::xywh (
	    0, (height - newHeight) / 2, width, newHeight);
    }
}

void GLWidget::rotate2DTimeDisplacement () const
{
    /**
     *  y        z
     *    x ->     x
     * z        -y
     */
    const static G3D::Matrix3 axes (1, 0, 0,  0, 0, 1,  0, -1, 0);
    glMultMatrix (axes);
}

void GLWidget::rotate2DRight90 () const
{
    /**
     *  y       -x
     *    x ->     y
     * z        z
     */
    const static G3D::Matrix3 axes (0, 1, 0,  -1, 0, 0,  0, 0, 1);
    glMultMatrix (axes);
}



void GLWidget::rotate3D () const
{
    /**
     *  y        z
     *    x ->     y
     * z        x
     */
    const static G3D::Matrix3 evolverAxes (0, 1, 0,  0, 0, 1,  1, 0, 0);
    glMultMatrix (evolverAxes);
    const Foam& foam = *GetFoamAlongTime ().GetFoam (0);
    glMultMatrix (foam.GetViewMatrix ().approxCoordinateFrame ().rotation);
}

void GLWidget::calculateCameraDistance ()
{
    G3D::AABox centeredViewingVolume = 
	calculateCenteredViewingVolume (
	    static_cast<double> (width ()) / height ());
    G3D::Vector3 diagonal =
	centeredViewingVolume.high () - centeredViewingVolume.low ();
    if (m_angleOfView == 0)
	m_cameraDistance = diagonal.z;
    else
    {
	// distance from the camera to the middle of the bounding box
	m_cameraDistance = diagonal.y / 2 /
	    tan (m_angleOfView * M_PI / 360) + diagonal.z / 2;
    }
}


void GLWidget::ResetTransformation ()
{
    m_rotationModel = G3D::Matrix3::identity ();
    m_scaleRatio = 1;
    m_translation = G3D::Vector3::zero ();
    projectionTransform ();
    update ();
}

void GLWidget::ResetSelectedLightPosition ()
{
    setInitialLightPosition (m_selectedLight);
    positionLight (m_selectedLight);
    update ();
}


void GLWidget::SelectBodiesByIdList ()
{
    if (m_selectBodiesById->exec () == QDialog::Accepted)
    {
	SetBodySelector (
	    boost::shared_ptr<IdBodySelector> (
		new IdBodySelector (m_selectBodiesById->GetIds ())));
    }
}

void GLWidget::SelectAll ()
{
    SetBodySelector (AllBodySelector::Get (), BodySelectorType::ID);
    m_selectBodiesById->ClearEditIds ();
    m_selectedBodyIndex = DISPLAY_ALL;
    m_selectedFaceIndex = DISPLAY_ALL;
    m_selectedEdgeIndex = DISPLAY_ALL;
    update ();
}

void GLWidget::DeselectAll ()
{
    SetBodySelector (
	boost::shared_ptr<IdBodySelector> (new IdBodySelector ()));
}

void GLWidget::InfoFoam ()
{
    string message = (AllBodiesSelected ()) ?
	GetFoamAlongTime ().ToHtml () : GetSelectedBody ()->ToString ();
    QMessageBox msgBox (this);
    msgBox.setText(message.c_str ());
    msgBox.exec();
}

void GLWidget::InfoFocus ()
{
    QMessageBox msgBox (this);
    ostringstream ostr;
    switch (m_bodySelector->GetType ())
    {
    case BodySelectorType::ALL:
    {
	vector<size_t> bodies;
	brushedBodies (m_contextMenuPos, &bodies);
	Foam::Bodies::const_iterator it = GetCurrentFoam ().FindBody (bodies[0]);
	ostr << *it;
	break;
    }

    case BodySelectorType::ID:
    {
	break;
    }

    case BodySelectorType::PROPERTY_VALUE:
    {
	break;
    }

    case BodySelectorType::COMPOSITE:
	break;
    }

    msgBox.setText(ostr.str ().c_str ());
    msgBox.exec();
}


// Uses antialiased points and lines
// See OpenGL Programming Guide, 7th edition, Chapter 6: Blending,
// Antialiasing, Fog and Polygon Offset page 293
void GLWidget::initializeGL()
{
    initializeGLFunctions ();
    glClearColor (Qt::white);
    glEnable(GL_DEPTH_TEST);
    initializeTextures ();
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_displayFaceStatistics->InitShaders ();
    initializeLighting ();
    setEdgeRadius ();
    m_listCenterPaths = glGenLists (1);
    detectOpenGLError ("initializeGl");
}

void GLWidget::paintGL ()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ModelViewTransform (GetTimeStep ());
    if (! m_hideContent)
    {
	DisplayViewType ();
	displayTextureColorBar ();
    }
    displayAxes ();
    displayBoundingBox ();
    displayOriginalDomain ();
    displayFocusBox ();
    showLightPositions ();
    displayT1s ();
    detectOpenGLError ("paintGl");
    Q_EMIT PaintedGL ();
}

void GLWidget::resizeGL(int w, int h)
{
    (void)w;(void)h;
    projectionTransform ();
    ViewportTransform ();
    if (m_viewType == ViewType::FACES_STATISTICS)
    {
	pair<double, double> minMax = getStatisticsMinMax ();
	m_displayFaceStatistics->InitStep (minMax.first, minMax.second);
    }
    detectOpenGLError ("resizeGl");
}

void GLWidget::RenderFromFbo (QGLFramebufferObject& fbo) const
{
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, fbo.texture ());
    glPushAttrib (GL_VIEWPORT_BIT);
    glViewport (0, 0, width (), height ());

    //glMatrixMode (GL_MODELVIEW);
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
    Vector2 center (width () / 2, height () / 2);
    Vector2 lastPos (m_lastPos.x (), m_lastPos.y());
    Vector2 currentPos (p.x (), p.y ());
    double ratio =
	(currentPos - center).length () / (lastPos - center).length ();
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

void GLWidget::translate (const QPoint& position,
			  G3D::Vector3::Axis screenXTranslation,
			  G3D::Vector3::Axis screenYTranslation)
{
    G3D::Vector3 translationRatio;
    translationRatio[screenXTranslation] =
	static_cast<double>(position.x() - m_lastPos.x()) /
	m_viewport.width ();
    translationRatio[screenYTranslation] =
	- static_cast<double> (position.y() - m_lastPos.y()) / 
	m_viewport.height ();


    G3D::AABox vv = calculateCenteredViewingVolume (
	static_cast<double> (width ()) / height ());
    G3D::Vector3 focusBoxExtent = vv.extent () / m_scaleRatio;
    if (m_contextView)
	m_translation -= (translationRatio * focusBoxExtent);
    else
	m_translation += (translationRatio * focusBoxExtent);
}


void GLWidget::scale (const QPoint& position)
{
    double ratio = ratioFromCenter (position);
    if (m_contextView)
	m_scaleRatio = m_scaleRatio / ratio;
    else
	m_scaleRatio = m_scaleRatio * ratio;
}

/** 
 * @todo Use body indexes instead of body IDs as the IDs might not be unique
 */
void GLWidget::brushedBodies (
    const QPoint& position, vector<size_t>* bodies) const
{
    G3D::Vector3 end = gluUnProject (MapToOpenGl (position, height ()));
    if (GetFoamAlongTime ().GetDimension () == 2)
	end.z = 0;
    const Foam& foam = GetCurrentFoam ();
    BOOST_FOREACH (boost::shared_ptr<Body> body, foam.GetBodies ())
    {
	G3D::AABox box = body->GetBoundingBox ();
	if (box.contains (end))
	    bodies->push_back (body->GetId ());
    }
}

void GLWidget::displayStationaryBodyAndContext () const
{
    if (GetStationaryBodyId () != NONE)
    {
	Foam::Bodies focusBody (1);
	focusBody[0] = *GetCurrentFoam ().FindBody (GetStationaryBodyId ());
	displayFacesContour<0> (focusBody);
    }
    if (m_stationaryBodyContext.size () > 0)
    {
	const Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
	Foam::Bodies contextBodies (bodies.size ());
	Foam::Bodies::const_iterator end = remove_copy_if (
	    bodies.begin (), bodies.end (), contextBodies.begin (),
	    ! boost::bind (&GLWidget::IsStationaryBodyContext, this, 
			   boost::bind (&Body::GetId, _1)));
	contextBodies.resize (end - contextBodies.begin ());
	displayFacesContour<1> (contextBodies);
    }
}


void GLWidget::setStationaryBodyLabel ()
{
    bitset<2> stationaryParameters;
    const char* message[] = 
    {
	"",
	"Stationary body",
	"Context for stationary body",
	"Stationary body + context"
    };
    stationaryParameters.set (0, m_stationaryBodyId != NONE);
    stationaryParameters.set (1, m_stationaryBodyContext.size () != 0);
    m_labelStatusBar->setText (message[stationaryParameters.to_ulong ()]);
}


void GLWidget::StationarySet ()
{
    vector<size_t> bodies;
    brushedBodies (m_contextMenuPos, &bodies);
    m_stationaryBodyId = bodies[0];
    m_stationaryBodyTimeStep = m_timeStep;
    setStationaryBodyLabel ();
    update ();
}

void GLWidget::StationaryReset ()
{
    m_stationaryBodyId = NONE;
    m_stationaryBodyTimeStep = 0;
    m_stationaryBodyContext.clear ();
    setStationaryBodyLabel ();
    update ();
}

void GLWidget::StationaryContextAdd ()
{
    vector<size_t> bodies;
    brushedBodies (m_contextMenuPos, &bodies);
    m_stationaryBodyContext.insert (bodies[0]);
    setStationaryBodyLabel ();
    update ();
}


void GLWidget::select (const QPoint& position)
{
    vector<size_t> bodyIds;
    brushedBodies (position, &bodyIds);
    UnionBodySelector (bodyIds);
}

void GLWidget::deselect (const QPoint& position)
{
    vector<size_t> bodyIds;
    brushedBodies (position, &bodyIds);
    DifferenceBodySelector (bodyIds);
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    switch (m_interactionMode)
    {
    case InteractionMode::ROTATE:
	rotate (event->pos (), &m_rotationModel);
	break;

    case InteractionMode::TRANSLATE:
	if (event->modifiers () & Qt::ControlModifier)
	{
	    QPoint point (m_lastPos.x (), event->pos ().y ());
	    translate (point, G3D::Vector3::X_AXIS, G3D::Vector3::Z_AXIS);
	}
	else
	    translate (event->pos (), G3D::Vector3::X_AXIS,
		       G3D::Vector3::Y_AXIS);
	projectionTransform ();
	break;
    case InteractionMode::SCALE:
	scale (event->pos ());
	projectionTransform ();
	break;

    case InteractionMode::ROTATE_LIGHT:
	rotate (event->pos (), &m_rotationLight[m_selectedLight]);
	positionLight (m_selectedLight);
	break;
    case InteractionMode::TRANSLATE_LIGHT:
	positionLight (m_selectedLight);
	translateLight (event->pos ());
	break;

    case InteractionMode::SELECT:
	select (event->pos ());
	break;
    case InteractionMode::DESELECT:
	deselect (event->pos ());
	break;

    default:
	break;
    }
    m_lastPos = event->pos();
    update ();
}


void GLWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button () != Qt::LeftButton)
	return;
    switch (m_interactionMode)
    {
    case InteractionMode::SELECT:
	select (event->pos ());
	break;
    case InteractionMode::DESELECT:
	deselect (event->pos ());
	break;
    default:
	break;
    }
    update ();
    m_lastPos = event->pos();
}


void GLWidget::displayOriginalDomain () const
{
    if (m_torusOriginalDomainDisplay)
	DisplayBox (GetCurrentFoam().GetOriginalDomain ());
}

/**
 * @todo display a pyramid frustum for angle of view > 0.
 */
void GLWidget::displayFocusBox () const
{
    if (m_contextView)
    {
	glPushMatrix ();
	glLoadIdentity ();
	glTranslatef (0, 0, - m_cameraDistance);

	G3D::AABox focusBox = calculateCenteredViewingVolume (
	    static_cast<double> (width ()) / height ());
	translateAndScale (1 / m_scaleRatio, - m_translation, m_contextView);
	DisplayBox (focusBox, Qt::black, GL_LINE);
	glPopMatrix ();
    }
}

void GLWidget::displayBoundingBox () const
{
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT);
    if (isLightingEnabled ())
	glDisable (GL_LIGHTING);
    if (m_boundingBoxShown)
	DisplayBox (GetFoamAlongTime ().GetBoundingBox (), Qt::black, GL_LINE);
    if (m_bodiesBoundingBoxesShown)
    {
	const Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
	for_each (
	    bodies.begin (), bodies.end (),
	    boost::bind (
		DisplayBox< boost::shared_ptr<Body> >,
		_1, Qt::black, GL_LINE));
    }
    glPopAttrib ();
}

void GLWidget::displayAxes () const
{
    if (m_axesShown)
    {
	glPushAttrib (GL_CURRENT_BIT);
	using G3D::Vector3;
	const G3D::AABox& aabb = GetFoamAlongTime ().GetBoundingBox ();
	Vector3 origin = aabb.low ();
	Vector3 diagonal = aabb.high () - origin;
	Vector3 first = origin + diagonal.x * Vector3::unitX ();
	Vector3 second = origin + diagonal.y * Vector3::unitY ();
	Vector3 third = origin + diagonal.z * Vector3::unitZ ();
	double edgeRadius, arrowBaseRadius, arrowHeight;

	calculateEdgeRadius (0, &edgeRadius,
			     &arrowBaseRadius, &arrowHeight);
	DisplayOrientedEdgeQuadric displayOrientedEdge (
	    GetQuadricObject (), arrowBaseRadius, edgeRadius, arrowHeight,
	    DisplayArrow::TOP_END);

	glColor (Qt::red);
	displayOrientedEdge (origin, first);

	glColor (Qt::green);
	displayOrientedEdge (origin, second);

	glColor (Qt::blue);
	displayOrientedEdge (origin, third);
	glPopAttrib ();
    }
}


template<typename displayEdge>
void GLWidget::displayEdges () const
{
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);

    const Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<
	      DisplayFaceWithHighlightColor<0,
	      DisplayEdges<displayEdge> > > (*this, *m_bodySelector));
    displayStandaloneEdges<displayEdge> ();

    glPopAttrib ();
    displayCenterOfBodies ();
}

template<typename displayEdge>
void GLWidget::displayStandaloneEdges (bool useZPos, double zPos) const
{
    glPushAttrib (GL_ENABLE_BIT);    
    glDisable (GL_DEPTH_TEST);
    const Foam::Edges& standaloneEdges =
	GetCurrentFoam ().GetStandaloneEdges ();
    BOOST_FOREACH (boost::shared_ptr<Edge> edge, standaloneEdges)
	displayEdge (*this, DisplayElement::FOCUS, useZPos, zPos) (edge);
    glPopAttrib ();
}

void GLWidget::displayEdgesNormal () const
{
    glPushAttrib (GL_ENABLE_BIT);
    if (isLightingEnabled ())
	glDisable (GL_LIGHTING);
    m_torusOriginalDomainClipped ?
	displayEdges <DisplayEdgeTorusClipped> () :
	displayEdges <DisplayEdgeWithColor<> >();
    glPopAttrib ();
}

void GLWidget::displayT1s () const
{
    if (m_t1sShown)
    {
	if (ViewType::IsGlobal (GetViewType ()))
	    displayT1sGlobal ();
	else
	    displayT1s (GetTimeStep ());
    }
}


void GLWidget::displayT1sGlobal () const
{
    for (size_t i = 0; i < GetFoamAlongTime ().GetTimeSteps (); ++i)
	displayT1s (i);
}

void GLWidget::displayT1s (size_t timeStep) const
{
    glPushAttrib (GL_ENABLE_BIT | GL_POINT_BIT | GL_CURRENT_BIT);
    glDisable (GL_DEPTH_TEST);
    glPointSize (4.0);
    glColor (GetHighlightColor (0));
    glBegin (GL_POINTS);
    BOOST_FOREACH (const G3D::Vector3 v, 
		   GetFoamAlongTime ().GetT1s (timeStep))
	::glVertex (v);
    glEnd ();
    glPopAttrib ();
}

QColor GLWidget::GetHighlightColor (size_t i) const
{
    if (m_colorBarModel.get () == 0)
    {
	if (i == 0)
	    return Qt::black;
	else
	    return Qt::red;
    }
    else
	return m_colorBarModel->GetHighlightColor (i);
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
	DisplayEdgeTorus<DisplayEdgeQuadric, DisplayArrowQuadric, false>(*this));
    glPopAttrib ();
}

void GLWidget::displayEdgesTorusLines () const
{
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);

    EdgeSet edgeSet;
    GetCurrentFoam ().GetEdgeSet (&edgeSet);
    for_each (edgeSet.begin (), edgeSet.end (),
	      DisplayEdgeTorus<DisplayEdge, DisplayArrow, false> (*this));
    glPopAttrib ();
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
    if (m_facesShowEdges)
	displayFacesContour<0> (bodies);
    displayFacesInterior (bodies);
    displayStandaloneFaces ();
    displayStandaloneEdges< DisplayEdgeWithColor<> > ();
}

pair<double, double> GLWidget::getStatisticsMinMax () const
{
    double minValue, maxValue;
    if (GetStatisticsType () == StatisticsType::COUNT)
    {
	minValue = 0;
	maxValue = GetFoamAlongTime ().GetTimeSteps ();
    }
    else
    {
	minValue = GetFoamAlongTime ().GetMin (GetBodyProperty ());
	maxValue = GetFoamAlongTime ().GetMax (GetBodyProperty ());
    }
    return pair<double, double> (minValue, maxValue);
}


void GLWidget::displayFacesStatistics () const
{
    glPushAttrib (GL_ENABLE_BIT);    
    glDisable (GL_DEPTH_TEST);
    pair<double, double> minMax = getStatisticsMinMax ();
    m_displayFaceStatistics->Display (
	minMax.first, minMax.second, GetStatisticsType ());
    displayStandaloneEdges< DisplayEdgeWithColor<> > ();
    displayStationaryBodyAndContext ();
    glPopAttrib ();
}

void GLWidget::displayStandaloneFaces () const
{
    const Foam::Faces& faces = GetCurrentFoam ().GetStandaloneFaces ();
    displayFacesContour<0> (faces);
    displayFacesInterior (faces);
}

template<size_t highlightColorIndex>
void GLWidget::displayFacesContour (const Foam::Faces& faces) const
{
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    for_each (faces.begin (), faces.end (),
	      DisplayFaceWithHighlightColor<highlightColorIndex, 
	      DisplayFaceLineStrip> (*this));
    glPopAttrib ();
}

template<size_t highlightColorIndex>
void GLWidget::displayFacesContour (
    const Foam::Bodies& bodies) const
{
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody< DisplayFaceWithHighlightColor<highlightColorIndex, 
	      DisplayFaceLineStrip> > (
		  *this, *m_bodySelector));
    glPopAttrib ();
}

// See OpenGL Programming Guide, 7th edition, Chapter 6: Blending,
// Antialiasing, Fog and Polygon Offset page 293
void GLWidget::displayFacesInterior (const Foam::Bodies& bodies) const
{
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | 
		  GL_TEXTURE_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1, 1);
    glEnable(GL_TEXTURE_1D);
    //See OpenGL FAQ 21.030 Why doesn't lighting work when I turn on 
    //texture mapping?
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture (GL_TEXTURE_1D, GetColorBarTexture ());
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<DisplayFaceWithBodyPropertyColor<
	      DisplayFaceTriangleFan> > (
		  *this, *m_bodySelector,
		  DisplayElement::TRANSPARENT_CONTEXT, m_bodyProperty));
    glPopAttrib ();
}

void GLWidget::displayFacesInterior (const Foam::Faces& faces) const
{
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1, 1);
    for_each (faces.begin (), faces.end (),
	      DisplayFaceWithBodyPropertyColor<DisplayFaceTriangleFan> (*this));
    glPopAttrib ();
}

void GLWidget::displayFacesTorusTubes () const
{
    glPushAttrib (GL_POLYGON_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    FaceSet faceSet;
    GetCurrentFoam ().GetFaceSet (&faceSet);
    for_each (
	faceSet.begin (), faceSet.end (),
	DisplayFaceWithHighlightColor<0, DisplayEdges<
	DisplayEdgeTorus<DisplayEdgeQuadric, DisplayArrowQuadric, true> > > (
	    *this));
    glPopAttrib ();
}


void GLWidget::displayFacesTorusLines () const
{
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);

    FaceSet faceSet;
    GetCurrentFoam ().GetFaceSet (&faceSet);
    for_each (faceSet.begin (), faceSet.end (),
	      DisplayFaceWithHighlightColor<0,
	      DisplayEdges<
	      DisplayEdgeTorus<DisplayEdge, DisplayArrow, true> > > (
		  *this, DisplayElement::FOCUS) );
    glPopAttrib ();
}

void GLWidget::displayCenterPathsWithBodies () const
{
    glLineWidth (1.0);
    displayCenterPaths ();
    
    glPushAttrib (GL_ENABLE_BIT);
    if (isLightingEnabled ())
	glDisable (GL_LIGHTING);
    if (IsCenterPathBodyShown ())
    {
	const Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
	double zPos = GetTimeStep () * GetTimeDisplacement ();
	for_each (
	    bodies.begin (), bodies.end (),
	    DisplayBody<DisplayFaceWithHighlightColor<0,
	    DisplayEdges<DisplayEdgeWithColor<
	    DisplayElement::DONT_DISPLAY_TESSELLATION> > > > (
		*this, *m_bodySelector, DisplayElement::INVISIBLE_CONTEXT,
		BodyProperty::NONE, IsTimeDisplacementUsed (), zPos));
	displayCenterOfBodies (IsTimeDisplacementUsed ());
    }
    displayStandaloneEdges< DisplayEdgeWithColor<> > (true, 0);
    if (GetTimeDisplacement () != 0)
    {

	displayStandaloneEdges< DisplayEdgeWithColor<> > (
	    IsTimeDisplacementUsed (),
	    (GetFoamAlongTime ().GetTimeSteps () - 1)*GetTimeDisplacement ());
    }
    glPopAttrib ();
}

void GLWidget::displayCenterPaths () const
{
    glCallList (m_listCenterPaths);
}

void GLWidget::compile () const
{
    switch (m_viewType)
    {
    case ViewType::CENTER_PATHS:
	compileCenterPaths ();
	break;
    default:
	break;
    }    
}

void GLWidget::compileCenterPaths () const
{
    glNewList (m_listCenterPaths, GL_COMPILE);
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT | 
		  GL_POLYGON_BIT);
    glEnable(GL_TEXTURE_1D);
    glBindTexture (GL_TEXTURE_1D, GetColorBarTexture ());
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_CULL_FACE);

    //See OpenGL FAQ 21.030 Why doesn't lighting work when I turn on 
    //texture mapping?
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    const BodiesAlongTime::BodyMap& bats = GetBodiesAlongTime ().GetBodyMap ();
    if (m_edgesTubes)
    {
	if (m_tubeCenterPathUsed)
	    for_each (
		bats.begin (), bats.end (),
		DisplayCenterPath<
		SetterValueTextureCoordinate, DisplayEdgeTube> (
		    *this, m_bodyProperty, *m_bodySelector,
		    IsTimeDisplacementUsed (), GetTimeDisplacement ()));
	else
	    for_each (
		bats.begin (), bats.end (),
		DisplayCenterPath<
		SetterValueTextureCoordinate, DisplayEdgeQuadric> (
		    *this, m_bodyProperty, *m_bodySelector,
		    IsTimeDisplacementUsed (), GetTimeDisplacement ()));
    }
    else
	for_each (bats.begin (), bats.end (),
		  DisplayCenterPath<SetterValueTextureCoordinate, DisplayEdge> (
		      *this, m_bodyProperty, *m_bodySelector,
		      IsTimeDisplacementUsed (), GetTimeDisplacement ()));
    glPopAttrib ();
    glEndList ();
}

void GLWidget::DisplayViewType () const
{
    (this->*(m_viewTypeDisplay[m_viewType])) ();
}

bool GLWidget::IsDisplayedBody (size_t bodyId) const
{
    return (AllBodiesSelected () || GetSelectedBodyId () == bodyId);
}

bool GLWidget::IsDisplayedBody (const boost::shared_ptr<Body> body) const
{
    return IsDisplayedBody (body->GetId ());
}

bool GLWidget::IsDisplayedFace (size_t faceI) const
{
    size_t faceIndex = GetSelectedFaceIndex ();
    return (faceIndex == DISPLAY_ALL || faceIndex == faceI);
}

bool GLWidget::IsDisplayedEdge (size_t oeI) const
{
    size_t edgeIndex = GetSelectedEdgeIndex ();
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
	m_selectedBodyIndex != DISPLAY_ALL;
}

bool GLWidget::doesSelectEdge () const
{
    return
	m_selectedFaceIndex != DISPLAY_ALL &&
	m_viewType != ViewType::FACES;
}


void GLWidget::IncrementSelectedBodyIndex ()
{
    if (doesSelectBody ())
    {
	++m_selectedBodyIndex;
	m_selectedFaceIndex = DISPLAY_ALL;
	if (m_selectedBodyIndex ==
	    GetFoamAlongTime ().GetFoam (0)->GetBodies ().size ())
	{
	    m_selectedBodyIndex = DISPLAY_ALL;
	    SetBodySelector (AllBodySelector::Get (), BodySelectorType::ID);
	}
	else
	{
	    size_t id = GetCurrentFoam ().GetBodies ()[
		m_selectedBodyIndex]->GetId ();
	    cdbg << "IncrementSelectedBodyIndex index: " << m_selectedBodyIndex
		 << " id: " << id << endl;
	    SetBodySelector (
		boost::shared_ptr<IdBodySelector> (new IdBodySelector (id)));
	}
	update ();
    }
}


void GLWidget::IncrementSelectedFaceIndex ()
{
    if (doesSelectFace ())
    {
	++m_selectedFaceIndex;
        Body& body = *GetCurrentFoam ().GetBodies ()[m_selectedBodyIndex];
        if (m_selectedFaceIndex == body.GetOrientedFaces ().size ())
            m_selectedFaceIndex = DISPLAY_ALL;
	update ();
    }
}

void GLWidget::IncrementSelectedEdgeIndex ()
{
    if (doesSelectEdge ())
    {
	++m_selectedEdgeIndex;
	Face& face = *GetSelectedFace ();
	if (m_selectedEdgeIndex == face.GetOrientedEdges ().size ())
	    m_selectedEdgeIndex = DISPLAY_ALL;
	update ();
    }
}

void GLWidget::DecrementSelectedBodyIndex ()
{
    if (doesSelectBody ())
    {
	if (m_selectedBodyIndex == DISPLAY_ALL)
	    m_selectedBodyIndex =
		GetFoamAlongTime ().GetFoam (0)->GetBodies ().size ();
	--m_selectedBodyIndex;
	m_selectedFaceIndex = DISPLAY_ALL;
	if (m_selectedBodyIndex != DISPLAY_ALL)
	{
	    size_t id = GetCurrentFoam ().GetBodies ()[
		m_selectedBodyIndex]->GetId ();
	    cdbg << "IncrementSelectedBodyIndex index: "
		 << m_selectedBodyIndex << " id: " << id << endl;
	    SetBodySelector (
		boost::shared_ptr<IdBodySelector> (new IdBodySelector (id)));
	}
	else
	{
	    SetBodySelector (AllBodySelector::Get (), BodySelectorType::ID);
	}
	update ();
    }
}

void GLWidget::DecrementSelectedFaceIndex ()
{
    if (doesSelectFace ())
    {
        Body& body = *GetCurrentFoam ().GetBodies ()[m_selectedBodyIndex];
        if (m_selectedFaceIndex == DISPLAY_ALL)
            m_selectedFaceIndex = body.GetOrientedFaces ().size ();
	--m_selectedFaceIndex;
	update ();
    }
}

void GLWidget::DecrementSelectedEdgeIndex ()
{
    if (doesSelectEdge ())
    {
	Face& face = *GetSelectedFace ();
	if (m_selectedEdgeIndex == DISPLAY_ALL)
	    m_selectedEdgeIndex = face.GetOrientedEdges ().size ();
	--m_selectedEdgeIndex;
	update ();
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

boost::shared_ptr<Body> GLWidget::GetSelectedBody () const
{
    return GetBodyAlongTime (GetSelectedBodyId ()).GetBody (GetTimeStep ());
}

size_t GLWidget::GetSelectedBodyId () const
{
    return GetFoamAlongTime ().GetFoam (0)->GetBody (
	m_selectedBodyIndex)->GetId ();
}

size_t GLWidget::GetSelectedFaceId () const
{
    return GetSelectedFace ()->GetId ();
}

boost::shared_ptr<Face> GLWidget::GetSelectedFace () const
{
    size_t i = GetSelectedFaceIndex ();
    if (m_selectedBodyIndex != DISPLAY_ALL)
    {
	Body& body = *GetSelectedBody ();
	return body.GetFace (i);
    }
    RuntimeAssert (false, "There is no displayed face");
    return boost::shared_ptr<Face>();
}

boost::shared_ptr<Edge> GLWidget::GetSelectedEdge () const
{
    if (m_selectedBodyIndex != DISPLAY_ALL &&
	m_selectedFaceIndex != DISPLAY_ALL)
    {
	boost::shared_ptr<Face> face = GetSelectedFace ();
	return face->GetEdge (m_selectedEdgeIndex);
    }
    RuntimeAssert (false, "There is no displayed edge");
    return boost::shared_ptr<Edge>();
}

size_t GLWidget::GetSelectedEdgeId () const
{
    return GetSelectedEdge ()->GetId ();
}

void GLWidget::enableLighting (bool checked)
{
    m_lightingEnabled = checked;
    if (checked)
	glEnable (GL_LIGHTING);
    else
	glDisable (GL_LIGHTING);
    update ();
}



// Slots
// ======================================================================

void GLWidget::ToggledDirectionalLightEnabled (bool checked)
{
    m_directionalLightEnabled[m_selectedLight] = checked;
    positionLight (m_selectedLight);
    update ();
}

void GLWidget::ToggledLightPositionShown (bool checked)
{
    m_lightPositionShown[m_selectedLight] = checked;
    update ();
}

void GLWidget::ToggledLightEnabled (bool checked)
{
    makeCurrent ();
    m_lightEnabled[m_selectedLight] = checked;
    if (checked)
    {
	glEnable(GL_LIGHT0 + m_selectedLight);
	positionLight (m_selectedLight);
    }
    else
	glDisable (GL_LIGHT0 + m_selectedLight);
    enableLighting (m_lightEnabled.any ());
    update ();
}


void GLWidget::ToggledBoundingBoxShown (bool checked)
{
    m_boundingBoxShown = checked;
    update ();
}

void GLWidget::ToggledBodiesBoundingBoxesShown (bool checked)
{
    m_bodiesBoundingBoxesShown = checked;
    update ();
}


void GLWidget::ToggledColorBarShown (bool checked)
{
    m_textureColorBarShown = ! checked;
    update ();
}

void GLWidget::ToggledContextView (bool checked)
{
    m_contextView = checked;
    projectionTransform ();
    update ();
}


void GLWidget::ToggledHideContent (bool checked)
{
    m_hideContent = checked;
    update ();
}

void GLWidget::ToggledAxesShown (bool checked)
{
    m_axesShown = checked;
    update ();
}


void GLWidget::ToggledCenterPathBodyShown (bool checked)
{
    m_centerPathBodyShown = checked;
    update ();
}

void GLWidget::ToggledIsContextHidden (bool checked)
{
    m_contextHidden = checked;
    compile ();
    update ();
}


void GLWidget::ToggledEdgesNormal (bool checked)
{
    changeView (checked, ViewType::EDGES);
}

void GLWidget::ToggledEdgesTorus (bool checked)
{
    changeView (checked, ViewType::EDGES_TORUS);
}

void GLWidget::ToggledEdgesBodyCenter (bool checked)
{
    m_edgesBodyCenter = checked;
    update ();
}

void GLWidget::ToggledFacesNormal (bool checked)
{
    changeView (checked, ViewType::FACES);
}

void GLWidget::ToggledFacesShowEdges (bool checked)
{
    m_facesShowEdges = checked;
    update ();
}

void GLWidget::ToggledFaceEdgesTorus (bool checked)
{
    changeView (checked, ViewType::FACES_TORUS);
}


void GLWidget::ToggledEdgesTessellation (bool checked)
{
    m_edgesTessellation = checked;
    update ();
}


void GLWidget::ToggledTorusOriginalDomainShown (bool checked)
{
    m_torusOriginalDomainDisplay = checked;
    update ();
}

void GLWidget::ToggledTubeCenterPathUsed (bool checked)
{
    m_tubeCenterPathUsed = checked;
    update ();
}


void GLWidget::ToggledTorusOriginalDomainClipped (bool checked)
{
    m_torusOriginalDomainClipped = checked;
    update ();
}

void GLWidget::ToggledT1sShown (bool checked)
{
    m_t1sShown = checked;
    update ();
}

void GLWidget::ToggledCenterPath (bool checked)
{
    changeView (checked, ViewType::CENTER_PATHS);
}


void GLWidget::CurrentIndexChangedSelectedLight (int selectedLight)
{
    m_selectedLight = LightPosition::Enum (selectedLight);
}

void GLWidget::CurrentIndexChangedInteractionMode (int index)
{
    m_interactionMode = InteractionMode::Enum(index);
}

void GLWidget::ToggledFacesStatistics (bool checked)
{
    makeCurrent ();
    if (checked)
    {
	pair<double, double> minMax = getStatisticsMinMax ();
	m_displayFaceStatistics->InitStep (minMax.first, minMax.second);
    }
    else
	m_displayFaceStatistics->Release ();
    changeView (checked, ViewType::FACES_STATISTICS);
}

void GLWidget::CurrentIndexChangedStatisticsType (int index)
{
    m_statisticsType = StatisticsType::Enum(index);
    update ();
}

void GLWidget::CurrentIndexChangedAxesOrder (int index)
{
    m_axesOrder = AxesOrder::Enum(index);
    ResetTransformation ();
}

// @todo add a color bar model for BodyProperty::None
void GLWidget::SetBodyProperty (
    boost::shared_ptr<ColorBarModel> colorBarModel,
    BodyProperty::Enum property)
{
    m_bodyProperty = property;
    if (m_bodyProperty != BodyProperty::NONE)
	SetColorBarModel (colorBarModel);
    else
	m_colorBarModel.reset ();
    compile ();
    update ();
}

void GLWidget::SetColorBarModel (boost::shared_ptr<ColorBarModel> colorBarModel)
{
    makeCurrent ();
    m_colorBarModel = colorBarModel;
    const QImage image = colorBarModel->GetImage ();
    glTexImage1D (GL_TEXTURE_1D, 0, GL_RGB, image.width (),
		  0, GL_BGRA, GL_UNSIGNED_BYTE, image.scanLine (0));
    update ();
}

void GLWidget::ValueChangedSliderTimeSteps (int timeStep)
{
    m_timeStep = timeStep;
    if (m_viewType == ViewType::FACES_STATISTICS)
    {
	makeCurrent ();
	pair<double, double> minMax = getStatisticsMinMax ();
	m_displayFaceStatistics->Step (minMax.first, minMax.second);
    }
    update ();
}

void GLWidget::ValueChangedStatisticsHistory (int timeSteps)
{
    m_displayFaceStatistics->SetHistoryCount (timeSteps);
}

void GLWidget::ValueChangedTimeDisplacement (int timeDisplacement)
{
    QSlider* slider = static_cast<QSlider*> (sender ());
    size_t maximum = slider->maximum ();
    G3D::AABox bb = GetFoamAlongTime ().GetBoundingBox ();
    m_timeDisplacement =
	(bb.high () - bb.low ()).z * timeDisplacement /
	GetFoamAlongTime ().GetTimeSteps () / maximum;
    compile ();
    update ();
}

void GLWidget::ValueChangedEdgesRadius (int sliderValue)
{
    makeCurrent ();
    size_t maximum = static_cast<QSlider*> (sender ())->maximum ();
    m_edgeRadiusMultiplier = static_cast<double>(sliderValue) / maximum;
    setEdgeRadius ();
    enableLighting (m_lightEnabled.any ());
    compile ();
    update ();
}

void GLWidget::ValueChangedContextAlpha (int sliderValue)
{
    size_t maximum = static_cast<QSlider*> (sender ())->maximum ();
    m_contextAlpha = MIN_CONTEXT_ALPHA +
	(MAX_CONTEXT_ALPHA - MIN_CONTEXT_ALPHA) * sliderValue / maximum;
    compile ();
    update ();
}

void GLWidget::ValueChangedLightAmbientRed (int sliderValue)
{
    size_t maximum = static_cast<QSlider*> (sender ())->maximum ();
    m_lightAmbient[m_selectedLight][0] = 
	static_cast<double>(sliderValue) / maximum;
    glLightfv(GL_LIGHT0 + m_selectedLight, GL_AMBIENT, 
	      &m_lightAmbient[m_selectedLight][0]);
    update ();
}

void GLWidget::ValueChangedLightAmbientGreen (int sliderValue)
{
    size_t maximum = static_cast<QSlider*> (sender ())->maximum ();
    m_lightAmbient[m_selectedLight][1] = 
	static_cast<double>(sliderValue) / maximum;
    glLightfv(GL_LIGHT0 + m_selectedLight, GL_AMBIENT, 
	      &m_lightAmbient[m_selectedLight][0]);
    update ();
}

void GLWidget::ValueChangedLightAmbientBlue (int sliderValue)
{
    size_t maximum = static_cast<QSlider*> (sender ())->maximum ();
    m_lightAmbient[m_selectedLight][2] = 
	static_cast<double>(sliderValue) / maximum;
    glLightfv(GL_LIGHT0 + m_selectedLight, GL_AMBIENT, 
	      &m_lightAmbient[m_selectedLight][0]);
    update ();
}

void GLWidget::ValueChangedLightDiffuseRed (int sliderValue)
{
    size_t maximum = static_cast<QSlider*> (sender ())->maximum ();
    m_lightDiffuse[m_selectedLight][0] = 
	static_cast<double>(sliderValue) / maximum;
    glLightfv(GL_LIGHT0 + m_selectedLight, GL_DIFFUSE, 
	      &m_lightDiffuse[m_selectedLight][0]);
    update ();
}

void GLWidget::ValueChangedLightDiffuseGreen (int sliderValue)
{
    size_t maximum = static_cast<QSlider*> (sender ())->maximum ();
    m_lightDiffuse[m_selectedLight][1] = 
	static_cast<double>(sliderValue) / maximum;
    glLightfv(GL_LIGHT0 + m_selectedLight, GL_DIFFUSE, 
	      &m_lightDiffuse[m_selectedLight][0]);
    update ();
}

void GLWidget::ValueChangedLightDiffuseBlue (int sliderValue)
{
    size_t maximum = static_cast<QSlider*> (sender ())->maximum ();
    m_lightDiffuse[m_selectedLight][2] = 
	static_cast<double>(sliderValue) / maximum;
    glLightfv(GL_LIGHT0 + m_selectedLight, GL_DIFFUSE, 
	      &m_lightDiffuse[m_selectedLight][0]);
    update ();
}

void GLWidget::ValueChangedLightSpecularRed (int sliderValue)
{
    size_t maximum = static_cast<QSlider*> (sender ())->maximum ();
    m_lightSpecular[m_selectedLight][0] = 
	static_cast<double>(sliderValue) / maximum;
    glLightfv(GL_LIGHT0 + m_selectedLight, GL_SPECULAR, 
	      &m_lightSpecular[m_selectedLight][0]);
    update ();
}

void GLWidget::ValueChangedLightSpecularGreen (int sliderValue)
{
    size_t maximum = static_cast<QSlider*> (sender ())->maximum ();
    m_lightSpecular[m_selectedLight][1] = 
	static_cast<double>(sliderValue) / maximum;
    glLightfv(GL_LIGHT0 + m_selectedLight, GL_SPECULAR, 
	      &m_lightSpecular[m_selectedLight][0]);
    update ();
}

void GLWidget::ValueChangedLightSpecularBlue (int sliderValue)
{
    size_t maximum = static_cast<QSlider*> (sender ())->maximum ();
    m_lightSpecular[m_selectedLight][2] = 
	static_cast<double>(sliderValue) / maximum;
    glLightfv(GL_LIGHT0 + m_selectedLight, GL_SPECULAR, 
	      &m_lightSpecular[m_selectedLight][0]);
    update ();
}

void GLWidget::ValueChangedAngleOfView (int angleOfView)
{
    m_angleOfView = angleOfView;
    calculateCameraDistance ();
    projectionTransform ();
    update ();
}


void GLWidget::InfoOpenGL ()
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


void GLWidget::contextMenuEvent(QContextMenuEvent *event)
{
    m_contextMenuPos = event->pos ();
    QMenu menu (this);
    menu.addAction (m_actionResetTransformation.get ());
    menu.addAction (m_actionResetSelectedLightPosition.get ());
    menu.addAction (m_actionSelectAll.get ());
    menu.addAction (m_actionDeselectAll.get ());
    menu.addAction (m_actionSelectBodiesById.get ());
    {
	QMenu* menuStationary = menu.addMenu ("Stationary");
	menuStationary->addAction (m_actionStationarySet.get ());
	menuStationary->addAction (m_actionStationaryReset.get ());
	menuStationary->addAction (m_actionStationaryContextAdd.get ());
    }
    {
	QMenu* menuInfo = menu.addMenu ("Info");
	menuInfo->addAction (m_actionInfoFocus.get ());
	menuInfo->addAction (m_actionInfoFoam.get ());
	menuInfo->addAction (m_actionInfoOpenGL.get ());
    }
    menu.exec (event->globalPos());
}

void GLWidget::initializeTextures ()
{
    glGenTextures (1, &m_colorBarTexture);
    glBindTexture (GL_TEXTURE_1D, m_colorBarTexture);

    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void GLWidget::displayTextureColorBar () const
{
    if (! m_textureColorBarShown)
	return;
    glPushAttrib (
	GL_CURRENT_BIT | GL_VIEWPORT_BIT | GL_TEXTURE_BIT | GL_ENABLE_BIT);
    if (isLightingEnabled ())
	glDisable (GL_LIGHTING);
    // modelview
    glPushMatrix ();
    glLoadIdentity ();
    
    // projection
    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity ();
    glOrtho (0, width (), 0, height (), -1, 1);

    glViewport (0, 0, width (), height ());

    glEnable(GL_TEXTURE_1D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture (GL_TEXTURE_1D, m_colorBarTexture);

    const int BAR_WIDTH = 8;
    const int BAR_HEIGHT = max (height () / 4, 50);
    glBegin (GL_QUADS);
    glTexCoord1f(0);glVertex2s (0, 0);
    glTexCoord1f(1);glVertex2s (0, BAR_HEIGHT);
    glTexCoord1f(1);glVertex2s (BAR_WIDTH, BAR_HEIGHT);
    glTexCoord1f(0);glVertex2s (BAR_WIDTH, 0);
    glEnd ();
    glPopMatrix ();
    glMatrixMode (GL_MODELVIEW);
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


void GLWidget::setBodySelectorLabel (BodySelectorType::Enum type)
{
    switch (type)
    {
    case BodySelectorType::PROPERTY_VALUE:
	m_labelStatusBar->setText ("Selection: by property");
	break;
    case BodySelectorType::ID:
	m_labelStatusBar->setText ("Selection: by id");
	break;
    case BodySelectorType::COMPOSITE:
	m_labelStatusBar->setText ("Selection: by id and property");
	break;
    default:
	return m_labelStatusBar->setText ("");
    }
}

void GLWidget::UnionBodySelector (const vector<size_t>& bodyIds)
{
    switch (m_bodySelector->GetType ())
    {
    case BodySelectorType::ALL:
	m_bodySelector = boost::make_shared<IdBodySelector> (bodyIds);
	break;

    case BodySelectorType::ID:
    {
	IdBodySelector& selector =
	    *boost::static_pointer_cast<IdBodySelector> (m_bodySelector);
	selector.SetUnion (bodyIds);
	break;
    }

    case BodySelectorType::PROPERTY_VALUE:
    {
	boost::shared_ptr<IdBodySelector> idSelector =
	    boost::make_shared<IdBodySelector> (bodyIds);
	m_bodySelector = boost::make_shared<CompositeBodySelector> (
	    idSelector,
	    boost::static_pointer_cast<PropertyValueBodySelector> (
		m_bodySelector));
	break;
    }

    case BodySelectorType::COMPOSITE:
	boost::static_pointer_cast<CompositeBodySelector> (
	    m_bodySelector)->GetIdSelector ()->SetUnion (bodyIds);
	break;
    }
    setBodySelectorLabel (m_bodySelector->GetType ());
    compile ();
    update ();
}

void GLWidget::DifferenceBodySelector (const vector<size_t>& bodyIds)
{
    switch (m_bodySelector->GetType ())
    {
    case BodySelectorType::ALL:
    {
	m_bodySelector = idBodySelectorComplement (GetCurrentFoam (), bodyIds);
	break;
    }
    case BodySelectorType::ID:
	boost::static_pointer_cast<IdBodySelector> (
	    m_bodySelector)->SetDifference (bodyIds);
	break;

    case BodySelectorType::PROPERTY_VALUE:
    {
	boost::shared_ptr<IdBodySelector> idSelector =
	    idBodySelectorComplement (GetCurrentFoam (), bodyIds);
	m_bodySelector = boost::make_shared<CompositeBodySelector> (
	    idSelector,
	    boost::static_pointer_cast<PropertyValueBodySelector> (
		m_bodySelector));
	break;
    }

    case BodySelectorType::COMPOSITE:
	boost::static_pointer_cast<CompositeBodySelector> (
	    m_bodySelector)->GetIdSelector ()->SetDifference (bodyIds);
	break;
    }
    setBodySelectorLabel (m_bodySelector->GetType ());
    compile ();
    update ();
}

void GLWidget::SetBodySelector (boost::shared_ptr<IdBodySelector> selector)
{
    switch (m_bodySelector->GetType ())
    {
    case BodySelectorType::ALL:
	m_bodySelector = selector;
	break;
    case BodySelectorType::ID:
	m_bodySelector = selector;
	break;
    case BodySelectorType::PROPERTY_VALUE:
	m_bodySelector = boost::shared_ptr<BodySelector> (
	    new CompositeBodySelector (
		selector,
		boost::static_pointer_cast<PropertyValueBodySelector> (
		    m_bodySelector)));
	break;
    case BodySelectorType::COMPOSITE:
	boost::static_pointer_cast<CompositeBodySelector> (
	    m_bodySelector)->SetSelector (selector);
	break;
    }
    setBodySelectorLabel (m_bodySelector->GetType ());
    compile ();
    update ();
}



void GLWidget::SetBodySelector (
    boost::shared_ptr<PropertyValueBodySelector> selector)
{
    switch (m_bodySelector->GetType ())
    {
    case BodySelectorType::ALL:
	m_bodySelector = selector;
	break;
    case BodySelectorType::ID:
	m_bodySelector = boost::shared_ptr<BodySelector> (
	    new CompositeBodySelector (
		boost::static_pointer_cast<IdBodySelector> (m_bodySelector),
		selector));
	break;
    case BodySelectorType::PROPERTY_VALUE:
	m_bodySelector = selector;
	break;
    case BodySelectorType::COMPOSITE:
	boost::static_pointer_cast<CompositeBodySelector> (
	    m_bodySelector)->SetSelector (selector);
	break;
    }
    setBodySelectorLabel (m_bodySelector->GetType ());
    compile ();
    update ();
}

void GLWidget::SetBodySelector (
    boost::shared_ptr<AllBodySelector> selector, BodySelectorType::Enum type)
{
    switch (m_bodySelector->GetType ())
    {
    case BodySelectorType::ALL:
	break;
    case BodySelectorType::ID:
    case BodySelectorType::PROPERTY_VALUE:
	if (type == m_bodySelector->GetType ())
	    m_bodySelector = selector;
    	break;
    case BodySelectorType::COMPOSITE:
	if (type == BodySelectorType::ID)
	    m_bodySelector = boost::static_pointer_cast<CompositeBodySelector> (
		m_bodySelector)->GetPropertyValueSelector ();
	else
	    m_bodySelector = boost::static_pointer_cast<CompositeBodySelector> (
		m_bodySelector)->GetIdSelector ();
	break;
    }
    setBodySelectorLabel (m_bodySelector->GetType ());
    compile ();
    update ();
}

bool GLWidget::IsTimeDisplacementUsed () const
{
    if (GetFoamAlongTime ().GetDimension () == 2)
    {
	return GetTimeDisplacement () > 0;
    }
    else
	return false;
}
