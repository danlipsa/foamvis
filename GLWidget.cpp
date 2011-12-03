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
#include "ScalarAverage.h"
#include "ForceAverage.h"
#include "DisplayBodyFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "DisplayFaceFunctors.h"
#include "DisplayVertexFunctors.h"
#include "Edge.h"
#include "Face.h"
#include "Foam.h"
#include "Simulation.h"
#include "GLWidget.h"
#include "Info.h"
#include "OpenGLUtils.h"
#include "OrientedEdge.h"
#include "OrientedFace.h"
#include "SelectBodiesById.h"
#include "T1sPDE.h"
#include "TensorAverage.h"
#include "Utils.h"
#include "Vertex.h"
#include "ViewSettings.h"
#include "VectorAverage.h"

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
    G3D::AABox result (low, high);
    return result;
}

G3D::AABox ExtendMaxXY (const G3D::AABox& box)
{
    G3D::Vector3 center = box.center ();
    G3D::Vector3 extent = box.extent ();
    G3D::Vector3 low = box.low (), high = box.high ();
    float length = max (extent.x, extent.y) / 2.0;
    G3D::AABox result (
	G3D::Vector3 (center.x - length, center.y - length, low.z),
	G3D::Vector3 (center.x + length, center.y + length, high.z));
    return result;
}


G3D::AABox ExtendAlongZFor3D (
    const G3D::AABox& b, double scaleRatio)
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

void displayBodyNeighbors2D (boost::shared_ptr<Body> body, 
			     const OOBox& originalDomain)
{
    if (body->IsConstraint ())
	return;
    BOOST_FOREACH (Body::Neighbor neighbor, body->GetNeighbors ())
    {
	G3D::Vector3 s;
	if (neighbor.m_body)
	{
	    s = neighbor.m_body->GetCenter ();
	}
	else
	{
	    // debug
	    // no reflection displayed
	    // continue;
	    s = neighbor.m_centerReflection;
	}
	G3D::Vector3 first = body->GetCenter ();	    
	G3D::Vector3 second = 
	    originalDomain.TorusTranslate (s, neighbor.m_translation);
	::glVertex (first);
	::glVertex (second);
    }
}

void sendQuad (const G3D::Rect2D& srcRect, const G3D::Rect2D& srcTexRect)
{
    glTexCoord (srcTexRect.x0y0 ());
    ::glVertex (srcRect.x0y0 ());
    glTexCoord (srcTexRect.x1y0 ());
    ::glVertex (srcRect.x1y0 ());
    glTexCoord (srcTexRect.x1y1 ());
    ::glVertex (srcRect.x1y1 ());
    glTexCoord (srcTexRect.x0y1 ());
    ::glVertex (srcRect.x0y1 ());
}



// Static Fields
// ======================================================================

const size_t GLWidget::DISPLAY_ALL(numeric_limits<size_t>::max());
// quadrics
const size_t GLWidget::QUADRIC_SLICES = 8;
const size_t GLWidget::QUADRIC_STACKS = 1;

const pair<float,float> GLWidget::T1S_SIZE (1, 32);
const pair<float,float> GLWidget::TENSOR_SIZE_EXP2 (0, 10);
const pair<float,float> GLWidget::TENSOR_LINE_WIDTH_EXP2 (0, 3);
const pair<float,float> GLWidget::FORCE_SIZE_EXP2 (-2, 2);

const pair<float,float> GLWidget::CONTEXT_ALPHA (0.05, 0.5);
const GLfloat GLWidget::HIGHLIGHT_LINE_WIDTH = 2.0;

// Methods
// ======================================================================

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent),
      m_torusOriginalDomainDisplay (false),
      m_torusOriginalDomainClipped (false),
      m_interactionMode (InteractionMode::ROTATE),
      m_interactionObject (InteractionObject::FOCUS),
      m_simulationGroup (0), 
      m_minimumEdgeRadius (0),
      m_edgeRadiusRatio (0),
      m_facesShowEdges (true),
      m_bodyCenterShown (false),
      m_bodyNeighborsShown (false),
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
      m_t1sSize (1.0),
      m_contextAlpha (CONTEXT_ALPHA.first),
      m_highlightLineWidth (HIGHLIGHT_LINE_WIDTH),
      m_missingPressureShown (true),
      m_missingVolumeShown (true),
      m_titleShown (false),
      m_averageAroundMarked (true),
      m_viewFocusShown (true),
      m_constraintsShown (true),
      m_constraintPointsShown (false),
      m_viewCount (ViewCount::ONE),
      m_viewLayout (ViewLayout::HORIZONTAL),
      m_viewNumber (ViewNumber::VIEW0),
      m_timeLinkage (TimeLinkage::LINKED),
      m_transformLinkage (TransformLinkage::INDEPENDENT),
      m_linkedTime (0),
      m_showType (SHOW_NOTHING)
{
    makeCurrent ();
    initEndTranslationColor ();
    initQuadrics ();
    initDisplayView ();
    createActions ();
}

GLWidget::~GLWidget()
{
    makeCurrent();
    gluDeleteQuadric (m_quadric);
    m_quadric = 0;
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
    m_actionLinkedTimeBegin = boost::make_shared<QAction> (
	tr("&Begin interval"), this);
    m_actionLinkedTimeBegin->setStatusTip(tr("Linked time begin interval"));
    connect(m_actionLinkedTimeBegin.get (), SIGNAL(triggered()),
	    this, SLOT(LinkedTimeBegin ()));

    m_actionLinkedTimeEnd = boost::make_shared<QAction> (
	tr("&End interval"), this);
    m_actionLinkedTimeEnd->setStatusTip(tr("Linked time end interval"));
    connect(m_actionLinkedTimeEnd.get (), SIGNAL(triggered()),
	    this, SLOT(LinkedTimeEnd ()));

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

    m_actionResetTransformAll = boost::make_shared<QAction> (tr("&All"), this);
    m_actionResetTransformAll->setShortcut(QKeySequence (tr ("Ctrl+R")));
    m_actionResetTransformAll->setStatusTip(tr("Reset transform all"));
    connect(m_actionResetTransformAll.get (), SIGNAL(triggered()),
	    this, SLOT(ResetTransformAll ()));

    m_actionResetTransformFocus = boost::make_shared<QAction> (
	tr("&Focus"), this);
    m_actionResetTransformFocus->setStatusTip(tr("Reset transform focus"));
    connect(m_actionResetTransformFocus.get (), SIGNAL(triggered()),
	    this, SLOT(ResetTransformFocus ()));

    m_actionResetTransformContext = boost::make_shared<QAction> (
	tr("&Context"), this);
    m_actionResetTransformContext->setStatusTip(tr("Reset transform context"));
    connect(m_actionResetTransformContext.get (), SIGNAL(triggered()),
	    this, SLOT(ResetTransformContext ()));

    m_actionResetTransformLight = boost::make_shared<QAction> (
	tr("&Light"), this);
    m_actionResetTransformLight->setStatusTip(
	tr("Reset transform light"));
    connect(m_actionResetTransformLight.get (), SIGNAL(triggered()),
	    this, SLOT(ResetTransformLight ()));

    m_actionResetTransformGrid = boost::make_shared<QAction> (
	tr("&Grid"), this);
    m_actionResetTransformGrid->setStatusTip(tr("Reset transform grid"));
    connect(m_actionResetTransformGrid.get (), SIGNAL(triggered()),
	    this, SLOT(ResetTransformGrid ()));

    m_actionAverageAroundBody = boost::make_shared<QAction> (
	tr("&Body"), this);
    m_actionAverageAroundBody->setStatusTip(tr("Averaged around body"));
    connect(m_actionAverageAroundBody.get (), SIGNAL(triggered()),
	    this, SLOT(AverageAroundBody ()));

    m_actionAverageAroundSecondBody = boost::make_shared<QAction> (
	tr("&Second Body"), this);
    m_actionAverageAroundSecondBody->setStatusTip(
	tr("Averaged around second body"));
    connect(m_actionAverageAroundSecondBody.get (), SIGNAL(triggered()),
	    this, SLOT(AverageAroundSecondBody ()));

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

    m_actionAverageAroundShowRotation = boost::make_shared<QAction> (
	tr("&Show rotation"), this);
    m_actionAverageAroundShowRotation->setStatusTip(tr("Show rotation"));
    m_actionAverageAroundShowRotation->setCheckable (true);
    connect(m_actionAverageAroundShowRotation.get (), SIGNAL(toggled(bool)),
	    this, SLOT(ToggledAverageAroundAllowRotation (bool)));

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

    m_actionShowDeformation = boost::make_shared<QAction> (
	tr("&Deformation"), this);
    m_actionShowDeformation->setStatusTip(tr("Shown deformation"));
    connect(m_actionShowDeformation.get (), SIGNAL(triggered()),
	    this, SLOT(ShowDeformation ()));

    m_actionShowVelocity = boost::make_shared<QAction> (
	tr("&Velocity"), this);
    m_actionShowVelocity->setStatusTip(tr("Shown velocity"));
    connect(m_actionShowVelocity.get (), SIGNAL(triggered()),
	    this, SLOT(ShowVelocity ()));

    m_actionShowReset = boost::make_shared<QAction> (tr("&Reset"), this);
    m_actionShowReset->setStatusTip(tr("Shown reset"));
    connect(m_actionShowReset.get (), SIGNAL(triggered()),
	    this, SLOT(ShowReset ()));

    // actions for the color bar
    m_actionEditColorMap.reset (
	new QAction (tr("&Edit Color Map"), this));
    m_actionEditColorMap->setStatusTip(tr("Edit Color Map"));
    // connected in MainWindow

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


void GLWidget::initDisplayView ()
{
    // WARNING: This has to be in the same order as ViewType::Enum
    boost::array<ViewTypeDisplay, ViewType::COUNT> displayView =
	{{&GLWidget::displayEdgesNormal,
	  &GLWidget::displayEdgesTorus,
	  &GLWidget::displayFacesTorus,
	  &GLWidget::displayFacesNormal,
	  &GLWidget::displayCenterPathsWithBodies,
	  &GLWidget::displayFacesAverage,
	  &GLWidget::displayT1sPDE,
	    }};
    copy (displayView.begin (), displayView.end (), m_display.begin ());
}

void GLWidget::initViewSettings ()
{
    const Simulation& simulation = GetSimulation (0);
    int rotation2D = simulation.GetRotation2D ();
    ViewNumber::Enum viewNumber (ViewNumber::VIEW0);
    BOOST_FOREACH (boost::shared_ptr<ViewSettings>& vs, m_viewSettings)
    {
	vs = boost::make_shared <ViewSettings> (*this);
	vs->SetAxesOrder (
	    simulation.Is2D () ? 
	    (rotation2D == 0 ? AxesOrder::TWO_D :
	     (rotation2D == 90 ? AxesOrder::TWO_D_ROTATE_LEFT90 : 
	      AxesOrder::TWO_D_ROTATE_RIGHT90)): AxesOrder::THREE_D);
	vs->SetViewType (ViewType::FACES);
	vs->SetT1sShiftLower (simulation.GetT1sShiftLower ());
	vs->AverageSetTimeWindow (simulation.GetTimeSteps ());
	vs->GetT1sPDE ().AverageSetTimeWindow (simulation.GetT1sTimeSteps ());
	vs->AverageSetTimeWindow (simulation.GetTimeSteps ());
	vs->SetScaleCenter (
	    CalculateViewingVolume (
		viewNumber, 
		ViewingVolumeOperation::DONT_ENCLOSE2D).center ().xy ());
	viewNumber = ViewNumber::Enum (viewNumber + 1);
    }
}


const Simulation& GLWidget::GetSimulation () const
{
    return GetSimulation (GetViewNumber ());
}

const Simulation& GLWidget::GetSimulation (
    ViewNumber::Enum viewNumber) const
{
    return GetSimulation (
	GetViewSettings (viewNumber).GetSimulationIndex ());
}

const Simulation& GLWidget::GetSimulation (size_t i) const
{
    return m_simulationGroup->GetSimulation (i);
}

void GLWidget::SetSimulationGroup (const SimulationGroup* simulationGroup)
{
    m_simulationGroup = simulationGroup;
    initViewSettings ();
    Foam::Bodies bodies = GetSimulation ().GetFoam (0).GetBodies ();
    if (bodies.size () != 0)
    {
	size_t maxIndex = bodies.size () - 1;
	m_selectBodiesById->SetMinBodyId (bodies[0]->GetId ());
	m_selectBodiesById->SetMaxBodyId (bodies[maxIndex]->GetId ());
	m_selectBodiesById->UpdateLabelMinMax ();
    }
    update ();
}


float GLWidget::GetOnePixelInObjectSpace () const
{
    G3D::Vector3 first = toObject (QPoint (0, 0));
    G3D::Vector3 second = toObject (QPoint (1, 0));
    float onePixelInObjectSpace = (second - first).length ();
    return onePixelInObjectSpace;
}

double GLWidget::GetCellLength (ViewNumber::Enum viewNumber) const
{
    const Body& body = GetSimulation (viewNumber).GetFoam (0).GetBody (0);
    G3D::AABox box = body.GetBoundingBox ();
    G3D::Vector3 extent = box.extent ();
    return min (extent.x, extent.y);
}

float GLWidget::GetDeformationSizeInitialRatio (
    ViewNumber::Enum viewNumber) const
{
    float cellLength = GetCellLength (viewNumber);
    return cellLength / 
	(2 * GetSimulation (viewNumber).GetMaxDeformationEigenValue ());
}

float GLWidget::GetVelocitySizeInitialRatio (
    ViewNumber::Enum viewNumber) const
{
    float cellLength = GetCellLength (viewNumber);
    float velocityMagnitude = 
	GetSimulation (viewNumber).GetMax (BodyProperty::VELOCITY_MAGNITUDE);
    return cellLength / velocityMagnitude;
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


QSize GLWidget::minimumSizeHint()
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint()
{
    return QSize(512, 512);
}

G3D::Vector3 GLWidget::getInitialLightPosition (
    ViewNumber::Enum viewNumber,
    LightNumber::Enum lightPosition) const
{
    
    G3D::AABox bb = calculateCenteredViewingVolume (
	viewNumber, double (width ()) / height (), 1.0);
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
	    viewNumber, LightNumber::Enum (i)) / sqrt3;
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


/**
 * @todo: make sure context view works for 3D
 */
void GLWidget::translateAndScale (
    ViewNumber::Enum viewNumber, double scaleRatio,
    const G3D::Vector3& translation, bool contextView) const
{
    glScale (scaleRatio);
    // if 2D, the back plane stays in the same place
    if (GetSimulation (viewNumber).Is2D () && ! IsTimeDisplacementUsed ())
    {
	G3D::AABox boundingBox = 
	    GetSimulation (viewNumber).GetBoundingBox ();
	float zTranslation = boundingBox.center ().z - boundingBox.low ().z;
	zTranslation = zTranslation - zTranslation / scaleRatio;
	glTranslatef (0, 0, zTranslation);
    }
    glTranslate (contextView ? (translation / scaleRatio) : translation);
}


void GLWidget::transformFoamAverageAround (
    ViewNumber::Enum viewNumber, size_t timeStep) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    ViewSettings::AverageAroundType type = vs.GetAverageAroundType ();
    if (type == ViewSettings::AVERAGE_AROUND)
	RotateAndTranslateAverageAround (viewNumber, timeStep, 1);
}

void GLWidget::RotateAndTranslateAverageAround (
    ViewNumber::Enum viewNumber,
    size_t timeStep, int direction) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const ObjectPosition rotationBegin = vs.GetAverageAroundPosition (0);
    const ObjectPosition rotationCurrent = 
	vs.GetAverageAroundPosition (timeStep);
    float angleRadians = rotationCurrent.m_angle - rotationBegin.m_angle;
    if (direction > 0)
    {
	G3D::Vector2 translation = 
	    rotationBegin.m_rotationCenter - rotationCurrent.m_rotationCenter;
	glTranslate (translation);
    }
    if (angleRadians != 0)
    {
	G3D::Vector2 rotationCenter = rotationCurrent.m_rotationCenter;
	glTranslate (rotationCenter);
	float angleDegrees =  G3D::toDegrees (angleRadians);
	//cdbg << "angle degrees = " << angleDegrees << endl;
	angleDegrees = direction > 0 ? angleDegrees : - angleDegrees;
	glRotatef (angleDegrees, 0, 0, 1);
	glTranslate (-rotationCenter);
    }
}

