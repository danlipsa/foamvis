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
#include "DisplayForces.h"
#include "DisplayBodyFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "DisplayFaceFunctors.h"
#include "DisplayVertexFunctors.h"
#include "Edge.h"
#include "Face.h"
#include "Foam.h"
#include "FoamAlongTime.h"
#include "GLWidget.h"
#include "Info.h"
#include "OpenGLUtils.h"
#include "OrientedEdge.h"
#include "OrientedFace.h"
#include "SelectBodiesById.h"
#include "Utils.h"
#include "Vertex.h"
#include "ViewSettings.h"

// Private Classes/Functions
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


template <typename T>
void display (const char* name, const T& what)
{
    ostream_iterator<GLfloat> out (cdbg, " ");
    cdbg << name;
    copy (what.begin (), what.end (), out);
    cdbg << endl;
}

void displayBodyDeformationTensor2D (boost::shared_ptr<Body> body, double size)
{
    G3D::Matrix3 rotation;
    for (size_t i = 0; i < 3; ++i)
	rotation.setColumn (i, body->GetDeformationEigenVector (i));
    G3D::CoordinateFrame cf (rotation, body->GetCenter ());
    glPushMatrix ();
    glMultMatrix (cf);
    drawEllipsis2D (body->GetDeformationEigenValue (0), 
		    body->GetDeformationEigenValue (1), size);
    glPopMatrix ();
}


// Static Fields
// ======================================================================

const size_t GLWidget::DISPLAY_ALL(numeric_limits<size_t>::max());
// quadrics
const size_t GLWidget::QUADRIC_SLICES = 8;
const size_t GLWidget::QUADRIC_STACKS = 1;

const double GLWidget::ENCLOSE_ROTATION_RATIO = 1;
const pair<double,double> GLWidget::T1_SIZE (1, 10);
const pair<double,double> GLWidget::DEFORMATION_TENSOR_SIZE (1, 100);
const pair<double,double> GLWidget::CONTEXT_ALPHA (0.05, 0.5);
const pair<double,double> GLWidget::FORCE_LENGTH (.25, 6);
const GLfloat GLWidget::HIGHLIGHT_LINE_WIDTH = 2.0;

// Methods
// ======================================================================

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent),
      m_torusOriginalDomainDisplay (false),
      m_torusOriginalDomainClipped (false),
      m_interactionMode (InteractionMode::ROTATE),
      m_foamAlongTime (0), m_timeStep (0),
      m_minimumEdgeRadius (0),
      m_edgeRadiusRatio (0),
      m_facesShowEdges (true),
      m_bodyCenterShown (false),
      m_faceCenterShown (false),
      m_edgesTessellation (true),
      m_centerPathBodyShown (false),
      m_boundingBoxShown (false),
      m_bodiesBoundingBoxesShown (false),
      m_axesShown (false),
      m_standaloneElementsShown (true),
      m_timeDisplacement (0.0),
      m_selectBodiesById (new SelectBodiesById (this)),
      m_centerPathTubeUsed (true),
      m_centerPathLineUsed (false),
      m_t1sShown (false),
      m_t1Size (T1_SIZE.first),
      m_deformationTensorSize (DEFORMATION_TENSOR_SIZE.first),
      m_contextAlpha (CONTEXT_ALPHA.first),
      m_forceLength (FORCE_LENGTH.first),
      m_highlightLineWidth (HIGHLIGHT_LINE_WIDTH),
      m_missingPressureShown (true),
      m_missingVolumeShown (true),
      m_titleShown (false),
      m_timeStepShown (false),
      m_averageAroundBody (true),
      m_viewCount (ViewCount::ONE),
      m_viewLayout (ViewLayout::HORIZONTAL),
      m_viewNumber (ViewNumber::VIEW0),
      m_showType (SHOW_NOTHING)
{
    makeCurrent ();
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

    m_actionAverageAroundBody = boost::make_shared<QAction> (
	tr("&Body"), this);
    m_actionAverageAroundBody->setStatusTip(tr("Averaged around body"));
    connect(m_actionAverageAroundBody.get (), SIGNAL(triggered()),
	    this, SLOT(AverageAroundBody ()));

    m_actionAverageAroundConstraint = boost::make_shared<QAction> (
	tr("&Constraint"), this);
    m_actionAverageAroundConstraint->setStatusTip(
	tr("Averaged around constraint"));
    connect(m_actionAverageAroundConstraint.get (), SIGNAL(triggered()),
	    this, SLOT(AverageAroundConstraint ()));

    m_actionAverageAroundReset = boost::make_shared<QAction> (
	tr("&Reset"), this);
    m_actionAverageAroundReset->setStatusTip(tr("Average around reset"));
    connect(m_actionAverageAroundReset.get (), SIGNAL(triggered()),
	    this, SLOT(AverageAroundReset ()));

    m_actionContextDisplayBody = boost::make_shared<QAction> (
	tr("&Body"), this);
    m_actionContextDisplayBody->setStatusTip(tr("Context body"));
    connect(m_actionContextDisplayBody.get (), SIGNAL(triggered()),
	    this, SLOT(ContextDisplayBody ()));

    m_actionContextStationaryFoam = boost::make_shared<QAction> (
	tr("&Foam"), this);
    m_actionContextStationaryFoam->setStatusTip(tr("Context stationary foam"));
    connect(m_actionContextStationaryFoam.get (), SIGNAL(triggered()),
	    this, SLOT(ContextStationaryFoam ()));

    m_actionContextStationaryReset = boost::make_shared<QAction> (
	tr("&Reset"), this);
    m_actionContextStationaryReset->setStatusTip(
	tr("Context stationary reset"));
    connect(m_actionContextStationaryReset.get (), SIGNAL(triggered()),
	    this, SLOT(ContextStationaryReset ()));


    m_actionContextDisplayReset = boost::make_shared<QAction> (
	tr("&Reset"), this);
    m_actionContextDisplayReset->setStatusTip(tr("Context reset"));
    connect(m_actionContextDisplayReset.get (), SIGNAL(triggered()),
	    this, SLOT(ContextDisplayReset ()));

    m_actionInfoPoint = boost::make_shared<QAction> (tr("&Point"), this);
    m_actionInfoPoint->setStatusTip(tr("Info point"));
    connect(m_actionInfoPoint.get (), SIGNAL(triggered()), this, 
	    SLOT(InfoPoint ()));

    m_actionInfoEdge = boost::make_shared<QAction> (tr("&Edge"), this);
    m_actionInfoEdge->setStatusTip(tr("Info edge"));
    connect(m_actionInfoEdge.get (), SIGNAL(triggered()), this, 
	    SLOT(InfoEdge ()));

    m_actionInfoFace = boost::make_shared<QAction> (tr("&Face"), this);
    m_actionInfoFace->setStatusTip(tr("Info face"));
    connect(m_actionInfoFace.get (), SIGNAL(triggered()), this, 
	    SLOT(InfoFace ()));

    m_actionInfoBody = boost::make_shared<QAction> (tr("&Body"), this);
    m_actionInfoBody->setStatusTip(tr("Info body"));
    connect(m_actionInfoBody.get (), SIGNAL(triggered()), this, 
	    SLOT(InfoBody ()));

    m_actionInfoFoam = boost::make_shared<QAction> (tr("&Foam"), this);
    m_actionInfoFoam->setStatusTip(tr("Info foam"));
    connect(m_actionInfoFoam.get (), SIGNAL(triggered()), this, 
	    SLOT(InfoFoam ()));

    m_actionInfoOpenGL = boost::make_shared<QAction> (
	tr("&OpenGL"), this);
    m_actionInfoOpenGL->setStatusTip(tr("Info OpenGL"));
    connect(m_actionInfoOpenGL.get (), SIGNAL(triggered()),
	    this, SLOT(InfoOpenGL ()));

    m_actionShowNeighbors = boost::make_shared<QAction> (
	tr("&Neighbors"), this);
    m_actionShowNeighbors->setStatusTip(tr("Shown neighbors"));
    connect(m_actionShowNeighbors.get (), SIGNAL(triggered()),
	    this, SLOT(ShowNeighbors ()));

    m_actionShowTextureTensor = boost::make_shared<QAction> (
	tr("&Texture tensor"), this);
    m_actionShowTextureTensor->setStatusTip(tr("Shown texture tensor"));
    connect(m_actionShowTextureTensor.get (), SIGNAL(triggered()),
	    this, SLOT(ShowTextureTensor ()));

    m_actionShowReset = boost::make_shared<QAction> (tr("&Reset"), this);
    m_actionShowReset->setStatusTip(tr("Shown reset"));
    connect(m_actionShowReset.get (), SIGNAL(triggered()),
	    this, SLOT(ShowReset ()));

    // actions for the color bar
    m_actionEditColorMap.reset (
	new QAction (tr("&Edit Color Map"), this));
    m_actionEditColorMap->setStatusTip(tr("Edit Color Map"));
    connect(m_actionEditColorMap.get (), SIGNAL(triggered()),
	    this, SLOT(EditColorMapDispatch ()));

    m_actionClampClear.reset (
	new QAction (tr("&Clamp Clear"), this));
    m_actionClampClear->setStatusTip(tr("Clamp Clear"));
    connect(m_actionClampClear.get (), SIGNAL(triggered()),
	    this, SLOT(ColorBarClampClear ()));

    initCopy (m_actionCopyTransformation, m_signalMapperCopyTransformation);
    connect (m_signalMapperCopyTransformation.get (),
	     SIGNAL (mapped (int)),
	     this,
	     SLOT (CopyTransformationFrom (int)));

    initCopy (m_actionCopySelection, m_signalMapperCopySelection);
    connect (m_signalMapperCopySelection.get (),
	     SIGNAL (mapped (int)),
	     this,
	     SLOT (CopySelectionFrom (int)));

    initCopy (m_actionCopyColorMap, m_signalMapperCopyColorBar);
    connect (m_signalMapperCopyColorBar.get (),
	     SIGNAL (mapped (int)),
	     this,
	     SLOT (CopyColorBarFrom (int)));
}

