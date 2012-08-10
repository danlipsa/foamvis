/**
 * @file   WidgetGl.cpp
 * @author Dan R. Lipsa
 *
 * Definitions for the widget for displaying foam bubbles using OpenGL
 */

// @todo fix the (slow) movement of the focus in context view
// @todo replace glScale with camera movement

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
#include "WidgetGl.h"
#include "Info.h"
#include "MainWindow.h"
#include "OpenGLUtils.h"
#include "OrientedEdge.h"
#include "OrientedFace.h"
#include "SelectBodiesById.h"
#include "Settings.h"
#include "T1sPDE.h"
#include "TensorAverage.h"
#include "Utils.h"
#include "Vertex.h"
#include "ViewSettings.h"
#include "ViewAverage.h"
#include "VectorAverage.h"

#define __LOG__(code) code
//#define __LOG__(code)


// Private Classes/Functions
// ======================================================================

struct FocusContextInfo
{
    Foam::Bodies::const_iterator m_begin;
    Foam::Bodies::const_iterator m_end;
    bool m_isContext;	
};

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

template <typename T>
void display (const char* name, const T& what)
{
    ostream_iterator<GLfloat> out (cdbg, " ");
    cdbg << name;
    copy (what.begin (), what.end (), out);
    cdbg << endl;
}

void displayBodyNeighbors (boost::shared_ptr<Body> body, 
			     const OOBox& originalDomain)
{
    if (body->IsObject ())
	return;
    BOOST_FOREACH (Body::Neighbor neighbor, body->GetNeighbors ())
    {
	G3D::Vector3 s;
	if (neighbor.GetBody ())
	{
	    s = neighbor.GetBody ()->GetCenter ();
	}
	else
	{
	    // debug
	    // no reflection displayed
	    // continue;
	    s = neighbor.GetCenterReflection ();
	}
	G3D::Vector3 first = body->GetCenter ();	    
	G3D::Vector3 second = 
	    originalDomain.TorusTranslate (s, neighbor.GetTranslation ());
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

const size_t WidgetGl::DISPLAY_ALL(numeric_limits<size_t>::max());

const pair<float,float> WidgetGl::T1S_SIZE (1, 32);
const pair<float,float> WidgetGl::TENSOR_SIZE_EXP2 (0, 10);
const pair<float,float> WidgetGl::TENSOR_LINE_WIDTH_EXP2 (0, 3);
const pair<float,float> WidgetGl::FORCE_SIZE_EXP2 (-2, 2);
const pair<float,float> WidgetGl::TORQUE_SIZE_EXP2 (-4, 4);
const GLfloat WidgetGl::HIGHLIGHT_LINE_WIDTH = 2.0;

// Methods
// ======================================================================

WidgetGl::WidgetGl(QWidget *parent)
    : QGLWidget(parent),
      m_torusDomainShown (false),
      m_torusOriginalDomainClipped (false),
      m_interactionMode (InteractionMode::ROTATE),
      m_interactionObject (InteractionObject::FOCUS),
      m_simulationGroup (0), 
      m_edgesShown (true),
      m_bodyCenterShown (false),
      m_bodyNeighborsShown (false),
      m_faceCenterShown (false),
      m_centerPathBodyShown (false),
      m_boundingBoxSimulationShown (false),
      m_boundingBoxFoamShown (false),
      m_boundingBoxBodyShown (false),
      m_axesShown (false),
      m_standaloneElementsShown (true),
      m_timeDisplacement (0.0),
      m_selectBodiesByIdList (new SelectBodiesById (this)),
      m_t1sShown (false),
      m_t1sSize (1.0),
      m_highlightLineWidth (HIGHLIGHT_LINE_WIDTH),
      m_titleShown (false),
      m_averageAroundMarked (true),
      m_viewFocusShown (true),
      m_contextBoxShown (true),
      m_reflectedHalfView (false),
      m_showType (SHOW_NOTHING)
{
    makeCurrent ();
    initList ();
    initTexture ();
    initQuadrics ();
    initDisplayView ();
    createActions ();
}


WidgetGl::~WidgetGl()
{
    makeCurrent();
    gluDeleteQuadric (m_quadric);
    m_quadric = 0;
    glDeleteLists (m_listCenterPaths[0], m_listCenterPaths.size ());
    glDeleteLists (m_listFacesNormal[0], m_listFacesNormal.size ());
    glDeleteTextures (m_colorBarTexture.size (), &m_colorBarTexture[0]);
    glDeleteTextures (m_overlayBarTexture.size (), &m_overlayBarTexture[0]);
}


void WidgetGl::initTexture ()
{
    initTexture (&m_colorBarTexture);
    initTexture (&m_overlayBarTexture);
}

void WidgetGl::initTexture (boost::array<GLuint, ViewNumber::COUNT>* texture)
{
    glGenTextures (texture->size (), &(*texture)[0]);
    for (size_t i = 0; i < texture->size (); ++i)
    {
	glBindTexture (GL_TEXTURE_1D, (*texture)[i]);
	glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
}



void WidgetGl::initList (boost::array<GLuint, ViewNumber::COUNT>* list)
{
    GLuint n =  glGenLists (list->size ());
    for (size_t i = 0; i < list->size (); ++i)
	(*list)[i] = n + i;
}

void WidgetGl::initList ()
{
    initList (&m_listCenterPaths);
    initList (&m_listFacesNormal);
}



void WidgetGl::initQuadrics ()
{
    m_quadric = gluNewQuadric ();
    gluQuadricCallback (m_quadric, GLU_ERROR,
			reinterpret_cast<void (*)()>(&quadricErrorCallback));
    gluQuadricDrawStyle (m_quadric, GLU_FILL);
    gluQuadricNormals (m_quadric, GLU_SMOOTH);
    gluQuadricOrientation (m_quadric, GLU_OUTSIDE);
}


void WidgetGl::createActions ()
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

    m_actionSelectBodiesByIdList = boost::make_shared<QAction> (
	tr("&Bodies by id"), this);
    m_actionSelectBodiesByIdList->setStatusTip(tr("Select bodies by id"));
    connect(m_actionSelectBodiesByIdList.get (), SIGNAL(triggered()),
	    this, SLOT(SelectBodiesByIdList ()));

    m_actionSelectThisBodyOnly = boost::make_shared<QAction> (
	tr("&This body only"), this);
    m_actionSelectThisBodyOnly->setStatusTip(tr("This body only"));
    connect(m_actionSelectThisBodyOnly.get (), SIGNAL(triggered()),
	    this, SLOT(SelectThisBodyOnly ()));

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

    m_actionRotationCenterBody = boost::make_shared<QAction> (
	tr("&Body"), this);
    m_actionRotationCenterBody->setStatusTip(tr("Rotation center body"));
    connect(m_actionRotationCenterBody.get (), SIGNAL(triggered()),
	    this, SLOT(RotationCenterBody ()));

    m_actionRotationCenterFoam = boost::make_shared<QAction> (
	tr("&Foam"), this);
    m_actionRotationCenterFoam->setStatusTip(tr("Rotation center foam"));
    connect(m_actionRotationCenterFoam.get (), SIGNAL(triggered()),
	    this, SLOT(RotationCenterFoam ()));

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

    m_actionInfoSelectedBodies = 
	boost::make_shared<QAction> (tr("&Selected bodies"), this);
    m_actionInfoSelectedBodies->setStatusTip(tr("Info selected bodies"));
    connect(m_actionInfoSelectedBodies.get (), SIGNAL(triggered()), this, 
	    SLOT(InfoSelectedBodies ()));

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
    m_actionEditOverlayMap.reset (
	new QAction (tr("&Edit overlay map"), this));
    m_actionEditOverlayMap->setStatusTip(
	tr("Edit overlay map"));
    // connected in MainWindow

    // actions for the color bar
    m_actionEditColorMap.reset (
	new QAction (tr("&Edit color map"), this));
    m_actionEditColorMap->setStatusTip(tr("Edit color map"));
    // connected in MainWindow

    m_actionColorBarClampClear.reset (
	new QAction (tr("&Clamp clear"), this));
    m_actionColorBarClampClear->setStatusTip(tr("Clamp clear"));
    connect(m_actionColorBarClampClear.get (), SIGNAL(triggered()),
	    this, SLOT(ColorBarClampClear ()));

    m_actionOverlayBarClampClear.reset (
	new QAction (tr("&Clamp clear"), this));
    m_actionOverlayBarClampClear->setStatusTip(tr("Clamp clear"));
    connect(m_actionOverlayBarClampClear.get (), SIGNAL(triggered()),
	    this, SLOT(OverlayBarClampClear ()));

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

void WidgetGl::initCopy (
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


void WidgetGl::initDisplayView ()
{
    // WARNING: This has to be in the same order as ViewType::Enum
    boost::array<ViewTypeDisplay, ViewType::COUNT> displayView =
	{{&WidgetGl::displayEdgesNormal,
	  &WidgetGl::displayEdgesTorus,
	  &WidgetGl::displayFacesTorus,
	  &WidgetGl::displayFacesNormal,
	  &WidgetGl::displayCenterPathsWithBodies,
	  &WidgetGl::displayFacesAverage,
	  &WidgetGl::displayFacesAverage,
	    }};
    copy (displayView.begin (), displayView.end (), m_display.begin ());
}


const Simulation& WidgetGl::GetSimulation () const
{
    return GetSimulation (GetViewNumber ());
}

const Simulation& WidgetGl::GetSimulation (ViewNumber::Enum viewNumber) const
{
    return GetSimulation (
	GetViewSettings (viewNumber).GetSimulationIndex ());
}

const Simulation& WidgetGl::GetSimulation (size_t i) const
{
    return m_simulationGroup->GetSimulation (i);
}

void WidgetGl::setSimulationGroup (SimulationGroup* simulationGroup)
{
    m_simulationGroup = simulationGroup;
    Foam::Bodies bodies = GetSimulation ().GetFoam (0).GetBodies ();
    if (bodies.size () != 0)
    {
	size_t maxIndex = bodies.size () - 1;
	m_selectBodiesByIdList->SetMinBodyId (bodies[0]->GetId ());
	m_selectBodiesByIdList->SetMaxBodyId (bodies[maxIndex]->GetId ());
	m_selectBodiesByIdList->UpdateLabelMinMax ();
    }
}

void WidgetGl::Init (
    boost::shared_ptr<Settings> settings, SimulationGroup* simulationGroup)
{
    m_settings = settings;
    setSimulationGroup (simulationGroup);
    for (size_t i = 0; i < ViewNumber::COUNT; ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	m_viewAverage[i].reset (
	    new ViewAverage (viewNumber,
			     *this, m_settings->GetViewSettings (viewNumber)));
	m_viewAverage[i]->SetSimulation (simulationGroup->GetSimulation (0));
    }
    update ();
}

float WidgetGl::GetBubbleSize (ViewNumber::Enum defaultViewNumber) const
{    
    vector<ViewNumber::Enum> vn = GetConnectedViewNumbers (defaultViewNumber);
    float size = GetSimulation (vn[0]).GetBubbleSize ();
    for (size_t i = 1; i < vn.size (); ++i)
    {
	float s = GetSimulation (vn[i]).GetBubbleSize ();
	size = min (size, s);
    }
    return size;
}

float WidgetGl::GetDeformationSizeInitialRatio (
    ViewNumber::Enum viewNumber) const
{
    const Simulation& simulation = GetSimulation (viewNumber);
    float cellLength = GetBubbleSize (viewNumber);
    return cellLength / (2 * simulation.GetMaxDeformationEigenValue ());
}

float WidgetGl::GetVelocitySizeInitialRatio (
    ViewNumber::Enum viewNumber) const
{
    float cellLength = GetBubbleSize (viewNumber);
    float velocityMagnitude = 
	GetSimulation (viewNumber).GetMax (BodyScalar::VELOCITY_MAGNITUDE);
    return cellLength / velocityMagnitude;
}


QSize WidgetGl::minimumSizeHint()
{
    return QSize(50, 50);
}

QSize WidgetGl::sizeHint()
{
    return QSize(512, 512);
}


void WidgetGl::displayLightDirection (ViewNumber::Enum viewNumber) const
{
    for (size_t i = 0; i < LightNumber::COUNT; ++i)
	displayLightDirection (viewNumber, LightNumber::Enum (i));
}

void WidgetGl::displayLightDirection (
    ViewNumber::Enum viewNumber, LightNumber::Enum i) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    if (vs.IsLightPositionShown (i))
    {
	const double sqrt3 = sqrt (3.0);
	glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT | GL_POINT_BIT);
	glPushMatrix ();
	glLoadIdentity ();
	glTranslatef (0, 0, - vs.GetCameraDistance ());
	glMultMatrix (vs.GetRotationLight (i));
	G3D::Vector3 initialLightPosition = 
	    vs.GetInitialLightPosition (
		calculateCenteredViewingVolume (viewNumber), i);
	G3D::Vector3 lp =  initialLightPosition / sqrt3;
	::glColor (QColor (vs.IsLightEnabled (i) ? Qt::red : Qt::gray));
	if (vs.IsLightingEnabled ())
	    glDisable (GL_LIGHTING);
	DisplayOrientedSegment () (lp, G3D::Vector3::zero ());

	glPointSize (8.0);
	glBegin (GL_POINTS);
	::glVertex (initialLightPosition * vs.GetLightPositionRatio (i));
	glEnd ();

	glPopMatrix ();
	glPopAttrib ();
    }
}

void WidgetGl::translateLight (ViewNumber::Enum viewNumber, 
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

    vs.SetLightPositionRatio (
	vs.GetSelectedLight (),
	(1 + ratio) * vs.GetLightPositionRatio (vs.GetSelectedLight ()));
}


void WidgetGl::initializeLighting ()
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


void WidgetGl::transformFoamAverageAround (
    ViewNumber::Enum viewNumber, size_t timeStep) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    if (vs.IsAverageAround ())
	vs.RotateAndTranslateAverageAround (timeStep, 1);
}




G3D::Vector3 WidgetGl::calculateViewingVolumeScaledExtent (
    ViewNumber::Enum viewNumber) const
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    return CalculateViewingVolume (viewNumber).extent () / 
	vs.GetScaleRatio ();
}


G3D::AABox WidgetGl::calculateEyeViewingVolume (
    ViewNumber::Enum viewNumber, ViewingVolumeOperation::Enum enclose) const
{
    return m_settings->CalculateEyeViewingVolume (
	viewNumber, GetSimulation (viewNumber), GetXOverY (),
	enclose);
}


G3D::AABox WidgetGl::CalculateViewingVolume (
    ViewNumber::Enum viewNumber, ViewingVolumeOperation::Enum enclose) const
{    
    return m_settings->CalculateViewingVolume (
	viewNumber, GetSimulation (viewNumber), GetXOverY (), enclose);
}

G3D::AABox WidgetGl::calculateCenteredViewingVolume (
    ViewNumber::Enum viewNumber) const
{
    return m_settings->CalculateCenteredViewingVolume (
	viewNumber, GetSimulation (viewNumber), GetXOverY (),
	ViewingVolumeOperation::DONT_ENCLOSE2D);
}