G3D::AABox GLWidget::calculateViewingVolume (
    ViewNumber::Enum viewNumber,
    double xOverY, double extendAlongZRatio, 
    ViewingVolumeOperation::Enum enclose) const
{
    const Simulation& simulation = GetSimulation (viewNumber);
    G3D::AABox bb = simulation.GetBoundingBox ();
    G3D::AABox vv = AdjustXOverYRatio (EncloseRotation (bb), xOverY);
    if (! simulation.Is2D ())
	// ExtendAlongZFor3D is used for 3D, 
	// so that you keep the 3D objects outside the camera
	vv = ExtendAlongZFor3D (vv, extendAlongZRatio);
    if (enclose == ViewingVolumeOperation::ENCLOSE2D)
	vv = EncloseRotation2D (vv);
    return vv;
}

G3D::AABox GLWidget::CalculateViewingVolume (
    ViewNumber::Enum viewNumber, ViewingVolumeOperation::Enum enclose) const
{
    double xOverY = getViewXOverY ();
    const ViewSettings& vs = GetViewSettings (viewNumber);
    double extendAlongZRatio = vs.GetScaleRatio ();
    return calculateViewingVolume (viewNumber, 
				   xOverY, extendAlongZRatio, enclose);
}


G3D::AABox GLWidget::calculateCenteredViewingVolume (
    ViewNumber::Enum viewNumber,
    double xOverY, double extendAlongZRatio) const
{
    G3D::AABox vv = calculateViewingVolume (
	viewNumber, xOverY, extendAlongZRatio);
    return vv - vv.center ();
}

G3D::AABox GLWidget::calculateEyeViewingVolume (
    ViewNumber::Enum viewNumber, ViewingVolumeOperation::Enum enclose) const
{
    double xOverY = getViewXOverY ();
    const ViewSettings& vs = GetViewSettings (viewNumber);
    double extendAlongZRatio = vs.GetScaleRatio ();
    G3D::AABox vv = calculateViewingVolume (
	viewNumber,
	xOverY, extendAlongZRatio, enclose);
    vv = vv - vv.center ();
    G3D::Vector3 translation (vs.GetCameraDistance () * G3D::Vector3::unitZ ());
    G3D::AABox result = vv - translation;
    return result;
}

G3D::Vector3 GLWidget::getEyeTransform (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    return G3D::Vector3 (0, 0, - vs.GetCameraDistance ()) -
	GetSimulation (viewNumber).GetBoundingBox ().center ();
}

void GLWidget::ModelViewTransform (ViewNumber::Enum viewNumber, 
				   size_t timeStep) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const Simulation& simulation = GetSimulation (viewNumber);
    glLoadIdentity ();
    glTranslate (
	getScaleCenterTranslation (viewNumber) +
	G3D::Vector3 (0, 0, - vs.GetCameraDistance ()));
    bool contextView = vs.IsContextView ();
    if (contextView)
	translateAndScale (
	    viewNumber, vs.GetContextScaleRatio (), G3D::Vector3::zero (),
	    false);
    else
	translateAndScale (
	    viewNumber, vs.GetScaleRatio (), vs.GetTranslation (), false);
    const Foam& foam = simulation.GetFoam (0);
    glMultMatrix (vs.GetRotationModel () * vs.GetRotationForAxesOrder (foam));
    glTranslate (- simulation.GetBoundingBox ().center ());
    transformFoamAverageAround (viewNumber, timeStep);
}

void GLWidget::ProjectionTransform (
    ViewNumber::Enum viewNumber,
    ViewingVolumeOperation::Enum enclose) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::AABox vv = calculateEyeViewingVolume (viewNumber, enclose);
    glLoadIdentity();
    if (vs.GetAngleOfView () == 0)
	glOrtho (vv.low ().x, vv.high ().x, vv.low ().y, vv.high ().y,
		 -vv.high ().z, -vv.low ().z);
    else
	glFrustum (vv.low ().x, vv.high ().x, vv.low ().y, vv.high ().y,
		   -vv.high ().z, -vv.low ().z);
}



void GLWidget::viewportTransform (ViewNumber::Enum viewNumber) const
{
    G3D::Rect2D viewRect = GetViewRect (viewNumber);
    ViewSettings& vs = GetViewSettings (viewNumber);
    vs.SetViewport (viewRect);
    glViewport (viewRect);
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
	    SetViewNumber (viewNumber);
	    break;
	}
    }
}

void GLWidget::SetViewNumber (ViewNumber::Enum viewNumber)
{
    m_viewNumber = viewNumber;
    Q_EMIT ViewChanged ();
}

G3D::Rect2D GLWidget::getViewColorBarRect (const G3D::Rect2D& viewRect)
{
    return G3D::Rect2D::xywh (
	viewRect.x0 () + 5, viewRect.y0 () + 5,
	10, max (viewRect.height () / 4, 50.0f));
}

G3D::Rect2D GLWidget::getViewOverlayBarRect (const G3D::Rect2D& viewRect)
{
    return G3D::Rect2D::xywh (
	viewRect.x0 () + 5 + 10 + 5, viewRect.y0 () + 5,
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

void GLWidget::ResetTransformAll ()
{
    ResetTransformFocus ();
    ResetTransformContext ();
    ResetTransformGrid ();
    ResetTransformLight ();
}

void GLWidget::ResetTransformFocus ()
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings ();
    vs.SetRotationModel (G3D::Matrix3::identity ());
    vs.SetScaleRatio (1);
    vs.SetTranslation (G3D::Vector3::zero ());
    glMatrixMode (GL_PROJECTION);
    ProjectionTransform (viewNumber);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    vs.AverageInitStep (viewNumber);
    update ();
}

void GLWidget::ResetTransformContext ()
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings ();
    vs.SetContextScaleRatio (1);
    glMatrixMode (GL_PROJECTION);
    ProjectionTransform (viewNumber);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    update ();
}

void GLWidget::ResetTransformGrid ()
{
    ViewSettings& vs = GetViewSettings ();
    vs.SetGridScaleRatio (1);
    vs.SetGridTranslation (G3D::Vector3::zero ());
    update ();
}

void GLWidget::ResetTransformLight ()
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    LightNumber::Enum lightNumber = vs.GetSelectedLight ();
    vs.SetInitialLightPosition (lightNumber);
    vs.PositionLight (lightNumber, 
		      getInitialLightPosition (viewNumber, lightNumber));
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

bool GLWidget::linkedTimesValid (size_t timeBegin, size_t timeEnd)
{
    if (timeBegin <= timeEnd)
	return true;
    else
    {
	ostringstream ostr;
	ostr << "Error: timeBegin: " << timeBegin 
	     << " smaller than timeEnd: " << timeEnd;
	QMessageBox msgBox (this);
	msgBox.setText(ostr.str ().c_str ());
	msgBox.exec();
	return false;
    }
}


bool GLWidget::linkedTimesValid ()
{
    if (GetTimeLinkage () == TimeLinkage::LINKED)
    {
	QMessageBox msgBox (this);
	msgBox.setText("You need to be in Settings > Show > "
		       "View > Time linkage > Independent mode");
	msgBox.exec();
	return false;
    }
    else
	return true;
}


void GLWidget::LinkedTimeBegin ()
{
    if (! linkedTimesValid ())
	return;
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    size_t linkedTimeBegin = GetCurrentTime (viewNumber);
    size_t linkedTimeEnd = vs.GetLinkedTimeEnd ();
    if (linkedTimesValid (linkedTimeBegin, linkedTimeEnd))
    {
	vs.SetLinkedTimeBegin (linkedTimeBegin);
	Q_EMIT ViewChanged ();
    }	
}