void GLWidget::initCopy (
    boost::array<boost::shared_ptr<QAction>, 
    ViewNumber::COUNT>& actionCopyTransformation,
    boost::shared_ptr<QSignalMapper>& signalMapperCopyTransformation)
{
    signalMapperCopyTransformation.reset (new QSignalMapper (this));
    for (size_t i = 0; i < ViewNumber::COUNT; ++i)
    {
	ostringstream ostr;
	ostr << "View " << i;
	QString text (ostr.str ().c_str ());
	actionCopyTransformation[i] = boost::make_shared<QAction> (
	    text, this);
	actionCopyTransformation[i]->setStatusTip(text);
	connect(actionCopyTransformation[i].get (), 
		SIGNAL(triggered()),
		signalMapperCopyTransformation.get (), 
		SLOT(map ()));
	signalMapperCopyTransformation->setMapping (
	    actionCopyTransformation[i].get (), i);
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

void GLWidget::initViewSettings ()
{
    BOOST_FOREACH (boost::shared_ptr<ViewSettings>& vs, m_viewSettings)
	vs = boost::make_shared <ViewSettings> (*this);
    if (GetFoamAlongTime ().Is2D ())
    {
	BOOST_FOREACH (boost::shared_ptr<ViewSettings> vs, m_viewSettings)
	{
	    vs->SetViewType (ViewType::FACES);
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
    BOOST_FOREACH (boost::shared_ptr<ViewSettings> vs, m_viewSettings)
    {
	size_t timeSteps = GetFoamAlongTime ().GetTimeSteps ();
	vs->GetDisplayFaceStatistics ().SetTimeWindow (timeSteps);
	vs->GetDisplayForces ().SetTimeWindow (timeSteps);
    }
    CurrentIndexChangedViewCount (ViewCount::ONE);
}

void GLWidget::SetFoamAlongTime (FoamAlongTime* foamAlongTime)
{
    m_foamAlongTime = foamAlongTime;
    initViewSettings ();
    Foam::Bodies bodies = GetFoamAlongTime ().GetFoam (0).GetBodies ();
    if (bodies.size () != 0)
    {
	size_t maxIndex = bodies.size () - 1;
	m_selectBodiesById->SetMinBodyId (bodies[0]->GetId ());
	m_selectBodiesById->SetMaxBodyId (bodies[maxIndex]->GetId ());
	m_selectBodiesById->UpdateLabelMinMax ();
    }
    update ();
}


double GLWidget::getMinimumEdgeRadius () const
{
    G3D::Vector3 first = toObject (QPoint (0, 0));
    G3D::Vector3 second = toObject (QPoint (1, 0));
    float onePixelInObjectSpace = (second - first).length ();
    return onePixelInObjectSpace;
}

void GLWidget::calculateEdgeRadius (
    double edgeRadiusRatio,
    double* edgeRadius, double* arrowBaseRadius, 
    double* arrowHeight, double* edgeWidth) const
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

ViewType::Enum GLWidget::changeViewType (bool checked, 
					 ViewType::Enum newViewType)
{
    if (checked)
    {
	ViewType::Enum oldViewType = GetViewSettings ().GetViewType ();
	GetViewSettings ().SetViewType (newViewType);
	compile (GetViewNumber ());
	update ();
	return oldViewType;
    }
    else
	return ViewType::COUNT;
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
    const ViewSettings& vs = GetViewSettings (viewNumber);
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
    ViewSettings& vs = GetViewSettings (viewNumber);
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
    const ViewSettings& vs = GetViewSettings (viewNumber);
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
    const ViewSettings& vs = GetViewSettings (viewNumber);
    ViewSettings::AverageAroundType type = vs.GetAverageAroundType ();
    switch (type)
    {
    case ViewSettings::AVERAGE_AROUND_BODY:
    {
	size_t id = vs.GetAverageAroundBodyId ();
	G3D::Vector3 centerBegin = (*GetFoamAlongTime ().GetFoam (0).
				    FindBody (id))->GetCenter ();
	G3D::Vector3 centerCurrent = (*GetFoamAlongTime ().GetFoam (timeStep).
				      FindBody (id))->GetCenter ();
	G3D::Vector3 translation = centerBegin - centerCurrent;
	glTranslate (translation);
	glTranslate (-GetFoamAlongTime ().GetBoundingBox ().center ());
	break;
    }
    case ViewSettings::AVERAGE_AROUND_CONSTRAINT:
    {
	glTranslate (- GetFoamAlongTime ().GetBoundingBox ().center ());
	rotateAverageAroundConstraint (timeStep, 1);
	break;
    }
    default:
	glTranslate (- GetFoamAlongTime ().GetBoundingBox ().center ());
	break;
    }
}

void GLWidget::rotateAverageAroundConstraint (
    size_t timeStep, int direction) const
{
    const ConstraintRotation& rotationBegin = GetFoamAlongTime ().
	GetFoam (0).GetConstraintRotation ();
    const ConstraintRotation& rotationCurrent = GetFoamAlongTime ().
	GetFoam (timeStep).GetConstraintRotation ();
    float angleRadians = rotationCurrent.m_angle - rotationBegin.m_angle;
    if (angleRadians != 0)
    {
	G3D::Vector3 rotationCenter = G3D::Vector3 (rotationBegin.m_center, 0.0);
	glTranslate (rotationCenter);
	float angleDegrees =  G3D::toDegrees (angleRadians);
	angleDegrees = direction > 0 ? angleDegrees : - angleDegrees;
	glRotatef (angleDegrees, 0, 0, 1);
	glTranslate (-rotationCenter);
    }
}



G3D::AABox GLWidget::calculateViewingVolume (
    ViewNumber::Enum viewNumber, double xOverY, double scaleRatio) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::AABox centeredViewingVolume = 
	calculateCenteredViewingVolume (xOverY, scaleRatio);
    G3D::Vector3 translation (vs.GetCameraDistance () * G3D::Vector3::unitZ ());
    G3D::AABox result = centeredViewingVolume - translation;
    return result;
}


void GLWidget::projectionTransform (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
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
    ViewSettings& vs = GetViewSettings (viewNumber);
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
	Rect2D v[][2] = {
	    // 0 | 1
	    // horizontal layout
	    {Rect2D::xywh (0, 0, w/2, h), Rect2D::xywh (w/2, 0, w/2, h)},
	    // 0
	    // -
	    // 1
	    // vertical layout
	    {Rect2D::xywh (0, h/2, w, h/2), Rect2D::xywh (0, 0, w, h/2)}
	};
	return v[m_viewLayout][view];
    }
    case ViewCount::THREE:
    {
	RuntimeAssert (view < 3, "Invalid view: ", view);
	Rect2D v[][3] = {
	    // 0 | 1 | 3
	    // horizontal layout
	    {Rect2D::xywh (0, 0, w/3, h), Rect2D::xywh (w/3, 0, w/3, h),
	     Rect2D::xywh (2*w/3, 0, w/3, h)},
	    // 0
	    // -
	    // 1
	    // -
	    // 3
	    // vertical layout
	    {Rect2D::xywh (0, 2*h/3, w, h/3), Rect2D::xywh (0, h/3, w, h/3), 
	     Rect2D::xywh (0, 0, w, h/3)}
	};
	return v[m_viewLayout][view];
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
	xOverY/3, 3*xOverY, // THREE (HORIZONTAL, VERTICAL)
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
    const Foam& foam = GetFoamAlongTime ().GetFoam (0);
    glMultMatrix (foam.GetViewMatrix ().approxCoordinateFrame ().rotation);
}


void GLWidget::ResetTransformation ()
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings ();
    vs.SetRotationModel (G3D::Matrix3::identity ());
    vs.SetScaleRatio (1);
    vs.SetContextScaleRatio (1);
    vs.SetTranslation (G3D::Vector3::zero ());
    projectionTransform (viewNumber);
    update ();
}

void GLWidget::ResetSelectedLightNumber ()
{
    ViewSettings& vs = GetViewSettings ();
    LightNumber::Enum lightNumber = vs.GetSelectedLight ();
    vs.SetInitialLightPosition (lightNumber);
    vs.PositionLight (lightNumber, getInitialLightPosition (lightNumber));
    update ();
}


void GLWidget::SelectBodiesByIdList ()
{
    if (m_selectBodiesById->exec () == QDialog::Accepted)
    {
	GetViewSettings ().SetBodySelector (
	    boost::shared_ptr<IdBodySelector> (
		new IdBodySelector (m_selectBodiesById->GetIds ())));
	labelCompileUpdate ();
    }
}

void GLWidget::SelectAll ()
{
    GetViewSettings ().
	SetBodySelector (AllBodySelector::Get (), BodySelectorType::ID);
    labelCompileUpdate ();
    m_selectBodiesById->ClearEditIds ();
    update ();
}

void GLWidget::DeselectAll ()
{
    GetViewSettings ().SetBodySelector (
	boost::shared_ptr<IdBodySelector> (new IdBodySelector ()));
    labelCompileUpdate ();
}

void GLWidget::InfoFoam ()
{
    string message = GetFoamAlongTime ().ToHtml ();
    QMessageBox msgBox (this);
    msgBox.setText(message.c_str ());
    msgBox.exec();
}

void GLWidget::InfoPoint ()
{
    QMessageBox msgBox (this);
    ostringstream ostr;
    ostr << "Point: " << m_contextMenuPosObject;
    msgBox.setText(ostr.str ().c_str ());
    msgBox.exec();
}

void GLWidget::InfoEdge ()
{
    Info msgBox (this, "Info");
    ostringstream ostr;
    OrientedEdge oe = brushedEdge ();
    if (! oe.GetEdge ())
	ostr << "No body focused.";
    else
	ostr << oe;
    msgBox.setText(ostr.str ().c_str ());
    msgBox.exec();
}

void GLWidget::InfoFace ()
{
    Info msgBox (this, "Info");
    ostringstream ostr;
    const OrientedFace* of = 0;
    brushedFace (&of);
    if (of == 0)
	ostr << "No face focused.";
    else
	ostr << *of;
    msgBox.setText(ostr.str ().c_str ());
    msgBox.exec();
}

void GLWidget::InfoBody ()
{
    Info msgBox (this, "Info");
    const BodySelector& bodySelector = GetViewSettings ().GetBodySelector ();
    string message;
    switch (bodySelector.GetType ())
    {
    case BodySelectorType::ALL:
	message = infoSelectedBody ();
	break;

    case BodySelectorType::ID:
	message = infoSelectedBodies ();
	break;

    default:
	break;
    }

    msgBox.setText (message.c_str ());
    msgBox.exec();
}

string GLWidget::infoSelectedBody () const
{
    ostringstream ostr;
    vector< boost::shared_ptr<Body> > bodies;
    brushedBodies (m_contextMenuPosScreen, &bodies);
    if (bodies.size () == 0)
	ostr << "No bodies focused.";
    else
	ostr << bodies[0];
    return ostr.str ();
}

string GLWidget::infoSelectedBodies () const
{
    ostringstream ostr;
    const BodySelector& bodySelector = GetViewSettings ().GetBodySelector ();
    const vector<size_t>& ids = 
	(static_cast<const IdBodySelector&> (bodySelector)).GetIds ();
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
	if (GetViewSettings ().GetBodyProperty () != 
	    BodyProperty::NONE)
	{
	    ostr << endl;
		
	}
    }
    return ostr.str ();
}

void GLWidget::InfoOpenGL ()
{
    ostringstream ostr;
    printOpenGLInfo (ostr);
    Info openGLInfo (this, "OpenGL Info", ostr.str ().c_str ());
    openGLInfo.exec ();
}

void GLWidget::ShowNeighbors ()
{
    m_showType = SHOW_NEIGHBORS;
    vector<size_t> bodies;
    brushedBodies (m_contextMenuPosScreen, &bodies);
    m_showBodyId = bodies[0];
    update ();
}

void GLWidget::ShowTextureTensor ()
{
    m_showType = SHOW_DEFORMATION_TENSOR;
    vector<size_t> bodies;
    brushedBodies (m_contextMenuPosScreen, &bodies);
    m_showBodyId = bodies[0];
    update ();
}

void GLWidget::ShowReset ()
{
    m_showType = SHOW_NOTHING;
    update ();
}

void GLWidget::EditColorMapDispatch ()
{
    Q_EMIT EditColorMap ();
}

void GLWidget::ColorBarClampClear ()
{
    ViewSettings& vs = GetViewSettings ();
    boost::shared_ptr<ColorBarModel> colorBarModel = vs.GetColorBarModel ();
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
    glEnable (GL_MULTISAMPLE);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    DisplayFaceStatistics::InitShaders ();
    initializeLighting ();
    WarnOnOpenGLError ("initializeGl");
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
	for (size_t i = 0; i < 4; ++i)
	    displayView (ViewNumber::Enum (i));
	break;
    case ViewCount::THREE:
	for (size_t i = 0; i < 3; ++i)
	    displayView (ViewNumber::Enum (i));
	break;
    case ViewCount::TWO:
	displayView (ViewNumber::VIEW1);
    case ViewCount::ONE:
	displayView (ViewNumber::VIEW0);
    }
}