G3D::Vector3 WidgetGl::getEyeTransform (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    return G3D::Vector3 (0, 0, - vs.GetCameraDistance ()) -
	GetSimulation (viewNumber).GetBoundingBox ().center ();
}


/**
 * @todo: make sure context view works for 3D
 */
void WidgetGl::translateAndScale (
    ViewNumber::Enum viewNumber, double scaleRatio,
    const G3D::Vector3& translation, bool contextView) const
{
    const Simulation& simulation = GetSimulation (viewNumber);
    glScale (scaleRatio);
    // if 2D, the back plane stays in the same place
    if (simulation.Is2D () && ! IsTimeDisplacementUsed ())
    {
	G3D::AABox boundingBox = simulation.GetBoundingBox ();
	float zTranslation = boundingBox.center ().z - boundingBox.low ().z;
	zTranslation = zTranslation - zTranslation / scaleRatio;
	glTranslatef (0, 0, zTranslation);
    }
    glTranslate (contextView ? (translation / scaleRatio) : translation);
}


/**
 * The camera is at (0,0,0), 
 * the model is centered at (0, 0, - vs.GetCameraDistance ())
 */
void WidgetGl::ModelViewTransform (ViewNumber::Enum viewNumber, 
				   size_t timeStep) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const Simulation& simulation = GetSimulation (viewNumber);
    G3D::Vector3 center = simulation.GetBoundingBox ().center ();
    const Foam& foam = simulation.GetFoam (0);

    glLoadIdentity ();
    if (simulation.Is2D ())
	glTranslate (G3D::Vector3 (vs.GetScaleCenter () - center.xy (), 0));
    glTranslate (G3D::Vector3 (0, 0, - vs.GetCameraDistance ()));
    bool contextView = vs.IsContextView ();
    if (contextView)
	translateAndScale (
	    viewNumber, vs.GetContextScaleRatio (), G3D::Vector3::zero (),
	    false);
    else
	translateAndScale (
	    viewNumber, vs.GetScaleRatio (), vs.GetTranslation (), false);
    G3D::Vector3 translate = 
	vs.GetRotationForAxesOrder (foam) * (vs.GetRotationCenter () - center);
    // rotate around the center of rotation
    glTranslate (translate);
    glMultMatrix (vs.GetRotation ());
    glTranslate (- translate);
    // rotate around the center of the simulation
    glMultMatrix (vs.GetRotationForAxesOrder (foam));
    glTranslate (- center);
    transformFoamAverageAround (viewNumber, timeStep);
}

boost::array<GLdouble,16>* WidgetGl::GetModelViewMatrix (
    boost::array<GLdouble, 16>* mv,
    ViewNumber::Enum viewNumber, 
    size_t timeStep) const
{
    glPushMatrix ();
    ModelViewTransform (viewNumber, timeStep);
    glGetDoublev (GL_MODELVIEW_MATRIX, &(*mv)[0]);
    glPopMatrix ();
    return mv;
}

void WidgetGl::ProjectionTransform (
    ViewNumber::Enum viewNumber,
    ViewingVolumeOperation::Enum enclose) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::AABox vv = calculateEyeViewingVolume (viewNumber, enclose);
    G3D::Vector3 low = vv.low (), high = vv.high ();
    glLoadIdentity();
    if (vs.GetAngleOfView () == 0)
	glOrtho (low.x, high.x, low.y, high.y, -high.z, -low.z);
    else
	glFrustum (low.x, high.x, low.y, high.y, -high.z, -low.z);
    //cdbg << "ProjectionTransform" << vv << endl;
}



void WidgetGl::viewportTransform (ViewNumber::Enum viewNumber) const
{
    G3D::Rect2D viewRect = GetViewRect (viewNumber);
    ViewSettings& vs = GetViewSettings (viewNumber);
    vs.SetViewport (viewRect);
    glViewport (viewRect);
}

G3D::Rect2D WidgetGl::GetViewRect (ViewNumber::Enum viewNumber) const
{
    return m_settings->GetViewRect (width (), height (), viewNumber);
}


float WidgetGl::GetXOverY () const
{
    return float (width ()) / height ();    
}


void WidgetGl::setView (const G3D::Vector2& clickedPoint)
{
    for (size_t i = 0; 
	 i < ViewCount::GetCount (m_settings->GetViewCount ()); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	G3D::Rect2D viewRect = GetViewRect (viewNumber);
	if (viewRect.contains (clickedPoint))
	{
	    m_settings->SetViewNumber (viewNumber);
	    break;
	}
    }
}

void WidgetGl::LinkedTimeBegin ()
{
    try
    {
	m_settings->LinkedTimeBegin ();
    }
    catch (exception& e)
    {
	QMessageBox msgBox (this);
	msgBox.setText(e.what ());
	msgBox.exec();	
    }
}

void WidgetGl::LinkedTimeEnd ()
{
    try
    {
	m_settings->LinkedTimeEnd ();
    }
    catch (exception& e)
    {
	QMessageBox msgBox (this);
	msgBox.setText(e.what ());
	msgBox.exec();	
    }
}

void WidgetGl::SelectAll ()
{
    GetViewSettings ().
	SetBodySelector (AllBodySelector::Get (), BodySelectorType::ID);
    CompileUpdate ();
    m_selectBodiesByIdList->ClearEditIds ();
    update ();
}

void WidgetGl::DeselectAll ()
{
    GetViewSettings ().SetBodySelector (
	boost::shared_ptr<IdBodySelector> (new IdBodySelector ()));
    CompileUpdate ();
}

void WidgetGl::SelectBodiesByIdList ()
{
    if (m_selectBodiesByIdList->exec () == QDialog::Accepted)
    {
	GetViewSettings ().SetBodySelector (
	    boost::shared_ptr<IdBodySelector> (
		new IdBodySelector (m_selectBodiesByIdList->GetIds ())));
	CompileUpdate ();
    }
}

void WidgetGl::SelectThisBodyOnly ()
{
    vector<size_t> bodyIds;
    ViewSettings& vs = GetViewSettings ();
    brushedBodies (m_contextMenuPosScreen, &bodyIds);
    if (bodyIds.size () > 0)
    {
	vs.SetBodySelector (
	    boost::shared_ptr<IdBodySelector> (new IdBodySelector ()));
	vs.UnionBodySelector (bodyIds[0]);
    }
    CompileUpdate ();
}


void WidgetGl::InfoFoam ()
{
    string message = GetSimulation ().ToHtml ();
    QMessageBox msgBox (this);
    msgBox.setText(message.c_str ());
    msgBox.exec();
}

void WidgetGl::InfoPoint ()
{
    QMessageBox msgBox (this);
    ostringstream ostr;
    ostr << "Point: " << m_contextMenuPosObject;
    msgBox.setText(ostr.str ().c_str ());
    msgBox.exec();
}

void WidgetGl::InfoEdge ()
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


void WidgetGl::InfoFace ()
{
    Info msgBox (this, "Info");
    ostringstream ostr;
    const OrientedFace* of = 0;
    brushedFace (&of);
    if (of == 0)
	ostr << "No face focused.";
    else
    {
	const Foam& foam = GetSimulation ().GetFoam (GetCurrentTime ());
	ostr << of->ToString (&foam.GetAttributesInfoElements ().GetInfoFace ());
    }
    msgBox.setText(ostr.str ().c_str ());
    msgBox.exec();
}

void WidgetGl::InfoBody ()
{
    Info msgBox (this, "Info");
    string message = infoSelectedBody ();
    msgBox.setText (message.c_str ());
    msgBox.exec();
}

void WidgetGl::InfoSelectedBodies ()
{
    Info msgBox (this, "Info");
    const BodySelector& bodySelector = GetViewSettings ().GetBodySelector ();
    string message;
    switch (bodySelector.GetType ())
    {
    case BodySelectorType::ALL:
	message = "All bodies selected.";
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


string WidgetGl::infoSelectedBody () const
{
    ostringstream ostr;
    vector< boost::shared_ptr<Body> > bodies;
    const Foam& foam = GetSimulation ().GetFoam (GetCurrentTime ());
    brushedBodies (m_contextMenuPosScreen, &bodies);
    if (bodies.size () == 0)
	ostr << "No bodies focused.";
    else
	ostr << bodies[0]->ToString (
	    &foam.GetAttributesInfoElements ().GetInfoBody ());
    return ostr.str ();
}

string WidgetGl::infoSelectedBodies () const
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
	if (GetViewSettings ().GetBodyOrFaceScalar () != 
	    FaceScalar::DMP_COLOR)
	{
	    ostr << endl;
		
	}
    }
    return ostr.str ();
}

void WidgetGl::InfoOpenGL ()
{
    ostringstream ostr;
    printOpenGLInfo (ostr);
    Info openGLInfo (this, "OpenGL Info", ostr.str ().c_str ());
    openGLInfo.exec ();
}

void WidgetGl::ShowNeighbors ()
{
    m_showType = SHOW_NEIGHBORS;
    vector<size_t> bodies;
    brushedBodies (m_contextMenuPosScreen, &bodies);
    m_showBodyId = bodies[0];
    update ();
}

void WidgetGl::ShowDeformation ()
{
    m_showType = SHOW_DEFORMATION_TENSOR;
    vector<size_t> bodies;
    brushedBodies (m_contextMenuPosScreen, &bodies);
    m_showBodyId = bodies[0];
    update ();
}

void WidgetGl::ShowVelocity ()
{
    m_showType = SHOW_VELOCITY;
    vector<size_t> bodies;
    brushedBodies (m_contextMenuPosScreen, &bodies);
    m_showBodyId = bodies[0];
    update ();
}


void WidgetGl::ShowReset ()
{
    m_showType = SHOW_NOTHING;
    update ();
}

void WidgetGl::ColorBarClampClear ()
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    boost::shared_ptr<ColorBarModel> colorBarModel = vs.GetColorBarModel ();
    colorBarModel->SetClampClear ();
    Q_EMIT ColorBarModelChanged (viewNumber, colorBarModel);
}

void WidgetGl::OverlayBarClampClear ()
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    boost::shared_ptr<ColorBarModel> colorBarModel = 
	vs.GetOverlayBarModel ();
    colorBarModel->SetClampClear ();
    Q_EMIT OverlayBarModelChanged (viewNumber, colorBarModel);
}


// Uses antialiased points and lines
// See OpenGL Programming Guide, 7th edition, Chapter 6: Blending,
// Antialiasing, Fog and Polygon Offset page 293
void WidgetGl::initializeGL()
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
	m_settings->SetViewNumber (ViewNumber::VIEW0);
	WarnOnOpenGLError ("initializeGL");
    }
    catch (const exception& e)
    {
	cdbg << "Exception: " << e.what () << endl;
    }

}

void WidgetGl::paintGL ()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    displayViews ();
    displayViewsGrid ();
    Q_EMIT PaintEnd ();
}

void WidgetGl::resizeGL(int w, int h)
{
    (void)w;(void)h;
    for (size_t i = 0; 
	 i < ViewCount::GetCount (m_settings->GetViewCount ()); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	GetViewAverage (viewNumber).AverageInitStep ();
    }
    WarnOnOpenGLError ("resizeGl");
}


void WidgetGl::displayViews ()
{
    switch (m_settings->GetViewCount ())
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

void WidgetGl::displayView (ViewNumber::Enum viewNumber)
{
    //QTime t;t.start ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    vs.SetGlLightParameters (calculateCenteredViewingVolume (viewNumber));
    allTransform (viewNumber);
    m_settings->SetEdgeArrow (GetOnePixelInObjectSpace ());
    (this->*(m_display[vs.GetViewType ()])) (viewNumber);
    displayViewDecorations (viewNumber);
    displayAxes (viewNumber);
    displayBoundingBox (viewNumber);
    displayTorusDomain (viewNumber);
    displayFocusBox (viewNumber);
    displayLightDirection (viewNumber);
    displayBodyCenters (viewNumber);
    displayRotationCenter (viewNumber);
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

void WidgetGl::allTransform (ViewNumber::Enum viewNumber) const
{
    viewportTransform (viewNumber);    
    glMatrixMode (GL_PROJECTION);
    ProjectionTransform (viewNumber);
    glMatrixMode (GL_MODELVIEW);
    ModelViewTransform (viewNumber, GetCurrentTime (viewNumber));
}


G3D::Matrix3 WidgetGl::getRotationAround (int axis, double angleRadians)
{
    using G3D::Matrix3;using G3D::Vector3;
    Vector3 axes[3] = {
	Vector3::unitX (), Vector3::unitY (), Vector3::unitZ ()
    };
    return Matrix3::fromAxisAngle (axes[axis], angleRadians);
}

G3D::Vector2 WidgetGl::calculateScaleCenter (
    ViewNumber::Enum viewNumber, const G3D::Rect2D& rect) const
{
    if (! m_reflectedHalfView)
	return rect.center ();
    else if (viewNumber == ViewNumber::VIEW0)
	return (rect.x0y0 () + rect.x1y0 ()) / 2;
    else
	return (rect.x0y1 () + rect.x1y1 ()) / 2;
}


float WidgetGl::ratioFromScaleCenter (
    ViewNumber::Enum viewNumber, const QPoint& p)
{
    
    G3D::Vector2 center = calculateScaleCenter (
	viewNumber, GetViewRect (viewNumber));
    int windowHeight = height ();
    G3D::Vector2 lastPos = QtToOpenGl (m_lastPos, windowHeight);
    G3D::Vector2 currentPos = QtToOpenGl (p, windowHeight);
    float ratio =
	(currentPos - center).length () / (lastPos - center).length ();
    return ratio;
}

G3D::Matrix3 WidgetGl::rotate (
    ViewNumber::Enum viewNumber,
    const QPoint& position, Qt::KeyboardModifiers modifiers,
    const G3D::Matrix3& r)
{
    G3D::Matrix3 rotate = r;
    const G3D::Rect2D& viewport = GetViewSettings (viewNumber).GetViewport ();
    int dx = position.x() - m_lastPos.x();
    int dy = position.y() - m_lastPos.y();

    // scale this with the size of the window
    int side = std::min (viewport.width (), viewport.height ());
    double dxRadians = static_cast<double>(dx) * (M_PI / 2) / side;
    double dyRadians = static_cast<double>(dy) * (M_PI / 2) / side;
    if (modifiers == Qt::NoModifier)
    {
	rotate = getRotationAround (0, dyRadians) * rotate;
	rotate = getRotationAround (1, dxRadians) * rotate;
    }
    // rotate around X axis
    else if (modifiers == Qt::ControlModifier)
    {
	rotate = getRotationAround (0, dyRadians) * rotate;
    }
    // rotate around Y axis
    else if (modifiers == Qt::ShiftModifier)
    {
	rotate = getRotationAround (1, dxRadians) * rotate;
    }
    // rotate around Z axis
    else if (modifiers == Qt::AltModifier ||
	     modifiers == (Qt::ControlModifier | Qt::ShiftModifier))
    {
	rotate = getRotationAround (2, - dxRadians) * rotate;
    }
    return rotate;
}

G3D::Vector3 WidgetGl::calculateTranslationRatio (
    ViewNumber::Enum viewNumber, const QPoint& position,
    G3D::Vector3::Axis screenXTranslation,
    G3D::Vector3::Axis screenYTranslation) const
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::Vector3 translationRatio;
    if (screenXTranslation != G3D::Vector3::DETECT_AXIS)
	translationRatio[screenXTranslation] =
	    static_cast<float>(position.x() - m_lastPos.x()) /
	    vs.GetViewport ().width ();
    if (screenYTranslation != G3D::Vector3::DETECT_AXIS)
	translationRatio[screenYTranslation] =
	    - static_cast<float> (position.y() - m_lastPos.y()) / 
	    vs.GetViewport ().height ();
    return translationRatio;
}


void WidgetGl::translate (
    ViewNumber::Enum viewNumber, const QPoint& position,
    Qt::KeyboardModifiers modifiers)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::Vector3 translationRatio;
    if (modifiers == Qt::NoModifier)
    {
	translationRatio = calculateTranslationRatio (
	    viewNumber, position, G3D::Vector3::X_AXIS, G3D::Vector3::Y_AXIS);
    }
    // translate along X axis
    else if (modifiers == Qt::ControlModifier)
    {
	translationRatio = calculateTranslationRatio (
	    viewNumber, position, G3D::Vector3::X_AXIS, 
	    G3D::Vector3::DETECT_AXIS);	
    }
    // translate along Y axis
    else if (modifiers == Qt::ShiftModifier)
    {
	translationRatio = calculateTranslationRatio (
	    viewNumber, position, G3D::Vector3::DETECT_AXIS, 
	    G3D::Vector3::Y_AXIS);	
    }
    // translate along Z axis
    else if (modifiers == Qt::AltModifier ||
	     modifiers == (Qt::ControlModifier | Qt::ShiftModifier))
    {
	translationRatio = - calculateTranslationRatio (
	    viewNumber, position, G3D::Vector3::DETECT_AXIS, 
	    G3D::Vector3::Z_AXIS);	
    }
    G3D::Vector3 extent = calculateViewingVolumeScaledExtent (viewNumber);
    if (vs.IsContextView ())
	vs.SetTranslation (
	    vs.GetTranslation () - (translationRatio * extent));
    else
    {
	G3D::Vector3 translation = 
	    vs.GetTranslation () + (translationRatio * extent);
	vs.SetTranslation (translation);
    }
}