void GLWidget::LinkedTimeEnd ()
{
    if (! linkedTimesValid ())
	return;
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    size_t linkedTimeBegin = vs.GetLinkedTimeBegin ();
    size_t linkedTimeEnd = GetCurrentTime (viewNumber);
    if (linkedTimesValid (linkedTimeBegin, linkedTimeEnd))
    {
	vs.SetLinkedTimeEnd (linkedTimeEnd);
	Q_EMIT ViewChanged ();
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
    string message = GetSimulation ().ToHtml ();
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
	Foam::Bodies::const_iterator it = 
	    GetSimulation ().GetFoam (0).FindBody (ids[0]);
	ostr << *it;
    }
    else
    {
	ostr << "Selected ids: ";
	ostream_iterator<size_t> out (ostr, " ");
	copy (ids.begin (), ids.end (), out);
	if (GetViewSettings ().GetBodyOrFaceProperty () != 
	    FaceProperty::DMP_COLOR)
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

void GLWidget::ShowDeformation ()
{
    m_showType = SHOW_DEFORMATION_TENSOR;
    vector<size_t> bodies;
    brushedBodies (m_contextMenuPosScreen, &bodies);
    m_showBodyId = bodies[0];
    update ();
}

void GLWidget::ShowVelocity ()
{
    m_showType = SHOW_VELOCITY;
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
    try {
	initializeGLFunctions ();
	glClearColor (Qt::white);
	glEnable(GL_DEPTH_TEST);
	glEnable (GL_MULTISAMPLE);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	cdbg << "Compiling shaders..." << endl;
	ScalarAverage::InitShaders ();
	TensorAverage::InitShaders ();
	VectorAverage::InitShaders ();
	T1sPDE::InitShaders ();
	initializeLighting ();
	WarnOnOpenGLError ("initializeGl");
    }
    catch (const exception& e)
    {
	cdbg << "Exception: " << e.what () << endl;
    }

}

void GLWidget::paintGL ()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    displayViews ();
    displayViewsGrid ();
    Q_EMIT PaintedGL ();
}

void GLWidget::resizeGL(int w, int h)
{
    (void)w;(void)h;
    for (size_t i = 0; i < ViewCount::GetCount (m_viewCount); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	ViewSettings& vs = GetViewSettings (viewNumber);
	vs.AverageInitStep (viewNumber);
    }
    WarnOnOpenGLError ("resizeGl");
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
    //QTime t;t.start ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    vs.SetLightingParameters (
	getInitialLightPosition (viewNumber, vs.GetSelectedLight ()));
    allTransform (viewNumber);
    m_minimumEdgeRadius = GetOnePixelInObjectSpace ();
    calculateEdgeRadius (m_edgeRadiusRatio,
			 &m_edgeRadius, &m_arrowBaseRadius,
			 &m_arrowHeight, &m_edgeWidth);
    (this->*(m_display[vs.GetViewType ()])) (viewNumber);
    displayViewDecorations (viewNumber);
    displayAxes (viewNumber);
    displayBoundingBox (viewNumber);
    displayOriginalDomain (viewNumber);
    displayFocusBox (viewNumber);
    displayLightDirection (viewNumber);
    displayBodyCenters (viewNumber);
    displayFaceCenters (viewNumber);
    ViewNumber::Enum currentView = GetViewNumber ();
    if (currentView == viewNumber)
    {
	displayBodyNeighbors (currentView);
	displayBodyDeformation (currentView);
	displayBodyVelocity (currentView);
    }
    displayBodiesNeighbors ();
    displayStatus ();    
    //displayContextMenuPos (viewNumber);
    WarnOnOpenGLError ("displayView");
    //cdbg << "displayView(" <<  viewNumber << "): " 
    //<< t.elapsed () << " ms" << endl;
}


void GLWidget::allTransform (ViewNumber::Enum viewNumber) const
{
    viewportTransform (viewNumber);    
    glMatrixMode (GL_PROJECTION);
    ProjectionTransform (viewNumber);
    glMatrixMode (GL_MODELVIEW);
    ModelViewTransform (viewNumber, GetCurrentTime (viewNumber));
}


G3D::Matrix3 GLWidget::getRotationAround (int axis, double angleRadians)
{
    using G3D::Matrix3;using G3D::Vector3;
    Vector3 axes[3] = {
	Vector3::unitX (), Vector3::unitY (), Vector3::unitZ ()
    };
    return Matrix3::fromAxisAngle (axes[axis], angleRadians);
}

G3D::Vector2 GLWidget::getScaleCenter (
    ViewNumber::Enum viewNumber, const G3D::Rect2D& rect) const
{
    if (m_transformLinkage == TransformLinkage::INDEPENDENT)
	return rect.center ();
    else if (viewNumber == ViewNumber::VIEW0)
	return (rect.x0y0 () + rect.x1y0 ()) / 2;
    else
	return (rect.x0y1 () + rect.x1y1 ()) / 2;
}


float GLWidget::ratioFromScaleCenter (
    ViewNumber::Enum viewNumber, const QPoint& p)
{
    
    G3D::Vector2 center = getScaleCenter (viewNumber, GetViewRect (viewNumber));
    int windowHeight = height ();
    G3D::Vector2 lastPos = QtToOpenGl (m_lastPos, windowHeight);
    G3D::Vector2 currentPos = QtToOpenGl (p, windowHeight);
    float ratio =
	(currentPos - center).length () / (lastPos - center).length ();
    return ratio;
}

// @todo add a 2D rotation which rotates only around Z axis.
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


void GLWidget::calculateTranslationRatio (
    ViewNumber::Enum viewNumber, const QPoint& position,
    G3D::Vector3::Axis screenXTranslation,
    G3D::Vector3::Axis screenYTranslation, 
    G3D::Vector3* translationRatio, G3D::Vector3* focusBoxExtent)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    (*translationRatio)[screenXTranslation] =
	static_cast<double>(position.x() - m_lastPos.x()) /
	vs.GetViewport ().width ();
    (*translationRatio)[screenYTranslation] =
	- static_cast<double> (position.y() - m_lastPos.y()) / 
	vs.GetViewport ().height ();


    G3D::AABox vv = calculateCenteredViewingVolume (
	viewNumber, double (width ()) / height (), vs.GetScaleRatio ());
    *focusBoxExtent = vv.extent () / vs.GetScaleRatio ();
}


void GLWidget::translate (
    ViewNumber::Enum viewNumber, const QPoint& position,
    G3D::Vector3::Axis screenXTranslation,
    G3D::Vector3::Axis screenYTranslation)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::Vector3 translationRatio, focusBoxExtent;
    calculateTranslationRatio (viewNumber, position, 
			       screenXTranslation, screenYTranslation, 
			       &translationRatio, &focusBoxExtent);
    if (vs.IsContextView ())
	vs.SetTranslation (
	    vs.GetTranslation () - (translationRatio * focusBoxExtent));
    else
    {
	G3D::Vector3 translation = 
	    vs.GetTranslation () + (translationRatio * focusBoxExtent);
	vs.SetTranslation (translation);
    }
}

void GLWidget::translateGrid (
    ViewNumber::Enum viewNumber, const QPoint& position)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::Vector3 translationRatio, focusBoxExtent;
    calculateTranslationRatio (viewNumber, position, 
			       G3D::Vector3::X_AXIS,
			       G3D::Vector3::Y_AXIS, 
			       &translationRatio, &focusBoxExtent);
    vs.SetGridTranslation (
	vs.GetGridTranslation () + (translationRatio * focusBoxExtent));
}

void GLWidget::scale (ViewNumber::Enum viewNumber, const QPoint& position)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    float ratio = ratioFromScaleCenter (viewNumber, position);
    if (vs.IsContextView ())
	vs.SetScaleRatio (vs.GetScaleRatio () / ratio);
    else
    {
	if (m_transformLinkage == TransformLinkage::LINKED_HORIZONTAL_AXIS)
	    for (size_t i = 0; i < 2; ++i)
	    {
		ViewNumber::Enum vn = ViewNumber::Enum (i);
		ViewSettings& vs = GetViewSettings (vn);
		vs.SetScaleRatio (vs.GetScaleRatio () * ratio);
	    }
	else
	    vs.SetScaleRatio (vs.GetScaleRatio () * ratio);
    }
}

void GLWidget::scaleGrid (ViewNumber::Enum viewNumber, const QPoint& position)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    float ratio = ratioFromScaleCenter (viewNumber, position);
    vs.SetGridScaleRatio (vs.GetGridScaleRatio () * ratio);
}


void GLWidget::scaleContext (
    ViewNumber::Enum viewNumber, const QPoint& position)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    float ratio = ratioFromScaleCenter (viewNumber, position);
    vs.SetContextScaleRatio (vs.GetContextScaleRatio () * ratio);
}

G3D::Vector3 GLWidget::brushedBodies (
    const QPoint& position, vector<size_t>* bodies) const
{
    vector< boost::shared_ptr<Body> > b;
    G3D::Vector3 op = brushedBodies (position, &b);
    bodies->resize (b.size ());
    ::transform (b.begin (), b.end (), bodies->begin (),
	       boost::bind (&Element::GetId, _1));
    return op;
}

G3D::Vector3 GLWidget::brushedBodies (
    const QPoint& position, vector< boost::shared_ptr<Body> >* bodies) const
{
    G3D::Vector3 op = toObjectTransform (position);
    const Foam& foam = GetSimulation ().GetFoam (GetCurrentTime ());
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
    bool is2D = GetSimulation ().Is2D ();
    G3D::Vector3 op = gluUnProject (
	QtToOpenGl (position, height ()), 
	is2D ? GluUnProjectZOperation::SET0 : GluUnProjectZOperation::READ);
    if (is2D)
	op.z = 0;
    return op;
}


G3D::Vector3 GLWidget::toObjectTransform (const QPoint& position, 
					  ViewNumber::Enum viewNumber) const
{
    allTransform (viewNumber);
    return toObject (position);
}

G3D::Vector3 GLWidget::toObjectTransform (const QPoint& position) const
{
    return toObjectTransform (position, GetViewNumber ());
}

void GLWidget::displayAverageAroundBodies (
    ViewNumber::Enum viewNumber,
    bool adjustForAverageAroundMovementRotation) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    ViewSettings::AverageAroundType type = vs.GetAverageAroundType ();
    if (m_averageAroundMarked && type == ViewSettings::AVERAGE_AROUND)
    {
	const Simulation& simulation = GetSimulation (viewNumber);
	glPushAttrib (GL_CURRENT_BIT |
		      GL_ENABLE_BIT | GL_LINE_BIT | GL_POINT_BIT);
	if (adjustForAverageAroundMovementRotation)
	{
	    glMatrixMode (GL_MODELVIEW);
	    glPushMatrix ();
	    RotateAndTranslateAverageAround (
		viewNumber, GetCurrentTime (viewNumber), -1);
	}
	glDisable (GL_DEPTH_TEST);
	Foam::Bodies focusBody (1);
	size_t bodyId = vs.GetAverageAroundBodyId ();
	focusBody[0] = *simulation.GetFoam (
	    vs.GetCurrentTime ()).FindBody (bodyId);
	displayFacesContour<HighlightNumber::H0> (
	    focusBody, viewNumber, GetHighlightLineWidth ());
	glPointSize (4.0);
	glColor (Qt::black);
	DisplayBodyCenter (
	    *this, simulation.GetFoam (vs.GetCurrentTime ()).GetProperties (), 
	    IdBodySelector (bodyId)) (focusBody[0]);
	size_t secondBodyId = vs.GetAverageAroundSecondBodyId ();
	if (secondBodyId != INVALID_INDEX)
	{
	    focusBody[0] = 
		*simulation.GetFoam (vs.GetCurrentTime ()).
		FindBody (secondBodyId);
	    displayFacesContour<HighlightNumber::H0> (
		focusBody, viewNumber, GetHighlightLineWidth ());
	}
	if (adjustForAverageAroundMovementRotation)
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
	const Foam::Bodies& bodies = 
	    GetSimulation (viewNumber).GetFoam (
		GetCurrentTime (viewNumber)).GetBodies ();
	Foam::Bodies contextBodies (bodies.size ());
	

	size_t j = 0;
	for (size_t i = 0; i < bodies.size (); ++i)
	    if (vs.IsContextDisplayBody (bodies[i]->GetId ()))
		contextBodies[j++] = bodies[i];
	contextBodies.resize (j);
	displayFacesContour<HighlightNumber::H1> (
	    contextBodies, viewNumber, GetHighlightLineWidth ());
	glPopAttrib ();
    }
}

void GLWidget::displayContextStationaryFoam (
    ViewNumber::Enum viewNumber,
    bool adjustForAverageAroundMovementRotation) const
{
    ViewSettings::AverageAroundMovementShown type = 
	GetViewSettings (viewNumber).GetAverageAroundMovementShown ();
    if (type == ViewSettings::AVERAGE_AROUND_MOVEMENT_ROTATION)
    {
	glPushAttrib (GL_ENABLE_BIT);
	glDisable (GL_DEPTH_TEST);
	if (adjustForAverageAroundMovementRotation)
	{
	    glMatrixMode (GL_MODELVIEW);
	    glPushMatrix ();
	    RotateAndTranslateAverageAround (
		viewNumber, GetCurrentTime (viewNumber), -1);
	}
	DisplayBox (GetSimulation (viewNumber), 
		    GetHighlightColor (viewNumber, HighlightNumber::H1),
		    GetHighlightLineWidth ());
	if (adjustForAverageAroundMovementRotation)
	    glPopMatrix ();
	glPopAttrib ();
    }
}


