/**
 * @file   GLWidget.cpp
 * @author Dan R. Lipsa
 *
 * Definitions for the widget for displaying foam bubbles using OpenGL
 */

// @todo fix the (slow) movement of the focus in context view

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
#include "OrientedEdge.h"
#include "SelectBodiesById.h"
#include "Utils.h"
#include "Vertex.h"
#include "ViewSettings.h"

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

G3D::AABox EncloseRotation (const G3D::AABox& box)
{
    using G3D::Vector3;
    Vector3 center = box.center ();
    double maxHalfSideLength = (box.high () - center).length ();
    double minHalfSideLength = box.extent ().min () / 2;
    double halfSideLength = minHalfSideLength + 
	(maxHalfSideLength - minHalfSideLength);
    Vector3 halfDiagonal = halfSideLength * 
	(Vector3::unitX () + Vector3::unitY () + Vector3::unitZ ());
    return G3D::AABox (center - halfDiagonal, center + halfDiagonal);
}

G3D::AABox AdjustZ (const G3D::AABox& b, double scaleRatio)
{
    if (scaleRatio > 1)
    {
	double zExtent2 = b.extent ().z * scaleRatio / 2;
	double zCenter = b.center ().z;
	G3D::Vector3 low = b.low (), high = b.high ();
	return G3D::AABox (
	    G3D::Vector3 (low.x, low.y, zCenter - zExtent2),
	    G3D::Vector3 (high.x, high.y, zCenter + zExtent2));
    }
    else
	return b;
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
// quadrics
const size_t GLWidget::QUADRIC_SLICES = 8;
const size_t GLWidget::QUADRIC_STACKS = 1;
// alpha
const double GLWidget::MIN_CONTEXT_ALPHA = 0.05;
const double GLWidget::MAX_CONTEXT_ALPHA = 0.5;

const double GLWidget::ENCLOSE_ROTATION_RATIO = 1;
const GLfloat GLWidget::MAX_T1_SIZE = 10.0;
const GLfloat GLWidget::MIN_T1_SIZE = 1.0;
const GLfloat GLWidget::HIGHLIGHT_LINE_WIDTH = 2.0;

// Methods
// ======================================================================

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent),
      m_torusOriginalDomainDisplay (false),
      m_torusOriginalDomainClipped (false),
      m_interactionMode (InteractionMode::ROTATE),
      m_foamAlongTime (0), m_timeStep (0),
      m_selectedBodyIndex (DISPLAY_ALL), m_selectedFaceIndex (DISPLAY_ALL),
      m_selectedEdgeIndex (DISPLAY_ALL),
      m_contextAlpha (MIN_CONTEXT_ALPHA),
      m_minimumEdgeRadius (0),
      m_edgeRadiusRatio (0),
      m_facesShowEdges (true),
      m_bodyCenterShown (false),
      m_edgesTessellation (true),
      m_centerPathBodyShown (false),
      m_contextHidden (false),
      m_boundingBoxShown (false),
      m_bodiesBoundingBoxesShown (false),
      m_axesShown (false),
      m_standaloneElementsShown (true),
      m_bodySelector (AllBodySelector::Get ()),
      m_timeDisplacement (0.0),
      m_playMovie (false),
      m_selectBodiesById (new SelectBodiesById (this)),
      m_hideContent(false),
      m_centerPathTubeUsed (true),
      m_centerPathLineUsed (false),
      m_t1sShown (false),
      m_t1Size (MIN_T1_SIZE),
      m_zeroedPressureShown (false),
      m_titleShown (false),
      m_timeStepShown (false),
      m_bodyStationaryMarked (true),
      m_viewCount (ViewCount::ONE),
      m_viewLayout (ViewLayout::HORIZONTAL),
      m_viewNumber (ViewNumber::VIEW0)
{
    makeCurrent ();
    BOOST_FOREACH (boost::shared_ptr<ViewSettings>& vs, m_viewSettings)
	vs = boost::make_shared <ViewSettings> (*this);
    initEndTranslationColor ();
    initQuadrics ();
    initViewTypeDisplay ();
    createActions ();
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
    m_actionSelectAll = boost::make_shared<QAction> (tr("&All"), this);
    m_actionSelectAll->setStatusTip(tr("Select all"));
    connect(m_actionSelectAll.get (), SIGNAL(triggered()),
	    this, SLOT(SelectAll ()));

    m_actionDeselectAll = boost::make_shared<QAction> (
	tr("&Deselect all"), this);
    m_actionDeselectAll->setStatusTip(tr("Deselect all"));
    connect(m_actionDeselectAll.get (), SIGNAL(triggered()),
	    this, SLOT(DeselectAll ()));

    m_actionSelectBodiesById = boost::make_shared<QAction> (
	tr("&Bodies by id"), this);
    m_actionSelectBodiesById->setStatusTip(tr("Select bodies by id"));
    connect(m_actionSelectBodiesById.get (), SIGNAL(triggered()),
	    this, SLOT(SelectBodiesByIdList ()));


    m_actionResetTransformation = boost::make_shared<QAction> (
	tr("&Transformation"), this);
    m_actionResetTransformation->setShortcut(
	QKeySequence (tr ("Ctrl+R")));
    m_actionResetTransformation->setStatusTip(tr("Reset transformation"));
    connect(m_actionResetTransformation.get (), SIGNAL(triggered()),
	    this, SLOT(ResetTransformation ()));


    m_actionResetSelectedLightNumber = boost::make_shared<QAction> (
	tr("&Selected light"), this);
    m_actionResetSelectedLightNumber->setShortcut(
	QKeySequence (tr ("Ctrl+L")));
    m_actionResetSelectedLightNumber->setStatusTip(
	tr("Reset selected light"));
    connect(m_actionResetSelectedLightNumber.get (), SIGNAL(triggered()),
	    this, SLOT(ResetSelectedLightNumber ()));

    m_actionStationaryBody = boost::make_shared<QAction> (
	tr("&Body"), this);
    m_actionStationaryBody->setStatusTip(tr("Stationary body"));
    connect(m_actionStationaryBody.get (), SIGNAL(triggered()),
	    this, SLOT(StationaryBody ()));

    m_actionStationaryConstraint = boost::make_shared<QAction> (
	tr("&Constraint"), this);
    m_actionStationaryConstraint->setStatusTip(tr("Stationary constraint"));
    connect(m_actionStationaryConstraint.get (), SIGNAL(triggered()),
	    this, SLOT(StationaryConstraint ()));

    m_actionStationaryReset = boost::make_shared<QAction> (
	tr("&Reset"), this);
    m_actionStationaryReset->setStatusTip(tr("Stationary reset"));
    connect(m_actionStationaryReset.get (), SIGNAL(triggered()),
	    this, SLOT(StationaryReset ()));

    m_actionContextBody = boost::make_shared<QAction> (
	tr("&Body"), this);
    m_actionContextBody->setStatusTip(tr("Context body"));
    connect(m_actionContextBody.get (), SIGNAL(triggered()),
	    this, SLOT(ContextBody ()));

    m_actionContextReset = boost::make_shared<QAction> (
	tr("&Reset"), this);
    m_actionContextReset->setStatusTip(tr("Context reset"));
    connect(m_actionContextReset.get (), SIGNAL(triggered()),
	    this, SLOT(ContextReset ()));

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

    // actions for the color bar
    m_actionEditColorMap.reset (
	new QAction (tr("&Edit Color Map"), this));
    m_actionEditColorMap->setStatusTip(tr("Edit Color Map"));
    connect(m_actionEditColorMap.get (), SIGNAL(triggered()),
	    this, SLOT(ColorBarEdit ()));

    m_actionClampClear.reset (
	new QAction (tr("&Clamp Clear"), this));
    m_actionClampClear->setStatusTip(tr("Clamp Clear"));
    connect(m_actionClampClear.get (), SIGNAL(triggered()),
	    this, SLOT(ColorBarClampClear ()));

    initCopy (m_actionCopyTransformations, m_signalMapperCopyTransformations);
    connect (m_signalMapperCopyTransformations.get (),
	     SIGNAL (mapped (int)),
	     this,
	     SLOT (CopyTransformationsFrom (int)));

    initCopy (m_actionCopyColorBar, m_signalMapperCopyColorBar);
    connect (m_signalMapperCopyColorBar.get (),
	     SIGNAL (mapped (int)),
	     this,
	     SLOT (CopyColorBarFrom (int)));
}

void GLWidget::initCopy (
    boost::array<boost::shared_ptr<QAction>, 
    ViewNumber::COUNT>& actionCopyTransformations,
    boost::shared_ptr<QSignalMapper>& signalMapperCopyTransformations)
{
    signalMapperCopyTransformations.reset (new QSignalMapper (this));
    for (size_t i = 0; i < ViewNumber::COUNT; ++i)
    {
	ostringstream ostr;
	ostr << "View " << i;
	QString text (ostr.str ().c_str ());
	actionCopyTransformations[i] = boost::make_shared<QAction> (
	    text, this);
	actionCopyTransformations[i]->setStatusTip(text);
	connect(actionCopyTransformations[i].get (), 
		SIGNAL(triggered()),
		signalMapperCopyTransformations.get (), 
		SLOT(map ()));
	signalMapperCopyTransformations->setMapping (
	    actionCopyTransformations[i].get (), i);
    }
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
    if (foamAlongTime->Is2D ())
    {
	BOOST_FOREACH (boost::shared_ptr<ViewSettings> vs, m_viewSettings)
	{
	    vs->SetViewType (ViewType::EDGES);
	    vs->SetAxesOrder (AxesOrder::TWO_D);
	}
    }
    else
    {
	BOOST_FOREACH (boost::shared_ptr<ViewSettings> vs, m_viewSettings)
	{
	    vs->SetViewType (ViewType::FACES);
	    vs->SetAxesOrder (AxesOrder::THREE_D);
	}
    }
    Foam::Bodies bodies = foamAlongTime->GetFoam (0)->GetBodies ();
    if (bodies.size () != 0)
    {
	size_t maxIndex = bodies.size () - 1;
	m_selectBodiesById->SetMinBodyId (bodies[0]->GetId ());
	m_selectBodiesById->SetMaxBodyId (bodies[maxIndex]->GetId ());
	m_selectBodiesById->UpdateLabelMinMax ();
    }
    BOOST_FOREACH (boost::shared_ptr<ViewSettings> vs, m_viewSettings)
    {
	vs->GetDisplayFaceStatistics ()->SetHistoryCount (
	    foamAlongTime->GetTimeSteps ());
    }
}