void WidgetGl::translateGrid (
    ViewNumber::Enum viewNumber, const QPoint& position)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::Vector3 translationRatio = calculateTranslationRatio (
	viewNumber, position, G3D::Vector3::X_AXIS, G3D::Vector3::Y_AXIS);
    G3D::Vector3 extent = calculateViewingVolumeScaledExtent (viewNumber);
    vs.SetGridTranslation (
	vs.GetGridTranslation () + (translationRatio * extent));
}

void WidgetGl::scale (ViewNumber::Enum viewNumber, const QPoint& position)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    float ratio = ratioFromScaleCenter (viewNumber, position);
    if (vs.IsContextView ())
	vs.SetScaleRatio (vs.GetScaleRatio () / ratio);
    else
	vs.SetScaleRatio (vs.GetScaleRatio () * ratio);
}

void WidgetGl::scaleGrid (ViewNumber::Enum viewNumber, const QPoint& position)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    float ratio = ratioFromScaleCenter (viewNumber, position);
    vs.SetGridScaleRatio (vs.GetGridScaleRatio () * ratio);
}


void WidgetGl::scaleContext (
    ViewNumber::Enum viewNumber, const QPoint& position)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    float ratio = ratioFromScaleCenter (viewNumber, position);
    vs.SetContextScaleRatio (vs.GetContextScaleRatio () * ratio);
}

G3D::Vector3 WidgetGl::brushedBodies (
    const QPoint& position, vector<size_t>* bodies, bool selected) const
{
    vector< boost::shared_ptr<Body> > b;
    G3D::Vector3 op = brushedBodies (position, &b, selected);
    bodies->resize (b.size ());
    ::transform (b.begin (), b.end (), bodies->begin (),
	       boost::bind (&Element::GetId, _1));
    return op;
}

G3D::Vector3 WidgetGl::brushedBodies (
    const QPoint& position, 
    vector< boost::shared_ptr<Body> >* bodies, bool selected) const
{
    const BodySelector& selector = GetViewSettings ().GetBodySelector ();
    G3D::Vector3 op = toObjectTransform (position);
    const Foam& foam = GetSimulation ().GetFoam (GetCurrentTime ());
    BOOST_FOREACH (boost::shared_ptr<Body> body, foam.GetBodies ())
    {
	G3D::AABox box = body->GetBoundingBox ();
	if (box.contains (op))
	{
	    if (selector (body) == selected)
		bodies->push_back (body);
	}
    }
    return op;
}

G3D::Vector3 WidgetGl::brushedFace (const OrientedFace** of) const
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

OrientedEdge WidgetGl::brushedEdge () const
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



G3D::Vector3 WidgetGl::toObjectTransform (const QPoint& position, 
					  ViewNumber::Enum viewNumber) const
{
    allTransform (viewNumber);
    return toObject (position, height ());
}

G3D::Vector3 WidgetGl::toObjectTransform (const QPoint& position) const
{
    return toObjectTransform (position, GetViewNumber ());
}

void WidgetGl::displayAverageAroundBodies (
    ViewNumber::Enum viewNumber,
    bool adjustForAverageAroundMovementRotation) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    if (m_averageAroundMarked && vs.IsAverageAround ())
    {
	const Simulation& simulation = GetSimulation (viewNumber);
	glPushAttrib (GL_CURRENT_BIT |
		      GL_ENABLE_BIT | GL_LINE_BIT | GL_POINT_BIT);
	if (adjustForAverageAroundMovementRotation)
	{
	    glMatrixMode (GL_MODELVIEW);
	    glPushMatrix ();
	    vs.RotateAndTranslateAverageAround (vs.GetCurrentTime (), -1);
	}
	glDisable (GL_DEPTH_TEST);
	// display focus body 1
	Foam::Bodies focusBody (1);
	size_t bodyId = vs.GetAverageAroundBodyId ();
	focusBody[0] = *simulation.GetFoam (
	    vs.GetCurrentTime ()).FindBody (bodyId);
	displayFacesContour (focusBody, viewNumber, GetHighlightLineWidth ());
	glEnable (GL_DEPTH_TEST);
	displayFacesInterior (focusBody, viewNumber);
	glDisable (GL_DEPTH_TEST);
	


	// display body center for focus body 1.
	glPointSize (4.0);
	glColor (Qt::black);
	DisplayBodyCenter (
	    *m_settings, simulation.GetFoam (vs.GetCurrentTime ()), 
	    IdBodySelector (bodyId)) (focusBody[0]);

	// display focus body 2
	size_t secondBodyId = vs.GetAverageAroundSecondBodyId ();
	if (secondBodyId != INVALID_INDEX)
	{
	    focusBody[0] = *simulation.GetFoam (vs.GetCurrentTime ()).
		FindBody (secondBodyId);
	    displayFacesContour (focusBody, viewNumber, 
				 GetHighlightLineWidth ());
	    glEnable (GL_DEPTH_TEST);
	    displayFacesInterior (focusBody, viewNumber);
	}

	if (adjustForAverageAroundMovementRotation)
	    glPopMatrix ();
	glPopAttrib ();
    }
}


void WidgetGl::displayContextBodies (ViewNumber::Enum viewNumber) const
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    if (vs.GetContextDisplayBodySize () > 0)
    {
	glPushAttrib (GL_ENABLE_BIT);
	const Foam::Bodies& bodies = 
	    GetSimulation (viewNumber).GetFoam (
		GetCurrentTime (viewNumber)).GetBodies ();
	Foam::Bodies contextBodies (bodies.size ());       
	size_t j = 0;
	for (size_t i = 0; i < bodies.size (); ++i)
	    if (vs.IsContextDisplayBody (bodies[i]->GetId ()))
		contextBodies[j++] = bodies[i];
	contextBodies.resize (j);
	glDisable (GL_DEPTH_TEST);
	displayFacesContour (contextBodies, viewNumber, 
			     GetHighlightLineWidth ());
	glEnable (GL_DEPTH_TEST);
	displayFacesInterior (contextBodies, viewNumber);
	glPopAttrib ();
    }
}

void WidgetGl::displayContextBox (
    ViewNumber::Enum viewNumber,
    bool adjustForAverageAroundMovementRotation) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    if (m_contextBoxShown && vs.IsAverageAroundRotationShown ())
    {
	glPushAttrib (GL_ENABLE_BIT);
	glDisable (GL_DEPTH_TEST);
	if (adjustForAverageAroundMovementRotation)
	{
	    glMatrixMode (GL_MODELVIEW);
	    glPushMatrix ();
	    vs.RotateAndTranslateAverageAround (vs.GetCurrentTime (), -1);
	}
	DisplayBox (GetSimulation (viewNumber), 
		    m_settings->GetHighlightColor (
			viewNumber, HighlightNumber::H1),
		    GetHighlightLineWidth ());
	if (adjustForAverageAroundMovementRotation)
	    glPopMatrix ();
	glPopAttrib ();
    }
}


string WidgetGl::getAverageAroundLabel ()
{
    ostringstream ostr;
    const ViewSettings& vs = GetViewSettings ();
    if (vs.IsAverageAround ())
    {
	ostr << "Average around";
	if (vs.GetAverageAroundSecondBodyId () == INVALID_INDEX)
	    ostr << " (1)";
	else
	    ostr << " (2)";
	
	const ObjectPosition rotationBegin = vs.GetAverageAroundPosition (0);
	const ObjectPosition rotationCurrent = 
	    vs.GetAverageAroundPosition (GetCurrentTime ());
	float angleRadians = 
	    rotationCurrent.m_angleRadians - rotationBegin.m_angleRadians;
	float angleDegrees =  G3D::toDegrees (angleRadians);
	ostr << " Rotation: " << vs.AngleDisplay (angleDegrees);
    }
    return ostr.str ();
}

string WidgetGl::getContextLabel ()
{
    ostringstream ostr;
    const ViewSettings& vs = GetViewSettings ();
    size_t count = vs.GetContextDisplayBodySize ();
    if (count != 0)
	ostr << "Context (" << count << ")";
    return ostr.str ();
}

string WidgetGl::getAverageAroundMovementShownLabel ()
{
    ostringstream ostr;
    const ViewSettings& vs = GetViewSettings ();
    if (vs.IsAverageAroundRotationShown ())
	ostr << "Show rotation";
    return ostr.str ();
}


string WidgetGl::getBodySelectorLabel ()
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

string WidgetGl::getInteractionLabel ()
{
    ostringstream ostr;
    const ViewSettings& vs = GetViewSettings ();
    switch (m_interactionMode)
    {
    case InteractionMode::ROTATE:
	ostr << "Rotate: " << 
	    (vs.GetRotationCenterType () == ViewSettings::ROTATION_CENTER_FOAM ? 
	     "foam" : "body");
	break;
    case InteractionMode::SCALE:
	ostr << "Scale: " << setprecision (3) << vs.GetScaleRatio ();
	break;
    case InteractionMode::TRANSLATE:
	ostr << "Translate";
	break;
    case InteractionMode::SELECT:
	ostr << "Select";
	break;
    case InteractionMode::DESELECT:
	ostr << "Deselect";
	break;
    default:
	ostr << "Ready";
	break;
    }
    return ostr.str ();
}

void WidgetGl::displayStatus ()
{
    ostringstream ostr;
    boost::array<string, 5> labels = {{
	    getInteractionLabel (),
	    getAverageAroundLabel (),
	    getContextLabel (),
	    getAverageAroundMovementShownLabel (),
	    getBodySelectorLabel ()
	}};
    ostream_iterator<string> o (ostr, " | ");
    remove_copy_if (labels.begin (), labels.end (), o,
		    boost::bind (&string::empty, _1));
    m_labelStatusBar->setText (QString (ostr.str ().c_str ()));
}