void GLWidget::displayView (ViewNumber::Enum viewNumber)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    vs.SetLightingParameters (getInitialLightPosition (vs.GetSelectedLight ()));
    viewportTransform (viewNumber);    
    projectionTransform (viewNumber);
    ModelViewTransform (viewNumber, GetTimeStep ());
    m_minimumEdgeRadius = getMinimumEdgeRadius ();
    calculateEdgeRadius (m_edgeRadiusRatio,
			 &m_edgeRadius, &m_arrowBaseRadius,
			 &m_arrowHeight, &m_edgeWidth);
    (this->*(m_viewTypeDisplay[vs.GetViewType ()])) (viewNumber);
    displayViewDecorations (viewNumber);
    displayAxes ();
    displayBoundingBox (viewNumber);
    displayOriginalDomain ();
    displayFocusBox (viewNumber);
    displayLightDirection (viewNumber);
    displayT1s (viewNumber);
    displayBodyNeighbors ();
    displayBodyDeformationTensor2D ();
    //displayContextMenuPos (viewNumber);
    WarnOnOpenGLError ("displayView");
}


void GLWidget::resizeGL(int w, int h)
{
    (void)w;(void)h;
    for (size_t i = 0; i < ViewCount::GetCount (m_viewCount); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	ViewSettings& vs = GetViewSettings (viewNumber);
	if (vs.GetViewType () == ViewType::FACES_STATISTICS)
	{
	    vs.GetDisplayFaceStatistics ().InitStep (viewNumber);
	    vs.GetDisplayForces ().InitStep (viewNumber);
	}
    }
    WarnOnOpenGLError ("resizeGl");
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
    const G3D::Rect2D& viewport = GetViewSettings (viewNumber).GetViewport ();
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
    ViewSettings& vs = GetViewSettings (viewNumber);
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
    ViewSettings& vs = GetViewSettings (viewNumber);
    double ratio = ratioFromCenter (position);
    if (vs.IsContextView ())
	vs.SetScaleRatio (vs.GetScaleRatio () / ratio);
    else
	vs.SetScaleRatio (vs.GetScaleRatio () * ratio);
}

void GLWidget::scaleContext (
    ViewNumber::Enum viewNumber, const QPoint& position)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    double ratio = ratioFromCenter (position);
    vs.SetContextScaleRatio (vs.GetContextScaleRatio () * ratio);
}