string GLWidget::getAverageAroundLabel ()
{
    ostringstream ostr;
    const ViewSettings& vs = GetViewSettings ();
    ViewSettings::AverageAroundType type = vs.GetAverageAroundType ();
    if (type == ViewSettings::AVERAGE_AROUND)
    {
	ostr << "Average around";
	if (vs.GetAverageAroundSecondBodyId () == INVALID_INDEX)
	    ostr << " (1)";
	else
	    ostr << " (2)";	    
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

string GLWidget::getAverageAroundMovementShownLabel ()
{
    ostringstream ostr;
    const ViewSettings& vs = GetViewSettings ();
    ViewSettings::AverageAroundMovementShown type = 
	vs.GetAverageAroundMovementShown ();
    if (type == ViewSettings::AVERAGE_AROUND_MOVEMENT_ROTATION)
	ostr << "Show rotation";
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

string GLWidget::getBodyScaleLabel ()
{
    if (m_interactionMode == InteractionMode::SCALE)
    {
	ostringstream ostr;
	const ViewSettings& vs = GetViewSettings ();
	ostr << "Scale: " << setprecision (3) << vs.GetScaleRatio ();
	return ostr.str ();
    }
    else
	return "";
}

void GLWidget::displayStatus ()
{
    ostringstream ostr;
    boost::array<string, 5> labels = {{
	    getAverageAroundLabel (),
	    getContextLabel (),
	    getAverageAroundMovementShownLabel (),
	    getBodySelectorLabel (),
	    getBodyScaleLabel ()
	}};
    ostream_iterator<string> o (ostr, " ");
    remove_copy_if (labels.begin (), labels.end (), o,
		    boost::bind (&string::empty, _1));
    string label;
    if (ostr.str ().empty ())
	label = "Ready";
    else
	label = " " + ostr.str ();
    m_labelStatusBar->setText (QString (label.c_str ()));
}


void GLWidget::AverageAroundBody ()
{
    ViewSettings& vs = GetViewSettings ();
    vector< boost::shared_ptr<Body> > bodies;
    brushedBodies (m_contextMenuPosScreen, &bodies);
    if (bodies.size () != 0)
    {
	const Simulation& simulation = GetSimulation ();
	size_t bodyId = bodies[0]->GetId ();
	vs.SetAverageAroundBodyId (bodyId);
	vs.SetAverageAroundSecondBodyId (INVALID_INDEX);
	vs.SetAverageAroundType (ViewSettings::AVERAGE_AROUND);
	if (bodies[0]->IsConstraint () && 
	    simulation.GetDmpObjectInfo ().RotationUsed ())
	    vs.SetAverageAroundPositions (simulation);
	else
	    vs.SetAverageAroundPositions (simulation, bodyId);
	update ();
    }
    else
    {
	QMessageBox msgBox (this);
	msgBox.setText("No body selected");
	msgBox.exec();
    }
}

void GLWidget::AverageAroundSecondBody ()
{
    ViewSettings& vs = GetViewSettings ();
    vector< boost::shared_ptr<Body> > bodies;
    brushedBodies (m_contextMenuPosScreen, &bodies);
    string message;
    if (bodies.size () != 0)
    {
	const Simulation& simulation = GetSimulation ();
	size_t secondBodyId = bodies[0]->GetId ();
	size_t bodyId = vs.GetAverageAroundBodyId ();
	if (bodyId != INVALID_INDEX)
	{
	    if (bodyId == secondBodyId)
		message = "\"Average around > Second body\" needs to "
		    "be different than \"Average around > Body\"";
	    else
	    {
		vs.SetAverageAroundSecondBodyId (secondBodyId);
		vs.SetAverageAroundType (ViewSettings::AVERAGE_AROUND);
		vs.SetAverageAroundPositions (simulation, bodyId, secondBodyId);
		update ();
		return;
	    }
	}
	else
	    message = "Select \"Average around > Body\" first";
    }
    else
	message = "No body selected";
    QMessageBox msgBox (this);
    msgBox.setText(message.c_str ());
    msgBox.exec();
}


void GLWidget::AverageAroundReset ()
{
    ViewSettings& vs = GetViewSettings ();
    vs.SetAverageAroundType (ViewSettings::AVERAGE_AROUND_NONE);
    vs.SetAverageAroundBodyId (INVALID_INDEX);
    vs.SetAverageAroundSecondBodyId (INVALID_INDEX);
    update ();
}

void GLWidget::ContextDisplayBody ()
{
    ViewSettings& vs = GetViewSettings ();
    vector<size_t> bodies;
    brushedBodies (m_contextMenuPosScreen, &bodies);
    vs.AddContextDisplayBody (bodies[0]);
    update ();
}

void GLWidget::ContextDisplayReset ()
{
    ViewSettings& vs = GetViewSettings ();
    vs.ContextDisplayReset ();
    update ();
}

void GLWidget::ToggledAverageAroundAllowRotation (bool checked)
{
    ViewSettings& vs = GetViewSettings ();
    vs.SetAverageAroundMovementShown (
	checked ? ViewSettings::AVERAGE_AROUND_MOVEMENT_ROTATION : 
	ViewSettings::AVERAGE_AROUND_MOVEMENT_NONE);
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
    GetViewSettings ().DifferenceBodySelector (
	GetSimulation ().GetFoam (0), bodyIds);
    labelCompileUpdate ();
}

void GLWidget::mouseMoveRotate (QMouseEvent *event)
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    switch (m_interactionObject)
    {
    case InteractionObject::FOCUS:
	vs.SetRotationModel (
	    rotate (viewNumber, event->pos (), vs.GetRotationModel ()));
    case InteractionObject::LIGHT:
    {
	LightNumber::Enum i = vs.GetSelectedLight ();
	vs.SetRotationLight (
	    i, rotate (viewNumber, event->pos (), vs.GetRotationLight (i)));
	vs.PositionLight (i, getInitialLightPosition (viewNumber, i));
	break;
    }
    default:
	break;
    }
}

void GLWidget::mouseMoveTranslate (QMouseEvent *event)
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    switch (m_interactionObject)
    {
    case InteractionObject::FOCUS:
	translate (viewNumber, event->pos (), G3D::Vector3::X_AXIS,
		   G3D::Vector3::Y_AXIS);
	vs.AverageInitStep (viewNumber);
	break;
    case InteractionObject::LIGHT:
    {
	LightNumber::Enum i = vs.GetSelectedLight ();
	vs.PositionLight (i, getInitialLightPosition (viewNumber, i));
	translateLight (viewNumber, event->pos ());
	break;
    }
    case InteractionObject::CONTEXT:
    {
	// translate for context view
	QPoint point (m_lastPos.x (), event->pos ().y ());
	translate (viewNumber, 
		   point, G3D::Vector3::X_AXIS, G3D::Vector3::Z_AXIS);
	break;
    }
    case InteractionObject::GRID:
	translateGrid (viewNumber, event->pos ());
	break;
    }
}

void GLWidget::mouseMoveScale (QMouseEvent *event)
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    switch (m_interactionObject)
    {
    case InteractionObject::FOCUS:
	scale (viewNumber, event->pos ());
	vs.AverageInitStep (viewNumber);
	break;
    case InteractionObject::CONTEXT:
	scaleContext (viewNumber, event->pos ());
	break;
    case InteractionObject::GRID:
	scaleGrid (viewNumber, event->pos ());
	break;
    default:
	break;
    }
}


void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    switch (m_interactionMode)
    {
    case InteractionMode::ROTATE:
	mouseMoveRotate (event);
	break;

    case InteractionMode::TRANSLATE:
	mouseMoveTranslate (event);
	break;

    case InteractionMode::SCALE:
	mouseMoveScale (event);
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


void GLWidget::displayOriginalDomain (ViewNumber::Enum viewNumber) const
{
    if (m_torusOriginalDomainDisplay)
	DisplayBox (GetSimulation (viewNumber).GetFoam(0).GetOriginalDomain ());
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
	    viewNumber, double (viewRect.width ()) / viewRect.height (), 
	    vs.GetScaleRatio ());
	translateAndScale ( viewNumber, 1 / vs.GetScaleRatio (), 
			    - vs.GetContextScaleRatio () * 
			    vs.GetTranslation (), true);
	glScale (vs.GetContextScaleRatio ());
	DisplayBox (focusBox, GetHighlightColor (
			viewNumber, HighlightNumber::H0), 
		    GetHighlightLineWidth ());
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
	DisplayBox (GetSimulation (viewNumber), Qt::black);
    if (m_bodiesBoundingBoxesShown)
    {
	const Foam::Bodies& bodies = 
	    GetSimulation (viewNumber).GetFoam (0).GetBodies ();
	const BodySelector& bodySelector = 
	    GetViewSettings (viewNumber).GetBodySelector ();
	BOOST_FOREACH (boost::shared_ptr<Body> body, bodies)
	    if (bodySelector (body))
		DisplayBox (body, Qt::black);
    }
    glPopAttrib ();
}

void GLWidget::displayAxes (ViewNumber::Enum viewNumber)
{
    if (m_axesShown)
    {
	const Simulation& simulation = GetSimulation (viewNumber);

	QFont font;
	float a;
	QFontMetrics fm (font);
	ostringstream ostr;
	ostr << setprecision (4);
	glPushAttrib (GL_CURRENT_BIT);
	const G3D::AABox& aabb = simulation.GetBoundingBox ();
	G3D::Vector3 origin = aabb.low ();
	G3D::Vector3 diagonal = aabb.high () - origin;
	G3D::Vector3 first = origin + diagonal.x * G3D::Vector3::unitX ();
	G3D::Vector3 second = origin + diagonal.y * G3D::Vector3::unitY ();
	G3D::Vector3 third = origin + diagonal.z * G3D::Vector3::unitZ ();

	DisplayOrientedSegmentQuadric displayOrientedEdge (
	    GetQuadricObject (), m_arrowBaseRadius, m_edgeRadius, m_arrowHeight,
	    DisplaySegmentArrow1::TOP_END);

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

	if (! simulation.Is2D ())
	{
	    glColor (Qt::blue);
	    displayOrientedEdge (origin, third);
	    glColor (Qt::black);
	    ostr.str ("");ostr << third.z;
	    renderText (third.x - a, third.y, third.z + a, ostr.str ().c_str ());
	    ostr.str ("");ostr << origin.z;
	    renderText (origin.x - a, origin.y, 
			origin.z + a, ostr.str ().c_str ());
	}
	glPopAttrib ();
    }
}


template<typename displayEdge>
void GLWidget::displayEdges (ViewNumber::Enum viewNumber) const
{
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);
    const BodySelector& bodySelector = 
	GetViewSettings (viewNumber).GetBodySelector ();
    const Foam::Bodies& bodies = 
	GetSimulation (viewNumber).GetFoam (
	    GetCurrentTime (viewNumber)).GetBodies ();
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<
	      DisplayFaceHighlightColor<HighlightNumber::H0,
	      DisplayFaceEdges<displayEdge> > > (
		  *this, GetSimulation (viewNumber).
		  GetFoam (0).GetProperties (), bodySelector));
    displayStandaloneEdges<displayEdge> (GetSimulation (viewNumber).GetFoam (0));

    glPopAttrib ();
}