void WidgetGl::AverageAroundBody ()
{
    ViewSettings& vs = GetViewSettings ();
    vector< boost::shared_ptr<Body> > bodies;
    brushedBodies (m_contextMenuPosScreen, &bodies);
    if (bodies.size () != 0)
    {
	boost::shared_ptr<Body> body = bodies[0];
	const Simulation& simulation = GetSimulation ();
	size_t bodyId = body->GetId ();
	vs.SetAverageAroundBodyId (bodyId);
	vs.SetAverageAroundSecondBodyId (INVALID_INDEX);
	vs.SetAverageAround (true);
	if (body->IsObject () && simulation.GetDmpObjectInfo ().RotationUsed ())
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

void WidgetGl::AverageAroundSecondBody ()
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
		vs.SetAverageAround (true);
		vs.SetAverageAroundPositions (simulation, bodyId, secondBodyId);
		vs.SetDifferenceBodyId (secondBodyId);
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


void WidgetGl::AverageAroundReset ()
{
    ViewSettings& vs = GetViewSettings ();
    vs.SetAverageAround (false);
    vs.SetAverageAroundBodyId (INVALID_INDEX);
    vs.SetAverageAroundSecondBodyId (INVALID_INDEX);
    update ();
}

void WidgetGl::ContextDisplayBody ()
{
    ViewSettings& vs = GetViewSettings ();
    vector<size_t> bodies;
    brushedBodies (m_contextMenuPosScreen, &bodies);
    vs.AddContextDisplayBody (bodies[0]);
    update ();
}

void WidgetGl::ContextDisplayReset ()
{
    ViewSettings& vs = GetViewSettings ();
    vs.ContextDisplayReset ();
    update ();
}

void WidgetGl::ToggledAverageAroundAllowRotation (bool checked)
{
    ViewSettings& vs = GetViewSettings ();
    vs.SetAverageAroundRotationShown (checked);
    update ();
}

void WidgetGl::select (const QPoint& position)
{
    vector<size_t> bodyIds;
    brushedBodies (position, &bodyIds, false);
    GetViewSettings ().UnionBodySelector (bodyIds);
    CompileUpdate ();
}

void WidgetGl::deselect (const QPoint& position)
{
    vector<size_t> bodyIds;
    brushedBodies (position, &bodyIds);
    GetViewSettings ().DifferenceBodySelector (
	GetSimulation ().GetFoam (0), bodyIds);
    CompileUpdate ();
}

void WidgetGl::mouseMoveRotate (QMouseEvent *event, ViewNumber::Enum viewNumber)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    switch (m_interactionObject)
    {
    case InteractionObject::FOCUS:
	vs.SetRotation (
	    rotate (viewNumber, event->pos (), event->modifiers (), 
		    vs.GetRotation ()));
	break;
    case InteractionObject::LIGHT:
    {
	LightNumber::Enum i = vs.GetSelectedLight ();
	vs.SetRotationLight (
	    i, rotate (viewNumber, event->pos (), event->modifiers (), 
		       vs.GetRotationLight (i)));
	break;
    }
    default:
	break;
    }
}

void WidgetGl::mouseMoveTranslate (QMouseEvent *event, 
				   ViewNumber::Enum viewNumber)
{
    switch (m_interactionObject)
    {
    case InteractionObject::FOCUS:
	translate (viewNumber, event->pos (), event->modifiers ());
	GetViewAverage (viewNumber).AverageInitStep ();
	break;
    case InteractionObject::LIGHT:
    {
	translateLight (viewNumber, event->pos ());
	break;
    }
    case InteractionObject::GRID:
	translateGrid (viewNumber, event->pos ());
	break;
    default:
	break;
    }
}

void WidgetGl::mouseMoveScale (QMouseEvent *event, ViewNumber::Enum viewNumber)
{
    switch (m_interactionObject)
    {
    case InteractionObject::FOCUS:
	scale (viewNumber, event->pos ());
	GetViewAverage (viewNumber).AverageInitStep ();
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


void WidgetGl::displayTorusDomain (ViewNumber::Enum viewNumber) const
{
    if (m_torusDomainShown)
	DisplayBox (GetSimulation (viewNumber).
		    GetFoam(GetCurrentTime ()).GetTorusDomain ());
}

/**
 * @todo display a pyramid frustum for angle of view > 0.
 */
void WidgetGl::displayFocusBox (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    if (vs.IsContextView ())
    {
	glPushMatrix ();
	glLoadIdentity ();
	glTranslatef (0, 0, - vs.GetCameraDistance ());

	G3D::AABox focusBox = calculateCenteredViewingVolume (viewNumber);
	translateAndScale ( viewNumber, 1 / vs.GetScaleRatio (), 
			    - vs.GetContextScaleRatio () * 
			    vs.GetTranslation (), true);
	glScale (vs.GetContextScaleRatio ());
	DisplayBox (focusBox, m_settings->GetHighlightColor (
			viewNumber, HighlightNumber::H0), 
		    GetHighlightLineWidth ());
	glPopMatrix ();
    }
}

void WidgetGl::displayBoundingBox (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const Simulation& simulation = GetSimulation (viewNumber);
    const Foam& foam = simulation.GetFoam (GetCurrentTime (viewNumber));
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT);
    if (vs.IsLightingEnabled ())
	glDisable (GL_LIGHTING);
    if (m_boundingBoxSimulationShown)
	DisplayBox (simulation, Qt::black);
    if (m_boundingBoxFoamShown)
	DisplayBox (foam, Qt::black);
    if (m_boundingBoxBodyShown)
    {
	const Foam::Bodies& bodies = foam.GetBodies ();
	const BodySelector& bodySelector = vs.GetBodySelector ();
	BOOST_FOREACH (boost::shared_ptr<Body> body, bodies)
	    if (bodySelector (body))
		DisplayBox (body, Qt::black);
    }
    glPopAttrib ();
}

void WidgetGl::displayAxes (ViewNumber::Enum viewNumber)
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
	    GetQuadricObject (), 
	    m_settings->GetArrowBaseRadius (), 
	    m_settings->GetEdgeRadius (), m_settings->GetArrowHeight (),
	    DisplaySegmentArrow1::TOP_END);

	a = fm.height () * GetOnePixelInObjectSpace ();
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
void WidgetGl::displayEdges (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const Simulation& simulation = GetSimulation (viewNumber);
    const BodySelector& bodySelector = vs.GetBodySelector ();
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);
    const Foam::Bodies& bodies = 
	simulation.GetFoam (GetCurrentTime (viewNumber)).GetBodies ();
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<
	      DisplayFaceHighlightColor<HighlightNumber::H0,
	      DisplayFaceEdges<displayEdge> > > (
		  *m_settings, simulation.GetFoam (0), bodySelector));
    displayStandaloneEdges<displayEdge> (simulation.GetFoam (0));
    glPopAttrib ();
}

template<typename displayEdge>
void WidgetGl::displayStandaloneEdges (
    const Foam& foam, ViewNumber::Enum viewNumber,
    bool useZPos, double zPos) const
{
    if (m_standaloneElementsShown)
    {
	glPushAttrib (GL_ENABLE_BIT);    
	glDisable (GL_DEPTH_TEST);
	displayEdge de (*m_settings, foam,
			DisplayElement::FOCUS, viewNumber, useZPos, zPos);
	const Foam::Edges& standaloneEdges = foam.GetStandaloneEdges ();
	BOOST_FOREACH (boost::shared_ptr<Edge> edge, standaloneEdges)
	    de(edge);
	glPopAttrib ();
    }
}

void WidgetGl::displayEdgesNormal (ViewNumber::Enum viewNumber) const
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

void WidgetGl::displayDeformation (ViewNumber::Enum viewNumber) const
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
		  *m_settings, viewNumber,
		  GetSimulation (viewNumber).GetFoam (
		      GetCurrentTime (viewNumber)), vs.GetBodySelector (),
		  GetDeformationSizeInitialRatio (viewNumber)));
    glPopAttrib ();    
}

void WidgetGl::displayVelocity (ViewNumber::Enum viewNumber) const
{
    const Foam& foam = 
	GetSimulation (viewNumber).GetFoam (GetCurrentTime (viewNumber));
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const VectorAverage& va = GetViewAverage (viewNumber).GetVelocityAverage ();
    if (! foam.Is2D () || ! vs.IsVelocityShown ())
	return;
    Foam::Bodies bodies = foam.GetBodies ();
    glPushAttrib (GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable (GL_DEPTH_TEST);
    if (va.IsColorMapped ())
    {
	glEnable(GL_TEXTURE_1D);
	glBindTexture (GL_TEXTURE_1D, m_overlayBarTexture[viewNumber]);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    }
    glColor (Qt::black);
    for_each (
	bodies.begin (), bodies.end (),
	DisplayBodyVelocity (
	    *m_settings, viewNumber,
	    GetSimulation (viewNumber).GetFoam (GetCurrentTime (viewNumber)),
	    vs.GetBodySelector (), GetBubbleSize (viewNumber), 
	    GetVelocitySizeInitialRatio (viewNumber),
	    GetOnePixelInObjectSpace (), 
	    va.IsSameSize (), va.IsClampingShown ()));
    glPopAttrib ();    
}



void WidgetGl::displayBodyDeformation (
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
	    *m_settings, viewNumber,
	    GetSimulation (viewNumber).GetFoam (GetCurrentTime (viewNumber)),
	    vs.GetBodySelector (), 
	    GetDeformationSizeInitialRatio (viewNumber)) (
		*foam.FindBody (m_showBodyId));
	glPopAttrib ();
    }
}

void WidgetGl::displayBodyVelocity (
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
	const VectorAverage& va = 
	    m_viewAverage[viewNumber]->GetVelocityAverage ();
	DisplayBodyVelocity (
	    *m_settings, viewNumber,
	    GetSimulation (viewNumber).GetFoam (GetCurrentTime (viewNumber)),
	    vs.GetBodySelector (), 
	    GetBubbleSize (viewNumber), 
	    GetVelocitySizeInitialRatio (viewNumber),
	    GetOnePixelInObjectSpace (), va.IsSameSize (), 
	    va.IsClampingShown ()) (*foam.FindBody (m_showBodyId));
	glPopAttrib ();
    }
}


void WidgetGl::displayBodyNeighbors (ViewNumber::Enum viewNumber) const
{
    if (m_showType != SHOW_NEIGHBORS)
	return;
    glPushAttrib (GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable (GL_DEPTH_TEST);
    glColor (Qt::black);
    glBegin (GL_LINES);

    const Foam& foam = GetSimulation (viewNumber).GetFoam (
	GetCurrentTime(viewNumber));
    const OOBox& originalDomain = foam.GetTorusDomain ();
    Foam::Bodies::const_iterator showBody = foam.FindBody (m_showBodyId);
    ::displayBodyNeighbors (*showBody, originalDomain);
    glEnd ();
    glPopAttrib ();
}


void WidgetGl::displayBodiesNeighbors () const
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
		      ::displayBodyNeighbors, _1, 
		      GetSimulation ().GetFoam (0).GetTorusDomain ()));
	glEnd ();
	glPopAttrib ();
    }
}

void WidgetGl::displayT1sDot (ViewNumber::Enum viewNumber) const
{
    for (size_t i = 0; i < GetSimulation (viewNumber).GetT1sTimeSteps (); ++i)
	displayT1sDot (viewNumber, i);
}

void WidgetGl::displayT1sDot (
    ViewNumber::Enum viewNumber, size_t timeStep) const
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    glPushAttrib (GL_ENABLE_BIT | GL_POINT_BIT | 
		  GL_CURRENT_BIT | GL_POLYGON_BIT);
    glDisable (GL_DEPTH_TEST);
    glPointSize (m_t1sSize);
    glColor (m_settings->GetHighlightColor (viewNumber, HighlightNumber::H0));
    glBegin (GL_POINTS);
    BOOST_FOREACH (const G3D::Vector3 t1Pos, 
		   GetSimulation (viewNumber).GetT1s (timeStep, 
						      vs.T1sShiftLower ()))
	::glVertex (t1Pos);
    glEnd ();
    glPopAttrib ();
}

void WidgetGl::DisplayT1Quad (
    ViewNumber::Enum viewNumber, size_t timeStep, size_t t1Index) const
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    T1sPDE& t1sPDE = GetViewAverage (viewNumber).GetT1sPDE ();
    float rectSize = t1sPDE.GetKernelTextureSize () * 
	GetOnePixelInObjectSpace ();
    float half = rectSize / 2;
    G3D::Rect2D srcTexRect = G3D::Rect2D::xyxy (0., 0., 1., 1.);
    const G3D::Vector3 t1Pos = 
	GetSimulation (viewNumber).GetT1s (
	    timeStep, vs.T1sShiftLower ())[t1Index];
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
pair<float, float> WidgetGl::GetRange (ViewNumber::Enum viewNumber) const
{
    const Simulation& simulation = GetSimulation (viewNumber);
    ViewSettings& vs = GetViewSettings (viewNumber);
    float minValue = 0.0, maxValue = 0.0;
    switch (vs.GetViewType ())
    {
    case ViewType::AVERAGE:
	if (vs.GetStatisticsType () == StatisticsType::COUNT)
	    return GetRangeCount (viewNumber);
	else
	{
	    BodyScalar::Enum bodyProperty = 
		BodyScalar::FromSizeT (vs.GetBodyOrFaceScalar ());
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

pair<float, float> WidgetGl::GetVelocityMagnitudeRange (
    ViewNumber::Enum viewNumber) const
{
    const Simulation& simulation = GetSimulation (viewNumber);
    BodyScalar::Enum bodyProperty = BodyScalar::VELOCITY_MAGNITUDE;
    float minValue = simulation.GetMin (bodyProperty);
    float maxValue = simulation.GetMax (bodyProperty);
    return pair<float, float> (minValue, maxValue);
}


pair<float, float> WidgetGl::GetRangeCount (ViewNumber::Enum viewNumber) const
{
    return pair<float, float> (0, GetSimulation (viewNumber).GetTimeSteps ());
}

pair<float, float> WidgetGl::GetRangeCount () const
{
    return GetRangeCount (GetViewNumber ());
}

pair<float, float> WidgetGl::GetRangeT1sPDE (ViewNumber::Enum viewNumber) const
{
    T1sPDE& t1sPDE = GetViewAverage (viewNumber).GetT1sPDE ();
    float sigma = t1sPDE.GetKernelSigma ();
    return pair<float, float> (0.0, 1 / (2 * M_PI * sigma * sigma));
}

void WidgetGl::displayEdgesTorus (ViewNumber::Enum viewNumber) const
{
    (void)viewNumber;
    if (m_settings->GetEdgeRadiusRatio () > 0)
	displayEdgesTorusTubes ();
    else
	displayEdgesTorusLines ();
}

void WidgetGl::displayFacesTorus (ViewNumber::Enum viewNumber) const
{
    if (m_settings->GetEdgeRadiusRatio () > 0)
	displayFacesTorusTubes ();
    else
	displayFacesTorusLines ();
    displayStandaloneEdges< DisplayEdgePropertyColor<> > (
	GetSimulation (viewNumber).GetFoam (0));
}


void WidgetGl::displayEdgesTorusTubes () const
{
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);
    EdgeSet edgeSet;
    GetSimulation ().GetFoam (0).GetEdgeSet (&edgeSet);
    for_each (
	edgeSet.begin (), edgeSet.end (),
	DisplayEdgeTorus<DisplaySegmentQuadric, 
	DisplaySegmentArrowQuadric, false>(
	    *m_settings, GetSimulation ().GetFoam (0), DisplayElement::FOCUS, 
	    false, 0.0, GetQuadricObject ()));
    glPopAttrib ();
}

void WidgetGl::displayEdgesTorusLines () const
{
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);

    EdgeSet edgeSet;
    GetSimulation ().GetFoam (0).GetEdgeSet (&edgeSet);
    for_each (edgeSet.begin (), edgeSet.end (),
	      DisplayEdgeTorus<DisplaySegment, 
	      DisplaySegmentArrow1, false> (
		  *m_settings, GetSimulation ().GetFoam (0), DisplayElement::FOCUS,
		  false, 0.0, GetQuadricObject ()));
    glPopAttrib ();
}


void WidgetGl::displayRotationCenter (ViewNumber::Enum viewNumber) const
{
    if (m_interactionMode == InteractionMode::ROTATE)
    {
	const ViewSettings& vs = GetViewSettings (viewNumber);
	glPushAttrib (GL_POINT_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
	glDisable (GL_DEPTH_TEST);
	// display rotation center
	glPointSize (4.0);
	glColor (Qt::black);
	glBegin(GL_POINTS);
	::glVertex(vs.GetRotationCenter ());
	glEnd ();	
	glPopAttrib ();

    }
}


void WidgetGl::displayBodyCenters (
    ViewNumber::Enum viewNumber, bool useZPos) const
{
    if (m_bodyCenterShown)
    {
	const ViewSettings& vs = GetViewSettings (viewNumber);
	size_t currentTime = GetCurrentTime (viewNumber);
	const Simulation& simulation = GetSimulation (viewNumber);
	const BodySelector& bodySelector = vs.GetBodySelector ();
	double zPos = (vs.GetViewType () == ViewType::CENTER_PATHS) ?
	    currentTime * GetTimeDisplacement () : 0;
	glPushAttrib (GL_POINT_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
	glDisable (GL_DEPTH_TEST);
	glPointSize (4.0);
	glColor (Qt::red);
	const Foam::Bodies& bodies = 
	    simulation.GetFoam (currentTime).GetBodies ();
	for_each (bodies.begin (), bodies.end (),
		  DisplayBodyCenter (
		      *m_settings, simulation.GetFoam (currentTime),
		      bodySelector, useZPos, zPos));
	glPopAttrib ();
    }
}

void WidgetGl::displayFaceCenters (ViewNumber::Enum viewNumber) const
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

    
void WidgetGl::displayContextMenuPos (ViewNumber::Enum viewNumber) const
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


void WidgetGl::displayFacesNormal (ViewNumber::Enum viewNumber) const
{
    glCallList (m_listFacesNormal[viewNumber]);
}


void WidgetGl::compileFacesNormal (ViewNumber::Enum viewNumber) const
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    const Foam& foam = 
	GetSimulation (viewNumber).GetFoam (vs.GetCurrentTime ());
    const Foam::Bodies& bodies = foam.GetBodies ();

    glNewList (m_listFacesNormal[viewNumber], GL_COMPILE);
    if (EdgesShown ())
	displayFacesContour (bodies, viewNumber);
    displayFacesInterior (bodies, viewNumber);
    displayStandaloneEdges< DisplayEdgePropertyColor<> > (foam);
    displayAverageAroundBodies (viewNumber);
    displayContextBodies (viewNumber);
    displayContextBox (viewNumber);
    displayStandaloneFaces ();    
    displayDeformation (viewNumber);
    displayVelocity (viewNumber);
    if (m_t1sShown)
	displayT1sDot (viewNumber, GetCurrentTime (viewNumber));
    GetViewAverage (viewNumber).GetForceAverage ().DisplayOneTimeStep ();
    glEndList ();
}


G3D::Vector2 WidgetGl::toTexture (ViewNumber::Enum viewNumber, 
				  G3D::Vector2 object) const
{
    G3D::Vector2 eye = toEye (G3D::Vector3 (object, 0)).xy ();
    G3D::AABox vv = calculateCenteredViewingVolume (viewNumber);
    return (eye - vv.low ().xy ()) / (vv.high ().xy () - vv.low ().xy ());
}

void WidgetGl::displayFacesAverage (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const Foam& foam = GetSimulation (viewNumber).GetFoam (0);
    const DataProperties& foamProperties = foam.GetDataProperties ();
    if (! foamProperties.Is2D ())
	return;
    glPushAttrib (GL_ENABLE_BIT);    
    glDisable (GL_DEPTH_TEST);
    glBindTexture (GL_TEXTURE_1D, m_colorBarTexture[viewNumber]);
    bool adjustForAverageAroundMovementRotation = 
	vs.IsAverageAroundRotationShown ();
    const Simulation& simulation = GetSimulation (viewNumber);

    G3D::Vector3 rotationCenter;
    float angleDegrees;
    if (vs.IsAverageAround ())
    {
	const ObjectPosition rotationBegin = vs.GetAverageAroundPosition (0);
	const ObjectPosition rotationCurrent = 
	    vs.GetAverageAroundPosition (GetCurrentTime (viewNumber));
	rotationCenter = 
	    toEye (rotationCurrent.m_rotationCenter) - 
	    getEyeTransform (viewNumber);
	angleDegrees =
	    adjustForAverageAroundMovementRotation ? 
	    - G3D::toDegrees (
		rotationCurrent.m_angleRadians - rotationBegin.m_angleRadians) : 
	    0;
	if (simulation.GetReflectionAxis () == 1)
	    angleDegrees = - angleDegrees;
    }
    else
    {
	rotationCenter = 
	    toEye (simulation.GetFoam(0).GetBoundingBoxTorus ().center ())
	    - getEyeTransform (viewNumber);
	angleDegrees = 0;
    }
    GetViewAverage (viewNumber).AverageRotateAndDisplay (
	vs.GetStatisticsType (), rotationCenter.xy (), angleDegrees);
    GetViewAverage (viewNumber).GetForceAverage ().Display (
	adjustForAverageAroundMovementRotation);
    displayStandaloneEdges< DisplayEdgePropertyColor<> > (
	GetSimulation (viewNumber).GetFoam (0));
    if (m_t1sShown)
	displayT1sDot (viewNumber);
    displayAverageAroundBodies (
	viewNumber, adjustForAverageAroundMovementRotation);
    displayContextBodies (viewNumber);
    displayContextBox (
	viewNumber, adjustForAverageAroundMovementRotation);
    T1sPDE& t1sPDE = GetViewAverage (viewNumber).GetT1sPDE ();
    if (vs.GetViewType () == ViewType::T1S_PDE &&
	t1sPDE.IsKernelTextureSizeShown ())
    {
	size_t timeStep = GetCurrentTime (viewNumber);
	size_t stepSize = GetSimulation (viewNumber).GetT1s (
	    timeStep, vs.T1sShiftLower ()).size ();
	for (size_t i = 0; i < stepSize; ++i)
	    t1sPDE.DisplayTextureSize (viewNumber, timeStep, i);
    }
    glPopAttrib ();
}

void WidgetGl::displayStandaloneFaces () const
{
    if (m_standaloneElementsShown)
    {
	const Foam::Faces& faces = 
	    GetSimulation ().GetFoam (0).GetStandaloneFaces ();
	displayFacesContour (faces);
	displayFacesInterior (faces);
    }
}

void WidgetGl::displayFacesContour (const Foam::Faces& faces) const
{
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT);
    for_each (faces.begin (), faces.end (),
	      DisplayFaceHighlightColor<HighlightNumber::H0,
	      DisplayFaceEdges<DisplayEdgePropertyColor<> > > (
		  *m_settings, GetSimulation ().GetFoam (0)));

    glPopAttrib ();
}

void WidgetGl::displayFacesContour (
    const Foam::Bodies& bodies, ViewNumber::Enum viewNumber,
    GLfloat lineWidth) const
{
    const BodySelector& bodySelector = 
	GetViewSettings (viewNumber).GetBodySelector ();
    const Simulation& simulation = GetSimulation (viewNumber);
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LINE_BIT);
    glLineWidth (lineWidth);
    for_each (bodies.begin (), bodies.end (),
	      DisplayBody<
	      DisplayFaceHighlightColor<HighlightNumber::H0,
	      DisplayFaceEdges<DisplayEdgePropertyColor<> > > > (
		  *m_settings, simulation.GetFoam (0), bodySelector));

    glPopAttrib ();
}