G3D::Vector3 GLWidget::brushedBodies (
    const QPoint& position, vector<size_t>* bodies) const
{
    vector< boost::shared_ptr<Body> > b;
    G3D::Vector3 op = brushedBodies (position, &b);
    bodies->resize (b.size ());
    transform (b.begin (), b.end (), bodies->begin (),
	       boost::bind (&Element::GetId, _1));
    return op;
}

G3D::Vector3 GLWidget::brushedBodies (
    const QPoint& position, vector< boost::shared_ptr<Body> >* bodies) const
{
    G3D::Vector3 op = toObjectTransform (position);
    const Foam& foam = GetCurrentFoam ();
    BOOST_FOREACH (boost::shared_ptr<Body> body, foam.GetBodies ())
    {
	G3D::AABox box = body->GetBoundingBox ();
	if (box.contains (op))
	    bodies->push_back (body);
    }
    return op;
}

G3D::Vector3 GLWidget::brushedFace (const OrientedFace** of) const
{
    vector< boost::shared_ptr<Body> > bodies;
    G3D::Vector3 op = brushedBodies (m_contextMenuPosScreen, &bodies);
    //cdbg << "point=" << op << endl;
    if (bodies.size () == 0)
	*of = 0;
    else
    {
	float minDistance = numeric_limits<float>::max ();
	BOOST_FOREACH (boost::shared_ptr<OrientedFace> face, 
		       bodies[0]->GetOrientedFaces ())
	{
	    G3D::Plane plane = face->GetPlane ();
	    float distance = plane.distance (op);
	    //cdbg << "distance=" << distance 
	    //<< " to " << face->GetStringId () << " " << plane << endl;
	    distance = abs (distance);
	    if ( minDistance > distance)
	    {
		minDistance = distance;
		*of = face.get ();
	    }
	}
	//cdbg << clickedFace->GetStringId () << endl << endl;	
    }
    return op;
}

OrientedEdge GLWidget::brushedEdge () const
{
    const OrientedFace* of = 0;
    G3D::Vector3 op = brushedFace (&of);
    OrientedEdge result;
    if (of != 0)
    {
	float minDistance = numeric_limits<float>::max ();
	for (size_t i = 0; i < of->size (); ++i)
	{
	    OrientedEdge oe = of->GetOrientedEdge (i);
	    G3D::Line line = G3D::Line::fromTwoPoints (
		oe.GetBeginVector (), oe.GetEndVector ());
	    float distance = line.distance (op);
	    if (minDistance > distance)
	    {
		minDistance = distance;
		result = oe;
	    }
	}
    }
    return result;
}

G3D::Vector3 GLWidget::toObject (const QPoint& position) const
{
    G3D::Vector3 op = gluUnProject (QtToOpenGl (position, height ()));
    if (GetFoamAlongTime ().Is2D ())
	op.z = 0;
    return op;
}


G3D::Vector3 GLWidget::toObjectTransform (const QPoint& position, 
					  ViewNumber::Enum viewNumber) const
{
    viewportTransform (viewNumber);    
    projectionTransform (viewNumber);
    ModelViewTransform (viewNumber, GetTimeStep ());
    return toObject (position);
}

G3D::Vector3 GLWidget::toObjectTransform (const QPoint& position) const
{
    return toObjectTransform (position, GetViewNumber ());
}

void GLWidget::displayAverageAroundBody (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    if (m_averageAroundBody && 
	vs.GetAverageAroundType () == ViewSettings::AVERAGE_AROUND_BODY)
    {
	glPushAttrib (GL_ENABLE_BIT);
	glDisable (GL_DEPTH_TEST);
	Foam::Bodies focusBody (1);
	focusBody[0] = *GetCurrentFoam ().FindBody (
	    vs.GetAverageAroundBodyId ());
	displayFacesContour<HighlightNumber::H0> (
	    focusBody, viewNumber, m_highlightLineWidth);
	glPopAttrib ();
    }
}

void GLWidget::displayAverageAroundConstraint (
    ViewNumber::Enum view,
    bool adjustForContextStationaryFoam) const
{
    const ViewSettings& vs = GetViewSettings (view);
    ViewSettings::AverageAroundType type = vs.GetAverageAroundType ();
    if (m_averageAroundBody && type == ViewSettings::AVERAGE_AROUND_CONSTRAINT)
    {
	glPushAttrib (GL_ENABLE_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
	if (adjustForContextStationaryFoam)
	{
	    glPushMatrix ();
	    rotateAverageAroundConstraint (GetTimeStep (), -1);
	}
	glDisable (GL_DEPTH_TEST);
	glLineWidth (m_highlightLineWidth);
	glColor (GetHighlightColor (view, HighlightNumber::H0));
	const Foam::Edges& constraintEdges = 
	    GetCurrentFoam ().GetConstraintEdges (
		GetFoamAlongTime ().
		GetConstraintRotationNames ().m_constraintIndex);
	DisplayEdgeHighlightColor<HighlightNumber::H0> displayEdge (
	    *this, DisplayElement::FOCUS, view);
	BOOST_FOREACH (boost::shared_ptr<Edge> edge, constraintEdges)
	    displayEdge (edge);
	if (adjustForContextStationaryFoam)
	    glPopMatrix ();
	glPopAttrib ();
    }
}


void GLWidget::displayContextBodies (ViewNumber::Enum viewNumber) const
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    if (vs.GetContextDisplayBodySize () > 0)
    {
	glPushAttrib (GL_ENABLE_BIT);
	glDisable (GL_DEPTH_TEST);
	const Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
	Foam::Bodies contextBodies (bodies.size ());
	

	size_t j = 0;
	for (size_t i = 0; i < bodies.size (); ++i)
	    if (vs.IsContextDisplayBody (bodies[i]->GetId ()))
		contextBodies[j++] = bodies[i];
	contextBodies.resize (j);
	displayFacesContour<HighlightNumber::H1> (
	    contextBodies, viewNumber, m_highlightLineWidth);
	glPopAttrib ();
    }
}

void GLWidget::displayContextStationaryFoam (
    ViewNumber::Enum viewNumber,
    bool adjustForContextStationaryFoam) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    ViewSettings::ContextStationaryType type = vs.GetContextStationaryType ();
    if (type == ViewSettings::CONTEXT_AVERAGE_AROUND_FOAM)
    {
	glPushAttrib (GL_ENABLE_BIT);
	glDisable (GL_DEPTH_TEST);
	if (adjustForContextStationaryFoam)
	{
	    glPushMatrix ();
	    rotateAverageAroundConstraint (GetTimeStep (), -1);
	}
	DisplayBox (GetFoamAlongTime (), 
		    GetHighlightColor (viewNumber, HighlightNumber::H1),
		    m_highlightLineWidth);
	if (adjustForContextStationaryFoam)
	    glPopMatrix ();
	glPopAttrib ();
    }
}