double GLWidget::getMinimumEdgeRadius () const
{
    G3D::Vector3 objectOrigin = gluUnProject (G3D::Vector2::zero (),
					      GluUnProjectZOperation::SET0);
    G3D::Vector3 objectOne = gluUnProject (G3D::Vector2::unitX (),
					   GluUnProjectZOperation::SET0);
    return (objectOne - objectOrigin).length ();
}

void GLWidget::calculateEdgeRadius (double edgeRadiusRatio,
				    double* edgeRadius, double* arrowBaseRadius, 
				    double* arrowHeight,
				    double* edgeWidth) const
{
    const int maxRadiusMultiplier = 5;

    double r = m_minimumEdgeRadius;
    double R = maxRadiusMultiplier * r;

    *edgeRadius = (R - r) * edgeRadiusRatio + r;
    *arrowBaseRadius = 4 * (*edgeRadius);
    *arrowHeight = 11 * (*edgeRadius);
    *edgeWidth = (maxRadiusMultiplier - 1) * edgeRadiusRatio + 1;
}

GLWidget::~GLWidget()
{
    makeCurrent();
    gluDeleteQuadric (m_quadric);
    m_quadric = 0;
}

void GLWidget::changeViewType (bool checked, ViewType::Enum viewType)
{
    if (checked)
    {
	GetViewSettings ()->SetViewType (viewType);
	compile (GetViewNumber ());
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

G3D::Vector3 GLWidget::getInitialLightPosition (
    LightNumber::Enum lightPosition) const
{
    
    G3D::AABox bb = calculateCenteredViewingVolume (
	double (width ()) / height (), 1.0);
    G3D::Vector3 high = bb.high (), low = bb.low ();
    G3D::Vector3 nearRectangle[] = {
	G3D::Vector3 (high.x, high.y, high.z),
	G3D::Vector3 (low.x, high.y, high.z),
	G3D::Vector3 (low.x, low.y, high.z),
	G3D::Vector3 (high.x, low.y, high.z),
    };
    return nearRectangle[lightPosition];
}


void GLWidget::displayLightDirection (ViewNumber::Enum viewNumber) const
{
    for (size_t i = 0; i < LightNumber::COUNT; ++i)
	displayLightDirection (viewNumber, LightNumber::Enum (i));
}

void GLWidget::displayLightDirection (
    ViewNumber::Enum viewNumber, LightNumber::Enum i) const
{
    const ViewSettings& vs = *GetViewSettings (viewNumber);
    if (vs.IsLightPositionShown (i))
    {
	const double sqrt3 = 1.7321;
	glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT);    
	glPushMatrix ();
	glLoadIdentity ();
	glTranslatef (0, 0, - vs.GetCameraDistance ());
	glMultMatrix (vs.GetRotationLight (i));
	G3D::Vector3 lp = getInitialLightPosition (
	    LightNumber::Enum (i)) / sqrt3;
	::glColor (QColor (vs.IsLightEnabled (i) ? Qt::red : Qt::gray));
	if (vs.IsLightingEnabled ())
	    glDisable (GL_LIGHTING);
	DisplayOrientedSegment () (lp, G3D::Vector3::zero ());
	glPopMatrix ();
	glPopAttrib ();
    }
}

void GLWidget::translateLight (ViewNumber::Enum viewNumber, 
			       const QPoint& position)
{
    ViewSettings& vs = *GetViewSettings (viewNumber);
    G3D::Rect2D viewport = vs.GetViewport ();
    G3D::Vector2 oldPosition = G3D::Vector2 (m_lastPos.x (), m_lastPos.y ());
    G3D::Vector2 newPosition = G3D::Vector2 (position.x (), position.y ());
    G3D::Vector2 viewportCenter = viewport.center ();
    float screenChange =
	((newPosition - viewportCenter).length () -
	 (oldPosition - viewportCenter).length ());
    float ratio = screenChange /
	(viewport.x1y1 () - viewport.x0y0 ()).length ();

    vs.SetLightNumberRatio (
	vs.GetSelectedLight (),
	(1 + ratio) * vs.GetLightNumberRatio (vs.GetSelectedLight ()));
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
    double xOverY, double scaleRatio) const
{
    G3D::AABox bb = GetFoamAlongTime ().GetBoundingBox ();
    G3D::AABox vv = AdjustZ (AdjustXOverYRatio (EncloseRotation (bb), xOverY), 
			     scaleRatio);
    return vv - vv.center ();
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
    if (GetFoamAlongTime ().Is2D () && ! IsTimeDisplacementUsed ())
    {
	G3D::AABox boundingBox = GetFoamAlongTime ().GetBoundingBox ();
	float zTranslation = boundingBox.center ().z - boundingBox.low ().z;
	zTranslation = zTranslation - zTranslation / scaleRatio;
	glTranslatef (0, 0, zTranslation);
    }
    glTranslate (contextView ? (translation / scaleRatio) : translation);
}

void GLWidget::ModelViewTransform (ViewNumber::Enum viewNumber, 
				   size_t timeStep) const
{
    const ViewSettings& vs = *GetViewSettings (viewNumber);
    glLoadIdentity ();
    // viewing transform
    glTranslatef (0, 0, - vs.GetCameraDistance ());
    
    // modeling transform
    if (vs.IsContextView ())
	translateAndScale (vs.GetContextScaleRatio (), G3D::Vector3::zero (),
			   false);
    else
	translateAndScale (vs.GetScaleRatio (), vs.GetTranslation (), 
			   vs.IsContextView ());
    glMultMatrix (vs.GetRotationModel ());
    switch (vs.GetAxesOrder ())
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
    transformFoamStationary (viewNumber, timeStep);
}

void GLWidget::transformFoamStationary (
    ViewNumber::Enum viewNumber, size_t timeStep) const
{
    const ViewSettings& vs = *GetViewSettings (viewNumber);
    ViewSettings::StationaryType type = vs.GetStationaryType ();
    switch (type)
    {
    case ViewSettings::STATIONARY_BODY:
    {
	size_t id = vs.GetStationaryBodyId ();
	G3D::Vector3 centerBegin = GetFoamAlongTime ().GetFoam (
	    vs.GetStationaryTimeStep ())->GetBody (id)->GetCenter ();
	G3D::Vector3 centerCurrent = GetFoamAlongTime ().GetFoam (
	    timeStep)->GetBody (id)->GetCenter ();
	G3D::Vector3 translation = centerBegin - centerCurrent;
	glTranslate (translation);
	glTranslate (-GetFoamAlongTime ().GetBoundingBox ().center ());
	break;
    }
    case ViewSettings::STATIONARY_CONSTRAINT:
    {
	const AffineMap& mapBegin = GetFoamAlongTime ().GetFoam (
	    vs.GetStationaryTimeStep ())->GetAffineMap ();
	const AffineMap& mapCurrent = GetFoamAlongTime ().GetFoam (
	    timeStep)->GetAffineMap ();
	G3D::Vector3 translation = G3D::Vector3 (
	    mapBegin.m_translation, 0.0);
	glTranslate (- GetFoamAlongTime ().GetBoundingBox ().center ());
	glTranslate (translation);
	float angle = 
	    (mapCurrent.m_angle - mapBegin.m_angle) * 180 / M_PI;
	glRotatef (angle, 0, 0, 1);	
	glTranslate (-translation);
	break;
    }
    default:
	glTranslate (- GetFoamAlongTime ().GetBoundingBox ().center ());
	break;
    }
}

G3D::AABox GLWidget::calculateViewingVolume (
    ViewNumber::Enum viewNumber, double xOverY, double scaleRatio) const
{
    const ViewSettings& vs = *GetViewSettings (viewNumber);
    G3D::AABox centeredViewingVolume = 
	calculateCenteredViewingVolume (xOverY, scaleRatio);
    G3D::Vector3 translation (vs.GetCameraDistance () * G3D::Vector3::unitZ ());
    G3D::AABox result = centeredViewingVolume - translation;
    return result;
}


void GLWidget::projectionTransform (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = *GetViewSettings (viewNumber);
    double xOverY = getViewXOverY ();
    G3D::AABox viewingVolume = calculateViewingVolume (
	viewNumber, xOverY, vs.GetScaleRatio ());
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    if (vs.GetAngleOfView () == 0)
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

void GLWidget::viewportTransform (ViewNumber::Enum viewNumber) const
{
    G3D::Rect2D viewRect = GetViewRect (viewNumber);
    ViewSettings& vs = *GetViewSettings (viewNumber);
    vs.SetViewport (viewRect);
    glViewport (vs.GetViewport ());
}

G3D::Rect2D GLWidget::GetViewRect (ViewNumber::Enum view) const
{
    float w = width ();
    float h = height ();
    using G3D::Rect2D;
    switch (m_viewCount)
    {
    case ViewCount::ONE:
	return Rect2D::xywh (0, 0, w, h);
    case ViewCount::TWO:
    {
	RuntimeAssert (view < 2, "Invalid view: ", view);
	Rect2D v[] = {
	    // 0 | 1
	    // horizontal layout
	    Rect2D::xywh (0, 0, w/2, h), Rect2D::xywh (w/2, 0, w/2, h),
	    // 0
	    // -
	    // 1
	    // vertical layout
	    Rect2D::xywh (0, h/2, w, h/2), Rect2D::xywh (0, 0, w, h/2)
	};
	return v[2 * m_viewLayout + view];
    }
    case ViewCount::FOUR:
    {
	//  0 | 1
	//  -----
	//  2 | 3
	RuntimeAssert (view < 4, "Invalid view: ", view);
	Rect2D v[] = {
	    Rect2D::xywh (0, h/2, w/2, h/2), Rect2D::xywh (w/2, h/2, w/2, h/2),
	    Rect2D::xywh (0, 0, w/2, h/2), Rect2D::xywh (w/2, 0, w/2, h/2)
	};
	return v[view];
    }
    default:
    {
	RuntimeAssert (false, "Illegal number of views: ", m_viewCount);
	return Rect2D ();	
    }
    }
}

void GLWidget::setView (const G3D::Vector2& clickedPoint)
{
    for (size_t i = 0; i < ViewCount::GetCount (m_viewCount); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	G3D::Rect2D viewRect = GetViewRect (viewNumber);
	if (viewRect.contains (clickedPoint))
	{
	    m_viewNumber = viewNumber;
	    break;
	}
    }
    setLabel ();
    Q_EMIT ViewChanged ();
}

G3D::Rect2D GLWidget::getViewColorBarRect (const G3D::Rect2D& viewRect)
{
    return G3D::Rect2D::xywh (
	viewRect.x0 () + 5, viewRect.y0 () + 5,
	10, max (viewRect.height () / 4, 50.0f));
}

double GLWidget::getViewXOverY () const
{
    double xOverY =  double (width ()) / height ();
    double v[] = { 
	xOverY, xOverY,     // ONE
	xOverY/2, 2*xOverY, // TWO (HORIZONTAL, VERTICAL)
	xOverY, xOverY      // FOUR
    };
    return v[m_viewCount * 2 + m_viewLayout];
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


void GLWidget::ResetTransformation ()
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = *GetViewSettings ();
    vs.SetRotationModel (G3D::Matrix3::identity ());
    vs.SetScaleRatio (1);
    vs.SetContextScaleRatio (1);
    vs.SetTranslation (G3D::Vector3::zero ());
    projectionTransform (viewNumber);
    update ();
}

void GLWidget::ResetSelectedLightNumber ()
{
    ViewSettings& vs = *GetViewSettings ();
    LightNumber::Enum lightNumber = vs.GetSelectedLight ();
    vs.SetInitialLightPosition (lightNumber);
    vs.PositionLight (lightNumber, getInitialLightPosition (lightNumber));
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
	G3D::Vector3 op = brushedBodies (m_contextMenuPos, &bodies);
	if (bodies.size () == 0)
	    ostr << "Point: " << op;
	else
	{
	    Foam::Bodies::const_iterator it = 
		GetCurrentFoam ().FindBody (bodies[0]);
	    ostr << *it;
	}
	break;
    }

    case BodySelectorType::ID:
    {
	const vector<size_t>& ids = 
	    (boost::static_pointer_cast<IdBodySelector> (
		m_bodySelector))->GetIds ();
	if (ids.size () == 1)
	{
	    Foam::Bodies::const_iterator it = GetCurrentFoam ().FindBody (
		ids[0]);
	    ostr << *it;
	}
	else
	{
	    ostr << "Selected ids: ";
	    ostream_iterator<size_t> out (ostr, " ");
	    copy (ids.begin (), ids.end (), out);
	    if (GetViewSettings ()->GetBodyProperty () != 
		BodyProperty::NONE)
	    {
		ostr << endl;
		
	    }
	}
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

void GLWidget::ColorBarEdit ()
{
    Q_EMIT EditColorMap ();
}

void GLWidget::ColorBarClampClear ()
{
    boost::shared_ptr<ViewSettings> vs = GetViewSettings ();
    boost::shared_ptr<ColorBarModel> colorBarModel = vs->GetColorBarModel ();
    colorBarModel->SetClampClear ();
    Q_EMIT ColorBarModelChanged (colorBarModel);
}



// Uses antialiased points and lines
// See OpenGL Programming Guide, 7th edition, Chapter 6: Blending,
// Antialiasing, Fog and Polygon Offset page 293
void GLWidget::initializeGL()
{
    initializeGLFunctions ();
    glClearColor (Qt::white);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    DisplayFaceStatistics::InitShaders ();
    initializeLighting ();
    detectOpenGLError ("initializeGl");
}

void GLWidget::paintGL ()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    displayViews ();
    Q_EMIT PaintedGL ();
}

void GLWidget::displayViews ()
{
    switch (m_viewCount)
    {
    case ViewCount::FOUR:
	displayView (ViewNumber::VIEW2);
	displayView (ViewNumber::VIEW3);
    case ViewCount::TWO:
	displayView (ViewNumber::VIEW1);
    case ViewCount::ONE:
	displayView (ViewNumber::VIEW0);
    }
}

void GLWidget::displayView (ViewNumber::Enum viewNumber)
{
    ViewSettings& vs = *GetViewSettings (viewNumber);
    vs.SetLightingParameters (getInitialLightPosition (vs.GetSelectedLight ()));
    viewportTransform (viewNumber);    
    projectionTransform (viewNumber);
    ModelViewTransform (viewNumber, GetTimeStep ());
    if (m_minimumEdgeRadius == 0)
    {
	m_minimumEdgeRadius = getMinimumEdgeRadius ();
	calculateEdgeRadius (m_edgeRadiusRatio,
			     &m_edgeRadius, &m_arrowBaseRadius,
			     &m_arrowHeight, &m_edgeWidth);
    }

    if (! m_hideContent)
    {
	DisplayViewType (viewNumber);
	displayViewDecorations (viewNumber);
    }
    displayAxes ();
    displayBoundingBox (viewNumber);
    displayOriginalDomain ();
    displayFocusBox (viewNumber);
    displayLightDirection (viewNumber);
    displayT1s (viewNumber);
    detectOpenGLError ("displayView");
}


void GLWidget::resizeGL(int w, int h)
{
    (void)w;(void)h;
    for (size_t i = 0; i < ViewCount::GetCount (m_viewCount); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	ViewSettings& vs = *GetViewSettings (viewNumber);
	if (vs.GetViewType () == ViewType::FACES_STATISTICS)
	{
	    pair<double, double> minMax = getStatisticsMinMax (viewNumber);
	    vs.GetDisplayFaceStatistics ()->InitStep (
		viewNumber, minMax.first, minMax.second);
	}
    }
    detectOpenGLError ("resizeGl");
}

// Based on OpenGL FAQ, 9.090 How do I draw a full-screen quad?
void GLWidget::RenderFromFbo (G3D::Rect2D destRect,
			      QGLFramebufferObject& fbo) const
{
    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, fbo.texture ());
    glPushAttrib (GL_VIEWPORT_BIT);
    glViewport (destRect.x0 (), destRect.y0 (),
		destRect.width (), destRect.height ());

    //glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glLoadIdentity ();
    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity ();
    glBegin (GL_QUADS);
    glTexCoord2i (0, 0);glVertex3i (-1, -1, -1);
    glTexCoord2i (1, 0);glVertex3i (1, -1, -1);
    glTexCoord2i (1, 1);glVertex3i (1, 1, -1);
    glTexCoord2i (0, 1);glVertex3i (-1, 1, -1);
    glEnd ();
    glPopMatrix ();
    glMatrixMode (GL_MODELVIEW);
    glPopMatrix ();

    glPopAttrib ();
    glDisable (GL_TEXTURE_2D);
}

G3D::Matrix3 GLWidget::getRotationAround (int axis, double angleRadians)
{
    using G3D::Matrix3;using G3D::Vector3;
    Vector3 axes[3] = {
	Vector3::unitX (), Vector3::unitY (), Vector3::unitZ ()
    };
    return Matrix3::fromAxisAngle (axes[axis], angleRadians);
}

double GLWidget::ratioFromCenter (const QPoint& p)
{
    using G3D::Vector2;
    G3D::Rect2D viewRect = GetViewRect ();
    Vector2 center = viewRect.center ();
    int windowHeight = height ();
    Vector2 lastPos = QtToOpenGl (m_lastPos, windowHeight);
    Vector2 currentPos = QtToOpenGl (p, windowHeight);
    double ratio =
	(currentPos - center).length () / (lastPos - center).length ();
    return ratio;
}

G3D::Matrix3 GLWidget::rotate (ViewNumber::Enum viewNumber,
			       const QPoint& position, const G3D::Matrix3& r)
{
    G3D::Matrix3 rotate = r;
    const G3D::Rect2D& viewport = GetViewSettings (viewNumber)->GetViewport ();
    int dx = position.x() - m_lastPos.x();
    int dy = position.y() - m_lastPos.y();

    // scale this with the size of the window
    int side = std::min (viewport.width (), viewport.height ());
    double dxRadians = static_cast<double>(dx) * (M_PI / 2) / side;
    double dyRadians = static_cast<double>(dy) * (M_PI / 2) / side;
    rotate = getRotationAround (0, dyRadians) * rotate;
    rotate = getRotationAround (1, dxRadians) * rotate;
    return rotate;
}

void GLWidget::translate (
    ViewNumber::Enum viewNumber, const QPoint& position,
    G3D::Vector3::Axis screenXTranslation,
    G3D::Vector3::Axis screenYTranslation)
{
    ViewSettings& vs = *GetViewSettings (viewNumber);
    G3D::Vector3 translationRatio;
    translationRatio[screenXTranslation] =
	static_cast<double>(position.x() - m_lastPos.x()) /
	vs.GetViewport ().width ();
    translationRatio[screenYTranslation] =
	- static_cast<double> (position.y() - m_lastPos.y()) / 
	vs.GetViewport ().height ();


    G3D::AABox vv = calculateCenteredViewingVolume (
	double (width ()) / height (), vs.GetScaleRatio ());
    G3D::Vector3 focusBoxExtent = vv.extent () / vs.GetScaleRatio ();
    if (vs.IsContextView ())
	vs.SetTranslation (
	    vs.GetTranslation () - (translationRatio * focusBoxExtent));
    else
	vs.SetTranslation (
	    vs.GetTranslation () + (translationRatio * focusBoxExtent));
}


void GLWidget::scale (ViewNumber::Enum viewNumber, const QPoint& position)
{
    ViewSettings& vs = *GetViewSettings (viewNumber);
    double ratio = ratioFromCenter (position);
    if (vs.IsContextView ())
	vs.SetScaleRatio (vs.GetScaleRatio () / ratio);
    else
	vs.SetScaleRatio (vs.GetScaleRatio () * ratio);
}

void GLWidget::scaleContext (
    ViewNumber::Enum viewNumber, const QPoint& position)
{
    ViewSettings& vs = *GetViewSettings (viewNumber);
    double ratio = ratioFromCenter (position);
    vs.SetContextScaleRatio (vs.GetContextScaleRatio () * ratio);
}


G3D::Vector3 GLWidget::brushedBodies (
    const QPoint& position, vector<size_t>* bodies) const
{
    G3D::Vector3 op = objectPosition (position);
    const Foam& foam = GetCurrentFoam ();
    BOOST_FOREACH (boost::shared_ptr<Body> body, foam.GetBodies ())
    {
	G3D::AABox box = body->GetBoundingBox ();
	if (box.contains (op))
	    bodies->push_back (body->GetId ());
    }
    return op;
}

G3D::Vector3 GLWidget::objectPosition (const QPoint& position) const
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    viewportTransform (viewNumber);    
    projectionTransform (viewNumber);
    ModelViewTransform (viewNumber, GetTimeStep ());

    G3D::Vector3 op = gluUnProject (
	QtToOpenGl (position, height ()));
    if (GetFoamAlongTime ().Is2D ())
	op.z = 0;
    return op;
}