// See OpenGL Programming Guide, 7th edition, Chapter 6: Blending,
// Antialiasing, Fog and Polygon Offset page 293
void WidgetGl::displayFacesInterior (
    const Foam::Bodies& b, ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const Foam& foam = GetSimulation (viewNumber).GetFoam (0);
    Foam::Bodies bodies = b;
    const BodySelector& bodySelector = vs.GetBodySelector ();
    // partition: opaque bodies first, then transparent bodies
    Foam::Bodies::const_iterator contextBodiesBegin = 
	partition (bodies.begin (), bodies.end (), 
		   BodySelectorPredicate (bodySelector));
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | 
		  GL_TEXTURE_BIT);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1, 1);

    if (DATA_PROPERTIES.Is2D ())
    {
	glEnable (GL_STENCIL_TEST);
	glClear(GL_STENCIL_BUFFER_BIT);
    }

    glEnable(GL_TEXTURE_1D);
    //See OpenGL FAQ 21.030 Why doesn't lighting work when I turn on 
    //texture mapping?
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture (GL_TEXTURE_1D, m_colorBarTexture[viewNumber]);
    // render opaque bodies and then transparent objects
    // See OpenGL Programming Guide, 7th edition, Chapter 6: Blending,
    // Antialiasing, Fog and Polygon Offset page 293

    boost::array<FocusContextInfo, 2> beginEnd =
    {{
	    {bodies.begin (), contextBodiesBegin, false},
	    {contextBodiesBegin, bodies.end (), true}
	}};
    for (size_t i = 0; i < beginEnd.size (); ++i)
    {
	if (beginEnd[i].m_isContext)
	    DisplayBodyBase<>::BeginContext ();
	for_each (beginEnd[i].m_begin, beginEnd[i].m_end,
		  DisplayBody<DisplayFaceBodyScalarColor<> > (
		      *m_settings, foam, bodySelector, 
		      DisplayElement::USER_DEFINED_CONTEXT, viewNumber));
	if (beginEnd[i].m_isContext)
	    DisplayBodyBase<>::EndContext ();
    }
    glPopAttrib ();
}


void WidgetGl::displayFacesInterior (const Foam::Faces& faces) const
{
    glPushAttrib (GL_POLYGON_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1, 1);
    for_each (faces.begin (), faces.end (),
	      DisplayFaceDmpColor<0xff000000>(
		  *m_settings, GetSimulation ().GetFoam (0)));
    glPopAttrib ();
}

void WidgetGl::displayFacesTorusTubes () const
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
	    *m_settings, GetSimulation ().GetFoam (0)));
    glPopAttrib ();
}


void WidgetGl::displayFacesTorusLines () const
{
    glPushAttrib (GL_LINE_BIT | GL_CURRENT_BIT);

    FaceSet faceSet;
    GetSimulation ().GetFoam (0).GetFaceSet (&faceSet);
    for_each (faceSet.begin (), faceSet.end (),
	      DisplayFaceHighlightColor<HighlightNumber::H0,
	      DisplayFaceEdges<
	      DisplayEdgeTorus<DisplaySegment, DisplaySegmentArrow1, true> > > (
		  *m_settings, 
		  GetSimulation ().GetFoam (0), DisplayElement::FOCUS) );
    glPopAttrib ();
}

void WidgetGl::displayCenterPathsWithBodies (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    size_t currentTime = GetCurrentTime (viewNumber);
    const Simulation& simulation = GetSimulation (viewNumber);
    const BodySelector& bodySelector = vs.GetBodySelector ();
    displayCenterPaths (viewNumber);
    
    glPushAttrib (GL_ENABLE_BIT);
    if (vs.IsLightingEnabled ())
	glDisable (GL_LIGHTING);
    if (IsCenterPathBodyShown ())
    {
	const Foam::Bodies& bodies = 
	    simulation.GetFoam (currentTime).GetBodies ();
	double zPos = currentTime * GetTimeDisplacement ();
	for_each (
	    bodies.begin (), bodies.end (),
	    DisplayBody<DisplayFaceHighlightColor<HighlightNumber::H0,
	    DisplayFaceEdges<DisplayEdgePropertyColor<
	    DisplayElement::DONT_DISPLAY_TESSELLATION_EDGES> > > > (
		*m_settings, simulation.GetFoam (currentTime),
		bodySelector, DisplayElement::USER_DEFINED_CONTEXT,
		viewNumber, IsTimeDisplacementUsed (), zPos));
    }
    displayStandaloneEdges< DisplayEdgePropertyColor<> > (
	simulation.GetFoam (currentTime), viewNumber, true, 0);
    if (GetTimeDisplacement () != 0)
    {

	displayStandaloneEdges< DisplayEdgePropertyColor<> > (
	    simulation.GetFoam (currentTime), viewNumber,
	    IsTimeDisplacementUsed (),
	    (simulation.GetTimeSteps () - 1) * GetTimeDisplacement ());
    }
    glPopAttrib ();
}


void WidgetGl::CompileUpdate ()
{
    makeCurrent ();
    compile (GetViewNumber ());
    update ();
}

void WidgetGl::compile (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = m_settings->GetViewSettings (viewNumber);
    switch (vs.GetViewType ())
    {
    case ViewType::CENTER_PATHS:
	compileCenterPaths (viewNumber);
	break;
    case ViewType::FACES:
	compileFacesNormal (viewNumber);
    default:
	break;
    }
}

void WidgetGl::displayCenterPaths (ViewNumber::Enum viewNumber) const
{
    glCallList (m_listCenterPaths[viewNumber]);
}

void WidgetGl::compileCenterPaths (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    const BodySelector& bodySelector = vs.GetBodySelector ();
    glNewList (m_listCenterPaths[viewNumber], GL_COMPILE);
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT | 
		  GL_POLYGON_BIT | GL_LINE_BIT);
    glEnable(GL_TEXTURE_1D);
    glBindTexture (GL_TEXTURE_1D, m_colorBarTexture[viewNumber]);
    glEnable (GL_CULL_FACE);

    //See OpenGL FAQ 21.030 Why doesn't lighting work when I turn on 
    //texture mapping?
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    const BodiesAlongTime::BodyMap& bats = 
	GetSimulation ().GetBodiesAlongTime ().GetBodyMap ();
    if (m_settings->GetEdgeRadiusRatio () > 0 && 
	! m_settings->IsCenterPathLineUsed ())
    {
	if (m_settings->IsCenterPathTubeUsed ())
	    for_each (
		bats.begin (), bats.end (),
		DisplayCenterPath<
		SetterTextureCoordinate, DisplaySegmentTube> (
		    *m_settings, 
		    GetSimulation (viewNumber).GetFoam (0), 
		    GetViewNumber (), bodySelector, GetQuadricObject (),
		    GetSimulation (viewNumber),
		    IsTimeDisplacementUsed (), GetTimeDisplacement ()));
	else
	    for_each (
		bats.begin (), bats.end (),
		DisplayCenterPath<
		SetterTextureCoordinate, DisplaySegmentQuadric> (
		    *m_settings, 
		    GetSimulation (viewNumber).GetFoam (0), 
		    GetViewNumber (), bodySelector, GetQuadricObject (),
		    GetSimulation (viewNumber),
		    IsTimeDisplacementUsed (), GetTimeDisplacement ()));
    }
    else
	for_each (bats.begin (), bats.end (),
		  DisplayCenterPath<SetterTextureCoordinate, 
		  DisplaySegment> (
		      *m_settings, 
		      GetSimulation (viewNumber).GetFoam (0), 
		      GetViewNumber (), bodySelector, GetQuadricObject (),
		      GetSimulation (viewNumber),
		      IsTimeDisplacementUsed (), GetTimeDisplacement ()));
    glPopAttrib ();
    glEndList ();
}

void WidgetGl::setLight (int sliderValue, int maximumValue, 
			 LightType::Enum lightType, 
			 ColorNumber::Enum colorNumber)
{
    ViewSettings& vs = GetViewSettings ();
    LightNumber::Enum selectedLight = vs.GetSelectedLight ();
    vs.SetLight (selectedLight, lightType, colorNumber,  
		 double(sliderValue) / maximumValue);
    update ();
}

ColorBarType::Enum WidgetGl::GetColorBarType () const
{
    return GetColorBarType (GetViewNumber ());
}

ColorBarType::Enum WidgetGl::GetColorBarType (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    ViewType::Enum viewType = vs.GetViewType ();
    size_t property = vs.GetBodyOrFaceScalar ();
    StatisticsType::Enum statisticsType = vs.GetStatisticsType ();
    return WidgetGl::GetColorBarType (viewType, property, statisticsType);
}


ColorBarType::Enum WidgetGl::GetColorBarType (
    ViewType::Enum viewType, size_t property, 
    StatisticsType::Enum statisticsType)
{
    switch (viewType)
    {
    case ViewType::T1S_PDE:
	return ColorBarType::T1S_PDE;
    case ViewType::AVERAGE:
	if (statisticsType == StatisticsType::COUNT)
	    return ColorBarType::STATISTICS_COUNT;
    case ViewType::FACES:
	if (property == FaceScalar::DMP_COLOR)
	    return ColorBarType::NONE;
    case ViewType::CENTER_PATHS:
	return ColorBarType::PROPERTY;
    default:
	return ColorBarType::NONE;
    }
}