string GLWidget::getAverageAroundLabel ()
{
    ostringstream ostr;
    const ViewSettings& vs = GetViewSettings ();
    ViewSettings::AverageAroundType type = vs.GetAverageAroundType ();
    switch (type)
    {
    case ViewSettings::AVERAGE_AROUND_BODY:
	ostr << "Stationary body";
	break;
    case ViewSettings::AVERAGE_AROUND_CONSTRAINT:
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
    const ViewSettings& vs = GetViewSettings ();
    size_t count = vs.GetContextDisplayBodySize ();
    if (count != 0)
	ostr << "Context (" << count << ")";
    return ostr.str ();
}

string GLWidget::getContextStationaryLabel ()
{
    ostringstream ostr;
    const ViewSettings& vs = GetViewSettings ();
    ViewSettings::ContextStationaryType type = vs.GetContextStationaryType ();
    if (type == ViewSettings::CONTEXT_AVERAGE_AROUND_FOAM)
	ostr << "Context stationary foam";
    return ostr.str ();
}


string GLWidget::getBodySelectorLabel ()
{
    const BodySelector& bodySelector = GetViewSettings ().GetBodySelector ();
    BodySelectorType::Enum type = bodySelector.GetType ();
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
    boost::array<string, 4> labels = {{
	    getAverageAroundLabel (),
	    getContextLabel (),
	    getContextStationaryLabel (),
	    getBodySelectorLabel ()
	}};
    ostream_iterator<string> o (ostr, "-");
    remove_copy_if (labels.begin (), labels.end (), o,
		    boost::bind (&string::empty, _1));
    string label;
    if (ostr.str ().empty ())
	label = "Ready";
    else
	label = "-" + ostr.str ();
    m_labelStatusBar->setText (QString (label.c_str ()));
}


void GLWidget::AverageAroundBody ()
{
    ViewSettings& vs = GetViewSettings ();
    vector<size_t> bodies;
    brushedBodies (m_contextMenuPosScreen, &bodies);
    if (bodies.size () != 0)
    {
	vs.SetAverageAroundType (ViewSettings::AVERAGE_AROUND_BODY);
	vs.SetAverageAroundBodyId (bodies[0]);
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

void GLWidget::AverageAroundConstraint ()
{
    ViewSettings& vs = GetViewSettings ();
    if (GetFoamAlongTime ().GetConstraintRotationNames ().RotationUsed ())
    {
	vs.SetAverageAroundType (ViewSettings::AVERAGE_AROUND_CONSTRAINT);
	vs.SetAverageAroundBodyId (INVALID_INDEX);
	setLabel ();
	update ();
    }
    else
    {
	QMessageBox msgBox (this);
	msgBox.setText("Use --constraint-rotation command line option.");
	msgBox.exec();	
    }
}

void GLWidget::AverageAroundReset ()
{
    ViewSettings& vs = GetViewSettings ();
    vs.SetAverageAroundType (ViewSettings::AVERAGE_AROUND_NONE);
    vs.SetAverageAroundBodyId (INVALID_INDEX);
    setLabel ();
    update ();
}

void GLWidget::ContextDisplayBody ()
{
    ViewSettings& vs = GetViewSettings ();
    vector<size_t> bodies;
    brushedBodies (m_contextMenuPosScreen, &bodies);
    vs.AddContextDisplayBody (bodies[0]);
    setLabel ();
    update ();
}

void GLWidget::ContextDisplayReset ()
{
    ViewSettings& vs = GetViewSettings ();
    vs.ContextDisplayReset ();
    setLabel ();
    update ();
}

void GLWidget::ContextStationaryFoam ()
{
    ViewSettings& vs = GetViewSettings ();
    vs.SetContextStationaryType (ViewSettings::CONTEXT_AVERAGE_AROUND_FOAM);
    setLabel ();
    update ();
}

void GLWidget::ContextStationaryReset ()
{
    ViewSettings& vs = GetViewSettings ();
    vs.SetContextStationaryType (ViewSettings::CONTEXT_AVERAGE_AROUND_NONE);
    setLabel ();
    update ();
}

void GLWidget::select (const QPoint& position)
{
    vector<size_t> bodyIds;
    brushedBodies (position, &bodyIds);
    GetViewSettings ().UnionBodySelector (bodyIds);
    labelCompileUpdate ();
}

void GLWidget::deselect (const QPoint& position)
{
    vector<size_t> bodyIds;
    brushedBodies (position, &bodyIds);
    GetViewSettings ().DifferenceBodySelector (GetCurrentFoam (), bodyIds);
    labelCompileUpdate ();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    switch (m_interactionMode)
    {
    case InteractionMode::ROTATE:
    {
	ViewSettings& vs = GetViewSettings ();
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
	ViewSettings& vs = GetViewSettings ();
	LightNumber::Enum i = vs.GetSelectedLight ();
	vs.SetRotationLight (
	    i, 
	    rotate (GetViewNumber (), event->pos (), vs.GetRotationLight (i)));
	vs.PositionLight (i, getInitialLightPosition (i));
	break;
    }
    case InteractionMode::TRANSLATE_LIGHT:
    {
	ViewSettings& vs = GetViewSettings ();
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
    const ViewSettings& vs = GetViewSettings (viewNumber);
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
		    m_highlightLineWidth);
	glPopMatrix ();
    }
}

void GLWidget::displayBoundingBox (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT);
    if (vs.IsLightingEnabled ())
	glDisable (GL_LIGHTING);
    if (m_boundingBoxShown)
	DisplayBox (GetFoamAlongTime (), Qt::black);
    if (m_bodiesBoundingBoxesShown)
    {
	const Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
	const BodySelector& bodySelector = 
	    GetViewSettings (viewNumber).GetBodySelector ();
	BOOST_FOREACH (boost::shared_ptr<Body> body, bodies)
	    if (bodySelector (body))
		DisplayBox (body, Qt::black);
    }
    glPopAttrib ();
}

void GLWidget::displayAxes ()
{
    if (m_axesShown)
    {
	QFont font;
	float a;
	QFontMetrics fm (font);
	ostringstream ostr;
	ostr << setprecision (4);
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

	a = fm.height () * m_minimumEdgeRadius;
	glColor (Qt::red);
	displayOrientedEdge (origin, first);
	glColor (Qt::black);
	ostr.str ("");
	ostr  << first.x;
	renderText (first.x + a, first.y - a, first.z, ostr.str ().c_str ());
	ostr.str ("");ostr  << origin.x;
	renderText (origin.x + a, origin.y - a, origin.z, ostr.str ().c_str ());

	glColor (Qt::green);
	displayOrientedEdge (origin, second);
	glColor (Qt::black);
	ostr.str ("");ostr << second.y;
	renderText (second.x, second.y + a, second.z - a, ostr.str ().c_str ());
	ostr.str ("");ostr  << origin.y;
	renderText (origin.x, origin.y + a, origin.z - a, ostr.str ().c_str ());


	glColor (Qt::blue);
	displayOrientedEdge (origin, third);
	glColor (Qt::black);
	ostr.str ("");ostr << third.z;
	renderText (third.x - a, third.y, third.z + a, ostr.str ().c_str ());
	ostr.str ("");ostr << origin.z;
	renderText (origin.x - a, origin.y, origin.z + a, ostr.str ().c_str ());
	glPopAttrib ();
    }
}


template<typename displayEdge>
void GLWidget::displayEdges (ViewNumber::Enum viewNumber) const
{
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);
    const BodySelector& bodySelector = 
	GetViewSettings (viewNumber).GetBodySelector ();
    const Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<
	      DisplayFaceHighlightColor<HighlightNumber::H0,
	      DisplayFaceEdges<displayEdge> > > (*this, bodySelector));
    displayStandaloneEdges<displayEdge> ();

    glPopAttrib ();
    displayBodyCenters (viewNumber);
    displayFaceCenters (viewNumber);
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

void GLWidget::displayEdgesNormal (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);    
    glPushAttrib (GL_ENABLE_BIT);
    if (vs.IsLightingEnabled ())
	glDisable (GL_LIGHTING);
    m_torusOriginalDomainClipped ?
	displayEdges <DisplayEdgeTorusClipped> (viewNumber) :
	displayEdges <DisplayEdgePropertyColor<> >(viewNumber);
    glPopAttrib ();
}

void GLWidget::displayT1s (ViewNumber::Enum view) const
{
    if (m_t1sShown)
    {
	if (ViewType::IsGlobal (GetViewSettings ().GetViewType ()))
	    displayT1sGlobal (view);
	else
	    displayT1s (view, GetTimeStep ());
    }
}