template<typename displayEdge>
void GLWidget::displayStandaloneEdges (const Foam& foam, 
				       bool useZPos, double zPos) const
{
    if (m_standaloneElementsShown)
    {
	glPushAttrib (GL_ENABLE_BIT);    
	glDisable (GL_DEPTH_TEST);
	const Foam::Edges& standaloneEdges = foam.GetStandaloneEdges ();
	BOOST_FOREACH (boost::shared_ptr<Edge> edge, standaloneEdges)
	    displayEdge (*this, foam.GetProperties (),
			 DisplayElement::FOCUS, useZPos, zPos) (edge);
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
    displayDeformation (viewNumber);
    displayAverageAroundBodies (viewNumber);
    glPopAttrib ();
}

void GLWidget::displayDeformation (ViewNumber::Enum viewNumber) const
{
    const Foam& foam = 
	GetSimulation (viewNumber).GetFoam (GetCurrentTime (viewNumber));
    const ViewSettings& vs = GetViewSettings (viewNumber);
    if (! foam.Is2D () || ! vs.IsDeformationShown ())
	return;
    Foam::Bodies bodies = foam.GetBodies ();
    glPushAttrib (GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable (GL_DEPTH_TEST);
    for_each (bodies.begin (), bodies.end (),
	      DisplayBodyDeformation (
		  *this, 
		  GetSimulation (viewNumber).GetFoam (
		      GetCurrentTime (viewNumber)).GetProperties (),
		  vs.GetBodySelector ()));
    glPopAttrib ();    
}

void GLWidget::displayVelocity (ViewNumber::Enum viewNumber) const
{
    const Foam& foam = 
	GetSimulation (viewNumber).GetFoam (GetCurrentTime (viewNumber));
    const ViewSettings& vs = GetViewSettings (viewNumber);
    if (! foam.Is2D () || ! vs.IsVelocityShown ())
	return;
    Foam::Bodies bodies = foam.GetBodies ();
    glPushAttrib (GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable (GL_DEPTH_TEST);
    glColor (Qt::black);
    for_each (
	bodies.begin (), bodies.end (),
	DisplayBodyVelocity (
	    *this, 
	    GetSimulation (viewNumber).GetFoam (
		GetCurrentTime (viewNumber)).GetProperties (),
	    vs.GetBodySelector ()));
    glPopAttrib ();    
}



void GLWidget::displayBodyDeformation (
    ViewNumber::Enum viewNumber) const
{
    if (m_showType == SHOW_DEFORMATION_TENSOR)
    {
	ViewSettings& vs = GetViewSettings (viewNumber);
	const Foam& foam = 
	    GetSimulation (viewNumber).GetFoam (GetCurrentTime (viewNumber));
	if (! foam.Is2D ())
	    return;
	glPushAttrib (GL_ENABLE_BIT | GL_CURRENT_BIT);
	glDisable (GL_DEPTH_TEST);
	glColor (Qt::black);
	DisplayBodyDeformation (
	    *this, 
	    GetSimulation (viewNumber).GetFoam (
		GetCurrentTime (viewNumber)).GetProperties (),
	    vs.GetBodySelector ()) (*foam.FindBody (m_showBodyId));
	glPopAttrib ();
    }
}

void GLWidget::displayBodyVelocity (
    ViewNumber::Enum viewNumber) const
{
    if (m_showType == SHOW_VELOCITY)
    {
	ViewSettings& vs = GetViewSettings (viewNumber);
	const Foam& foam = 
	    GetSimulation (viewNumber).GetFoam (GetCurrentTime (viewNumber));
	if (! foam.Is2D ())
	    return;
	glPushAttrib (GL_ENABLE_BIT | GL_CURRENT_BIT);
	glDisable (GL_DEPTH_TEST);
	glColor (Qt::black);
	DisplayBodyVelocity (
	    *this, 
	    GetSimulation (viewNumber).GetFoam (
		GetCurrentTime (viewNumber)).GetProperties (),
	    vs.GetBodySelector ()) (*foam.FindBody (m_showBodyId));
	glPopAttrib ();
    }
}


void GLWidget::displayBodyNeighbors (ViewNumber::Enum viewNumber) const
{
    if (m_showType != SHOW_NEIGHBORS)
	return;
    glPushAttrib (GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable (GL_DEPTH_TEST);
    glColor (Qt::black);
    glBegin (GL_LINES);

    const Foam& foam = GetSimulation (viewNumber).GetFoam (
	GetCurrentTime(viewNumber));
    const OOBox& originalDomain = foam.GetOriginalDomain ();
    Foam::Bodies::const_iterator showBody = foam.FindBody (m_showBodyId);
    ::displayBodyNeighbors2D (*showBody, originalDomain);
    glEnd ();
    glPopAttrib ();
}


void GLWidget::displayBodiesNeighbors () const
{
    if (m_bodyNeighborsShown)
    {
	const Foam& foam = GetSimulation ().GetFoam (0);
	if (! foam.Is2D ())
	    return;
	Foam::Bodies bodies = foam.GetBodies ();
	glPushAttrib (GL_ENABLE_BIT | GL_CURRENT_BIT);
	glDisable (GL_DEPTH_TEST);
	glColor (Qt::black);
	glBegin (GL_LINES);
	for_each (bodies.begin (), bodies.end (),
		  boost::bind (
		      ::displayBodyNeighbors2D, _1, 
		      GetSimulation ().GetFoam (0).GetOriginalDomain ()));
	glEnd ();
	glPopAttrib ();
    }
}

void GLWidget::displayT1sDot (ViewNumber::Enum viewNumber) const
{
    for (size_t i = 0; i < GetSimulation (viewNumber).GetT1sTimeSteps (); ++i)
	displayT1sDot (viewNumber, i);
}

void GLWidget::displayT1sDot (
    ViewNumber::Enum viewNumber, size_t timeStep) const
{
    glPushAttrib (GL_ENABLE_BIT | GL_POINT_BIT | 
		  GL_CURRENT_BIT | GL_POLYGON_BIT);
    glDisable (GL_DEPTH_TEST);
    glPointSize (m_t1sSize);
    glColor (GetHighlightColor (viewNumber, HighlightNumber::H0));
    glBegin (GL_POINTS);
    BOOST_FOREACH (const G3D::Vector3 t1Pos, 
		   GetSimulation (viewNumber).GetT1s (timeStep))
	::glVertex (t1Pos);
    glEnd ();
    glPopAttrib ();
}

void GLWidget::DisplayT1Quad (
    ViewNumber::Enum viewNumber, size_t timeStep, size_t t1Index) const
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    T1sPDE& t1sPDE = vs.GetT1sPDE ();
    float rectSize = t1sPDE.GetKernelTextureSize () * 
	GetOnePixelInObjectSpace ();
    float half = rectSize / 2;
    G3D::Rect2D srcTexRect = G3D::Rect2D::xyxy (0., 0., 1., 1.);
    const G3D::Vector3 t1Pos = 
	GetSimulation (viewNumber).GetT1s (timeStep)[t1Index];
    G3D::Vector2 v = t1Pos.xy ();
    G3D::Rect2D srcRect = G3D::Rect2D::xyxy (
	v + G3D::Vector2 (- half, - half),
	v + G3D::Vector2 (  half,   half));

    glPushAttrib (GL_ENABLE_BIT);
    glDisable (GL_DEPTH_TEST);
    glBegin (GL_QUADS);
    sendQuad (srcRect, srcTexRect);
    glEnd ();
    glPopAttrib ();
}

// Three types of minMax (and ColorBarModels)
pair<float, float> GLWidget::GetRange (ViewNumber::Enum viewNumber) const
{
    const Simulation& simulation = GetSimulation (viewNumber);
    ViewSettings& vs = GetViewSettings (viewNumber);
    float minValue = 0.0, maxValue = 0.0;
    switch (vs.GetViewType ())
    {
    case ViewType::FACES_STATISTICS:
	if (vs.GetStatisticsType () == StatisticsType::COUNT)
	    return GetRangeCount (viewNumber);
	else
	{
	    BodyProperty::Enum bodyProperty = 
		BodyProperty::FromSizeT (vs.GetBodyOrFaceProperty ());
	    minValue = simulation.GetMin (bodyProperty);
	    maxValue = simulation.GetMax (bodyProperty);
	}
	break;
    case ViewType::T1S_PDE:
	return GetRangeT1sPDE (viewNumber);
    default:
	break;
    }
    return pair<float, float> (minValue, maxValue);
}

pair<float, float> GLWidget::GetRangeCount (ViewNumber::Enum viewNumber) const
{
    return pair<float, float> (0, GetSimulation (viewNumber).GetTimeSteps ());
}

pair<float, float> GLWidget::GetRangeCount () const
{
    return GetRangeCount (GetViewNumber ());
}

pair<float, float> GLWidget::GetRangeT1sPDE (ViewNumber::Enum viewNumber) const
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    T1sPDE& t1sPDE = vs.GetT1sPDE ();
    float sigma = t1sPDE.GetKernelSigma ();
    return pair<float, float> (0.0, 1 / (2 * M_PI * sigma * sigma));
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
    (void)viewNumber;
    if (m_edgeRadiusRatio > 0)
	displayEdgesTorusTubes ();
    else
	displayEdgesTorusLines ();
}

void GLWidget::displayFacesTorus (ViewNumber::Enum viewNumber) const
{
    if (m_edgeRadiusRatio > 0)
	displayFacesTorusTubes ();
    else
	displayFacesTorusLines ();
    displayStandaloneEdges< DisplayEdgePropertyColor<> > (
	GetSimulation (viewNumber).GetFoam (0));
}


void GLWidget::displayEdgesTorusTubes () const
{
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);
    EdgeSet edgeSet;
    GetSimulation ().GetFoam (0).GetEdgeSet (&edgeSet);
    for_each (
	edgeSet.begin (), edgeSet.end (),
	DisplayEdgeTorus<DisplaySegmentQuadric, 
	DisplaySegmentArrowQuadric, false>(
	    *this, 
	    GetSimulation ().GetFoam (0).GetProperties ()));
    glPopAttrib ();
}

void GLWidget::displayEdgesTorusLines () const
{
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);

    EdgeSet edgeSet;
    GetSimulation ().GetFoam (0).GetEdgeSet (&edgeSet);
    for_each (edgeSet.begin (), edgeSet.end (),
	      DisplayEdgeTorus<DisplaySegment, 
	      DisplaySegmentArrow1, false> (
		  *this, 
		  GetSimulation ().GetFoam (0).GetProperties ()));
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
	    GetCurrentTime (viewNumber) * GetTimeDisplacement () : 0;
	glPushAttrib (GL_POINT_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
	glDisable (GL_DEPTH_TEST);
	glPointSize (4.0);
	glColor (Qt::red);
	const Foam::Bodies& bodies = 
	    GetSimulation (viewNumber).GetFoam (0).GetBodies ();
	for_each (bodies.begin (), bodies.end (),
		  DisplayBodyCenter (
		      *this, 
		      GetSimulation (viewNumber).GetFoam (
			  GetCurrentTime (viewNumber)).GetProperties (),
		      bodySelector, useZPos, zPos));
	glPopAttrib ();
    }
}

void GLWidget::displayFaceCenters (ViewNumber::Enum viewNumber) const
{
    (void)viewNumber;
    if (m_faceCenterShown)
    {
	FaceSet faces = 
	    GetSimulation (viewNumber).GetFoam (
		GetCurrentTime (viewNumber)).GetFaceSet ();
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
    const Foam& foam = 
	GetSimulation (viewNumber).GetFoam (vs.GetCurrentTime ());
    const Foam::Bodies& bodies = foam.GetBodies ();
    if (m_facesShowEdges)
	displayFacesContour (bodies, viewNumber);
    displayFacesInterior (bodies, viewNumber);
    displayStandaloneEdges< DisplayEdgePropertyColor<> > (foam);
    displayAverageAroundBodies (viewNumber);
    displayContextBodies (viewNumber);
    displayContextStationaryFoam (viewNumber);
    displayStandaloneFaces ();    
    displayDeformation (viewNumber);
    displayVelocity (viewNumber);
    if (m_t1sShown)
	displayT1sDot (viewNumber, GetCurrentTime (viewNumber));
    vs.GetForceAverage ().DisplayOneTimeStep (viewNumber);
}

void GLWidget::displayT1sPDE (ViewNumber::Enum viewNumber) const
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    vs.AverageRotateAndDisplay (viewNumber);
    displayStandaloneEdges< DisplayEdgePropertyColor<> > (
	GetSimulation (viewNumber).GetFoam (0));
    

    T1sPDE& t1sPDE = vs.GetT1sPDE ();
    if (t1sPDE.IsKernelTextureSizeShown ())
    {
	size_t timeStep = GetCurrentTime (viewNumber);
	size_t stepSize = GetSimulation (viewNumber).GetT1s (timeStep).size ();
	for (size_t i = 0; i < stepSize; ++i)
	    t1sPDE.DisplayTextureSize (viewNumber, timeStep, i);
    }
}

G3D::Vector2 GLWidget::toTexture (ViewNumber::Enum viewNumber, 
				  G3D::Vector2 object) const
{
    G3D::Vector2 eye = toEye (object);
    ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::Rect2D viewRect = GetViewRect (viewNumber);
    G3D::AABox vv = calculateCenteredViewingVolume (
	viewNumber, double (viewRect.width ()) / viewRect.height (), 
	vs.GetScaleRatio ());
    return (eye - vv.low ().xy ()) / (vv.high ().xy () - vv.low ().xy ());
}

void GLWidget::displayFacesAverage (ViewNumber::Enum viewNumber) const
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    glPushAttrib (GL_ENABLE_BIT);    
    glDisable (GL_DEPTH_TEST);
    glBindTexture (GL_TEXTURE_1D, vs.GetColorBarTexture ());
    bool adjustForAverageAroundMovementRotation = 
	(vs.GetAverageAroundMovementShown () == 
	 ViewSettings::AVERAGE_AROUND_MOVEMENT_ROTATION);
    const Simulation& simulation = GetSimulation (viewNumber);

    G3D::Vector2 rotationCenter;
    float angleDegrees;
    if (vs.GetAverageAroundType () == ViewSettings::AVERAGE_AROUND)
    {
	const ObjectPosition rotationBegin = vs.GetAverageAroundPosition (0);
	const ObjectPosition rotationCurrent = 
	    vs.GetAverageAroundPosition (GetCurrentTime (viewNumber));
	rotationCenter = toEye (rotationCurrent.m_rotationCenter) - 
	    getEyeTransform (viewNumber).xy ();
	angleDegrees =
	    adjustForAverageAroundMovementRotation ? - G3D::toDegrees (
		rotationCurrent.m_angle - rotationBegin.m_angle) : 0;
    }
    else
    {
	rotationCenter = toEye (simulation.GetLowerLeftCorner ()) - 
	    getEyeTransform (viewNumber).xy ();
	angleDegrees = 0;
    }
    vs.AverageRotateAndDisplay (
	viewNumber, vs.GetStatisticsType (), rotationCenter, angleDegrees);
    vs.GetForceAverage ().Display (
	viewNumber, adjustForAverageAroundMovementRotation);
    displayStandaloneEdges< DisplayEdgePropertyColor<> > (
	GetSimulation (viewNumber).GetFoam (0));
    if (m_t1sShown)
	displayT1sDot (viewNumber);
    displayAverageAroundBodies (
	viewNumber, adjustForAverageAroundMovementRotation);
    displayContextBodies (viewNumber);
    displayContextStationaryFoam (
	viewNumber, adjustForAverageAroundMovementRotation);
    glPopAttrib ();
}