size_t WidgetGl::GetCurrentTime (ViewNumber::Enum viewNumber) const
{
    return GetViewSettings (viewNumber).GetCurrentTime ();
}

pair<size_t, ViewNumber::Enum> WidgetGl::LinkedTimeMaxSteps () const
{
    pair<size_t, ViewNumber::Enum> maxInterval = m_settings->LinkedTimeMaxInterval ();    
    pair<size_t, ViewNumber::Enum> max (0, ViewNumber::COUNT);
    for (size_t i = 0; i < ViewCount::GetCount (m_settings->GetViewCount ()); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	size_t maxStep = 
	    (GetTimeSteps (viewNumber) - 1) *
	    m_settings->LinkedTimeStepStretch (maxInterval.first, viewNumber);
	if (max.first < maxStep)
	{
	    max.first = maxStep;
	    max.second = viewNumber;
	}
    }
    max.first += 1;
    return max;
}



size_t WidgetGl::GetTimeSteps (ViewNumber::Enum viewNumber) const
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    ViewType::Enum viewType = vs.GetViewType ();
    size_t simulationIndex = vs.GetSimulationIndex ();
    const Simulation& simulation = GetSimulation (simulationIndex);
    return (viewType == ViewType::T1S_PDE) ?
	simulation.GetT1sTimeSteps () :
	simulation.GetTimeSteps ();
}


void WidgetGl::quadricErrorCallback (GLenum errorCode)
{
    const GLubyte* message = gluErrorString (errorCode);
    cdbg << "Quadric error:" << message << endl;
}

void WidgetGl::contextMenuEventOverlayBar (QMenu* menu) const
{
    menu->addAction (m_actionOverlayBarClampClear.get ());
    menu->addAction (m_actionEditOverlayMap.get ());
}

void WidgetGl::contextMenuEventColorBar (QMenu* menu) const
{
    const ViewSettings& vs = GetViewSettings ();
    menu->addAction (m_actionColorBarClampClear.get ());
    bool actions = false;
    QMenu* menuCopy = menu->addMenu ("Copy");
    if (ViewCount::GetCount (m_settings->GetViewCount ()) > 1)
    {
	size_t currentProperty = vs.GetBodyOrFaceScalar ();
	for (size_t i = 0; i < ViewCount::GetCount (m_settings->GetViewCount ()); ++i)
	{
	    ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	    const ViewSettings& otherVs = GetViewSettings (viewNumber);
	    if (viewNumber == GetViewNumber () ||
		GetColorBarType (GetViewNumber ()) != GetColorBarType (viewNumber) ||
		currentProperty != otherVs.GetBodyOrFaceScalar () ||
		vs.GetSimulationIndex () != otherVs.GetSimulationIndex ())
		continue;
	    menuCopy->addAction (m_actionCopyColorMap[i].get ());
	    actions = true;
	}
    }
    if (! actions)
	menuCopy->setDisabled (true);
    menu->addAction (m_actionEditColorMap.get ());
}

void WidgetGl::contextMenuEventView (QMenu* menu) const
{
    ViewSettings& vs = GetViewSettings ();
    {
	QMenu* menuAverageAround = menu->addMenu ("Average around");
	menuAverageAround->addAction (m_actionAverageAroundBody.get ());
	menuAverageAround->addAction (
	    m_actionAverageAroundSecondBody.get ());
	menuAverageAround->addAction (m_actionAverageAroundReset.get ());
	m_actionAverageAroundShowRotation->setChecked (
	    vs.IsAverageAroundRotationShown ());
	menuAverageAround->addAction (
	    m_actionAverageAroundShowRotation.get ());
    }
    {
	QMenu* menuContext = menu->addMenu ("Context display");
	menuContext->addAction (m_actionContextDisplayBody.get ());
	menuContext->addAction (m_actionContextDisplayReset.get ());
    }
    if (ViewCount::GetCount (m_settings->GetViewCount ()) > 1)
    {
	QMenu* menuCopy = menu->addMenu ("Copy");
	QMenu* menuTransformation = menuCopy->addMenu ("Transformation");
	QMenu* menuSelection = menuCopy->addMenu ("Selection");
	for (size_t i = 0; i < ViewCount::GetCount (m_settings->GetViewCount ()); ++i)
	{
	    ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	    if (viewNumber == GetViewNumber ())
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
	menuInfo->addAction (m_actionInfoSelectedBodies.get ());
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
	QMenu* menuRotationCenter = menu->addMenu ("Rotation center");
	menuRotationCenter->addAction (m_actionRotationCenterBody.get ());
	menuRotationCenter->addAction (m_actionRotationCenterFoam.get ());
    }
    {
	QMenu* menuSelect = menu->addMenu ("Select");
	menuSelect->addAction (m_actionSelectAll.get ());
	menuSelect->addAction (m_actionDeselectAll.get ());
	menuSelect->addAction (m_actionSelectBodiesByIdList.get ());
	menuSelect->addAction (m_actionSelectThisBodyOnly.get ());
    }
    {
	QMenu* menuShow = menu->addMenu ("Show");
	menuShow->addAction (m_actionShowNeighbors.get ());
	menuShow->addAction (m_actionShowDeformation.get ());
	menuShow->addAction (m_actionShowVelocity.get ());
	menuShow->addAction (m_actionShowReset.get ());
    }
}

void WidgetGl::displayViewDecorations (ViewNumber::Enum viewNumber)
{
    const ViewSettings& vs = GetViewSettings (viewNumber);
    initTransformViewport ();
    if (vs.IsLightingEnabled ())
	glDisable (GL_LIGHTING);
    glDisable (GL_DEPTH_TEST);
    G3D::Rect2D viewRect = GetViewRect (viewNumber);
    if (GetColorBarType (viewNumber) != ColorBarType::NONE)
    {
	G3D::Rect2D viewColorBarRect = Settings::GetViewColorBarRect (viewRect);
	/*
	cdbg << viewRect << endl;
	cdbg << viewColorBarRect << endl;
	*/
	displayTextureColorBar (
	    m_colorBarTexture[viewNumber], viewNumber, viewColorBarRect);
    }
    if (vs.IsVelocityShown ())
    {
	if (GetViewAverage (viewNumber).GetVelocityAverage ().IsColorMapped ())
	    displayTextureColorBar (
		m_overlayBarTexture[viewNumber],
		viewNumber, Settings::GetViewOverlayBarRect (viewRect));
	else if ( 
	    ! GetViewAverage (viewNumber).GetVelocityAverage ().IsSameSize ())
	    displayOverlayBar (
		viewNumber, Settings::GetViewOverlayBarRect (viewRect));
    }
    displayViewTitle (viewNumber);
    if (viewNumber == GetViewNumber () && m_viewFocusShown &&
	m_settings->GetViewCount () != ViewCount::ONE)
	displayViewFocus (viewNumber);
    cleanupTransformViewport ();
}

void WidgetGl::initTransformViewport ()
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

void WidgetGl::cleanupTransformViewport ()
{
    glPopMatrix ();
    glMatrixMode (GL_MODELVIEW);
    glPopMatrix ();
    glPopAttrib ();
}

void WidgetGl::displayViewTitle (ViewNumber::Enum viewNumber)
{
    if (! m_titleShown)
	return;
    ostringstream ostr;
    ViewSettings& vs = GetViewSettings (viewNumber);
    ostr << "View " << viewNumber << " - "
	 << ViewType::ToString (vs.GetViewType ()) << " - "
	 << FaceScalar::ToString (vs.GetBodyOrFaceScalar ()) << " - "
	 << vs.GetCurrentTime ();    
    displayViewText (viewNumber, GetSimulation (viewNumber).GetName (), 0);
    displayViewText (viewNumber, ostr.str (), 1);
}

void WidgetGl::displayViewText (
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
    glColor (m_settings->GetHighlightColor (viewNumber, HighlightNumber::H0));
    renderText (textX, textY, text, font);    
}



size_t WidgetGl::GetBodyOrFaceScalar (ViewNumber::Enum viewNumber) const
{
    return GetViewSettings (viewNumber).GetBodyOrFaceScalar ();
}


void WidgetGl::displayViewFocus (ViewNumber::Enum viewNumber)
{
    G3D::Rect2D viewRect = GetViewRect (viewNumber);
    G3D::Vector2 margin (3, 3);
    G3D::Rect2D rect = G3D::Rect2D::xyxy(
	viewRect.x0y0 () + margin, viewRect.x1y1 () - margin);
    glColor (m_settings->GetHighlightColor (viewNumber, HighlightNumber::H0));
    glPolygonMode (GL_FRONT, GL_LINE);
    DisplayBox (rect);
}

void WidgetGl::displayTextureColorBar (
    GLuint texture,
    ViewNumber::Enum viewNumber, const G3D::Rect2D& colorBarRect)
{
    glPushAttrib (GL_POLYGON_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT);
    glDisable (GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_1D);
    glBindTexture (GL_TEXTURE_1D, texture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glBegin (GL_QUADS);
    glTexCoord1f(0);::glVertex (colorBarRect.x0y0 ());
    glTexCoord1f(1);::glVertex (colorBarRect.x0y1 ());
    glTexCoord1f(1);::glVertex (colorBarRect.x1y1 ());
    glTexCoord1f(0);::glVertex (colorBarRect.x1y0 ());
    glEnd ();
    glDisable (GL_TEXTURE_1D);

    glColor (m_settings->GetHighlightColor (viewNumber, HighlightNumber::H0));
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    DisplayBox (colorBarRect);
    glPopAttrib ();
}

void WidgetGl::displayOverlayBar (
    ViewNumber::Enum viewNumber, const G3D::Rect2D& barRect)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    glPushAttrib (GL_POLYGON_BIT | GL_ENABLE_BIT | GL_LINE_BIT);
    glDisable (GL_DEPTH_TEST);
    glColor (Qt::white);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    DisplayBox (barRect);
    glColor (m_settings->GetHighlightColor (viewNumber, HighlightNumber::H0));
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    DisplayBox (barRect);
    float y = 
	barRect.y0 () + (barRect.y1 () - barRect.y0 ()) / 
	vs.GetVelocityClampingRatio ();
    glLineWidth (2);    
    glBegin (GL_LINES);
    ::glVertex (G3D::Vector2 (barRect.x0 (), y));
    ::glVertex (G3D::Vector2 (barRect.x1 (), y));
    glEnd ();
    glPopAttrib ();
}


void WidgetGl::displayViewsGrid ()
{
    glPushAttrib (GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LINE_BIT);
    initTransformViewport ();
    size_t w = width ();
    size_t h = height ();
    glDisable (GL_DEPTH_TEST);
    glColor (Qt::black);
    if (m_reflectedHalfView)	
	glLineWidth (3);
    glBegin (GL_LINES);
    switch (m_settings->GetViewCount ())
    {
    case ViewCount::TWO:
    {
	if (m_settings->GetViewLayout () == ViewLayout::HORIZONTAL)
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
	if (m_settings->GetViewLayout () == ViewLayout::HORIZONTAL)
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
    glPopAttrib ();
}

bool WidgetGl::IsTimeDisplacementUsed () const
{
    return GetTimeDisplacement () > 0;
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
void WidgetGl::activateViewShader (
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

void WidgetGl::ActivateViewShader (ViewNumber::Enum viewNumber) const
{
    ActivateViewShader (viewNumber, ViewingVolumeOperation::ENCLOSE2D,
			G3D::Vector2::zero (), 0);
}

void WidgetGl::ActivateViewShader (ViewNumber::Enum viewNumber, 
				   ViewingVolumeOperation::Enum enclose,
				   G3D::Rect2D& srcRect) const
{
    activateViewShader (viewNumber, enclose,
			srcRect, G3D::Vector2::zero (), 0);
}


void WidgetGl::ActivateViewShader (
    ViewNumber::Enum viewNumber, ViewingVolumeOperation::Enum enclose,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    G3D::Rect2D srcRect = 
	toRect2D (CalculateViewingVolume (viewNumber,
					  ViewingVolumeOperation::ENCLOSE2D));
    activateViewShader (viewNumber, enclose, srcRect, 
			rotationCenter, angleDegrees);
}

void WidgetGl::setScaleCenter (ViewNumber::Enum viewNumber)
{
    ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::Rect2D rect = 
	toRect2D (CalculateViewingVolume (
		      viewNumber, 
		      ViewingVolumeOperation::DONT_ENCLOSE2D));
    G3D::Vector2 newCenter = calculateScaleCenter (viewNumber, rect);
    vs.SetScaleCenter (newCenter);
}


void WidgetGl::SetReflectedHalfView (bool reflectedHalfView)
{
    m_reflectedHalfView = reflectedHalfView;
    setScaleCenter (ViewNumber::VIEW0);
    setScaleCenter (ViewNumber::VIEW1);
    update ();
}

void WidgetGl::SetForceDifferenceShown (bool value)
{
    GetViewSettings ().SetForceDifferenceShown (value);
    update ();
}

void WidgetGl::valueChangedT1sKernelSigma (ViewNumber::Enum viewNumber)
{
    T1sPDE& t1sPDE = GetViewAverage (viewNumber).GetT1sPDE ();
    t1sPDE.SetKernelSigma (
	Index2Value (static_cast<QSlider*> (sender ()), T1sPDE::KERNEL_SIGMA));
    t1sPDE.AverageInitStep ();
}

void WidgetGl::valueChangedT1sKernelTextureSize (
    ViewNumber::Enum viewNumber)
{
    T1sPDE& t1sPDE = GetViewAverage (viewNumber).GetT1sPDE ();
    t1sPDE.SetKernelTextureSize (
	Index2Value (static_cast<QSlider*> (sender ()), 
		     T1sPDE::KERNEL_TEXTURE_SIZE));
    t1sPDE.AverageInitStep ();    
}

void WidgetGl::toggledT1sKernelTextureSizeShown (ViewNumber::Enum viewNumber)
{
    bool checked = static_cast<QCheckBox*> (sender ())->isChecked ();
    GetViewAverage (viewNumber).GetT1sPDE ().SetKernelTextureSizeShown (checked);
}


void WidgetGl::valueChangedT1sKernelIntervalPerPixel (
    ViewNumber::Enum viewNumber)
{
    T1sPDE& t1sPDE = GetViewAverage (viewNumber).GetT1sPDE ();
    t1sPDE.SetKernelIntervalPerPixel (
	Index2Value (static_cast<QSlider*> (sender ()), 
		     T1sPDE::KERNEL_INTERVAL_PER_PIXEL));
    t1sPDE.AverageInitStep ();
}

template<typename T>
void WidgetGl::SetOneOrTwoViews (T* t, void (T::*f) (ViewNumber::Enum))
{
    if (IsReflectedHalfView ())
    {
	CALL_MEMBER_FN (*t, f) (ViewNumber::VIEW0);
	CALL_MEMBER_FN (*t, f) (ViewNumber::VIEW1);
    }
    else
	CALL_MEMBER_FN (*t, f) (GetViewNumber ());
    update ();
}

vector<ViewNumber::Enum> WidgetGl::GetConnectedViewNumbers (
    ViewNumber::Enum viewNumber) const
{
    if (m_reflectedHalfView)
    {
	vector<ViewNumber::Enum> vn(2);
	vn[0] = ViewNumber::VIEW0;
	vn[1] = ViewNumber::VIEW1;
	return vn;
    }
    else
    {
	vector<ViewNumber::Enum> vn(1);
	vn[0] = 
	    (viewNumber == ViewNumber::COUNT ? GetViewNumber () : viewNumber);
	return vn;
    }
}


void WidgetGl::UpdateAverage (
    const boost::array<int, ViewNumber::COUNT>& direction)
{
    for (size_t i = 0;
	 i < ViewCount::GetCount (m_settings->GetViewCount ()); ++i)
	if (direction[i] != 0)
	    m_viewAverage[i]->AverageStep (direction[i]);
    update ();
}


// Slots and methods called by the UI
// ==================================

void WidgetGl::mousePressEvent(QMouseEvent *event)
{
    makeCurrent ();
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


void WidgetGl::mouseMoveEvent(QMouseEvent *event)
{
    makeCurrent ();
    vector<ViewNumber::Enum> vn = GetConnectedViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	switch (m_interactionMode)
	{
	case InteractionMode::ROTATE:
	    mouseMoveRotate (event, viewNumber);
	    break;

	case InteractionMode::TRANSLATE:
	    mouseMoveTranslate (event, viewNumber);
	    break;

	case InteractionMode::SCALE:
	    mouseMoveScale (event, viewNumber);
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
    }
    m_lastPos = event->pos();
    update ();
}

void WidgetGl::contextMenuEvent (QContextMenuEvent *event)
{
    // make sure you act on the current window's context
    makeCurrent ();
    m_contextMenuPosScreen = event->pos ();
    m_contextMenuPosObject = toObjectTransform (m_contextMenuPosScreen);
    QMenu menu (this);
    G3D::Rect2D colorBarRect = Settings::GetViewColorBarRect (GetViewRect ());
    G3D::Rect2D overlayBarRect = 
	Settings::GetViewOverlayBarRect (GetViewRect ());
    if (colorBarRect.contains (QtToOpenGl (m_contextMenuPosScreen, height ())))
	contextMenuEventColorBar (&menu);
    else if (overlayBarRect.contains (
		 QtToOpenGl (m_contextMenuPosScreen, height ())))
	contextMenuEventOverlayBar (&menu);
    else
	contextMenuEventView (&menu);
    menu.exec (event->globalPos());
}


void WidgetGl::ResetTransformAll ()
{
    makeCurrent ();
    ResetTransformFocus ();
    ResetTransformContext ();
    ResetTransformGrid ();
    ResetTransformLight ();
}

void WidgetGl::ResetTransformFocus ()
{
    makeCurrent ();
    vector<ViewNumber::Enum> vn = GetConnectedViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	ViewSettings& vs = GetViewSettings (viewNumber);
	vs.SetRotation (G3D::Matrix3::identity ());
	vs.SetScaleRatio (1);
	vs.SetTranslation (G3D::Vector3::zero ());
	glMatrixMode (GL_PROJECTION);
	ProjectionTransform (viewNumber);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	GetViewAverage (viewNumber).AverageInitStep ();
    }
    update ();
}

void WidgetGl::ResetTransformContext ()
{
    makeCurrent ();
    vector<ViewNumber::Enum> vn = GetConnectedViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	ViewSettings& vs = GetViewSettings (viewNumber);
	vs.SetContextScaleRatio (1);
	glMatrixMode (GL_PROJECTION);
	ProjectionTransform (viewNumber);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
    }
    update ();

}

void WidgetGl::ResetTransformGrid ()
{
    makeCurrent ();
    vector<ViewNumber::Enum> vn = GetConnectedViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	ViewSettings& vs = GetViewSettings (viewNumber);
	vs.SetGridScaleRatio (1);
	vs.SetGridTranslation (G3D::Vector3::zero ());
    }
    update ();
}

void WidgetGl::ResetTransformLight ()
{
    makeCurrent ();
    vector<ViewNumber::Enum> vn = GetConnectedViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	ViewSettings& vs = GetViewSettings (viewNumber);
	LightNumber::Enum lightNumber = vs.GetSelectedLight ();
	vs.SetInitialLightParameters (lightNumber);
    }
    update ();
}

void WidgetGl::RotationCenterBody ()
{
    makeCurrent ();
    vector< boost::shared_ptr<Body> > bodies;
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    brushedBodies (m_contextMenuPosScreen, &bodies);
    if (bodies.size () > 0)
    {
	vs.SetRotationCenter (bodies[0]->GetCenter ());
	vs.SetRotationCenterType (ViewSettings::ROTATION_CENTER_BODY);
    }
}

void WidgetGl::RotationCenterFoam ()
{
    makeCurrent ();
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    G3D::Vector3 center = CalculateViewingVolume (
	viewNumber, ViewingVolumeOperation::DONT_ENCLOSE2D).center ();
    vs.SetRotationCenter (center);
    vs.SetRotationCenterType (ViewSettings::ROTATION_CENTER_FOAM);
}


void WidgetGl::CopyTransformationFrom (int viewNumber)
{
    makeCurrent ();
    GetViewSettings ().CopyTransformation (
	GetViewSettings (ViewNumber::Enum (viewNumber)));
    update ();
}

void WidgetGl::CopySelectionFrom (int viewNumber)
{
    makeCurrent ();
    GetViewSettings ().CopySelection (
	GetViewSettings (ViewNumber::Enum (viewNumber)));
}

void WidgetGl::CopyColorBarFrom (int other)
{
    makeCurrent ();
    ViewSettings& otherVs = GetViewSettings (ViewNumber::Enum (other));
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings ();
    vs.CopyColorBar (otherVs);
    Q_EMIT ColorBarModelChanged (viewNumber, vs.GetColorBarModel ());
}


void WidgetGl::ToggledDirectionalLightEnabled (bool checked)
{
    makeCurrent ();
    ViewSettings& vs = GetViewSettings ();
    LightNumber::Enum selectedLight = vs.GetSelectedLight ();
    vs.SetDirectionalLightEnabled (selectedLight, checked);
    update ();
}

void WidgetGl::ToggledDeformationShown (bool checked)
{
    makeCurrent ();
    vector<ViewNumber::Enum> vn = GetConnectedViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	ViewSettings& vs = GetViewSettings (viewNumber);
	vs.SetDeformationTensorShown (checked);
    }
    update ();
}

void WidgetGl::ToggledDeformationShownGrid (bool checked)
{
    makeCurrent ();
    vector<ViewNumber::Enum> vn = GetConnectedViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	TensorAverage& ta = 
	    GetViewAverage (viewNumber).GetDeformationAverage ();
	ta.SetGridShown (checked);
    }
    update ();
}