void GLWidget::displayDeformationTensor2D (ViewNumber::Enum viewNumber) const
{
    const Foam& foam = GetCurrentFoam ();
    if (! foam.Is2D () || ! m_showDeformationTensor || 
	viewNumber != GetViewNumber ())
	return;
    Foam::Bodies bodies = foam.GetBodies ();
    glPushAttrib (GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable (GL_DEPTH_TEST);
    glColor (Qt::black);
    for_each (bodies.begin (), bodies.end (),
	      boost::bind (
		  ::displayBodyDeformationTensor2D, _1, 
		  m_deformationTensorSize));
    glPopAttrib ();    
}

void GLWidget::displayBodyDeformationTensor2D () const
{
    const Foam& foam = GetCurrentFoam ();
    if (! foam.Is2D () || m_showType != SHOW_DEFORMATION_TENSOR)
	return;
    glPushAttrib (GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable (GL_DEPTH_TEST);
    glColor (Qt::black);
    ::displayBodyDeformationTensor2D (
	*foam.FindBody (m_showBodyId), m_deformationTensorSize);
    glPopAttrib ();
}


void GLWidget::displayBodyNeighbors () const
{
    if (m_showType != SHOW_NEIGHBORS)
	return;
    glPushAttrib (GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable (GL_DEPTH_TEST);
    glColor (Qt::black);
    glBegin (GL_LINES);

    const Foam& foam = GetCurrentFoam ();
    const OOBox& originalDomain = foam.GetOriginalDomain ();
    Foam::Bodies::const_iterator showBody = foam.FindBody (m_showBodyId);
    BOOST_FOREACH (Body::Neighbor neighbor, (*showBody)->GetNeighbors ())
    {
	if (! neighbor.m_body)
	    continue;
	G3D::Vector3 first = (*showBody)->GetCenter ();	    
	G3D::Vector3 second = originalDomain.TorusTranslate (
	    neighbor.m_body->GetCenter (), neighbor.m_translation);
	::glVertex (first);
	::glVertex (second);
    }

    glEnd ();
    glPopAttrib ();
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
	GetViewSettings (viewNumber).GetColorBarModel ();
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

void GLWidget::displayEdgesTorus (ViewNumber::Enum viewNumber) const
{
    if (m_edgeRadiusRatio > 0)
	displayEdgesTorusTubes ();
    else
    {
	displayEdgesTorusLines ();
	displayBodyCenters (viewNumber);
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
	displayBodyCenters (view);
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
	      DisplayEdgeTorus<DisplaySegment, 
	      DisplaySegmentArrow, false> (*this));
    glPopAttrib ();
}


void GLWidget::displayBodyCenters (
    ViewNumber::Enum viewNumber, bool useZPos) const
{
    if (m_bodyCenterShown)
    {
	const BodySelector& bodySelector = GetViewSettings (viewNumber).
	    GetBodySelector ();
	double zPos = (GetViewSettings ().GetViewType () == 
		       ViewType::CENTER_PATHS) ?
	    GetTimeStep () * GetTimeDisplacement () : 0;
	glPushAttrib (GL_POINT_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
	glDisable (GL_DEPTH_TEST);
	glPointSize (4.0);
	glColor (Qt::red);
	const Foam::Bodies& bodies = GetCurrentFoam ().GetBodies ();
	for_each (bodies.begin (), bodies.end (),
		  DisplayBodyCenter (*this, bodySelector, useZPos, zPos));
	glPopAttrib ();
    }
}

void GLWidget::displayFaceCenters (ViewNumber::Enum viewNumber) const
{
    (void)viewNumber;
    if (m_faceCenterShown)
    {
	FaceSet faces = 
	    GetFoamAlongTime ().GetFoam (GetTimeStep ()).GetFaceSet ();
	glPushAttrib (GL_POINT_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
	glDisable (GL_DEPTH_TEST);
	glPointSize (4.0);
	glColor (Qt::red);
	glBegin (GL_POINTS);
	BOOST_FOREACH (boost::shared_ptr<Face> face, faces)
	    ::glVertex (face->GetCenter ());
	glEnd ();
	glPopAttrib ();
    }
}


void GLWidget::displayContextMenuPos (ViewNumber::Enum viewNumber) const
{
    (void)viewNumber;
    glPushAttrib (GL_POINT_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
    glDisable (GL_DEPTH_TEST);
    glPointSize (4.0);
    glColor (Qt::red);
    glBegin (GL_POINTS);
    ::glVertex (m_contextMenuPosObject);
    glEnd ();
    glPopAttrib ();
}


void GLWidget::displayFacesNormal (ViewNumber::Enum viewNumber) const
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    const Foam& foam = GetCurrentFoam ();
    const Foam::Bodies& bodies = foam.GetBodies ();
    if (m_facesShowEdges)
	displayFacesContour (bodies, viewNumber);
    displayFacesInterior (bodies, viewNumber);
    displayStandaloneEdges< DisplayEdgePropertyColor<> > ();
    displayAverageAroundBody (viewNumber);
    displayAverageAroundConstraint (viewNumber);
    displayContextBodies (viewNumber);
    displayContextStationaryFoam (viewNumber);
    displayStandaloneFaces ();    
    displayBodyCenters (viewNumber);
    displayFaceCenters (viewNumber);
    displayDeformationTensor2D (viewNumber);
    vs.GetDisplayForces ().Display (viewNumber);
}


void GLWidget::displayFacesStatistics (ViewNumber::Enum viewNumber) const
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    glPushAttrib (GL_ENABLE_BIT);    
    glDisable (GL_DEPTH_TEST);
    glBindTexture (GL_TEXTURE_1D, vs.GetColorBarTexture ());
    bool adjustForContextStationaryFoam = 
	(GetViewSettings (viewNumber).GetContextStationaryType () == 
	 ViewSettings::CONTEXT_AVERAGE_AROUND_FOAM);
    if (adjustForContextStationaryFoam)
    {
	const ConstraintRotation& rotationBegin = GetFoamAlongTime ().
	    GetFoam (0).GetConstraintRotation ();
	const ConstraintRotation& rotationCurrent = GetFoamAlongTime ().
	    GetFoam (GetTimeStep ()).GetConstraintRotation ();
	G3D::Vector3 rc = G3D::Vector3 (rotationBegin.m_center, 0.0);
	G3D::Vector2 rotationCenter = gluProject (rc).xy ();
	rotationCenter -= vs.GetViewport ().x0y0 ();
	float angleDegrees = G3D::toDegrees (
	    rotationCurrent.m_angle - rotationBegin.m_angle);
	vs.GetDisplayFaceStatistics ().DisplayAndRotate (
	    viewNumber, vs.GetStatisticsType (),
	    rotationCenter, - angleDegrees);
    }
    else
    {
	vs.GetDisplayFaceStatistics ().Display (
	    viewNumber, vs.GetStatisticsType ());
    }
    displayStandaloneEdges< DisplayEdgePropertyColor<> > ();
    displayAverageAroundBody (viewNumber);
    displayAverageAroundConstraint (viewNumber, adjustForContextStationaryFoam);
    displayContextBodies (viewNumber);
    displayContextStationaryFoam (viewNumber, adjustForContextStationaryFoam);
    vs.GetDisplayForces ().DisplayAverage (viewNumber);
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
	      DisplayFaceLineStripColor<0xff000000> (*this));
    glPopAttrib ();
}

void GLWidget::displayFacesContour (
    const Foam::Bodies& bodies, ViewNumber::Enum viewNumber) const
{
    const BodySelector& bodySelector = 
	GetViewSettings (viewNumber).GetBodySelector ();
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT);
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody< DisplayFaceLineStripColor<0xff000000> > (
		  *this, bodySelector));
    glPopAttrib ();
}


template<HighlightNumber::Enum highlightColorIndex>
void GLWidget::displayFacesContour (
    const Foam::Bodies& bodies, ViewNumber::Enum viewNumber, 
    GLfloat lineWidth) const
{
    const BodySelector& bodySelector = 
	GetViewSettings (viewNumber).GetBodySelector ();
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LINE_BIT);
    glLineWidth (lineWidth);
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody< DisplayFaceHighlightColor<highlightColorIndex, 
	      DisplayFaceLineStrip>,
	      SetterValueTextureCoordinate> (
		  *this, bodySelector, SetterValueTextureCoordinate (
		      *this, viewNumber)));
    glPopAttrib ();
}


// See OpenGL Programming Guide, 7th edition, Chapter 6: Blending,
// Antialiasing, Fog and Polygon Offset page 293
void GLWidget::displayFacesInterior (
    const Foam::Bodies& bodies, ViewNumber::Enum view) const
{
    const BodySelector& bodySelector = 
	GetViewSettings (view).GetBodySelector ();
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | 
		  GL_TEXTURE_BIT);
    glPolygonMode (GL_FRONT, GL_FILL);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1, 1);

    glEnable (GL_STENCIL_TEST);
    glClear(GL_STENCIL_BUFFER_BIT);

    glEnable(GL_TEXTURE_1D);
    //See OpenGL FAQ 21.030 Why doesn't lighting work when I turn on 
    //texture mapping?
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture (GL_TEXTURE_1D, 
		   GetViewSettings (view).GetColorBarTexture ());
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<DisplayFaceBodyPropertyColor<> > (
		  *this, bodySelector, 
		  DisplayElement::USER_DEFINED_CONTEXT, view));
    glPopAttrib ();
}


void GLWidget::displayFacesInterior (const Foam::Faces& faces) const
{
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1, 1);
    for_each (faces.begin (), faces.end (),
	      DisplayFaceDmpColor<0xff000000>(*this));
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
	DisplayEdgeTorus<DisplaySegmentQuadric, 
	                 DisplaySegmentArrowQuadric, true> > > (*this));
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
    const ViewSettings& vs = GetViewSettings (view);
    const BodySelector& bodySelector = vs.GetBodySelector ();
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
		*this, bodySelector, DisplayElement::USER_DEFINED_CONTEXT,
		view, IsTimeDisplacementUsed (), zPos));
	displayBodyCenters (view, IsTimeDisplacementUsed ());
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
    glCallList (GetViewSettings (view).GetListCenterPaths ());
}