void GLWidget::displayBodyStationaryContour (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = *GetViewSettings (viewNumber);
    if (vs.GetStationaryType () == ViewSettings::STATIONARY_BODY)
    {
	Foam::Bodies focusBody (1);
	focusBody[0] = *GetCurrentFoam ().FindBody (vs.GetStationaryBodyId ());
	displayFacesContour<HighlightNumber::H0> (focusBody, viewNumber, HIGHLIGHT_LINE_WIDTH);
    }
}

void GLWidget::displayBodyContextContour (ViewNumber::Enum viewNumber) const
{
    boost::shared_ptr<ViewSettings> vs = GetViewSettings (viewNumber);
    if (vs->GetBodyContextSize () > 0)
    {
	const Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
	Foam::Bodies contextBodies (bodies.size ());
	
	Foam::Bodies::const_iterator end = remove_copy_if (
	    bodies.begin (), bodies.end (), contextBodies.begin (),
	    ! boost::bind (&ViewSettings::IsBodyContext, vs, 
			   boost::bind (&Body::GetId, _1)));
	contextBodies.resize (end - contextBodies.begin ());
	displayFacesContour<HighlightNumber::H1> (
	    contextBodies, viewNumber, HIGHLIGHT_LINE_WIDTH);
    }
}


string GLWidget::getStationaryLabel ()
{
    ostringstream ostr;
    const ViewSettings& vs = *GetViewSettings ();
    ViewSettings::StationaryType type = vs.GetStationaryType ();
    switch (type)
    {
    case ViewSettings::STATIONARY_BODY:
	ostr << "Stationary body";
	break;
    case ViewSettings::STATIONARY_CONSTRAINT:
	ostr << "Stationary constraint";
	break;
    default:
	break;
    }
    return ostr.str ();
}