void GLWidget::displayStandaloneFaces () const
{
    if (m_standaloneElementsShown)
    {
	const Foam::Faces& faces = 
	    GetSimulation ().GetFoam (0).GetStandaloneFaces ();
	displayFacesContour (faces);
	displayFacesInterior (faces);
    }
}

void GLWidget::displayFacesContour (const Foam::Faces& faces) const
{
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT);
    for_each (faces.begin (), faces.end (),
	      DisplayFaceLineStripColor<0xff000000> (
		  *this, GetSimulation ().GetFoam (0).GetProperties ()));
    glPopAttrib ();
}

void GLWidget::displayFacesContour (
    const Foam::Bodies& bodies, ViewNumber::Enum viewNumber) const
{
    const BodySelector& bodySelector = 
	GetViewSettings (viewNumber).GetBodySelector ();
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT);
    for_each (
	bodies.begin (), bodies.end (),
	DisplayBody< DisplayFaceLineStripColor<0xff000000> > (
	    *this, 
	    GetSimulation (viewNumber).GetFoam (0).GetProperties (), 
	    bodySelector));
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
	      SetterTextureCoordinate> (
		  *this, 
		  GetSimulation (viewNumber).GetFoam (0).GetProperties (), 
		  bodySelector, SetterTextureCoordinate (
		      *this, viewNumber)));
    glPopAttrib ();
}


// See OpenGL Programming Guide, 7th edition, Chapter 6: Blending,
// Antialiasing, Fog and Polygon Offset page 293
void GLWidget::displayFacesInterior (
    const Foam::Bodies& bodies, ViewNumber::Enum viewNumber) const
{
    const BodySelector& bodySelector = 
	GetViewSettings (viewNumber).GetBodySelector ();
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | 
		  GL_TEXTURE_BIT);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1, 1);

    glEnable (GL_STENCIL_TEST);
    glClear(GL_STENCIL_BUFFER_BIT);

    glEnable(GL_TEXTURE_1D);
    //See OpenGL FAQ 21.030 Why doesn't lighting work when I turn on 
    //texture mapping?
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture (GL_TEXTURE_1D, 
		   GetViewSettings (viewNumber).GetColorBarTexture ());
    for_each (
	bodies.begin (), bodies.end (),
	DisplayBody<DisplayFaceBodyPropertyColor<> > (
	    *this, 
	    GetSimulation (viewNumber).GetFoam (0).
	    GetProperties (), bodySelector, 
	    DisplayElement::USER_DEFINED_CONTEXT, viewNumber));
    glPopAttrib ();
}


void GLWidget::displayFacesInterior (const Foam::Faces& faces) const
{
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1, 1);
    for_each (faces.begin (), faces.end (),
	      DisplayFaceDmpColor<0xff000000>(
		  *this, GetSimulation ().GetFoam (0).GetProperties ()));
    glPopAttrib ();
}

void GLWidget::displayFacesTorusTubes () const
{
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);
    FaceSet faceSet;
    GetSimulation ().GetFoam (0).GetFaceSet (&faceSet);
    for_each (
	faceSet.begin (), faceSet.end (),
	DisplayFaceHighlightColor<HighlightNumber::H0, DisplayFaceEdges<
	DisplayEdgeTorus<
	DisplaySegmentQuadric, 
	DisplaySegmentArrowQuadric, true> > > (
	    *this, 
	    GetSimulation ().GetFoam (0).GetProperties ()));
    glPopAttrib ();
}


void GLWidget::displayFacesTorusLines () const
{
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);

    FaceSet faceSet;
    GetSimulation ().GetFoam (0).GetFaceSet (&faceSet);
    for_each (faceSet.begin (), faceSet.end (),
	      DisplayFaceHighlightColor<HighlightNumber::H0,
	      DisplayFaceEdges<
	      DisplayEdgeTorus<DisplaySegment, DisplaySegmentArrow1, true> > > (
		  *this, 
		  GetSimulation ().GetFoam (0).GetProperties (), 
		  DisplayElement::FOCUS) );
    glPopAttrib ();
}

void GLWidget::displayCenterPathsWithBodies (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const BodySelector& bodySelector = vs.GetBodySelector ();
    displayCenterPaths (viewNumber);
    
    glPushAttrib (GL_ENABLE_BIT);
    if (vs.IsLightingEnabled ())
	glDisable (GL_LIGHTING);
    if (IsCenterPathBodyShown ())
    {
	const Foam::Bodies& bodies = 
	    GetSimulation (viewNumber).GetFoam (0).GetBodies ();
	double zPos = GetCurrentTime (viewNumber) * GetTimeDisplacement ();
	for_each (
	    bodies.begin (), bodies.end (),
	    DisplayBody<DisplayFaceHighlightColor<HighlightNumber::H0,
	    DisplayFaceEdges<DisplayEdgePropertyColor<
	    DisplayElement::DONT_DISPLAY_TESSELLATION> > > > (
		*this, GetSimulation (viewNumber).GetFoam (0).GetProperties (),
		bodySelector, DisplayElement::USER_DEFINED_CONTEXT,
		viewNumber, IsTimeDisplacementUsed (), zPos));
    }
    displayStandaloneEdges< DisplayEdgePropertyColor<> > (
	GetSimulation (viewNumber).GetFoam (0), true, 0);
    if (GetTimeDisplacement () != 0)
    {

	displayStandaloneEdges< DisplayEdgePropertyColor<> > (
	    GetSimulation (viewNumber).GetFoam (0),
	    IsTimeDisplacementUsed (),
	    (GetSimulation (viewNumber).GetTimeSteps () - 1) * 
	    GetTimeDisplacement ());
    }
    glPopAttrib ();
}

void GLWidget::displayCenterPaths (ViewNumber::Enum view) const
{
    glCallList (GetViewSettings (view).GetListCenterPaths ());
}

void GLWidget::labelCompileUpdate ()
{
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

void GLWidget::compileCenterPaths (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const BodySelector& bodySelector = vs.GetBodySelector ();
    glNewList (vs.GetListCenterPaths (), GL_COMPILE);
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT | 
		  GL_POLYGON_BIT | GL_LINE_BIT);
    glEnable(GL_TEXTURE_1D);
    glBindTexture (GL_TEXTURE_1D, vs.GetColorBarTexture ());
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
		SetterTextureCoordinate, DisplaySegmentTube> (
		    *this, 
		    GetSimulation (viewNumber).GetFoam (0).GetProperties (), 
		    m_viewNumber, bodySelector,
		    IsTimeDisplacementUsed (), GetTimeDisplacement ()));
	else
	    for_each (
		bats.begin (), bats.end (),
		DisplayCenterPath<
		SetterTextureCoordinate, DisplaySegmentQuadric> (
		    *this, 
		    GetSimulation (viewNumber).GetFoam (0).GetProperties (), 
		    m_viewNumber, bodySelector,
		    IsTimeDisplacementUsed (), GetTimeDisplacement ()));
    }
    else
	for_each (bats.begin (), bats.end (),
		  DisplayCenterPath<SetterTextureCoordinate, 
		  DisplaySegment> (
		      *this, 
		      GetSimulation (viewNumber).GetFoam (0).GetProperties (), 
		      m_viewNumber, bodySelector,
		      IsTimeDisplacementUsed (), GetTimeDisplacement ()));
    glPopAttrib ();
    glEndList ();
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
    return GetSimulation ().GetBodiesAlongTime ();
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

ColorBarType::Enum GLWidget::GetColorBarType () const
{
    return GetColorBarType (GetViewNumber ());
}

ColorBarType::Enum GLWidget::GetColorBarType (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    ViewType::Enum viewType = vs.GetViewType ();
    size_t property = vs.GetBodyOrFaceProperty ();
    StatisticsType::Enum statisticsType = vs.GetStatisticsType ();
    return GLWidget::GetColorBarType (viewType, property, statisticsType);
}


ColorBarType::Enum GLWidget::GetColorBarType (
    ViewType::Enum viewType, size_t property, 
    StatisticsType::Enum statisticsType)
{
    switch (viewType)
    {
    case ViewType::T1S_PDE:
	return ColorBarType::T1S_PDE;
    case ViewType::FACES_STATISTICS:
	if (statisticsType == StatisticsType::COUNT)
	    return ColorBarType::STATISTICS_COUNT;
    case ViewType::FACES:
	if (property == FaceProperty::DMP_COLOR)
	    return ColorBarType::NONE;
    case ViewType::CENTER_PATHS:
	return ColorBarType::PROPERTY;
    default:
	return ColorBarType::NONE;
    }
}

size_t GLWidget::GetCurrentTime (ViewNumber::Enum viewNumber) const
{
    return GetViewSettings (viewNumber).GetCurrentTime ();
}

void GLWidget::SetCurrentTime (size_t currentTime, bool setLastStep)
{
    switch (m_timeLinkage) 
    {
    case TimeLinkage::INDEPENDENT:
    {
	ViewNumber::Enum viewNumber = GetViewNumber ();
	ViewSettings& vs = GetViewSettings (viewNumber);
	vs.SetCurrentTime (currentTime, viewNumber);
	break;
    }
    case TimeLinkage::LINKED:
	m_linkedTime = currentTime;
	for (size_t i = 0; i < ViewCount::GetCount (m_viewCount); ++i)
	{
	    ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	    ViewSettings& vs = GetViewSettings (viewNumber);
	    float multiplier = LinkedTimeStepStretch (viewNumber);
	    size_t timeSteps = GetTimeSteps (viewNumber);
	    size_t time = floor (m_linkedTime / multiplier);
	    if (time < timeSteps)
		vs.SetCurrentTime (time, viewNumber);
	    else if (setLastStep)
		vs.SetCurrentTime (timeSteps - 1, viewNumber);
	}
	break;
    }
}


pair<size_t, ViewNumber::Enum> GLWidget::LinkedTimeMaxInterval () const
{
    pair<size_t, ViewNumber::Enum> max (0, ViewNumber::COUNT);
    for (size_t i = 0; i < ViewCount::GetCount (m_viewCount); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	ViewSettings& vs = GetViewSettings (viewNumber);
	size_t interval = vs.GetLinkedTimeInterval ();
	if (max.first < interval)
	{
	    max.first = interval;
	    max.second = viewNumber;
	}
    }
    return max;
}

pair<size_t, ViewNumber::Enum> GLWidget::LinkedTimeMaxSteps () const
{
    pair<size_t, ViewNumber::Enum> maxInterval = LinkedTimeMaxInterval ();    
    pair<size_t, ViewNumber::Enum> max (0, ViewNumber::COUNT);
    for (size_t i = 0; i < ViewCount::GetCount (m_viewCount); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	size_t maxStep = 
	    (GetTimeSteps (viewNumber) - 1) *
	    LinkedTimeStepStretch (maxInterval.first, viewNumber);
	if (max.first < maxStep)
	{
	    max.first = maxStep;
	    max.second = viewNumber;
	}
    }
    max.first += 1;
    return max;
}


float GLWidget::LinkedTimeStepStretch (ViewNumber::Enum viewNumber) const
{
    return LinkedTimeStepStretch (LinkedTimeMaxInterval ().first, viewNumber);
}

float GLWidget::LinkedTimeStepStretch (size_t max,
					  ViewNumber::Enum viewNumber) const
{
    return static_cast<float> (max) / 
	GetViewSettings (viewNumber).GetLinkedTimeInterval ();
}



size_t GLWidget::GetTimeSteps (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    ViewType::Enum viewType = vs.GetViewType ();
    size_t simulationIndex = vs.GetSimulationIndex ();
    const Simulation& simulation = GetSimulation (simulationIndex);
    return (viewType == ViewType::T1S_PDE) ?
	simulation.GetT1sTimeSteps () :
	simulation.GetTimeSteps ();
}


void GLWidget::quadricErrorCallback (GLenum errorCode)
{
    const GLubyte* message = gluErrorString (errorCode);
    cdbg << "Quadric error:" << message << endl;
}


void GLWidget::contextMenuEventColorBar (QMenu* menu) const
{
    ViewSettings& vs = GetViewSettings ();
    menu->addAction (m_actionClampClear.get ());
    bool actions = false;
    QMenu* menuCopy = menu->addMenu ("Copy");
    if (ViewCount::GetCount (m_viewCount) > 1)
    {
	size_t currentProperty = vs.GetBodyOrFaceProperty ();
	for (size_t i = 0; i < ViewCount::GetCount (m_viewCount); ++i)
	{
	    ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	    if (viewNumber == m_viewNumber ||
		GetColorBarType (m_viewNumber) != 
		GetColorBarType (viewNumber) ||
		currentProperty !=
		GetViewSettings (viewNumber).GetBodyOrFaceProperty ())
		continue;
	    menuCopy->addAction (m_actionCopyColorMap[i].get ());
	    actions = true;
	}
    }
    if (! actions)
	menuCopy->setDisabled (true);
    menu->addAction (m_actionEditColorMap.get ());
}