void WidgetGl::ToggledVelocityShown (bool checked)
{
    makeCurrent ();
    vector<ViewNumber::Enum> vn = GetConnectedViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	ViewSettings& vs = GetViewSettings (viewNumber);
	vs.SetVelocityShown (checked);
    }
    update ();
}

void WidgetGl::ToggledVelocityGridShown (bool checked)
{
    makeCurrent ();
    vector<ViewNumber::Enum> vn = GetConnectedViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	VectorAverage& va = GetViewAverage (viewNumber).GetVelocityAverage ();
	va.SetGridShown (checked);
    }
    update ();
}

void WidgetGl::ToggledVelocityClampingShown (bool checked)
{
    makeCurrent ();
    vector<ViewNumber::Enum> vn = GetConnectedViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	VectorAverage& va = GetViewAverage (viewNumber).GetVelocityAverage ();
	va.SetClampingShown (checked);
    }
    update ();
}


void WidgetGl::ToggledDeformationGridCellCenterShown (bool checked)
{
    makeCurrent ();
    vector<ViewNumber::Enum> vn = GetConnectedViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	TensorAverage& ta = 
	    GetViewAverage (viewNumber).GetDeformationAverage ();
	ta.SetGridCellCenterShown (checked);
    }
    update ();
}

void WidgetGl::ToggledVelocityGridCellCenterShown (bool checked)
{
    makeCurrent ();
    vector<ViewNumber::Enum> vn = GetConnectedViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	VectorAverage& ta = 
	    GetViewAverage (viewNumber).GetVelocityAverage ();
	ta.SetGridCellCenterShown (checked);
    }
    update ();
}

void WidgetGl::ToggledVelocitySameSize (bool checked)
{
    makeCurrent ();
    vector<ViewNumber::Enum> vn = GetConnectedViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
	GetViewAverage (vn[i]).GetVelocityAverage ().SetSameSize (checked);
    update ();    
}

void WidgetGl::ToggledVelocityColorMapped (bool checked)
{
    makeCurrent ();
    vector<ViewNumber::Enum> vn = GetConnectedViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
	GetViewAverage (vn[i]).GetVelocityAverage ().SetColorMapped (checked);
    update ();    
}

void WidgetGl::ToggledMissingPressureShown (bool checked)
{
    m_settings->SetMissingPressureShown (checked);
    update ();
}

void WidgetGl::ToggledMissingVolumeShown (bool checked)
{
    m_settings->SetMissingVolumeShown (checked);
    update ();
}

void WidgetGl::ToggledObjectVelocityShown (bool checked)
{
    m_settings->SetObjectVelocityShown (checked);
    update ();
}

void WidgetGl::ToggledLightNumberShown (bool checked)
{
    makeCurrent ();
    ViewSettings& vs = GetViewSettings ();
    vs.SetLightPositionShown (vs.GetSelectedLight (), checked);
    update ();
}

void WidgetGl::ToggledLightEnabled (bool checked)
{
    makeCurrent ();
    ViewSettings& vs = GetViewSettings ();
    LightNumber::Enum selectedLight = vs.GetSelectedLight ();
    vs.SetLightEnabled (selectedLight, checked);
    update ();
}


void WidgetGl::ToggledBoundingBoxSimulation (bool checked)
{
    m_boundingBoxSimulationShown = checked;
    update ();
}

void WidgetGl::ToggledBoundingBoxFoam (bool checked)
{
    m_boundingBoxFoamShown = checked;
    update ();
}

void WidgetGl::ToggledBoundingBoxBody (bool checked)
{
    m_boundingBoxBodyShown = checked;
    update ();
}


void WidgetGl::ToggledAverageAroundMarked (bool checked)
{
    m_averageAroundMarked = checked;
    update ();
}

void WidgetGl::ToggledViewFocusShown (bool checked)
{
    m_viewFocusShown = checked;
    update ();
}


void WidgetGl::ToggledContextView (bool checked)
{
    makeCurrent ();
    ViewSettings& vs = GetViewSettings ();
    vs.SetContextView (checked);
    update ();
}

void WidgetGl::ToggledContextBoxShown (bool checked)
{
    m_contextBoxShown = checked;
    update ();
}

void WidgetGl::ToggledForceNetworkShown (bool checked)
{
    makeCurrent ();
    GetViewSettings ().SetForceNetworkShown (checked);
    update ();
}

void WidgetGl::ToggledForcePressureShown (bool checked)
{
    makeCurrent ();
    GetViewSettings ().SetForcePressureShown (checked);
    update ();
}

void WidgetGl::ToggledForceResultShown (bool checked)
{
    makeCurrent ();
    GetViewSettings ().SetForceResultShown (checked);
    update ();
}

void WidgetGl::ToggledTorqueNetworkShown (bool checked)
{
    makeCurrent ();
    GetViewSettings ().SetTorqueNetworkShown (checked);
    update ();
}

void WidgetGl::ToggledTorquePressureShown (bool checked)
{
    makeCurrent ();
    GetViewSettings ().SetTorquePressureShown (checked);
    update ();
}

void WidgetGl::ToggledTorqueResultShown (bool checked)
{
    makeCurrent ();
    GetViewSettings ().SetTorqueResultShown (checked);
    update ();
}


void WidgetGl::ToggledAxesShown (bool checked)
{
    m_axesShown = checked;
    update ();
}

void WidgetGl::ToggledStandaloneElementsShown (bool checked)
{
    m_standaloneElementsShown = checked;
    CompileUpdate ();
}

void WidgetGl::ToggledConstraintsShown (bool checked)
{
    m_settings->SetConstraintsShown (checked);
    update ();
}

void WidgetGl::ToggledConstraintPointsShown (bool checked)
{
    m_settings->SetConstraintPointsShown (checked);
    update ();
}

void WidgetGl::ToggledCenterPathBodyShown (bool checked)
{
    m_centerPathBodyShown = checked;
    update ();
}

void WidgetGl::ToggledSelectionContextShown (bool checked)
{
    makeCurrent ();
    GetViewSettings ().SetSelectionContextShown (checked);
    CompileUpdate ();
}

void WidgetGl::ToggledCenterPathHidden (bool checked)
{
    makeCurrent ();
    GetViewSettings ().SetCenterPathHidden (checked);
    CompileUpdate ();
}

void WidgetGl::ButtonClickedViewType (int id)
{
    makeCurrent ();
    vector<ViewNumber::Enum> vn = GetConnectedViewNumbers ();
    for (size_t i = 0; i < vn.size (); ++i)
    {
	ViewNumber::Enum viewNumber = vn[i];
	ViewSettings& vs = GetViewSettings (viewNumber);
	ViewType::Enum newViewType = ViewType::Enum(id);
	ViewType::Enum oldViewType = vs.GetViewType ();
	if (oldViewType == newViewType)
	    continue;
	GetViewAverage (viewNumber).AverageRelease ();
	vs.SetViewType (newViewType);
	GetViewAverage (viewNumber).AverageInitStep ();
	compile (viewNumber);
    }
    update ();
}

void WidgetGl::ButtonClickedTimeLinkage (int id)
{
    makeCurrent ();
    m_settings->SetTimeLinkage (TimeLinkage::Enum (id));
    update ();
}

void WidgetGl::ToggledBodyCenterShown (bool checked)
{
    m_bodyCenterShown = checked;
    update ();
}

void WidgetGl::ToggledBodyNeighborsShown (bool checked)
{
    m_bodyNeighborsShown = checked;
    update ();
}


void WidgetGl::ToggledFaceCenterShown (bool checked)
{
    m_faceCenterShown = checked;
    update ();
}


void WidgetGl::ToggledEdgesShown (bool checked)
{
    m_edgesShown = checked;
    CompileUpdate ();
}

void WidgetGl::ToggledEdgesTessellationShown (bool checked)
{
    m_settings->SetEdgesTessellationShown (checked);
    update ();
}


void WidgetGl::ToggledTorusDomainShown (bool checked)
{
    m_torusDomainShown = checked;
    update ();
}

void WidgetGl::ToggledCenterPathTubeUsed (bool checked)
{
    makeCurrent ();
    m_settings->SetCenterPathTubeUsed (checked);
    CompileUpdate ();
}

void WidgetGl::ToggledCenterPathLineUsed (bool checked)
{
    makeCurrent ();
    m_settings->SetCenterPathLineUsed (checked);
    CompileUpdate ();
}