string GLWidget::getContextLabel ()
{
    ostringstream ostr;
    const ViewSettings& vs = *GetViewSettings ();
    size_t count = vs.GetBodyContextSize ();
    if (count != 0)
	ostr << "Context (" << count << ")";
    return ostr.str ();
}


string GLWidget::getBodySelectorLabel ()
{
    BodySelectorType::Enum type = m_bodySelector->GetType ();
    switch (type)
    {
    case BodySelectorType::PROPERTY_VALUE:
	return "Selection: by property";
	break;
    case BodySelectorType::ID:
	return "Selection: by id";
	break;
    case BodySelectorType::COMPOSITE:
	return "Selection: by id and property";
	break;
    default:
	return "";
    }
}

void GLWidget::setLabel ()
{
    ostringstream ostr;
    boost::array<string, 3> labels = {{
	    getStationaryLabel (),
	    getContextLabel (),
	    getBodySelectorLabel ()
	}};
    ostream_iterator<string> o (ostr, " ");
    remove_copy_if (labels.begin (), labels.end (), o,
		    boost::bind (&string::empty, _1));
    string label = ostr.str ();
    if (label.empty ())
	label = "Ready";
    m_labelStatusBar->setText (QString (label.c_str ()));
}


void GLWidget::StationaryBody ()
{
    ViewSettings& vs = *GetViewSettings ();
    vector<size_t> bodies;
    brushedBodies (m_contextMenuPos, &bodies);
    if (bodies.size () != 0)
    {
	vs.SetStationaryType (ViewSettings::STATIONARY_BODY);
	vs.SetStationaryBodyId (bodies[0]);
	vs.SetStationaryTimeStep (m_timeStep);
	setLabel ();
	update ();
    }
    else
    {
	QMessageBox msgBox (this);
	msgBox.setText("No body selected");
	msgBox.exec();
    }
}

void GLWidget::StationaryConstraint ()
{
    ViewSettings& vs = *GetViewSettings ();
    if (GetFoamAlongTime ().AffineMapNamesUsed ())
    {
	vs.SetStationaryType (ViewSettings::STATIONARY_CONSTRAINT);
	vs.SetStationaryBodyId (INVALID_INDEX);
	vs.SetStationaryTimeStep (m_timeStep);
	setLabel ();
	update ();
    }
    else
    {
	QMessageBox msgBox (this);
	msgBox.setText("No affine map read for a foam object: "
		       "use --affine-map command line option.");
	msgBox.exec();	
    }
}

void GLWidget::StationaryReset ()
{
    ViewSettings& vs = *GetViewSettings ();
    vs.SetStationaryType (ViewSettings::STATIONARY_NONE);
    vs.SetStationaryBodyId (INVALID_INDEX);
    vs.SetStationaryTimeStep (0);
    setLabel ();
    update ();
}

void GLWidget::ContextReset ()
{
    ViewSettings& vs = *GetViewSettings ();
    vs.ClearBodyContext ();
    setLabel ();
    update ();
}