void GLWidget::labelCompileUpdate ()
{
    setLabel ();
    compile (GetViewNumber ());
    update ();
}

void GLWidget::compile (ViewNumber::Enum view) const
{
    switch (GetViewSettings (view).GetViewType ())
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
    const ViewSettings& vs = GetViewSettings (view);
    const BodySelector& bodySelector = vs.GetBodySelector ();
    glNewList (vs.GetListCenterPaths (), GL_COMPILE);
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT | 
		  GL_POLYGON_BIT | GL_LINE_BIT);
    glEnable(GL_TEXTURE_1D);
    glBindTexture (GL_TEXTURE_1D, vs.GetColorBarTexture ());
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
		    *this, m_viewNumber, bodySelector,
		    IsTimeDisplacementUsed (), GetTimeDisplacement ()));
	else
	    for_each (
		bats.begin (), bats.end (),
		DisplayCenterPath<
		SetterValueTextureCoordinate, DisplaySegmentQuadric> (
		    *this, m_viewNumber, bodySelector,
		    IsTimeDisplacementUsed (), GetTimeDisplacement ()));
    }
    else
	for_each (bats.begin (), bats.end (),
		  DisplayCenterPath<SetterValueTextureCoordinate, 
		  DisplaySegment> (
		      *this, m_viewNumber, bodySelector,
		      IsTimeDisplacementUsed (), GetTimeDisplacement ()));
    glPopAttrib ();
    glEndList ();
}


const Foam& GLWidget::GetCurrentFoam () const
{
    return GetFoamAlongTime ().GetFoam (m_timeStep);
}

Foam& GLWidget::GetCurrentFoam ()
{
    return GetFoamAlongTime ().GetFoam (m_timeStep);
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


void GLWidget::setLight (int sliderValue, int maximumValue, 
			 LightType::Enum lightType, 
			 ColorNumber::Enum colorNumber)
{
    ViewSettings& vs = GetViewSettings ();
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
    ViewSettings& vs = GetViewSettings (view);
    switch (vs.GetViewType ())
    {
    case ViewType::FACES:
    case ViewType::FACES_STATISTICS:
    case ViewType::CENTER_PATHS:
	return vs.GetBodyProperty () != BodyProperty::NONE;
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
    m_contextMenuPosScreen = event->pos ();
    m_contextMenuPosObject = toObjectTransform (m_contextMenuPosScreen);
    QMenu menu (this);
    G3D::Rect2D colorBarRect = getViewColorBarRect (GetViewRect ());
    if (colorBarRect.contains (QtToOpenGl (m_contextMenuPosScreen, height ())))
    {
	QMenu* menuCopy = menu.addMenu ("Copy");
	bool actions = false;
	if (ViewCount::GetCount (m_viewCount) > 1)
	{
	    BodyProperty::Enum currentBodyProperty = 
		GetViewSettings ().GetBodyProperty ();
	    for (size_t i = 0; i < ViewCount::GetCount (m_viewCount); ++i)
	    {
		ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
		if (viewNumber == m_viewNumber ||
		    currentBodyProperty != 
		    GetViewSettings (viewNumber).GetBodyProperty ())
		    continue;
		menuCopy->addAction (m_actionCopyColorMap[i].get ());
		actions = true;
	    }
	}
	if (! actions)
	    menu.clear ();
	menu.addAction (m_actionEditColorMap.get ());
	menu.addAction (m_actionClampClear.get ());

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
	    QMenu* menuStationary = menu.addMenu ("Average around");
	    menuStationary->addAction (m_actionAverageAroundBody.get ());
	    menuStationary->addAction (m_actionAverageAroundConstraint.get ());
	    menuStationary->addAction (m_actionAverageAroundReset.get ());
	}
	{
	    QMenu* menuContext = menu.addMenu ("Context");
	    QMenu* menuContextDisplay = menuContext->addMenu ("Display");
	    menuContextDisplay->addAction (m_actionContextDisplayBody.get ());
	    menuContextDisplay->addAction (m_actionContextDisplayReset.get ());

	    QMenu* menuContextStationary = menuContext->addMenu ("Stationary");
	    menuContextStationary->addAction (
		m_actionContextStationaryFoam.get ());
	    menuContextStationary->addAction (
		m_actionContextStationaryReset.get ());
	}
	{
	    QMenu* menuInfo = menu.addMenu ("Info");
	    menuInfo->addAction (m_actionInfoPoint.get ());
	    menuInfo->addAction (m_actionInfoEdge.get ());
	    menuInfo->addAction (m_actionInfoFace.get ());
	    menuInfo->addAction (m_actionInfoBody.get ());
	    menuInfo->addAction (m_actionInfoFoam.get ());
	    menuInfo->addAction (m_actionInfoOpenGL.get ());
	}
	{
	    QMenu* menuShow = menu.addMenu ("Show");
	    menuShow->addAction (m_actionShowNeighbors.get ());
	    menuShow->addAction (m_actionShowTextureTensor.get ());
	    menuShow->addAction (m_actionShowReset.get ());
	}

	if (ViewCount::GetCount (m_viewCount) > 1)
	{
	    QMenu* menuCopy = menu.addMenu ("Copy");
	    QMenu* menuTransformation = menuCopy->addMenu ("Transformation");
	    QMenu* menuSelection = menuCopy->addMenu ("Selection");
	    for (size_t i = 0; i < ViewCount::GetCount (m_viewCount); ++i)
	    {
		ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
		if (viewNumber == m_viewNumber)
		    continue;
		menuTransformation->addAction (
		    m_actionCopyTransformation[i].get ());
		menuSelection->addAction (m_actionCopySelection[i].get ());
	    }
	}
    }
    menu.exec (event->globalPos());
}


void GLWidget::displayViewDecorations (ViewNumber::Enum view)
{
    const ViewSettings& vs = GetViewSettings (view);
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
    ViewSettings& vs = GetViewSettings (viewNumber);
    ostringstream ostr;
    ostr << "View " << viewNumber << " - " <<
	(isColorBarUsed (viewNumber) ? 
	 BodyProperty::ToString (vs.GetBodyProperty ()) :
	 ViewType::ToString (vs.GetViewType ()));
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
		   GetViewSettings (viewNumber).GetColorBarTexture ());
    
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glBegin (GL_QUADS);
    glTexCoord1f(0);::glVertex (colorBarRect.x0y0 ());
    glTexCoord1f(1);::glVertex (colorBarRect.x0y1 ());
    glTexCoord1f(1);::glVertex (colorBarRect.x1y1 ());
    glTexCoord1f(0);::glVertex (colorBarRect.x1y0 ());
    glEnd ();
    glDisable (GL_TEXTURE_1D);

    glColor (Qt::black);
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    glBegin (GL_QUADS);
    ::glVertex (colorBarRect.x0y0 ());
    ::glVertex (colorBarRect.x0y1 ());
    ::glVertex (colorBarRect.x1y1 ());
    ::glVertex (colorBarRect.x1y0 ());
    glEnd ();
}

void GLWidget::displayViewGrid ()
{
    size_t w = width ();
    size_t h = height ();
    glColor (Qt::blue);
    glBegin (GL_LINES);
    switch (m_viewCount)
    {
    case ViewCount::TWO:
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
	break;
    }
    case ViewCount::THREE:
    {
	if (m_viewLayout == ViewLayout::HORIZONTAL)
	{
	    glVertex2s (w/3, 0);
	    glVertex2s (w/3, h);
	    glVertex2s (2*w/3, 0);
	    glVertex2s (2*w/3, h);
	}
	else
	{
	    glVertex2s (0, h/3);
	    glVertex2s (w, h/3);
	    glVertex2s (0, 2*h/3);
	    glVertex2s (w, 2*h/3);
	}
	break;
    }
    case ViewCount::FOUR:
    {
	glVertex2s (w/2, 0);
	glVertex2s (w/2, h);
	glVertex2s (0, h/2);
	glVertex2s (w, h/2);	
	break;
    }
    default:
	break;
    }
    glEnd ();
}

QColor GLWidget::GetCenterPathContextColor () const
{
    QColor returnColor (Qt::black);
    returnColor.setAlphaF (GetContextAlpha ());
    return returnColor;
}

bool GLWidget::IsTimeDisplacementUsed () const
{
    return GetTimeDisplacement () > 0;
}

BodyProperty::Enum GLWidget::GetBodyProperty (ViewNumber::Enum viewNumber) const
{
    return GetViewSettings (viewNumber).GetBodyProperty ();
}