void WidgetGl::ToggledTitleShown (bool checked)
{
    m_titleShown = checked;
    update ();
}


void WidgetGl::ToggledTorusOriginalDomainClipped (bool checked)
{
    m_torusOriginalDomainClipped = checked;
    update ();
}

void WidgetGl::ToggledT1sShown (bool checked)
{
    m_t1sShown = checked;
    CompileUpdate ();
}

void WidgetGl::ToggledT1sShiftLower (bool checked)
{
    makeCurrent ();
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    vs.SetT1sShiftLower (checked);
    update ();
}

void WidgetGl::CurrentIndexChangedSelectedLight (int selectedLight)
{
    makeCurrent ();
    ViewSettings& vs = GetViewSettings ();
    vs.SetSelectedLight (LightNumber::Enum (selectedLight));
    update ();
}

void WidgetGl::CurrentIndexChangedSimulation (int i)
{
    makeCurrent ();
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    const Simulation& simulation = GetSimulation (i);
    G3D::Vector3 center = m_settings->CalculateViewingVolume (
	viewNumber, simulation, GetXOverY (),
	ViewingVolumeOperation::DONT_ENCLOSE2D).center ();
    vs.SetSimulation (i, simulation, center, vs.T1sShiftLower ());
    m_viewAverage[viewNumber]->SetSimulation (simulation);
    update ();
}

void WidgetGl::CurrentIndexChangedViewCount (int index)
{
    makeCurrent ();
    m_settings->SetViewCount (ViewCount::Enum (index));
    m_settings->SetViewNumber (ViewNumber::VIEW0);
    size_t n = ViewCount::GetCount (m_settings->GetViewCount ());
    for (size_t i = 0; i < n; ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	ViewSettings& vs = GetViewSettings (viewNumber);
	if (vs.GetViewType () == ViewType::COUNT)
	    vs.SetViewType (ViewType::FACES);
	vs.CalculateCameraDistance (
	    calculateCenteredViewingVolume (viewNumber));
	compile (viewNumber);
    }
    update ();
}

void WidgetGl::CurrentIndexChangedViewLayout (int index)
{
    m_settings->SetViewLayout (ViewLayout::Enum (index));
    update ();
}

void WidgetGl::CurrentIndexChangedInteractionMode (int index)
{
    m_interactionMode = InteractionMode::Enum(index);
    update ();
}

void WidgetGl::ButtonClickedInteractionObject (int index)
{
    m_interactionObject = InteractionObject::Enum (index);
}


void WidgetGl::CurrentIndexChangedStatisticsType (int index)
{
    makeCurrent ();
    GetViewSettings ().SetStatisticsType (StatisticsType::Enum(index));
    update ();
}

void WidgetGl::CurrentIndexChangedAxesOrder (int index)
{
    makeCurrent ();
    GetViewSettings ().SetAxesOrder (AxesOrder::Enum(index));
}

// @todo add a color bar model for BodyScalar::None
void WidgetGl::SetBodyOrFaceScalar (
    ViewNumber::Enum viewNumber,
    boost::shared_ptr<ColorBarModel> colorBarModel,
    size_t bodyOrFaceScalar)
{
    makeCurrent ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    vs.SetBodyOrFaceScalar (bodyOrFaceScalar);
    if (vs.GetBodyOrFaceScalar () != FaceScalar::DMP_COLOR)
    {
	vs.SetColorBarModel (colorBarModel);
	setTexture (colorBarModel, m_colorBarTexture[viewNumber]);
    }
    else
	vs.ResetColorBarModel ();
    CompileUpdate ();
}

void WidgetGl::setTexture (
    boost::shared_ptr<ColorBarModel> colorBarModel, GLuint texture)
{
    if (colorBarModel)
    {
	const QImage image = colorBarModel->GetImage ();
	glBindTexture (GL_TEXTURE_1D, texture);
	glTexImage1D (GL_TEXTURE_1D, 0, GL_RGB, image.width (),
		      0, GL_BGRA, GL_UNSIGNED_BYTE, image.scanLine (0));
    }
}


void WidgetGl::SetColorBarModel (ViewNumber::Enum viewNumber, 
				 boost::shared_ptr<ColorBarModel> colorBarModel)
{
    makeCurrent ();
    GetViewSettings (viewNumber).SetColorBarModel (colorBarModel);
    setTexture (colorBarModel, m_colorBarTexture[viewNumber]);
    update ();
}

void WidgetGl::SetOverlayBarModel (
    ViewNumber::Enum viewNumber, 
    boost::shared_ptr<ColorBarModel> overlayBarModel)
{
    makeCurrent ();
    ViewSettings& vs = GetViewSettings (viewNumber);
    vs.SetOverlayBarModel (overlayBarModel);
    setTexture (overlayBarModel, m_overlayBarTexture[viewNumber]);
    update ();
}

void WidgetGl::ValueChangedNoiseStart (int index)
{
    makeCurrent ();
    for (size_t i = 0; i < ViewCount::GetCount (m_settings->GetViewCount ()); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	float noiseStart = 0.5 + 0.5 * index / 99;
	__LOG__ (cdbg << "index=" << index 
		 << " noiseStart=" << noiseStart << endl;)
	GetViewAverage (viewNumber).GetVelocityAverage ().
	    SetNoiseStart (noiseStart);
	GetViewAverage (viewNumber).GetDeformationAverage ().
	    SetNoiseStart (noiseStart);
    }
    update ();
}

void WidgetGl::ValueChangedNoiseAmplitude (int index)
{
    makeCurrent ();
    for (size_t i = 0; i < ViewCount::GetCount (m_settings->GetViewCount ()); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	float noiseAmplitude = 5.0 + index / 10.0;
	__LOG__ (cdbg << "index=" << index 
		 << " noiseAmplitude=" << noiseAmplitude << endl;)
	GetViewAverage (viewNumber).GetVelocityAverage ().
	    SetNoiseAmplitude (noiseAmplitude);
	GetViewAverage (viewNumber).GetDeformationAverage ().
	    SetNoiseAmplitude (noiseAmplitude);
    }
    update ();
}

void WidgetGl::ValueChangedNoiseFrequency (int index)
{
    makeCurrent ();
    for (size_t i = 0; i < ViewCount::GetCount (m_settings->GetViewCount ()); ++i)
    {
	ViewNumber::Enum viewNumber = ViewNumber::Enum (i);
	float noiseFrequency = (1.0 + index) / 2.0;
	__LOG__ (cdbg << "index=" << index 
		 << " noiseFrequency=" << noiseFrequency << endl;)
	GetViewAverage (viewNumber).GetVelocityAverage ().
	    SetNoiseFrequency (noiseFrequency);
	GetViewAverage (viewNumber).GetDeformationAverage ().
	    SetNoiseFrequency (noiseFrequency);
    }
    update ();
}


void WidgetGl::ClickedEnd ()
{
    makeCurrent ();
    size_t steps = 
	((m_settings->GetTimeLinkage () == TimeLinkage::INDEPENDENT) ?
	 GetTimeSteps () : LinkedTimeMaxSteps ().first);
    boost::array<int, ViewNumber::COUNT> direction;
    m_settings->SetCurrentTime (steps - 1, &direction, true);
    for (size_t i = 0; i < ViewNumber::COUNT; ++i)
	if (direction[i] != 0)
	    m_viewAverage[i]->AverageStep (direction[i]);
    update ();
}

void WidgetGl::ValueChangedAverageTimeWindow (int timeSteps)
{
    makeCurrent ();
    GetViewAverage ().AverageSetTimeWindow (timeSteps);
}

void WidgetGl::ValueChangedT1sTimeWindow (int timeSteps)
{
    makeCurrent ();
    GetViewAverage ().GetT1sPDE ().AverageSetTimeWindow (timeSteps);
}

void WidgetGl::ValueChangedTimeDisplacement (int timeDisplacement)
{
    makeCurrent ();
    QSlider* slider = static_cast<QSlider*> (sender ());
    size_t maximum = slider->maximum ();
    G3D::AABox bb = GetSimulation ().GetBoundingBox ();
    m_timeDisplacement =
	(bb.high () - bb.low ()).z * timeDisplacement /
	GetSimulation ().GetTimeSteps () / maximum;
    CompileUpdate ();
}

void WidgetGl::ValueChangedT1Size (int index)
{
    makeCurrent ();
    (void)index;
    m_t1sSize = Index2Value (static_cast<QSlider*> (sender ()), T1S_SIZE);
    update ();
}


void WidgetGl::ValueChangedT1sKernelIntervalPerPixel (int index)
{
    makeCurrent ();
    (void)index;
    SetOneOrTwoViews (this,
		      &WidgetGl::valueChangedT1sKernelIntervalPerPixel);
}

void WidgetGl::ValueChangedT1sKernelSigma (int index)
{
    makeCurrent ();
    (void)index;
    SetOneOrTwoViews (this, &WidgetGl::valueChangedT1sKernelSigma);
}

void WidgetGl::ValueChangedT1sKernelTextureSize (int index)
{
    makeCurrent ();
    (void)index;
    SetOneOrTwoViews (this, &WidgetGl::valueChangedT1sKernelTextureSize);
}

void WidgetGl::ToggledT1sKernelTextureSizeShown (bool checked)
{
    makeCurrent ();
    (void)checked;
    SetOneOrTwoViews (this, &WidgetGl::toggledT1sKernelTextureSizeShown);
}

void WidgetGl::ValueChangedDeformationSizeExp (int index)
{
    makeCurrent ();
    (void)index;
    ViewSettings& vs = GetViewSettings ();
    vs.SetDeformationSize (
	IndexExponent2Value (
	    static_cast<QSlider*> (sender ()), TENSOR_SIZE_EXP2));
    update ();
}

void WidgetGl::ValueChangedDeformationLineWidthExp (int index)
{
    makeCurrent ();
    (void)index;
    ViewSettings& vs = GetViewSettings ();
    vs.SetDeformationLineWidth (
	IndexExponent2Value (
	    static_cast<QSlider*> (sender ()), TENSOR_LINE_WIDTH_EXP2));
    update ();
}

void WidgetGl::ValueChangedForceTorqueSize (int index)
{
    makeCurrent ();
    (void)index;
    ViewSettings& vs = GetViewSettings ();
    vs.SetForceTorqueSize (
	IndexExponent2Value (
	    static_cast<QSlider*> (sender ()), FORCE_SIZE_EXP2));
    update ();
}

void WidgetGl::ValueChangedTorqueDistance (int index)
{
    makeCurrent ();
    (void)index;
    ViewSettings& vs = GetViewSettings ();
    vs.SetTorqueDistance (
	IndexExponent2Value (
	    static_cast<QSlider*> (sender ()), FORCE_SIZE_EXP2));
    update ();
}



void WidgetGl::ValueChangedForceTorqueLineWidth (int index)
{
    makeCurrent ();
    (void)index;
    ViewSettings& vs = GetViewSettings ();
    vs.SetForceTorqueLineWidth (
	IndexExponent2Value (static_cast<QSlider*> (sender ()),
			     TENSOR_LINE_WIDTH_EXP2));
    update ();
}


void WidgetGl::ValueChangedVelocityLineWidthExp (int index)
{
    makeCurrent ();
    (void)index;
    ViewSettings& vs = GetViewSettings ();
    vs.SetVelocityLineWidth (
	IndexExponent2Value (static_cast<QSlider*> (sender ()),
			     TENSOR_LINE_WIDTH_EXP2));
    update ();
}


void WidgetGl::ValueChangedHighlightLineWidth (int newWidth)
{
    m_highlightLineWidth = newWidth;
    update ();
}

void WidgetGl::ValueChangedEdgesRadius (int sliderValue)
{
    makeCurrent ();
    size_t maximum = static_cast<QSlider*> (sender ())->maximum ();
    m_settings->SetEdgeRadiusRatio (static_cast<double>(sliderValue) / maximum);
    m_settings->SetEdgeArrow (GetOnePixelInObjectSpace ());
    CompileUpdate ();
}


void WidgetGl::ValueChangedLightAmbientRed (int sliderValue)
{
    makeCurrent ();
    setLight (sliderValue, static_cast<QSlider*> (sender ())->maximum (),
	      LightType::AMBIENT, ColorNumber::RED);
}


void WidgetGl::ValueChangedLightAmbientGreen (int sliderValue)
{
    makeCurrent ();
    setLight (sliderValue, static_cast<QSlider*> (sender ())->maximum (),
	      LightType::AMBIENT, ColorNumber::GREEN);
}

void WidgetGl::ValueChangedLightAmbientBlue (int sliderValue)
{
    makeCurrent ();
    setLight (sliderValue, static_cast<QSlider*> (sender ())->maximum (),
	      LightType::AMBIENT, ColorNumber::BLUE);
}

void WidgetGl::ValueChangedLightDiffuseRed (int sliderValue)
{
    makeCurrent ();
    setLight (sliderValue, static_cast<QSlider*> (sender ())->maximum (),
	      LightType::DIFFUSE, ColorNumber::RED);
}

void WidgetGl::ValueChangedLightDiffuseGreen (int sliderValue)
{
    makeCurrent ();
    setLight (sliderValue, static_cast<QSlider*> (sender ())->maximum (),
	      LightType::DIFFUSE, ColorNumber::GREEN);
}

void WidgetGl::ValueChangedLightDiffuseBlue (int sliderValue)
{
    makeCurrent ();
    setLight (sliderValue, static_cast<QSlider*> (sender ())->maximum (),
	      LightType::DIFFUSE, ColorNumber::BLUE);
}

void WidgetGl::ValueChangedLightSpecularRed (int sliderValue)
{
    makeCurrent ();
    setLight (sliderValue, static_cast<QSlider*> (sender ())->maximum (),
	      LightType::SPECULAR, ColorNumber::RED);
}

void WidgetGl::ValueChangedLightSpecularGreen (int sliderValue)
{
    makeCurrent ();
    setLight (sliderValue, static_cast<QSlider*> (sender ())->maximum (),
	      LightType::SPECULAR, ColorNumber::GREEN);
}

void WidgetGl::ValueChangedLightSpecularBlue (int sliderValue)
{
    makeCurrent ();
    setLight (sliderValue, static_cast<QSlider*> (sender ())->maximum (),
	      LightType::SPECULAR, ColorNumber::BLUE);
}

void WidgetGl::ValueChangedAngleOfView (int angleOfView)
{
    makeCurrent ();
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = GetViewSettings ();
    vs.SetAngleOfView (angleOfView);
    vs.CalculateCameraDistance (calculateCenteredViewingVolume (viewNumber));
    update ();
}

ViewNumber::Enum WidgetGl::GetViewNumber () const
{
    return m_settings->GetViewNumber ();
}

ViewSettings& WidgetGl::GetViewSettings (ViewNumber::Enum viewNumber) const
{
    return m_settings->GetViewSettings (viewNumber);
}


// Template instantiations
// ======================================================================
template
void WidgetGl::SetOneOrTwoViews<MainWindow> (
    MainWindow* t, void (MainWindow::*f) (ViewNumber::Enum));