void GLWidget::ContextBody ()
{
    ViewSettings& vs = *GetViewSettings ();
    vector<size_t> bodies;
    brushedBodies (m_contextMenuPos, &bodies);
    vs.AddBodyContext (bodies[0]);
    setLabel ();
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
    {
	ViewSettings& vs = *GetViewSettings ();
	vs.SetRotationModel (
	    rotate (GetViewNumber (), event->pos (), vs.GetRotationModel ()));
	break;
    }
    case InteractionMode::TRANSLATE:
	if (event->modifiers () & Qt::ControlModifier)
	{
	    QPoint point (m_lastPos.x (), event->pos ().y ());
	    translate (GetViewNumber (), 
		       point, G3D::Vector3::X_AXIS, G3D::Vector3::Z_AXIS);
	}
	else
	    translate (GetViewNumber (), event->pos (), G3D::Vector3::X_AXIS,
		       G3D::Vector3::Y_AXIS);
	break;
    case InteractionMode::SCALE:
	if (event->modifiers () & Qt::ControlModifier)
	    scaleContext (GetViewNumber (), event->pos ());
	else
	    scale (GetViewNumber (), event->pos ());
	break;

    case InteractionMode::ROTATE_LIGHT:
    {
	ViewSettings& vs = *GetViewSettings ();
	LightNumber::Enum i = vs.GetSelectedLight ();
	vs.SetRotationLight (
	    i, 
	    rotate (GetViewNumber (), event->pos (), vs.GetRotationLight (i)));
	vs.PositionLight (i, getInitialLightPosition (i));
	break;
    }
    case InteractionMode::TRANSLATE_LIGHT:
    {
	ViewSettings& vs = *GetViewSettings ();
	LightNumber::Enum i = vs.GetSelectedLight ();
	vs.PositionLight (i, getInitialLightPosition (i));
	translateLight (GetViewNumber (), event->pos ());
	break;
    }
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
    G3D::Vector2 p = QtToOpenGl (event->pos (), height ());
    setView (p);
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
void GLWidget::displayFocusBox (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = *GetViewSettings (viewNumber);
    if (vs.IsContextView ())
    {
	G3D::Rect2D viewRect = GetViewRect (viewNumber);

	glPushMatrix ();
	glLoadIdentity ();
	glTranslatef (0, 0, - vs.GetCameraDistance ());

	G3D::AABox focusBox = calculateCenteredViewingVolume (
	    double (viewRect.width ()) / viewRect.height (), 
	    vs.GetScaleRatio ());
	translateAndScale ( 1 / vs.GetScaleRatio (), 
			    - vs.GetContextScaleRatio () * 
			    vs.GetTranslation (), true);
	glScale (vs.GetContextScaleRatio ());
	DisplayBox (focusBox, GetHighlightColor (
			viewNumber, HighlightNumber::H0), 
		    HIGHLIGHT_LINE_WIDTH);
	glPopMatrix ();
    }
}

void GLWidget::displayBoundingBox (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = *GetViewSettings (viewNumber);
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT);
    if (vs.IsLightingEnabled ())
	glDisable (GL_LIGHTING);
    if (m_boundingBoxShown)
	DisplayBox (GetFoamAlongTime ().GetBoundingBox (), Qt::black);
    if (m_bodiesBoundingBoxesShown)
    {
	const Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
	for_each (
	    bodies.begin (), bodies.end (),
	    boost::bind (
		DisplayBox< boost::shared_ptr<Body> >, _1, Qt::black, 1.0));
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

	DisplayOrientedSegmentQuadric displayOrientedEdge (
	    GetQuadricObject (), m_arrowBaseRadius, m_edgeRadius, m_arrowHeight,
	    DisplaySegmentArrow::TOP_END);

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
	      DisplayFaceHighlightColor<HighlightNumber::H0,
	      DisplayFaceEdges<displayEdge> > > (*this, *m_bodySelector));
    displayStandaloneEdges<displayEdge> ();

    glPopAttrib ();
    displayBodyCenters ();
}

template<typename displayEdge>
void GLWidget::displayStandaloneEdges (bool useZPos, double zPos) const
{
    if (m_standaloneElementsShown)
    {
	glPushAttrib (GL_ENABLE_BIT);    
	glDisable (GL_DEPTH_TEST);
	const Foam::Edges& standaloneEdges =
	    GetCurrentFoam ().GetStandaloneEdges ();
	BOOST_FOREACH (boost::shared_ptr<Edge> edge, standaloneEdges)
	    displayEdge (*this, DisplayElement::FOCUS, useZPos, zPos) (edge);
	glPopAttrib ();
    }
}

void GLWidget::displayConstraintEdges (ViewNumber::Enum view) const
{
    const ViewSettings& vs = *GetViewSettings (view);
    ViewSettings::StationaryType type = vs.GetStationaryType ();
    if (type == ViewSettings::STATIONARY_CONSTRAINT)
    {
	glPushAttrib (GL_ENABLE_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
	glDisable (GL_DEPTH_TEST);
	glLineWidth (HIGHLIGHT_LINE_WIDTH);
	glColor (GetHighlightColor (view, HighlightNumber::H0));
	const Foam::Edges& constraintEdges = 
	    GetCurrentFoam ().GetConstraintEdges (
		GetFoamAlongTime ().GetAffineMapNames ().m_constraintIndex);
	DisplayEdgeHighlightColor<HighlightNumber::H0> displayEdge (
	    *this, DisplayElement::FOCUS, view);
	BOOST_FOREACH (boost::shared_ptr<Edge> edge, constraintEdges)
	    displayEdge (edge);
	glPopAttrib ();
    }
}


void GLWidget::displayEdgesNormal (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = *GetViewSettings (viewNumber);    
    glPushAttrib (GL_ENABLE_BIT);
    if (vs.IsLightingEnabled ())
	glDisable (GL_LIGHTING);
    m_torusOriginalDomainClipped ?
	displayEdges <DisplayEdgeTorusClipped> () :
	displayEdges <DisplayEdgePropertyColor<> >();
    glPopAttrib ();
}

void GLWidget::displayT1s (ViewNumber::Enum view) const
{
    if (m_t1sShown)
    {
	if (ViewType::IsGlobal (GetViewSettings ()->GetViewType ()))
	    displayT1sGlobal (view);
	else
	    displayT1s (view, GetTimeStep ());
    }
}


void GLWidget::displayT1sGlobal (ViewNumber::Enum view) const
{
    for (size_t i = 0; i < GetFoamAlongTime ().GetTimeSteps (); ++i)
	displayT1s (view, i);
}

void GLWidget::displayT1s (ViewNumber::Enum view, size_t timeStep) const
{
    glPushAttrib (GL_ENABLE_BIT | GL_POINT_BIT | GL_CURRENT_BIT);
    glDisable (GL_DEPTH_TEST);
    glPointSize (m_t1Size);
    glColor (GetHighlightColor (view, HighlightNumber::H0));
    glBegin (GL_POINTS);
    BOOST_FOREACH (const G3D::Vector3 v, 
		   GetFoamAlongTime ().GetT1s (timeStep))
	::glVertex (v);
    glEnd ();
    glPopAttrib ();
}

QColor GLWidget::GetHighlightColor (
    ViewNumber::Enum viewNumber, HighlightNumber::Enum highlight) const
{
    boost::shared_ptr<ColorBarModel> colorBarModel = 
	GetViewSettings (viewNumber)->GetColorBarModel ();
    if (colorBarModel)
	return colorBarModel->GetHighlightColor (highlight);
    else
    {
	if (highlight == HighlightNumber::H0)
	    return Qt::black;
	else
	    return Qt::red;
    }
}

void GLWidget::displayEdgesTorus (ViewNumber::Enum view) const
{
    (void)view;
    if (m_edgeRadiusRatio > 0)
	displayEdgesTorusTubes ();
    else
    {
	displayEdgesTorusLines ();
	displayBodyCenters ();
    }
}

void GLWidget::displayFacesTorus (ViewNumber::Enum view) const
{
    (void)view;
    if (m_edgeRadiusRatio > 0)
	displayFacesTorusTubes ();
    else
    {
	displayFacesTorusLines ();
	displayBodyCenters ();
    }
    displayStandaloneEdges< DisplayEdgePropertyColor<> > ();
}


void GLWidget::displayEdgesTorusTubes () const
{
    glPushAttrib (GL_POLYGON_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    EdgeSet edgeSet;
    GetCurrentFoam ().GetEdgeSet (&edgeSet);
    for_each (
	edgeSet.begin (), edgeSet.end (),
	DisplayEdgeTorus<DisplaySegmentQuadric, DisplaySegmentArrowQuadric, false>(*this));
    glPopAttrib ();
}

void GLWidget::displayEdgesTorusLines () const
{
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);

    EdgeSet edgeSet;
    GetCurrentFoam ().GetEdgeSet (&edgeSet);
    for_each (edgeSet.begin (), edgeSet.end (),
	      DisplayEdgeTorus<DisplaySegment, DisplaySegmentArrow, false> (*this));
    glPopAttrib ();
}


void GLWidget::displayBodyCenters (bool useZPos) const
{
    if (m_bodyCenterShown)
    {
	double zPos = (GetViewSettings ()->GetViewType () == 
		       ViewType::CENTER_PATHS) ?
	    GetTimeStep () * GetTimeDisplacement () : 0;
	glPushAttrib (GL_POINT_BIT | GL_CURRENT_BIT);
	glPointSize (4.0);
	glColor (Qt::red);
	const Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
	for_each (bodies.begin (), bodies.end (),
		  DisplayBodyCenter (*this, *m_bodySelector, useZPos, zPos));
	/*
	glPointSize (8.0);
	glBegin (GL_POINTS);
	Foam::Bodies::const_iterator it = GetCurrentFoam ().FindBody (357);
	Face& f = *(*it)->GetFace (0);
	f.CalculateCenter (true);
	glEnd ();
	*/
	glPopAttrib ();
    }
}

void GLWidget::displayFacesNormal (ViewNumber::Enum view) const
{
    const Foam& foam = GetCurrentFoam ();
    const Foam::Bodies& bodies = foam.GetBodies ();
    if (m_facesShowEdges)
	displayFacesContour (bodies);
    displayFacesInterior (bodies, view);
    displayStandaloneEdges< DisplayEdgePropertyColor<> > ();
    displayConstraintEdges (view);
    displayStandaloneFaces ();    
    displayBodyCenters ();
}

pair<double, double> GLWidget::getStatisticsMinMax (ViewNumber::Enum view) const
{
    double minValue, maxValue;
    if (GetViewSettings (view)->GetStatisticsType () == StatisticsType::COUNT)
    {
	minValue = 0;
	maxValue = GetFoamAlongTime ().GetTimeSteps ();
    }
    else
    {
	minValue = GetFoamAlongTime ().GetMin (
	    GetViewSettings (view)->GetBodyProperty ());
	maxValue = GetFoamAlongTime ().GetMax (
	    GetViewSettings (view)->GetBodyProperty ());
    }
    return pair<double, double> (minValue, maxValue);
}


void GLWidget::displayFacesStatistics (ViewNumber::Enum viewNumber) const
{
    boost::shared_ptr<ViewSettings> view = GetViewSettings (viewNumber);
    glPushAttrib (GL_ENABLE_BIT);    
    glDisable (GL_DEPTH_TEST);
    glBindTexture (GL_TEXTURE_1D, 
		   GetViewSettings (viewNumber)->GetColorBarTexture ());
    pair<double, double> minMax = getStatisticsMinMax (viewNumber);
    view->GetDisplayFaceStatistics ()->Display (
	GetViewRect (viewNumber),
	minMax.first, minMax.second, view->GetStatisticsType ());
    displayStandaloneEdges< DisplayEdgePropertyColor<> > ();
    displayBodyStationaryContour (viewNumber);
    displayBodyContextContour (viewNumber);
    glPopAttrib ();
}

void GLWidget::displayStandaloneFaces () const
{
    if (m_standaloneElementsShown)
    {
	const Foam::Faces& faces = GetCurrentFoam ().GetStandaloneFaces ();
	displayFacesContour (faces);
	displayFacesInterior (faces);
    }
}

void GLWidget::displayFacesContour (const Foam::Faces& faces) const
{
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT);
    for_each (faces.begin (), faces.end (),
	      DisplayFaceColor<0xff000000, DisplayFaceLineStrip> (*this));
    glPopAttrib ();
}