void GLWidget::contextMenuEventView (QMenu* menu) const
{
    ViewSettings& vs = GetViewSettings ();
    {
	QMenu* menuAverageAround = menu->addMenu ("Average around");
	menuAverageAround->addAction (m_actionAverageAroundBody.get ());
	menuAverageAround->addAction (
	    m_actionAverageAroundSecondBody.get ());
	menuAverageAround->addAction (m_actionAverageAroundReset.get ());
	m_actionAverageAroundShowRotation->setChecked (
	    vs.GetAverageAroundMovementShown () == 
	    ViewSettings::AVERAGE_AROUND_MOVEMENT_ROTATION);
	menuAverageAround->addAction (
	    m_actionAverageAroundShowRotation.get ());
    }
    {
	QMenu* menuContext = menu->addMenu ("Context display");
	menuContext->addAction (m_actionContextDisplayBody.get ());
	menuContext->addAction (m_actionContextDisplayReset.get ());
    }
    if (ViewCount::GetCount (m_viewCount) > 1)
    {
	QMenu* menuCopy = menu->addMenu ("Copy");
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
    {
	QMenu* menuInfo = menu->addMenu ("Info");
	menuInfo->addAction (m_actionInfoPoint.get ());
	menuInfo->addAction (m_actionInfoEdge.get ());
	menuInfo->addAction (m_actionInfoFace.get ());
	menuInfo->addAction (m_actionInfoBody.get ());
	menuInfo->addAction (m_actionInfoFoam.get ());
	menuInfo->addAction (m_actionInfoOpenGL.get ());
    }
    {
	QMenu* menuLinkedTime = menu->addMenu ("Linked time");
	menuLinkedTime->addAction (m_actionLinkedTimeBegin.get ());
	menuLinkedTime->addAction (m_actionLinkedTimeEnd.get ());
    }
    {
	QMenu* menuReset = menu->addMenu ("Reset transform");
	menuReset->addAction (m_actionResetTransformAll.get ());
	menuReset->addAction (m_actionResetTransformFocus.get ());
	menuReset->addAction (m_actionResetTransformContext.get ());
	menuReset->addAction (m_actionResetTransformLight.get ());
	menuReset->addAction (m_actionResetTransformGrid.get ());
    }
    {
	QMenu* menuSelect = menu->addMenu ("Select");
	menuSelect->addAction (m_actionSelectAll.get ());
	menuSelect->addAction (m_actionDeselectAll.get ());
	menuSelect->addAction (m_actionSelectBodiesById.get ());
    }
    {
	QMenu* menuShow = menu->addMenu ("Show");
	menuShow->addAction (m_actionShowNeighbors.get ());
	menuShow->addAction (m_actionShowDeformation.get ());
	menuShow->addAction (m_actionShowVelocity.get ());
	menuShow->addAction (m_actionShowReset.get ());
    }
}


void GLWidget::contextMenuEvent (QContextMenuEvent *event)
{
    m_contextMenuPosScreen = event->pos ();
    m_contextMenuPosObject = toObjectTransform (m_contextMenuPosScreen);
    QMenu menu (this);
    G3D::Rect2D colorBarRect = getViewColorBarRect (GetViewRect ());
    if (colorBarRect.contains (QtToOpenGl (m_contextMenuPosScreen, height ())))
	contextMenuEventColorBar (&menu);
    else
	contextMenuEventView (&menu);
    menu.exec (event->globalPos());
}


void GLWidget::displayViewDecorations (ViewNumber::Enum viewNumber)
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    initTransformViewport ();
    if (vs.IsLightingEnabled ())
	glDisable (GL_LIGHTING);
    glDisable (GL_DEPTH_TEST);
    G3D::Rect2D viewRect = GetViewRect (viewNumber);
    if (GetColorBarType (viewNumber) != ColorBarType::NONE)
	displayTextureColorBar (viewNumber, viewRect);
    if (vs.IsVelocityShown ())
	displayVelocityOverlayBar (viewNumber, viewRect);
    displayViewTitle (viewNumber);
    if (viewNumber == GetViewNumber () && m_viewFocusShown &&
	GetViewCount () != ViewCount::ONE)
	displayViewFocus (viewNumber);
    cleanupTransformViewport ();
}

void GLWidget::initTransformViewport ()
{
    glPushAttrib (
	GL_POLYGON_BIT | GL_CURRENT_BIT | 
	GL_VIEWPORT_BIT | GL_TEXTURE_BIT | GL_ENABLE_BIT);
    glPushMatrix ();
    glLoadIdentity ();
    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity ();
    glOrtho (0, width (), 0, height (), -1, 1);
    glViewport (0, 0, width (), height ());
}

void GLWidget::cleanupTransformViewport ()
{
    glPopMatrix ();
    glMatrixMode (GL_MODELVIEW);
    glPopMatrix ();
    glPopAttrib ();
}

void GLWidget::displayViewTitle (ViewNumber::Enum viewNumber)
{
    if (! m_titleShown)
	return;
    ostringstream ostr;
    ViewSettings& vs = GetViewSettings (viewNumber);
    ostr << "View " << viewNumber << " - "
	 << ViewType::ToString (vs.GetViewType ()) << " - "
	 << BodyOrFacePropertyToString (vs.GetBodyOrFaceProperty ()) << " - "
	 << vs.GetCurrentTime ();    
    displayViewText (viewNumber, GetSimulation (viewNumber).GetName (), 0);
    displayViewText (viewNumber, ostr.str (), 1);
}

void GLWidget::displayViewText (
    ViewNumber::Enum viewNumber, const string& t, size_t row)
{
    G3D::Rect2D viewRect = GetViewRect (viewNumber);
    QFont font;
    QString text (t.c_str ());
    QFontMetrics fm (font);
    const int textX = 
	viewRect.x0 () + (float (viewRect.width ()) - fm.width (text)) / 2;
    const int textY = OpenGlToQt (
	viewRect.y1 () - fm.lineSpacing () * (row + 1), height ());
    glColor (GetHighlightColor (viewNumber, HighlightNumber::H0));
    renderText (textX, textY, text, font);    
}



size_t GLWidget::GetBodyOrFaceProperty (ViewNumber::Enum viewNumber) const
{
    return GetViewSettings (viewNumber).GetBodyOrFaceProperty ();
}


void GLWidget::displayViewFocus (ViewNumber::Enum viewNumber)
{
    G3D::Rect2D viewRect = GetViewRect (viewNumber);
    G3D::Vector2 margin (3, 3);
    G3D::Rect2D rect = G3D::Rect2D::xyxy(
	viewRect.x0y0 () + margin, viewRect.x1y1 () - margin);
    glColor (GetHighlightColor (viewNumber, HighlightNumber::H0));
    glPolygonMode (GL_FRONT, GL_LINE);
    DisplayBox (rect);
}

void GLWidget::displayTextureColorBar (
    ViewNumber::Enum viewNumber, const G3D::Rect2D& viewRect)
{
    G3D::Rect2D colorBarRect = getViewColorBarRect (viewRect);
    glPushAttrib (GL_POLYGON_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT);

    glDisable (GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_1D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture (GL_TEXTURE_1D, 
		   GetViewSettings (viewNumber).GetColorBarTexture ());
    
    glBegin (GL_QUADS);
    glTexCoord1f(0);::glVertex (colorBarRect.x0y0 ());
    glTexCoord1f(1);::glVertex (colorBarRect.x0y1 ());
    glTexCoord1f(1);::glVertex (colorBarRect.x1y1 ());
    glTexCoord1f(0);::glVertex (colorBarRect.x1y0 ());
    glEnd ();
    glDisable (GL_TEXTURE_1D);

    glColor (GetHighlightColor (viewNumber, HighlightNumber::H0));
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    DisplayBox (colorBarRect);
    glPopAttrib ();
}

void GLWidget::displayVelocityOverlayBar (
    ViewNumber::Enum viewNumber, const G3D::Rect2D& viewRect)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::Rect2D r = getViewOverlayBarRect (viewRect);
    glPushAttrib (GL_POLYGON_BIT | GL_ENABLE_BIT | GL_LINE_BIT);
    glDisable (GL_DEPTH_TEST);
    glColor (Qt::white);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    DisplayBox (r);
    glColor (GetHighlightColor (viewNumber, HighlightNumber::H0));
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    DisplayBox (r);
    float y = r.y0 () + (r.y1 () - r.y0 ()) / vs.GetVelocitySize ();
    glLineWidth (2);    
    glBegin (GL_LINES);
    ::glVertex (G3D::Vector2 (r.x0 (), y));
    ::glVertex (G3D::Vector2 (r.x1 (), y));
    glEnd ();
    glPopAttrib ();
}


void GLWidget::displayViewsGrid ()
{
    initTransformViewport ();
    size_t w = width ();
    size_t h = height ();
    glDisable (GL_DEPTH_TEST);
    glColor (GetHighlightColor (ViewNumber::VIEW0, HighlightNumber::H0));
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
    cleanupTransformViewport ();
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

/**
 * Activate a shader for each fragment where the Quad is drawn on destRect. 
 * Rotate the Quad if angleDegrees != 0.
 * We use the following notation: VV = viewing volume, VP = viewport, 
 * Q = quad, 1 = original VV, 2 = enclosing VV
 * Can be called in 2 situations:
 *                        VV    VP, Q
 * 1. fbo -> fbo or img : 2  -> 2 , 2       ENCLOSE2D
 * 3. fbo -> scr        : 1  -> 1,  2       DONT_ENCLOSE2D
 *
 * @see doc/TensorDisplay.pdf
 */
void GLWidget::activateViewShader (
    ViewNumber::Enum viewNumber, 
    ViewingVolumeOperation::Enum enclose, G3D::Rect2D& srcRect,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    G3D::Rect2D destRect = GetViewRect (viewNumber);
    if (enclose == ViewingVolumeOperation::ENCLOSE2D)
    {
	destRect = EncloseRotation (destRect);
	destRect = destRect - destRect.x0y0 ();
    }
    glPushAttrib (GL_VIEWPORT_BIT);
    glViewport (destRect.x0 (), destRect.y0 (),
		destRect.width (), destRect.height ());
    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glLoadIdentity ();
    glTranslate (getEyeTransform (viewNumber));
    if (angleDegrees != 0)
    {
	glTranslate (rotationCenter);
	glRotatef (angleDegrees, 0, 0, 1);	
	glTranslate (- rotationCenter);
    }
    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    ProjectionTransform (viewNumber, enclose);
    glBegin (GL_QUADS);
    sendQuad (srcRect, G3D::Rect2D::xyxy (0., 0., 1., 1.));
    glEnd ();
    glMatrixMode (GL_PROJECTION);
    glPopMatrix ();
    glMatrixMode (GL_MODELVIEW);
    glPopMatrix ();
    glPopAttrib ();
}

void GLWidget::ActivateViewShader (ViewNumber::Enum viewNumber) const
{
    ActivateViewShader (viewNumber, ViewingVolumeOperation::ENCLOSE2D,
			G3D::Vector2::zero (), 0);
}

void GLWidget::ActivateViewShader (ViewNumber::Enum viewNumber, 
				   ViewingVolumeOperation::Enum enclose,
				   G3D::Rect2D& srcRect) const
{
    activateViewShader (viewNumber, enclose,
			srcRect, G3D::Vector2::zero (), 0);
}


void GLWidget::ActivateViewShader (
    ViewNumber::Enum viewNumber, ViewingVolumeOperation::Enum enclose,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    G3D::Rect2D srcRect = 
	toRect2D (CalculateViewingVolume (viewNumber,
					  ViewingVolumeOperation::ENCLOSE2D));
    activateViewShader (viewNumber, enclose, srcRect, 
			rotationCenter, angleDegrees);
}

G3D::Vector3 GLWidget::getScaleCenterTranslation (
    ViewNumber::Enum viewNumber) const
{
    G3D::Vector2 center = 
	GetSimulation (viewNumber).GetBoundingBox ().center ().xy ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::Vector2 newCenter = vs.GetScaleCenter ();
    return G3D::Vector3 (newCenter - center, 0);
}

void GLWidget::setScaleCenter (ViewNumber::Enum viewNumber)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::Rect2D rect = 
	toRect2D (CalculateViewingVolume (
		      viewNumber, 
		      ViewingVolumeOperation::DONT_ENCLOSE2D));
    G3D::Vector2 newCenter = getScaleCenter (viewNumber, rect);
    vs.SetScaleCenter (newCenter);
}


void GLWidget::SetTransformLinkage (TransformLinkage::Enum transformLinkage)
{
    m_transformLinkage = transformLinkage;
    setScaleCenter (ViewNumber::VIEW0);
    setScaleCenter (ViewNumber::VIEW1);
    update ();
}