void GLWidget::valueChanged (
    double* dest, const pair<double,double>& minMax, int index)
{
    QSlider* slider = static_cast<QSlider*> (sender ());
    size_t maxSlider = slider->maximum ();
    *dest = minMax.first + (double (index) / maxSlider) * 
	(minMax.second - minMax.first);
}

bool GLWidget::IsMissingPropertyShown (BodyProperty::Enum bodyProperty) const
{
    switch (bodyProperty)
    {
    case BodyProperty::PRESSURE:
	return m_missingPressureShown;
    case BodyProperty::TARGET_VOLUME:
    case BodyProperty::ACTUAL_VOLUME:
	return m_missingVolumeShown;
    default:
	return true;
    }
}


// Slots
// ======================================================================

void GLWidget::CopyTransformationFrom (int viewNumber)
{
    GetViewSettings ().CopyTransformation (
	GetViewSettings (ViewNumber::Enum (viewNumber)));
    update ();
}

void GLWidget::CopySelectionFrom (int viewNumber)
{
    GetViewSettings ().CopySelection (
	GetViewSettings (ViewNumber::Enum (viewNumber)));
}

void GLWidget::CopyColorBarFrom (int viewNumber)
{
    ViewSettings& vs = GetViewSettings (ViewNumber::Enum (viewNumber));
    GetViewSettings ().CopyColorBar (vs);
    Q_EMIT ColorBarModelChanged (GetViewSettings ().GetColorBarModel ());
}


void GLWidget::ToggledDirectionalLightEnabled (bool checked)
{
    ViewSettings& vs = GetViewSettings ();
    LightNumber::Enum selectedLight = vs.GetSelectedLight ();
    vs.SetDirectionalLightEnabled (selectedLight, checked);
    vs.PositionLight (selectedLight, getInitialLightPosition (selectedLight));
    update ();
}

void GLWidget::ToggledShowDeformationTensor (bool checked)
{
    m_showDeformationTensor = checked;
    update ();
}


void GLWidget::ToggledMissingPressureShown (bool checked)
{
    m_missingPressureShown = checked;
    update ();
}

void GLWidget::ToggledMissingVolumeShown (bool checked)
{
    m_missingVolumeShown = checked;
    update ();
}


void GLWidget::ToggledLightNumberShown (bool checked)
{
    ViewSettings& vs = GetViewSettings ();
    vs.SetLightPositionShown (vs.GetSelectedLight (), checked);
    update ();
}

void GLWidget::ToggledLightEnabled (bool checked)
{
    makeCurrent ();
    ViewSettings& vs = GetViewSettings ();
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

void GLWidget::ToggledAverageAroundBody (bool checked)
{
    m_averageAroundBody = checked;
    update ();
}

void GLWidget::ToggledBodiesBoundingBoxesShown (bool checked)
{
    m_bodiesBoundingBoxesShown = checked;
    update ();
}

void GLWidget::ToggledContextView (bool checked)
{
    ViewSettings& vs = GetViewSettings ();
    vs.SetContextView (checked);
    update ();
}

void GLWidget::ToggledForceNetworkShown (bool checked)
{
    GetViewSettings ().SetForceNetworkShown (checked);
    update ();
}

void GLWidget::ToggledForcePressureShown (bool checked)
{
    GetViewSettings ().SetForcePressureShown (checked);
    update ();
}

void GLWidget::ToggledForceResultShown (bool checked)
{
    GetViewSettings ().SetForceResultShown (checked);
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

void GLWidget::ToggledContextHidden (bool checked)
{
    GetViewSettings ().SetContextHidden (checked);
    compile (GetViewNumber ());
    update ();
}

void GLWidget::ToggledCenterPathHidden (bool checked)
{
    GetViewSettings ().SetCenterPathHidden (checked);
    compile (GetViewNumber ());
    update ();
}

void GLWidget::ButtonClickedViewType (int id)
{
    ViewType::Enum newViewType = ViewType::Enum(id);
    ViewType::Enum oldViewType = GetViewSettings ().GetViewType ();
    if (oldViewType == newViewType)
	return;
    if (newViewType == ViewType::FACES_STATISTICS)
    {
	ViewNumber::Enum vn = GetViewNumber ();
	GetViewSettings ().GetDisplayFaceStatistics ().InitStep (vn);
	GetViewSettings ().GetDisplayForces ().InitStep (vn);
    }
    if (oldViewType == ViewType::FACES_STATISTICS)
	GetViewSettings ().GetDisplayFaceStatistics ().Release ();
    changeViewType (true, newViewType);
}

void GLWidget::ToggledBodyCenterShown (bool checked)
{
    m_bodyCenterShown = checked;
    update ();
}

void GLWidget::ToggledFaceCenterShown (bool checked)
{
    m_faceCenterShown = checked;
    update ();
}


void GLWidget::ToggledFacesShowEdges (bool checked)
{
    m_facesShowEdges = checked;
    update ();
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

void GLWidget::CurrentIndexChangedSelectedLight (int selectedLight)
{
    ViewSettings& vs = GetViewSettings ();
    vs.SetSelectedLight (LightNumber::Enum (selectedLight));
}

void GLWidget::CurrentIndexChangedViewCount (int index)
{
    m_viewCount = ViewCount::Enum (index);
    m_viewNumber = ViewNumber::VIEW0;
    for (size_t i = 0; i < ViewCount::GetCount (m_viewCount); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	G3D::Rect2D viewRect = GetViewRect (viewNumber);
	ViewSettings& vs = GetViewSettings (viewNumber);
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

void GLWidget::CurrentIndexChangedStatisticsType (int index)
{
    GetViewSettings ().SetStatisticsType (StatisticsType::Enum(index));
    update ();
}

void GLWidget::CurrentIndexChangedAxesOrder (int index)
{
    GetViewSettings ().SetAxesOrder (AxesOrder::Enum(index));
}

// @todo add a color bar model for BodyProperty::None
void GLWidget::SetBodyProperty (
    boost::shared_ptr<ColorBarModel> colorBarModel,
    BodyProperty::Enum property)
{
    makeCurrent ();
    ViewNumber::Enum view = GetViewNumber ();
    ViewSettings& vs = GetViewSettings ();
    vs.SetBodyProperty (property);
    if (vs.GetBodyProperty () != BodyProperty::NONE)
	GetViewSettings ().SetColorBarModel (colorBarModel);
    else
	vs.ResetColorBarModel ();
    compile (view);
    update ();
}

void GLWidget::SetColorBarModel (boost::shared_ptr<ColorBarModel> colorBarModel)
{
    makeCurrent ();
    GetViewSettings ().SetColorBarModel (colorBarModel);
    update ();
}

void GLWidget::ValueChangedSliderTimeSteps (int timeStep)
{
    makeCurrent ();
    int direction = timeStep - m_timeStep;
    m_timeStep = timeStep;
    for (size_t i = 0; i < ViewCount::GetCount (m_viewCount); ++i)
    {
	ViewNumber::Enum view = ViewNumber::Enum (i);
	ViewSettings& vs = GetViewSettings (view);
	if (vs.GetViewType () == ViewType::FACES_STATISTICS)
	{
	    vs.GetDisplayFaceStatistics ().Step (view, direction);
	    vs.GetDisplayForces ().Step (view, direction);
	}
    }
    update ();
}

void GLWidget::ValueChangedStatisticsTimeWindow (int timeSteps)
{
    GetViewSettings ().GetDisplayFaceStatistics ().SetTimeWindow (timeSteps);
    GetViewSettings ().GetDisplayForces ().SetTimeWindow (timeSteps);
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
    valueChanged (&m_t1Size, T1_SIZE, index);
    update ();
}

void GLWidget::ValueChangedTensorSize (int index)
{
    valueChanged (&m_deformationTensorSize, DEFORMATION_TENSOR_SIZE, index);
    update ();
}


void GLWidget::ValueChangedContextAlpha (int index)
{
    valueChanged (&m_contextAlpha, CONTEXT_ALPHA, index);
    compile (GetViewNumber ());
    update ();
}

void GLWidget::ValueChangedForceLength (int index)
{
    valueChanged (&m_forceLength, FORCE_LENGTH, index);
    update ();
}


void GLWidget::ValueChangedHighlightLineWidth (int newWidth)
{
    m_highlightLineWidth = newWidth;
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
    ViewSettings& vs = GetViewSettings ();
    vs.SetAngleOfView (angleOfView);
    vs.CalculateCameraDistance (
	calculateCenteredViewingVolume (
	    viewRect.width () / viewRect.height (), vs.GetScaleRatio ()));
    update ();
}