void GLWidget::displayFacesContour (const Foam::Bodies& bodies) const
{
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT);
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody< DisplayFaceColor<0xff000000, 
	      DisplayFaceLineStrip> > (
		  *this, *m_bodySelector));
    glPopAttrib ();
}


template<HighlightNumber::Enum highlightColorIndex>
void GLWidget::displayFacesContour (
    const Foam::Bodies& bodies, ViewNumber::Enum viewNumber, 
    GLfloat lineWidth) const
{
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LINE_BIT);
    glLineWidth (lineWidth);
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody< DisplayFaceHighlightColor<highlightColorIndex, 
	      DisplayFaceLineStrip>,
	      SetterValueTextureCoordinate> (
		  *this, *m_bodySelector, SetterValueTextureCoordinate (
		      *this, viewNumber)));
    glPopAttrib ();
}


// See OpenGL Programming Guide, 7th edition, Chapter 6: Blending,
// Antialiasing, Fog and Polygon Offset page 293
void GLWidget::displayFacesInterior (
    const Foam::Bodies& bodies, ViewNumber::Enum view) const
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
    glBindTexture (GL_TEXTURE_1D, 
		   GetViewSettings (view)->GetColorBarTexture ());
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<DisplayFaceBodyPropertyColor<
	      DisplayFaceTriangleFan> > (
		  *this, *m_bodySelector, 
		  DisplayElement::TRANSPARENT_CONTEXT, view));
    glPopAttrib ();
}

void GLWidget::displayFacesInterior (const Foam::Faces& faces) const
{
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1, 1);
    for_each (faces.begin (), faces.end (),
	      DisplayFaceBodyPropertyColor<DisplayFaceTriangleFan> (*this));
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
	DisplayFaceHighlightColor<HighlightNumber::H0, DisplayFaceEdges<
	DisplayEdgeTorus<DisplaySegmentQuadric, DisplaySegmentArrowQuadric, true> > > (
	    *this));
    glPopAttrib ();
}


void GLWidget::displayFacesTorusLines () const
{
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);

    FaceSet faceSet;
    GetCurrentFoam ().GetFaceSet (&faceSet);
    for_each (faceSet.begin (), faceSet.end (),
	      DisplayFaceHighlightColor<HighlightNumber::H0,
	      DisplayFaceEdges<
	      DisplayEdgeTorus<DisplaySegment, DisplaySegmentArrow, true> > > (
		  *this, DisplayElement::FOCUS) );
    glPopAttrib ();
}

void GLWidget::displayCenterPathsWithBodies (ViewNumber::Enum view) const
{
    const ViewSettings& vs = *GetViewSettings (view);
    glLineWidth (1.0);
    displayCenterPaths (view);
    
    glPushAttrib (GL_ENABLE_BIT);
    if (vs.IsLightingEnabled ())
	glDisable (GL_LIGHTING);
    if (IsCenterPathBodyShown ())
    {
	const Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
	double zPos = GetTimeStep () * GetTimeDisplacement ();
	for_each (
	    bodies.begin (), bodies.end (),
	    DisplayBody<DisplayFaceHighlightColor<HighlightNumber::H0,
	    DisplayFaceEdges<DisplayEdgePropertyColor<
	    DisplayElement::DONT_DISPLAY_TESSELLATION> > > > (
		*this, *m_bodySelector, DisplayElement::INVISIBLE_CONTEXT,
		view, IsTimeDisplacementUsed (), zPos));
	displayBodyCenters (IsTimeDisplacementUsed ());
    }
    displayStandaloneEdges< DisplayEdgePropertyColor<> > (true, 0);
    if (GetTimeDisplacement () != 0)
    {

	displayStandaloneEdges< DisplayEdgePropertyColor<> > (
	    IsTimeDisplacementUsed (),
	    (GetFoamAlongTime ().GetTimeSteps () - 1)*GetTimeDisplacement ());
    }
    glPopAttrib ();
}

void GLWidget::displayCenterPaths (ViewNumber::Enum view) const
{
    glCallList (GetViewSettings (view)->GetListCenterPaths ());
}

void GLWidget::compile (ViewNumber::Enum view) const
{
    switch (GetViewSettings (view)->GetViewType ())
    {
    case ViewType::CENTER_PATHS:
	compileCenterPaths (view);
	break;
    default:
	break;
    }    
}

void GLWidget::compileCenterPaths (ViewNumber::Enum view) const
{
    glNewList (GetViewSettings (view)->GetListCenterPaths (), GL_COMPILE);
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT | 
		  GL_POLYGON_BIT | GL_LINE_BIT);
    glEnable(GL_TEXTURE_1D);
    glBindTexture (GL_TEXTURE_1D, GetViewSettings (view)->GetColorBarTexture ());
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_CULL_FACE);

    //See OpenGL FAQ 21.030 Why doesn't lighting work when I turn on 
    //texture mapping?
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    const BodiesAlongTime::BodyMap& bats = GetBodiesAlongTime ().GetBodyMap ();
    if (m_edgeRadiusRatio > 0 && ! m_centerPathLineUsed)
    {
	if (m_centerPathTubeUsed)
	    for_each (
		bats.begin (), bats.end (),
		DisplayCenterPath<
		SetterValueTextureCoordinate, DisplaySegmentTube> (
		    *this, m_viewNumber, *m_bodySelector,
		    IsTimeDisplacementUsed (), GetTimeDisplacement ()));
	else
	    for_each (
		bats.begin (), bats.end (),
		DisplayCenterPath<
		SetterValueTextureCoordinate, DisplaySegmentQuadric> (
		    *this, m_viewNumber, *m_bodySelector,
		    IsTimeDisplacementUsed (), GetTimeDisplacement ()));
    }
    else
	for_each (bats.begin (), bats.end (),
		  DisplayCenterPath<SetterValueTextureCoordinate, 
		  DisplaySegment> (
		      *this, m_viewNumber, *m_bodySelector,
		      IsTimeDisplacementUsed (), GetTimeDisplacement ()));
    glPopAttrib ();
    glEndList ();
}

void GLWidget::DisplayViewType (ViewNumber::Enum view) const
{
    (this->*(m_viewTypeDisplay[GetViewSettings (view)->GetViewType ()])) (view);
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
	return face->GetOrientedEdge (m_selectedEdgeIndex)->GetEdge ();
    }
    RuntimeAssert (false, "There is no displayed edge");
    return boost::shared_ptr<Edge>();
}

size_t GLWidget::GetSelectedEdgeId () const
{
    return GetSelectedEdge ()->GetId ();
}

void GLWidget::setLight (int sliderValue, int maximumValue, 
			 LightType::Enum lightType, 
			 ColorNumber::Enum colorNumber)
{
    ViewSettings& vs = *GetViewSettings ();
    LightNumber::Enum selectedLight = vs.GetSelectedLight ();
    vs.SetLight (selectedLight, lightType, colorNumber,  
		 double(sliderValue) / maximumValue);
    boost::array<GLfloat, 4> lightAmbient = vs.GetLight (
	selectedLight, LightType::AMBIENT);
    glLightfv(GL_LIGHT0 + selectedLight, GL_AMBIENT, &lightAmbient[0]);
    update ();
}