void GLWidget::valueChangedT1sKernelSigma (ViewNumber::Enum viewNumber)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    T1sPDE& t1sPDE = vs.GetT1sPDE ();
    t1sPDE.SetKernelSigma (
	Index2Value (static_cast<QSlider*> (sender ()), T1sPDE::KERNEL_SIGMA));
    t1sPDE.AverageInitStep (viewNumber);
}

void GLWidget::valueChangedT1sKernelTextureSize (
    ViewNumber::Enum viewNumber)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    T1sPDE& t1sPDE = vs.GetT1sPDE ();
    t1sPDE.SetKernelTextureSize (
	Index2Value (static_cast<QSlider*> (sender ()), 
		     T1sPDE::KERNEL_TEXTURE_SIZE));
    t1sPDE.AverageInitStep (viewNumber);    
}

void GLWidget::toggledT1sKernelTextureSizeShown (
    ViewNumber::Enum viewNumber, bool checked)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    T1sPDE& t1sPDE = vs.GetT1sPDE ();
    t1sPDE.SetKernelTextureSizeShown (checked);    
}

void GLWidget::setOneOrTwoViews (void (GLWidget::*f) (ViewNumber::Enum))
{
    if (m_transformLinkage == TransformLinkage::INDEPENDENT)
	CALL_MEMBER_FN (*this, f) (GetViewNumber ());
    else
    {
	CALL_MEMBER_FN (*this, f) (ViewNumber::VIEW0);
	CALL_MEMBER_FN (*this, f) (ViewNumber::VIEW1);
    }
}


void GLWidget::valueChangedT1sKernelIntervalPerPixel (
    ViewNumber::Enum viewNumber)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    T1sPDE& t1sPDE = vs.GetT1sPDE ();
    t1sPDE.SetKernelIntervalPerPixel (
	Index2Value (static_cast<QSlider*> (sender ()), 
		     T1sPDE::KERNEL_INTERVAL_PER_PIXEL));
    t1sPDE.AverageInitStep (viewNumber);
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
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings ();
    LightNumber::Enum selectedLight = vs.GetSelectedLight ();
    vs.SetDirectionalLightEnabled (selectedLight, checked);
    vs.PositionLight (selectedLight, 
		      getInitialLightPosition (viewNumber, selectedLight));
    update ();
}

void GLWidget::ToggledShowDeformation (bool checked)
{
    ViewSettings& vs = GetViewSettings ();
    vs.SetDeformationTensorShown (checked);
    update ();
}

void GLWidget::ToggledShowDeformationGrid (bool checked)
{
    TensorAverage& ta = GetViewSettings ().GetDeformationAverage ();
    ta.SetGridShown (checked);
    update ();
}

void GLWidget::ToggledShowVelocity (bool checked)
{
    ViewSettings& vs = GetViewSettings ();
    vs.SetVelocityShown (checked);
    update ();
}

void GLWidget::ToggledShowVelocityGrid (bool checked)
{
    VectorAverage& va = GetViewSettings ().GetVelocityAverage ();
    va.SetGridShown (checked);
    update ();
}



void GLWidget::ToggledShowDeformationGridCellCenter (bool checked)
{
    TensorAverage& ta = GetViewSettings ().GetDeformationAverage ();
    ta.SetGridCellCenterShown (checked);
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
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings ();
    LightNumber::Enum selectedLight = vs.GetSelectedLight ();
    vs.SetLightEnabled (selectedLight, checked);
    vs.EnableLighting ();
    if (checked)
	vs.PositionLight (
	    selectedLight, 
	    getInitialLightPosition (viewNumber, selectedLight));
    update ();
}


void GLWidget::ToggledBoundingBoxShown (bool checked)
{
    m_boundingBoxShown = checked;
    update ();
}

void GLWidget::ToggledAverageAroundMarked (bool checked)
{
    m_averageAroundMarked = checked;
    update ();
}

void GLWidget::ToggledViewFocusShown (bool checked)
{
    m_viewFocusShown = checked;
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

void GLWidget::ToggledConstraintsShown (bool checked)
{
    m_constraintsShown = checked;
    update ();
}

void GLWidget::ToggledConstraintPointsShown (bool checked)
{
    m_constraintPointsShown = checked;
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
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    ViewType::Enum newViewType = ViewType::Enum(id);
    ViewType::Enum oldViewType = GetViewSettings ().GetViewType ();
    if (oldViewType == newViewType)
	return;
    vs.AverageRelease ();
    vs.SetViewType (newViewType);
    vs.AverageInitStep (viewNumber);
    compile (viewNumber);
    update ();
}

void GLWidget::ButtonClickedTimeLinkage (int id)
{
    m_timeLinkage = TimeLinkage::Enum (id);
    SetCurrentTime (GetCurrentTime ());
    update ();
    Q_EMIT ViewChanged ();
}

void GLWidget::ToggledBodyCenterShown (bool checked)
{
    m_bodyCenterShown = checked;
    update ();
}

void GLWidget::ToggledBodyNeighborsShown (bool checked)
{
    m_bodyNeighborsShown = checked;
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
    ViewSettings& vs = GetViewSettings ();
    vs.SetT1sShiftLower (checked);
    update ();
}

void GLWidget::CurrentIndexChangedSelectedLight (int selectedLight)
{
    ViewSettings& vs = GetViewSettings ();
    vs.SetSelectedLight (LightNumber::Enum (selectedLight));
}

void GLWidget::CurrentIndexChangedSimulation (int i)
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    const Simulation& simulation = GetSimulation (i);
    int rotation2D = simulation.GetRotation2D ();
    vs.SetSimulationIndex (i);
    vs.SetAxesOrder (
	simulation.Is2D () ? 
	(rotation2D == 0 ? AxesOrder::TWO_D :
	 (rotation2D == 90 ? AxesOrder::TWO_D_ROTATE_LEFT90 : 
	  AxesOrder::TWO_D_ROTATE_RIGHT90)): AxesOrder::THREE_D);
    vs.SetT1sShiftLower (simulation.GetT1sShiftLower ());
    vs.AverageSetTimeWindow (simulation.GetTimeSteps ());
    vs.SetScaleCenter (
	CalculateViewingVolume (
	    viewNumber, 
	    ViewingVolumeOperation::DONT_ENCLOSE2D).center ().xy ());
    update ();
}

void GLWidget::CurrentIndexChangedViewCount (int index)
{
    m_viewCount = ViewCount::Enum (index);
    m_viewNumber = ViewNumber::VIEW0;
    size_t n = ViewCount::GetCount (m_viewCount);
    for (size_t i = 0; i < n; ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	G3D::Rect2D viewRect = GetViewRect (viewNumber);
	ViewSettings& vs = GetViewSettings (viewNumber);
	if (vs.GetViewType () == ViewType::COUNT)
	    vs.SetViewType (ViewType::FACES);
	vs.CalculateCameraDistance (
	    calculateCenteredViewingVolume (
		viewNumber,
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

void GLWidget::ButtonClickedInteractionObject (int index)
{
    m_interactionObject = InteractionObject::Enum (index);
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
void GLWidget::SetBodyOrFaceProperty (
    ViewNumber::Enum viewNumber,
    boost::shared_ptr<ColorBarModel> colorBarModel,
    size_t bodyOrFaceProperty)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    vs.SetBodyOrFaceProperty (bodyOrFaceProperty);
    if (vs.GetBodyOrFaceProperty () != FaceProperty::DMP_COLOR)
	vs.SetColorBarModel (colorBarModel);
    else
	vs.ResetColorBarModel ();
    compile (viewNumber);
    update ();
}

void GLWidget::SetBodyOrFaceProperty (
    boost::shared_ptr<ColorBarModel> colorBarModel,
    size_t bodyOrFaceProperty)
{
    SetBodyOrFaceProperty (GetViewNumber (), colorBarModel, bodyOrFaceProperty);
}



void GLWidget::SetColorBarModel (boost::shared_ptr<ColorBarModel> colorBarModel)
{
    makeCurrent ();
    GetViewSettings ().SetColorBarModel (colorBarModel);
    update ();
}

void GLWidget::ValueChangedSliderTimeSteps (int timeStep)
{
    SetCurrentTime (timeStep);
    update ();
}

void GLWidget::ClickedEnd ()
{
    size_t steps = ((GetTimeLinkage () == TimeLinkage::INDEPENDENT) ?
		    GetTimeSteps () : LinkedTimeMaxSteps ().first);
    SetCurrentTime (steps - 1, true);
    update ();
}

void GLWidget::ValueChangedStatisticsTimeWindow (int timeSteps)
{
    ViewSettings& vs = GetViewSettings ();
    vs.AverageSetTimeWindow (timeSteps);
}

void GLWidget::ValueChangedTimeDisplacement (int timeDisplacement)
{
    QSlider* slider = static_cast<QSlider*> (sender ());
    size_t maximum = slider->maximum ();
    G3D::AABox bb = GetSimulation ().GetBoundingBox ();
    m_timeDisplacement =
	(bb.high () - bb.low ()).z * timeDisplacement /
	GetSimulation ().GetTimeSteps () / maximum;
    compile (GetViewNumber ());
    update ();
}

void GLWidget::ValueChangedT1Size (int index)
{
    (void)index;
    m_t1sSize = Index2Value (static_cast<QSlider*> (sender ()), T1S_SIZE);
    update ();
}


void GLWidget::ValueChangedT1sKernelIntervalPerPixel (int index)
{
    (void)index;
    setOneOrTwoViews (&GLWidget::valueChangedT1sKernelIntervalPerPixel);
    update ();
}

void GLWidget::ValueChangedT1sKernelSigma (int index)
{
    (void)index;
    setOneOrTwoViews (&GLWidget::valueChangedT1sKernelSigma);
    update ();
}

void GLWidget::ValueChangedT1sKernelTextureSize (int index)
{
    (void)index;
    setOneOrTwoViews (&GLWidget::valueChangedT1sKernelTextureSize);
    update ();
}

void GLWidget::ToggledT1sKernelTextureSizeShown (bool checked)
{
    if (m_transformLinkage == TransformLinkage::INDEPENDENT)
	toggledT1sKernelTextureSizeShown (GetViewNumber (), checked);
    else
    {
	toggledT1sKernelTextureSizeShown (ViewNumber::VIEW0, checked);
	toggledT1sKernelTextureSizeShown (ViewNumber::VIEW1, checked);
    }
    update ();
}

void GLWidget::ValueChangedDeformationSizeExp (int index)
{
    (void)index;
    ViewSettings& vs = GetViewSettings ();
    vs.SetDeformationSize (
	IndexExponent2Value (
	    static_cast<QSlider*> (sender ()), TENSOR_SIZE_EXP2));
    update ();
}

void GLWidget::ValueChangedDeformationLineWidthExp (int index)
{
    (void)index;
    ViewSettings& vs = GetViewSettings ();
    vs.SetDeformationLineWidth (
	IndexExponent2Value (
	    static_cast<QSlider*> (sender ()), TENSOR_LINE_WIDTH_EXP2));
    update ();
}

void GLWidget::ValueChangedVelocitySizeExp (int index)
{
    (void)index;
    ViewSettings& vs = GetViewSettings ();
    vs.SetVelocitySize (
	IndexExponent2Value (static_cast<QSlider*> (sender ()), TENSOR_SIZE_EXP2));
    update ();
}

void GLWidget::ValueChangedForceSizeExp (int index)
{
    (void)index;
    ViewSettings& vs = GetViewSettings ();
    vs.SetForceSize (
	IndexExponent2Value (static_cast<QSlider*> (sender ()), FORCE_SIZE_EXP2));
    update ();
}

void GLWidget::ValueChangedForceLineWidthExp (int index)
{
    (void)index;
    ViewSettings& vs = GetViewSettings ();
    vs.SetForceLineWidth (IndexExponent2Value (static_cast<QSlider*> (sender ()),
				     TENSOR_LINE_WIDTH_EXP2));
    update ();
}


void GLWidget::ValueChangedVelocityLineWidthExp (int index)
{
    (void)index;
    ViewSettings& vs = GetViewSettings ();
    vs.SetVelocityLineWidth (
	IndexExponent2Value (static_cast<QSlider*> (sender ()),
			     TENSOR_LINE_WIDTH_EXP2));
    update ();
}

void GLWidget::ValueChangedContextAlpha (int index)
{
    (void)index;
    m_contextAlpha = Index2Value (static_cast<QSlider*> (sender ()), 
				  CONTEXT_ALPHA);
    compile (GetViewNumber ());
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
    ViewNumber::Enum viewNumber = GetViewNumber ();
    G3D::Rect2D viewRect = GetViewRect ();
    ViewSettings& vs = GetViewSettings ();
    vs.SetAngleOfView (angleOfView);
    vs.CalculateCameraDistance (
	calculateCenteredViewingVolume (
	    viewNumber,
	    viewRect.width () / viewRect.height (), vs.GetScaleRatio ()));
    update ();
}