bool GLWidget::isColorBarUsed (ViewNumber::Enum view) const
{
    boost::shared_ptr<ViewSettings> vs = GetViewSettings (view);
    switch (vs->GetViewType ())
    {
    case ViewType::FACES:
    case ViewType::FACES_STATISTICS:
    case ViewType::CENTER_PATHS:
	return vs->GetBodyProperty () != BodyProperty::NONE;
    default:
	return false;
    }
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
    QMenu menuCopy ("Copy", this);
    G3D::Rect2D colorBarRect = getViewColorBarRect (GetViewRect ());
    if (colorBarRect.contains (QtToOpenGl (m_contextMenuPos, height ())))
    {
	bool actions = false;
	if (ViewCount::GetCount (m_viewCount) > 1)
	{
	    BodyProperty::Enum currentBodyProperty = 
		GetViewSettings ()->GetBodyProperty ();
	    for (size_t i = 0; i < ViewCount::GetCount (m_viewCount); ++i)
	    {
		ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
		if (viewNumber == m_viewNumber ||
		    currentBodyProperty != 
		    GetViewSettings (viewNumber)->GetBodyProperty ())
		    continue;
		menuCopy.addAction (m_actionCopyColorBar[i].get ());
		actions = true;
	    }
	}
	menu.addAction (m_actionEditColorMap.get ());
	menu.addAction (m_actionClampClear.get ());
	if (actions)
	    menu.addMenu (&menuCopy);
    }
    else
    {
	{
	    QMenu* menuReset = menu.addMenu ("Reset");
	    menuReset->addAction (m_actionResetTransformation.get ());
	    menuReset->addAction (m_actionResetSelectedLightNumber.get ());
	}
	{
	    QMenu* menuSelect = menu.addMenu ("Select");
	    menuSelect->addAction (m_actionSelectAll.get ());
	    menuSelect->addAction (m_actionDeselectAll.get ());
	    menuSelect->addAction (m_actionSelectBodiesById.get ());
	}
	{
	    QMenu* menuStationary = menu.addMenu ("Stationary");
	    menuStationary->addAction (m_actionStationaryBody.get ());
	    menuStationary->addAction (m_actionStationaryConstraint.get ());
	    menuStationary->addAction (m_actionStationaryReset.get ());
	}
	{
	    QMenu* menuContext = menu.addMenu ("Context");
	    menuContext->addAction (m_actionContextBody.get ());
	    menuContext->addAction (m_actionContextReset.get ());
	}
	{
	    QMenu* menuInfo = menu.addMenu ("Info");
	    menuInfo->addAction (m_actionInfoFocus.get ());
	    menuInfo->addAction (m_actionInfoFoam.get ());
	    menuInfo->addAction (m_actionInfoOpenGL.get ());
	}
	if (ViewCount::GetCount (m_viewCount) > 1)
	{
	    QMenu* menuCopys = menu.addMenu ("Copy");
	    QMenu* menuTransformations = menuCopys->addMenu ("Transformations");
	    for (size_t i = 0; i < ViewCount::GetCount (m_viewCount); ++i)
	    {
		ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
		if (viewNumber == m_viewNumber)
		    continue;
		menuTransformations->addAction (
		    m_actionCopyTransformations[i].get ());
	    }
	}
    }
    menu.exec (event->globalPos());
}


void GLWidget::displayViewDecorations (ViewNumber::Enum view)
{
    const ViewSettings& vs = *GetViewSettings (view);
    glPushAttrib (
	GL_POLYGON_BIT | GL_CURRENT_BIT | 
	GL_VIEWPORT_BIT | GL_TEXTURE_BIT | GL_ENABLE_BIT);
    if (vs.IsLightingEnabled ())
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

    G3D::Rect2D viewRect = GetViewRect (view);
    if (isColorBarUsed (view))
	displayTextureColorBar (view, viewRect);
    displayViewTitle (viewRect, view);
    displayViewTimeStep (viewRect);
    displayViewGrid ();

    glPopMatrix ();
    glMatrixMode (GL_MODELVIEW);
    glPopMatrix ();
    glPopAttrib ();
}

void GLWidget::displayViewTitle (const G3D::Rect2D& viewRect, 
				 ViewNumber::Enum viewNumber)
{
    if (! m_titleShown)
	return;
    QFont font;
    if (viewNumber == m_viewNumber)
	font.setUnderline (true);
    boost::shared_ptr<ViewSettings> vs = GetViewSettings (viewNumber);
    ostringstream ostr;
    ostr << "View " << viewNumber << " - " <<
	(isColorBarUsed (viewNumber) ? 
	 BodyProperty::ToString (vs->GetBodyProperty ()) :
	 ViewType::ToString (vs->GetViewType ()));
    QString text = QString (ostr.str ().c_str ());
    QFontMetrics fm (font);
    const int textX = 
	viewRect.x0 () + (float (viewRect.width ()) - fm.width (text)) / 2;
    const int textY = OpenGlToQt (
	viewRect.y1 () - (fm.height () + 3), height ());
    glColor (Qt::black);
    renderText (textX, textY, text, font);
}


void GLWidget::displayViewTimeStep (const G3D::Rect2D& viewRect)
{
    if (! m_timeStepShown)
	return;
    QFont font;
    ostringstream ostr;
    ostr << GetTimeStep ();
    QString text = QString (ostr.str ().c_str ());
    QFontMetrics fm (font);
    const int textX = 
	viewRect.x0 () + (float (viewRect.width ()) - fm.width (text)) / 2;
    const int textY = OpenGlToQt (viewRect.y0 () + 3, height ());
    glColor (Qt::black);
    renderText (textX, textY, text, font);
}



void GLWidget::displayTextureColorBar (
    ViewNumber::Enum viewNumber, const G3D::Rect2D& viewRect)
{
    G3D::Rect2D colorBarRect = getViewColorBarRect (viewRect);
    glDisable (GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_1D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture (GL_TEXTURE_1D, 
		   GetViewSettings (viewNumber)->GetColorBarTexture ());
    
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glBegin (GL_QUADS);
    glTexCoord1f(0);glVertex (colorBarRect.x0y0 ());
    glTexCoord1f(1);glVertex (colorBarRect.x0y1 ());
    glTexCoord1f(1);glVertex (colorBarRect.x1y1 ());
    glTexCoord1f(0);glVertex (colorBarRect.x1y0 ());
    glEnd ();
    glDisable (GL_TEXTURE_1D);

    glColor (Qt::black);
    glEnable (GL_POLYGON_OFFSET_LINE);
    glPolygonOffset (-1, -1);
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    glBegin (GL_QUADS);
    glVertex (colorBarRect.x0y0 ());
    glVertex (colorBarRect.x0y1 ());
    glVertex (colorBarRect.x1y1 ());
    glVertex (colorBarRect.x1y0 ());
    glEnd ();
}

void GLWidget::displayViewGrid ()
{
    size_t w = width ();
    size_t h = height ();
    glColor (Qt::blue);
    glBegin (GL_LINES);
    if (m_viewCount == ViewCount::TWO)
    {
	if (m_viewLayout == ViewLayout::HORIZONTAL)
	{
	    glVertex2s (w/2, 0);
	    glVertex2s (w/2, h);
	}
	else
	{
	    glVertex2s (0, h/2);
	    glVertex2s (w, h/2);
	}
    }
    else if (m_viewCount == ViewCount::FOUR)
    {
	glVertex2s (w/2, 0);
	glVertex2s (w/2, h);
	glVertex2s (0, h/2);
	glVertex2s (w, h/2);	
    }
    glEnd ();
}

QColor GLWidget::GetCenterPathContextColor () const
{
    QColor returnColor (Qt::black);
    returnColor.setAlphaF (GetContextAlpha ());
    return returnColor;
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
    setLabel ();
    compile (GetViewNumber ());
    update ();
}

bool GLWidget::IsTimeDisplacementUsed () const
{
    return GetTimeDisplacement () > 0;
}

BodyProperty::Enum GLWidget::GetBodyProperty (ViewNumber::Enum viewNumber) const
{
    return GetViewSettings (viewNumber)->GetBodyProperty ();
}

void GLWidget::SetPlayMovie (bool playMovie)
{
    m_playMovie = playMovie;
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
    setLabel ();
    compile (GetViewNumber ());
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
    setLabel ();
    compile (GetViewNumber ());
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
    setLabel ();
    compile (GetViewNumber ());
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
    setLabel ();
    compile (GetViewNumber ());
    update ();
}


// Slots
// ======================================================================

void GLWidget::ToggledDirectionalLightEnabled (bool checked)
{
    ViewSettings& vs = *GetViewSettings ();
    LightNumber::Enum selectedLight = vs.GetSelectedLight ();
    vs.SetDirectionalLightEnabled (selectedLight, checked);
    vs.PositionLight (selectedLight, getInitialLightPosition (selectedLight));
    update ();
}


void GLWidget::ToggledZeroedPressureShown (bool checked)
{
    m_zeroedPressureShown = checked;
    update ();
}

void GLWidget::ToggledLightNumberShown (bool checked)
{
    ViewSettings& vs = *GetViewSettings ();
    vs.SetLightPositionShown (vs.GetSelectedLight (), checked);
    update ();
}

void GLWidget::ToggledLightEnabled (bool checked)
{
    makeCurrent ();
    ViewSettings& vs = *GetViewSettings ();
    LightNumber::Enum selectedLight = vs.GetSelectedLight ();
    vs.SetLightEnabled (selectedLight, checked);
    vs.EnableLighting ();
    if (checked)
	vs.PositionLight (
	    selectedLight, getInitialLightPosition (selectedLight));
    update ();
}


void GLWidget::ToggledBoundingBoxShown (bool checked)
{
    m_boundingBoxShown = checked;
    update ();
}

void GLWidget::ToggledBodyStationaryMarked (bool checked)
{
    m_bodyStationaryMarked = checked;
    update ();
}

void GLWidget::ToggledBodiesBoundingBoxesShown (bool checked)
{
    m_bodiesBoundingBoxesShown = checked;
    update ();
}

void GLWidget::ToggledContextView (bool checked)
{
    ViewSettings& vs = *GetViewSettings ();
    vs.SetContextView (checked);
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

void GLWidget::ToggledStandaloneElementsShown (bool checked)
{
    m_standaloneElementsShown = checked;
    update ();
}


void GLWidget::ToggledTimeStepShown (bool checked)
{
    m_timeStepShown = checked;
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
    compile (GetViewNumber ());
    update ();
}


void GLWidget::ToggledEdgesNormal (bool checked)
{
    changeViewType (checked, ViewType::EDGES);
}

void GLWidget::ToggledEdgesTorus (bool checked)
{
    changeViewType (checked, ViewType::EDGES_TORUS);
}

void GLWidget::ToggledBodyCenterShown (bool checked)
{
    m_bodyCenterShown = checked;
    update ();
}

void GLWidget::ToggledFacesNormal (bool checked)
{
    changeViewType (checked, ViewType::FACES);
}

void GLWidget::ToggledFacesShowEdges (bool checked)
{
    m_facesShowEdges = checked;
    update ();
}

void GLWidget::ToggledFaceEdgesTorus (bool checked)
{
    changeViewType (checked, ViewType::FACES_TORUS);
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

void GLWidget::ToggledCenterPathTubeUsed (bool checked)
{
    m_centerPathTubeUsed = checked;
    compile (GetViewNumber ());
    update ();
}

void GLWidget::ToggledCenterPathLineUsed (bool checked)
{
    m_centerPathLineUsed = checked;
    compile (GetViewNumber ());
    update ();
}

void GLWidget::ToggledTitleShown (bool checked)
{
    m_titleShown = checked;
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

void GLWidget::ToggledT1sShiftLower (bool checked)
{
    if (checked)
	GetFoamAlongTime ().SetT1sShift (1);
    else
	GetFoamAlongTime ().SetT1sShift (0);
    update ();
}



void GLWidget::ToggledCenterPath (bool checked)
{
    changeViewType (checked, ViewType::CENTER_PATHS);
}


void GLWidget::CurrentIndexChangedSelectedLight (int selectedLight)
{
    ViewSettings& vs = *GetViewSettings ();
    vs.SetSelectedLight (LightNumber::Enum (selectedLight));
}

void GLWidget::CurrentIndexChangedViewCount (int index)
{
    m_viewCount = ViewCount::Enum (index);
    if (m_viewCount == ViewCount::ONE)
    {
	m_viewNumber = ViewNumber::VIEW0;
    }
    for (size_t i = 0; i < ViewCount::GetCount (m_viewCount); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	G3D::Rect2D viewRect = GetViewRect (viewNumber);
	ViewSettings& vs = *GetViewSettings (viewNumber);
	vs.CalculateCameraDistance (
	    calculateCenteredViewingVolume (
		viewRect.width () / viewRect.height (), vs.GetScaleRatio ()));
    }
    update ();
}

void GLWidget::CurrentIndexChangedViewLayout (int index)
{
    m_viewLayout = ViewLayout::Enum (index);
    update ();
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
	pair<double, double> minMax = getStatisticsMinMax (GetViewNumber ());
	GetViewSettings ()->GetDisplayFaceStatistics ()->InitStep (
	    GetViewNumber (), minMax.first, minMax.second);
    }
    else
	GetViewSettings ()->GetDisplayFaceStatistics ()->Release ();
    changeViewType (checked, ViewType::FACES_STATISTICS);
}

void GLWidget::CurrentIndexChangedStatisticsType (int index)
{
    GetViewSettings ()->SetStatisticsType (StatisticsType::Enum(index));
    update ();
}

void GLWidget::CurrentIndexChangedAxesOrder (int index)
{
    GetViewSettings ()->SetAxesOrder (AxesOrder::Enum(index));
}

// @todo add a color bar model for BodyProperty::None
void GLWidget::SetBodyProperty (
    boost::shared_ptr<ColorBarModel> colorBarModel,
    BodyProperty::Enum property)
{
    makeCurrent ();
    ViewNumber::Enum view = GetViewNumber ();
    boost::shared_ptr<ViewSettings> vs = GetViewSettings ();
    vs->SetBodyProperty (property);
    if (vs->GetBodyProperty () != BodyProperty::NONE)
	GetViewSettings ()->SetColorBarModel (colorBarModel);
    else
	vs->ResetColorBarModel ();
    compile (view);
    update ();
}

void GLWidget::SetColorBarModel (boost::shared_ptr<ColorBarModel> colorBarModel)
{
    makeCurrent ();
    GetViewSettings ()->SetColorBarModel (colorBarModel);
    update ();
}

void GLWidget::ValueChangedSliderTimeSteps (int timeStep)
{
    makeCurrent ();
    m_timeStep = timeStep;
    for (size_t i = 0; i < ViewCount::GetCount (m_viewCount); ++i)
    {
	ViewNumber::Enum view = ViewNumber::Enum (i);
	boost::shared_ptr<ViewSettings> vs = GetViewSettings (view);
	if (vs->GetViewType () == ViewType::FACES_STATISTICS)
	{
	    pair<double, double> minMax = getStatisticsMinMax (view);
	    vs->GetDisplayFaceStatistics ()->Step (
		view, minMax.first, minMax.second);
	}
    }
    update ();
}

void GLWidget::ValueChangedStatisticsHistory (int timeSteps)
{
    GetViewSettings ()->GetDisplayFaceStatistics ()->SetHistoryCount (
	timeSteps);
}

void GLWidget::ValueChangedTimeDisplacement (int timeDisplacement)
{
    QSlider* slider = static_cast<QSlider*> (sender ());
    size_t maximum = slider->maximum ();
    G3D::AABox bb = GetFoamAlongTime ().GetBoundingBox ();
    m_timeDisplacement =
	(bb.high () - bb.low ()).z * timeDisplacement /
	GetFoamAlongTime ().GetTimeSteps () / maximum;
    compile (GetViewNumber ());
    update ();
}

void GLWidget::ValueChangedT1Size (int index)
{
    QSlider* slider = static_cast<QSlider*> (sender ());
    size_t maximum = slider->maximum ();
    m_t1Size = MIN_T1_SIZE + 
	(GLfloat (index) / maximum) * (MAX_T1_SIZE - MIN_T1_SIZE);
    update ();
}


void GLWidget::ValueChangedEdgesRadius (int sliderValue)
{
    makeCurrent ();
    size_t maximum = static_cast<QSlider*> (sender ())->maximum ();
    m_edgeRadiusRatio = static_cast<double>(sliderValue) / maximum;
    calculateEdgeRadius (m_edgeRadiusRatio,
			 &m_edgeRadius, &m_arrowBaseRadius,
			 &m_arrowHeight, &m_edgeWidth);
    compile (GetViewNumber ());
    update ();
}

void GLWidget::ValueChangedContextAlpha (int sliderValue)
{
    size_t maximum = static_cast<QSlider*> (sender ())->maximum ();
    m_contextAlpha = MIN_CONTEXT_ALPHA +
	(MAX_CONTEXT_ALPHA - MIN_CONTEXT_ALPHA) * sliderValue / maximum;
    compile (GetViewNumber ());
    update ();
}

void GLWidget::ValueChangedLightAmbientRed (int sliderValue)
{
    setLight (sliderValue, static_cast<QSlider*> (sender ())->maximum (),
	      LightType::AMBIENT, ColorNumber::RED);
}


void GLWidget::ValueChangedLightAmbientGreen (int sliderValue)
{
    setLight (sliderValue, static_cast<QSlider*> (sender ())->maximum (),
	      LightType::AMBIENT, ColorNumber::GREEN);
}

void GLWidget::ValueChangedLightAmbientBlue (int sliderValue)
{
    setLight (sliderValue, static_cast<QSlider*> (sender ())->maximum (),
	      LightType::AMBIENT, ColorNumber::BLUE);
}

void GLWidget::ValueChangedLightDiffuseRed (int sliderValue)
{
    setLight (sliderValue, static_cast<QSlider*> (sender ())->maximum (),
	      LightType::DIFFUSE, ColorNumber::RED);
}

void GLWidget::ValueChangedLightDiffuseGreen (int sliderValue)
{
    setLight (sliderValue, static_cast<QSlider*> (sender ())->maximum (),
	      LightType::DIFFUSE, ColorNumber::GREEN);
}

void GLWidget::ValueChangedLightDiffuseBlue (int sliderValue)
{
    setLight (sliderValue, static_cast<QSlider*> (sender ())->maximum (),
	      LightType::DIFFUSE, ColorNumber::BLUE);
}

void GLWidget::ValueChangedLightSpecularRed (int sliderValue)
{
    setLight (sliderValue, static_cast<QSlider*> (sender ())->maximum (),
	      LightType::SPECULAR, ColorNumber::RED);
}

void GLWidget::ValueChangedLightSpecularGreen (int sliderValue)
{
    setLight (sliderValue, static_cast<QSlider*> (sender ())->maximum (),
	      LightType::SPECULAR, ColorNumber::GREEN);
}

void GLWidget::ValueChangedLightSpecularBlue (int sliderValue)
{
    setLight (sliderValue, static_cast<QSlider*> (sender ())->maximum (),
	      LightType::SPECULAR, ColorNumber::BLUE);
}

void GLWidget::ValueChangedAngleOfView (int angleOfView)
{
    G3D::Rect2D viewRect = GetViewRect ();
    ViewSettings& vs = *GetViewSettings ();
    vs.SetAngleOfView (angleOfView);
    vs.CalculateCameraDistance (
	calculateCenteredViewingVolume (
	    viewRect.width () / viewRect.height (), vs.GetScaleRatio ()));
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

void GLWidget::CopyTransformationsFrom (int viewNumber)
{
    GetViewSettings ()->CopyTransformations (
	*GetViewSettings (ViewNumber::Enum (viewNumber)));
    update ();
}

void GLWidget::CopyColorBarFrom (int viewNumber)
{
    boost::shared_ptr<ViewSettings> vs = 
	GetViewSettings (ViewNumber::Enum (viewNumber));
    GetViewSettings ()->CopyColorBar (*vs);
    Q_EMIT ColorBarModelChanged (GetViewSettings ()->GetColorBarModel ());
}

