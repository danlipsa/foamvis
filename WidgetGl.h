/**
 * @file   WidgetGl.h
 * @author Dan R. Lipsa
 *
 * Widget for displaying foam bubbles using OpenGL
 */
#ifndef __WIDGETGL_H__
#define __WIDGETGL_H__

#include "Hashes.h"
#include "Enums.h"
#include "WidgetBase.h"

class Body;
class BodyAlongTime;
class BodiesAlongTime;
class ColorBarModel;
class Foam;
class Edge;
class EditColorMap;
class Simulation;
class SimulationGroup;
class AverageCache;
class ForcesOneObject;
class OrientedFace;
class OrientedEdge;
class OOBox;
class SelectBodiesById;
class Settings;
class ViewSettings;
class ViewAverage;
class PropertyValueBodySelector;
class IdBodySelector;
class AllBodySelector;

/**
 * Widget for displaying foam bubbles using OpenGL
 */
class WidgetGl : public QGLWidget, public QGLFunctions, public WidgetBase
{
public:
    enum RotateForAxisOrder
    {
        ROTATE_FOR_AXIS_ORDER,
        DONT_ROTATE_FOR_AXIS_ORDER
    };

public:
    /**
     * Constructor
     * @param parent parent widget
     */
    WidgetGl(QWidget *parent = 0);
    /**
     * Destructor
     */
    ~WidgetGl();
    /**
     * Gets the minimum size of the WidgetGl
     */
    QSize minimumSizeHint() ;
    /**
     * Gets the prefered size
     */
    QSize sizeHint() ;
    void Init (boost::shared_ptr<Settings> settings,
	       SimulationGroup* dataAlongTime, AverageCaches* averageCache);

    size_t GetHighlightLineWidth () const
    {
	return m_highlightLineWidth;
    }

    size_t GetTimeSteps () const
    {
	return GetTimeSteps (GetViewNumber ());
    }
    size_t GetTimeSteps (ViewNumber::Enum viewNumber) const;

    bool IsMissingPropertyShown (BodyScalar::Enum bodyProperty) const;    

    GLUquadricObj* GetQuadricObject () const 
    {
	return m_quadric;
    }

    bool EdgesShown () const
    {
	return m_edgesShown;
    }

    bool IsBubblePathsBodyShown () const
    {
	return m_bubblePathsBodyShown;
    }


    boost::shared_ptr<QAction> GetActionResetTransformAll () const
    {
	return m_actionResetTransformAll;
    }
    boost::shared_ptr<QAction> GetActionEditColorMap () const
    {
	return m_actionEditColorMap;
    }
    boost::shared_ptr<QAction> GetActionEditOverlayMap () const
    {
	return m_actionEditOverlayMap;
    }

    /**
     * Setup the viewing volume first centered around origin and then 
     * translated toward negative Z with m_cameraDistance.
     */
    void ProjectionTransform (
	ViewNumber::Enum viewNumber, ViewingVolumeOperation::Enum enclose =
        ViewingVolumeOperation::DONT_ENCLOSE2D) const;
    size_t GetBodyOrFaceScalar () const
    {
	return GetBodyOrFaceScalar (GetViewNumber ());
    }

    size_t GetBodyOrFaceScalar (ViewNumber::Enum viewNumber) const;

    void SetStatus (QLabel* labelStatusBar)
    {
	m_labelStatusBar = labelStatusBar;
    }
    /**
     * Draw a quad over destRect. If angleDegrees != 0, rotate the quad first 
     * and then draw it.
     */
    void ActivateViewShader (ViewNumber::Enum viewNumber) const;
    void ActivateViewShader (
	ViewNumber::Enum viewNumber,
	ViewingVolumeOperation::Enum enclose,
	G3D::Vector2 rotationCenter, 
	float angleDegrees) const;
    void ActivateViewShader (ViewNumber::Enum viewNumber,
			     ViewingVolumeOperation::Enum enclose,
			     G3D::Rect2D& srcRect) const;
    float GetBubbleDiameter (ViewNumber::Enum viewNumber) const;
    float GetBubbleDiameter () const
    {
        return GetBubbleDiameter (GetViewNumber ());
    }
    float GetDeformationSizeInitialRatio (ViewNumber::Enum viewNumber) const;
    float GetVelocitySizeInitialRatio (ViewNumber::Enum viewNumber) const;
    void DisplayT1Quad (ViewNumber::Enum view, 
			size_t timeStep, size_t t1Index) const;
    pair<float, float> GetRange (ViewNumber::Enum viewNumber) const;
    pair<float, float> GetRange () const
    {
        return GetRange (GetViewNumber ());
    }
    pair<float, float> GetVelocityMagnitudeRange (
	ViewNumber::Enum viewNumber) const;
    pair<float, float> GetRangeCount (ViewNumber::Enum viewNumber) const;
    pair<float, float> GetRangeCount () const;
    pair<float, float> GetRangeT1sKDE (ViewNumber::Enum viewNumber) const;
    pair<float, float> GetRangeT1sKDE () const
    {
	return GetRangeT1sKDE (GetViewNumber ());
    }
    void SetForceDifferenceShown (bool forceDifference);
    ViewAverage& GetViewAverage (ViewNumber::Enum viewNumber) const
    {
	return *m_viewAverage[viewNumber];
    }
    ViewAverage& GetViewAverage () const
    {
	return *m_viewAverage[GetViewNumber ()];
    }
    void CompileUpdate ()
    {
	CompileUpdate (GetViewNumber ());
    }
    void CompileUpdate (ViewNumber::Enum viewNumber);
    void CompileUpdateAll ();
    void UpdateAverage (ViewNumber::Enum viewNumber, int direction);
    GLuint GetColorBarTexture (ViewNumber::Enum viewNumber) const
    {
	return m_colorBarTexture[viewNumber];
    }
    GLuint GetOverlayBarTexture (ViewNumber::Enum viewNumber) const
    {
	return m_overlayBarTexture[viewNumber];
    }
    G3D::AABox CalculateCenteredViewingVolume (
	ViewNumber::Enum viewNumber) const;
    void ButtonClickedViewType (ViewType::Enum oldViewType);
    void SetViewTypeAndCameraDistance (ViewNumber::Enum viewNumber);
    void CalculateStreamline (ViewNumber::Enum viewNumber);
    void CacheUpdateSeedsCalculateStreamline (ViewNumber::Enum viewNumber);
    void CacheCalculateStreamline (ViewNumber::Enum viewNumber);
    void AllTransformAverage (
        ViewNumber::Enum viewNumber, size_t timeStep,
        RotateForAxisOrder rotateForAxisOrder) const;
    void AllTransformAverage (
        ViewNumber::Enum viewNumber) const
    {
        return AllTransformAverage (
            viewNumber, GetCurrentTime (viewNumber), ROTATE_FOR_AXIS_ORDER);
    }
    void GetGridParams (
        ViewNumber::Enum viewNumber, G3D::Vector2* gridOrigin, 
        float* gridCellLength, float* angleDegrees) const;
    float SliderToTimeDisplacement (
        const QSlider& slider,
        const Simulation& simulation) const;
    int TimeDisplacementToSlider (
        float timeDisplacement,
        const QSlider& slider,
        const Simulation& simulation) const;
    void CurrentIndexChangedSimulation (int index);
    
Q_SIGNALS:
    void PaintEnd ();
    void ColorBarModelChanged (
	ViewNumber::Enum viewNumber,
	boost::shared_ptr<ColorBarModel> colorBarModel);
    void OverlayBarModelChanged (
	ViewNumber::Enum viewNumber,
	boost::shared_ptr<ColorBarModel> colorBarModel);

public Q_SLOTS:
    void ToggledVelocityFieldSaved (bool saved);
    void ToggledBarLabelsShown (bool shown);
    void ToggledKDESeeds (bool toggled);
    void ToggledSeedsShown (bool shown);
    void ToggledStandaloneElementsShown (bool checked);
    void ToggledAxesShown (bool checked);
    void ToggledBoundingBoxSimulation (bool checked);
    void ToggledBoundingBoxFoam (bool checked);
    void ToggledBoundingBoxBody (bool checked);
    void ToggledAverageAroundMarked (bool checked);
    void ToggledConstraintsShown (bool checked);
    void ToggledConstraintPointsShown (bool checked);
    void ToggledBubblePathsBodyShown (bool checked);
    void ToggledBubblePathsLineUsed (bool checked);
    void ToggledBubblePathsTubeUsed (bool checked);
    void ToggledContextView (bool checked);
    void ToggledForceNetworkShown (bool checked);
    void ToggledForcePressureShown (bool checked);
    void ToggledForceResultShown (bool checked);
    void ToggledTorqueNetworkShown (bool checked);
    void ToggledTorquePressureShown (bool checked);
    void ToggledTorqueResultShown (bool checked);
    void ToggledDirectionalLightEnabled (bool checked);
    void ToggledBodyCenterShown (bool checked);
    void ToggledBodyNeighborsShown (bool checked);
    void ToggledFaceCenterShown (bool checked);
    void ToggledEdgesTessellationShown (bool checked);
    void ToggledEdgesShown (bool checked);
    void ToggledLightNumberShown (bool checked);
    void ToggledLightEnabled (bool checked);
    void ToggledSelectionContextShown (bool checked);
    void ToggledPartialPathHidden (bool checked);
    void ToggledTorusDomainClipped (bool checked);
    void ToggledTorusDomainShown (bool checked);
    void ToggledT1sShown (bool checked);
    void ToggledT1sAllTimesteps (bool checked);
    void ToggledT1sShiftLower (bool checked);
    void ToggledMissingPressureShown (bool checked);
    void ToggledMissingVolumeShown (bool checked);
    void ToggledObjectVelocityShown (bool checked);
    void ToggledDeformationShown (bool checked);
    void ToggledDeformationShownGrid (bool checked);
    void ToggledVelocityShown (bool checked);
    void ToggledVelocityGridShown (bool checked);
    void ToggledVelocityClampingShown (bool checked);
    void ToggledDeformationGridCellCenterShown (bool checked);
    void ToggledVelocityGridCellCenterShown (bool checked);
    void ToggledVelocitySameSize (bool checked);
    void ToggledVelocityColorMapped (bool checked);
    void ToggledContextBoxShown (bool checked);
    void ToggledT1sKernelTextureShown (bool checked);
    void ToggledAverageAroundAllowRotation (bool checked);
    void ButtonClickedTimeLinkage (int id);
    void ButtonClickedInteractionObject (int id);
    void ButtonClickedDuplicateDomain (int id);
    void ClickedEnd ();
    void SetBodyOrFaceScalar (
	ViewNumber::Enum viewNumber,
	boost::shared_ptr<ColorBarModel> colorBarModel,
	size_t property);
    void SetColorBarModel (ViewNumber::Enum viewNumber, 
			   boost::shared_ptr<ColorBarModel> colorBarModel);
    void SetOverlayBarModel (ViewNumber::Enum viewNumber, 
			     boost::shared_ptr<ColorBarModel> colorBarModel);
    void CurrentIndexChangedStatisticsType (int index);
    void CurrentIndexChangedAxesOrder (int index);
    void CurrentIndexChangedSelectedLight (int selectedLight);


    void ValueChangedKDEMultiplier (int multiplier);
    void ValueChangedKDEValue (double value);
    void ValueChangedBubblePathsTimeBegin (int time);
    void ValueChangedBubblePathsTimeEnd (int time);
    void ValueChangedStreamlineStepLength (double steps);
    void ValueChangedStreamlineLength (double value);
    void ValueChangedNoiseStart (int i);
    void ValueChangedNoiseAmplitude (int i);
    void ValueChangedNoiseFrequency (int i);

    void ValueChangedAngleOfView (int newIndex);
    void ValueChangedEdgesRadius (int sliderValue);

    void ValueChangedLightAmbientRed (int sliderValue);
    void ValueChangedLightAmbientGreen (int sliderValue);
    void ValueChangedLightAmbientBlue (int sliderValue);

    void ValueChangedLightDiffuseRed (int sliderValue);
    void ValueChangedLightDiffuseGreen (int sliderValue);
    void ValueChangedLightDiffuseBlue (int sliderValue);

    void ValueChangedLightSpecularRed (int sliderValue);
    void ValueChangedLightSpecularGreen (int sliderValue);
    void ValueChangedLightSpecularBlue (int sliderValue);

    void ValueChangedT1sTimeWindow (int timeSteps);
    void ValueChangedTimeDisplacement (int timeDisplacement);
    void ValueChangedT1Size (int index);
    void ValueChangedT1sKernelSigma (double value);
    void ValueChangedDeformationSizeExp (int index);
    void ValueChangedDeformationLineWidthExp (int index);
    void ValueChangedVelocityLineWidthExp (int index);
    void ValueChangedForceTorqueSize (int index);
    void ValueChangedTorqueDistance (int index);
    void ValueChangedForceTorqueLineWidth (int index);
    void ValueChangedHighlightLineWidth (int newWidth);
    // Actions
    void ResetTransformAll ();
    void ResetTransformFocus ();
    void ResetTransformContext ();
    void ResetTransformLight ();    
    void ResetTransformGrid ();
    void RotationCenterBody ();
    void RotationCenterFoam ();
    void AddLinkedTimeEvent ();
    void SelectAll ();
    void DeselectAll ();
    void SelectBodiesByIdList ();
    void SelectThisBodyOnly ();
    void SetAverageAroundBody ();
    void SetAverageAroundSecondBody ();
    void AverageAroundReset ();
    void ContextDisplayBody ();
    void ContextDisplayReset ();
    void InfoPoint ();
    void InfoEdge ();
    void InfoFace ();
    void InfoBody ();
    void InfoFoam ();
    void InfoOpenGL ();
    void InfoSelectedBodies ();
    void ShowNeighbors ();
    void ShowDeformation ();
    void ShowVelocity ();
    void ShowReset ();
    // Actions color bar
    void ColorBarClampClear ();
    void OverlayBarClampClear ();
    void CopyTransformationFrom (int viewNumber);
    void CopySelectionFrom (int viewNumber);
    void CopyPaletteClamping (int viewNumber);
    void OverlayBarCopyVelocityMagnitude ();

public:
    const static  size_t DISPLAY_ALL;
    const static pair<float,float> TENSOR_SIZE_EXP2;
    const static pair<float,float> TENSOR_LINE_WIDTH_EXP2;
    const static pair<float,float> FORCE_SIZE_EXP2;
    const static pair<float,float> TORQUE_SIZE_EXP2;

protected:
    /**
     * Initializes OpenGL
     */
    void initializeGL();
    /**
     * Called when the WidgetGl needs repainted
     */
    void paintGL();
    /**
     * Called when the WidgetGl is resized
     */
    void resizeGL(int width, int height);
    /**
     * Called  when the  mouse  is  pressed on  the  widget. Used  for
     * rotating the foam.
     * @param event specifies where did the click happened
     */
    void mousePressEvent(QMouseEvent *event);
    /**
     * Called  when the  mouse  is  moved over  the  widget. Used  for
     * rotating the foam.
     * @param event specifies how much did the mouse move
     */
    void mouseMoveEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);


private:

    enum Lighting
    {
	NO_LIGHTING,
	LIGHTING,
	LIGHTING_COUNT
    };

    enum Rotation
    {
	NO_ROTATION,
	ROTATION,
	ROTATION_COUNT
    };

    enum ShowType
    {
	SHOW_NOTHING,
	SHOW_NEIGHBORS,
	SHOW_DEFORMATION_TENSOR,
	SHOW_VELOCITY
    };

    typedef void (WidgetGl::*ViewTypeDisplay) (ViewNumber::Enum view) const;
    typedef bool (WidgetGl::*IsCopyCompatibleType) (
        ViewNumber::Enum vn, ViewNumber::Enum otherVn) const;

private:
    void saveVelocity (ViewNumber::Enum viewNumber,
                       vtkSmartPointer<vtkImageData> velocity) const;
    float timeDisplacementMultiplier (
        const QSlider& slider,
        const Simulation& simulation) const;
    void addCopyCompatibleMenu (
        QMenu* menuCopy, const char* nameOp, 
        const boost::shared_ptr<QAction>* actionCopyOp,
        IsCopyCompatibleType isCopyCompatible) const;
    bool isColorBarCopyCompatible (
        ViewNumber::Enum vn, ViewNumber::Enum otherVn) const;
    bool isSelectionCopyCompatible (
        ViewNumber::Enum vn, ViewNumber::Enum otherVn) const;

    void rotateAverageAroundStreamlines (ViewNumber::Enum viewNumber,
                                         bool isIsAverageAroundShown) const;

    void printVelocitiesDebug (ViewNumber::Enum viewNumber) const;
    void modelViewTransform (ViewNumber::Enum viewNumber, 
			     size_t timeStep,
                             RotateForAxisOrder rotateForAxisOrder) const;
    void setTorusDomainClipPlanes ();
    void enableTorusDomainClipPlanes (bool enable);
    void setSimulation (int i, ViewNumber::Enum viewNumber);
    void initTransformViewport ();
    void cleanupTransformViewport ();
    void toggledT1sKernelTextureShown (ViewNumber::Enum viewNumber);
    void valueChangedT1sKernelSigma (ViewNumber::Enum viewNumber);
    bool linkedTimesValid (size_t timeBegin, size_t timeEnd);
    bool linkedTimesValid ();
    void contextMenuEventColorBar (QMenu* menu) const;
    void contextMenuEventOverlayBar (QMenu* menu) const;
    void contextMenuEventView (QMenu* menu) const;
    void activateViewShader (
	ViewNumber::Enum viewNumber, 
	ViewingVolumeOperation::Enum enclose, G3D::Rect2D& srcRect,
	G3D::Vector2 rotationCenter, float angleDegrees) const;
    G3D::Vector3 getEyeTransform (ViewNumber::Enum viewNumber) const;
    G3D::AABox calculateEyeViewingVolume (
	ViewNumber::Enum viewNumber, 
	ViewingVolumeOperation::Enum enclose = 
	ViewingVolumeOperation::DONT_ENCLOSE2D) const;
    void mouseMoveRotate (QMouseEvent *event, ViewNumber::Enum viewNumber);
    void mouseMoveTranslate (QMouseEvent *event, ViewNumber::Enum viewNumber);
    void mouseMoveScale (QMouseEvent *event, ViewNumber::Enum viewNumber);
    void allTransform (ViewNumber::Enum viewNumber) const;
    void viewportTransform (ViewNumber::Enum viewNumber) const;
    void setLight (int sliderValue, int maximumValue, 
		   LightType::Enum lightType, ColorNumber::Enum colorNumber);
    void averageInitStep (ViewNumber::Enum viewNumber);
    void selectView (const G3D::Vector2& clickedPoint);
    void displayTwoHalvesLine (ViewNumber::Enum viewNumber) const;
    void displayContextMenuPos (ViewNumber::Enum viewNumber) const;
    void displayBodyCenters (ViewNumber::Enum viewNumber, 
			     bool useZPos = false) const;
    void displayRotationCenter (ViewNumber::Enum viewNumber) const;
    void displayFaceCenters (ViewNumber::Enum viewNumber) const;
    void displayVelocityStreamlines (ViewNumber::Enum viewNumber) const;
    void displayVelocityStreamline (
        ViewNumber::Enum viewNumber, vtkSmartPointer<vtkIdList> points) const;
    void displayVelocityStreamlineSeeds (ViewNumber::Enum viewNumber) const;
    void updateStreamlineSeeds (ViewNumber::Enum viewNumber);
    void updateStreamlineSeeds (ViewNumber::Enum viewNumber, 
                                vtkSmartPointer<vtkPoints> points,
                                vtkSmartPointer<vtkCellArray> vertices,
                                const G3D::Rect2D& r, 
                                G3D::Vector2 gridOrigin, float gridCellLength,
                                float angleDegrees, bool useKDESeeds);
    void updateKDESeeds (
        ViewNumber::Enum viewNumber,
        vtkSmartPointer<vtkPoints> points, 
        vtkSmartPointer<vtkCellArray> vertices, G3D::Vector2 cellCenter,
        float cellLength);
    void displayViewDecorations (ViewNumber::Enum viewNumber);
    void displayViewFocus (ViewNumber::Enum viewNumber);
    void displayTextureColorBar (GLuint texture,
				 ViewNumber::Enum viewNumber, 
				 const G3D::Rect2D& barRect);
    void displayBarLabels (
        const ColorBarModel& cbm, const G3D::Rect2D& colorBarRect);
    float getBarLabelsWidth (const ColorBarModel& cbm) const;
    void displayOverlayBar (
	ViewNumber::Enum viewNumber, const G3D::Rect2D& barRect);
    void displayBarClampLevels (const ColorBarModel& barModel,
                                const G3D::Rect2D& barRect) const;
    void displayViewTitle (ViewNumber::Enum viewNumber);
    void displayViewText (
	ViewNumber::Enum viewNumber, const string& text, size_t row);

    /**
     * First translate the data to be centered around origin, then
     * rotate and then translate toward negative Z with
     * m_cameraDistance
     */
    void displayLightDirection (
	ViewNumber::Enum viewNumber, LightNumber::Enum light) const;
    void displayLightDirection (ViewNumber::Enum viewNumber) const;
    G3D::Vector3 calculateViewingVolumeScaledExtent (
	ViewNumber::Enum viewNumber) const;
    void initQuadrics ();
    void initStreamlines ();
    void calculateCameraDistance (ViewNumber::Enum viewNumber);
    /**
     * Generates a display list for edges
     * @return the display list
     */
    template<typename displayEdge>
    void displayEdges (ViewNumber::Enum viewNumber) const;
    void displayView (ViewNumber::Enum view);
    void displayAllViewTransforms (ViewNumber::Enum viewNumber);
    void displayViews ();
    void displayContextBodies (ViewNumber::Enum view) const;
    void displayContextBox (
	ViewNumber::Enum view,
	bool isAverageAroundRotationShown = false) const;
    void displayAverageAroundBodies (
	ViewNumber::Enum view, 
	bool isAverageAroundRotationShown = false) const;
    void displayAverageAroundBodyOne (ViewNumber::Enum viewNumber) const;
    void displayAverageAroundBodyTwo (ViewNumber::Enum viewNumber) const;

    void displayEdgesNormal (ViewNumber::Enum view) const;
    template<typename displayEdge>
    void displayStandaloneEdges (
	const Foam& foam, ViewNumber::Enum viewNumber = ViewNumber::VIEW0, 
	bool useZPos = false, double zPos = 0) const;
    void displayStandaloneFaces () const;
    void displayEdgesTorus (ViewNumber::Enum view) const;
    void displayEdgesTorusTubes () const;
    void displayEdgesTorusLines () const;

    void displayFacesNormal (ViewNumber::Enum view) const;
    void compileFacesNormal (ViewNumber::Enum view) const;
    void displayFacesTorus (ViewNumber::Enum view) const;
    void displayFacesAverage (ViewNumber::Enum view) const;
    void displayFacesTorusTubes () const;
    void displayFacesTorusLines () const;


    void calculateRotationParams (
        ViewNumber::Enum viewNumber, size_t timeStep, 
        G3D::Vector3* rotationCenter, float* angleDegrees) const;
    void displayBubblePathsWithBodies (ViewNumber::Enum view) const;
    void displayBubblePathsBody (ViewNumber::Enum viewNumber) const;
    void displayTorusDomain (ViewNumber::Enum viewNumber) const;
    void displayBodyNeighbors (ViewNumber::Enum viewNumber) const;
    void displayBodiesNeighbors () const;
    void displayBodyDeformation (ViewNumber::Enum viewNumber) const;
    void displayBodyVelocity (ViewNumber::Enum viewNumber) const;
    void displayDeformation (ViewNumber::Enum viewNumber) const;
    void displayVelocityGlyphs (ViewNumber::Enum viewNumber) const;
    void displayT1sAllTimesteps (ViewNumber::Enum view) const;
    void displayT1s (ViewNumber::Enum view) const;
    void displayT1sTimestep (ViewNumber::Enum view, size_t timeStep) const;    
    void displayBubblePaths (ViewNumber::Enum view) const;
    void compileBubblePaths (ViewNumber::Enum view) const;

    void displayBoundingBox (ViewNumber::Enum viewNumber) const;
    void displayFocusBox (ViewNumber::Enum viewNumber) const;
    void displayAxes (ViewNumber::Enum viewNumber);

    /**
     * Returns a rotation around an axis with a certain angle
     * @param axis can be 0, 1 or 2 for X, Y or Z
     * @param angle angle we rotate the foam with
     */
    static G3D::Matrix3 getRotationAround (int axis, double angle);
    /**
     * Displays   the   contour   of   faces.   Used   together   with
     * displayFacesOffet   and  with  GL_POLYGON_OFFSET_FILL   to  get
     * continuous contours  for polygons.
     * @param bodies displays all the faces in these bodies
     */
    void displayFacesContour (
	const vector< boost::shared_ptr<Body> >& bodies, 
	ViewNumber::Enum viewNumber, GLfloat lineWidth = 1) const;
    void displayFacesContour (
	const vector< boost::shared_ptr<Face> >& faces) const;
    /**
     * Displays   the   content   of   faces.   Used   together   with
     * displayFacesContour  and  with  GL_POLYGON_OFFSET_FILL  to  get
     * continuous contours for polygons.
     * @param bodies displays all the faces in these bodies
     */
    void displayFacesInterior (
	const vector<boost::shared_ptr<Body> >& bodies, 
	ViewNumber::Enum view) const;
    void displayFacesInterior (
	const vector<boost::shared_ptr<Face> >& faces) const;

    /**
     * Setup lighting for shaded bodies
     */
    void initializeLighting ();
    float ratioFromScaleCenter (ViewNumber::Enum viewNumber, const QPoint& p);
    G3D::Matrix3 rotate (
	ViewNumber::Enum viewNumber, 
	const QPoint& position, Qt::KeyboardModifiers modifiers,
	const G3D::Matrix3& rotate);
    void translate (ViewNumber::Enum viewNumber,
		    const QPoint& position,
		    Qt::KeyboardModifiers modifiers);
    void translateGrid (ViewNumber::Enum viewNumber, const QPoint& position);
    G3D::Vector3 calculateTranslationRatio (
	ViewNumber::Enum viewNumber, const QPoint& position,
	G3D::Vector3::Axis screenXTranslation,
	G3D::Vector3::Axis screenYTranslation) const;
    void translateLight (ViewNumber::Enum viewNumber, const QPoint& position);
    void scale (ViewNumber::Enum viewNumber, const QPoint& position);
    void scaleContext (ViewNumber::Enum viewNumber, const QPoint& position);
    void scaleGrid (ViewNumber::Enum viewNumber, const QPoint& position);
    void translateAndScale (ViewNumber::Enum viewNumber,
	double scaleRatio, const G3D::Vector3& translation,
	bool contextView) const;
    void select (const QPoint& position);
    void deselect (const QPoint& position);
    G3D::Vector3 brushedBodies (
	const QPoint& position, vector<size_t>* bodies, bool selected = true);
    G3D::Vector3 brushedBodies (
	const QPoint& position, 
	vector< boost::shared_ptr<Body> >* bodies, bool selected = true);
    OrientedEdge brushedEdge ();
    G3D::Vector3 brushedFace (const OrientedFace** of);
    void brushedFace (const QPoint& position, vector<size_t>* bodies) const;
    G3D::Vector3 toObjectTransform (const QPoint& position) const;
    G3D::Vector3 toObjectTransform (const QPoint& position, 
				    ViewNumber::Enum viewNumber) const;
    
    void initDisplayView ();
    void createActions ();
    string getInteractionLabel ();
    string getBodySelectorLabel ();
    string getContextLabel ();
    string getAverageAroundMovementShownLabel ();
    string getAverageAroundLabel ();
    void displayStatus ();
    string infoSelectedBody ();
    string infoSelectedBodies ();
    void initList ();
    void initList (boost::array<GLuint, ViewNumber::COUNT>* list);
    void initTexture ();
    void initTexture (boost::array<GLuint, ViewNumber::COUNT>* texture);

    /**
     * Setup lighting for displaying faces edges and vertices
     */
    static void quadricErrorCallback (GLenum errorCode);
    static void setTexture (
	boost::shared_ptr<ColorBarModel> colorBarModel, GLuint texture);

    
private:
    // Min, max values for T1s, Context alpha, force length
    const static pair<float,float> T1S_SIZE;
    const static pair<float,float> CELL_LENGTH_EXP2;
    const static GLfloat HIGHLIGHT_LINE_WIDTH;

private:
    Q_OBJECT

    /**
     * What do we display
     */
    bool m_torusDomainShown;
    InteractionObject::Enum m_interactionObject;
    /**
     * Used for rotation, translation and scale
     */
    QPoint m_lastPos;
    QPoint m_contextMenuPosWindow;
    G3D::Vector3 m_contextMenuPosObject;
    GLUquadricObj* m_quadric;

    bool m_edgesShown;
    bool m_bodyCenterShown;
    bool m_bodyNeighborsShown;
    bool m_faceCenterShown;
    bool m_bubblePathsBodyShown;
    bool m_boundingBoxSimulationShown;    
    bool m_boundingBoxFoamShown;
    bool m_boundingBoxBodyShown;
    bool m_axesShown;
    bool m_standaloneElementsShown;
    boost::array<ViewTypeDisplay, ViewType::COUNT> m_display;


    boost::shared_ptr<QAction> m_actionLinkedTimeEnd;

    boost::shared_ptr<QAction> m_actionSelectAll;
    boost::shared_ptr<QAction> m_actionDeselectAll;
    boost::shared_ptr<QAction> m_actionSelectBodiesByIdList;
    boost::shared_ptr<QAction> m_actionSelectThisBodyOnly;

    boost::shared_ptr<QAction> m_actionResetTransformAll;
    boost::shared_ptr<QAction> m_actionResetTransformFocus;
    boost::shared_ptr<QAction> m_actionResetTransformContext;
    boost::shared_ptr<QAction> m_actionResetTransformLight;
    boost::shared_ptr<QAction> m_actionResetTransformGrid;

    boost::shared_ptr<QAction> m_actionRotationCenterBody;
    boost::shared_ptr<QAction> m_actionRotationCenterFoam;

    boost::shared_ptr<QAction> m_actionAverageAroundBody;
    boost::shared_ptr<QAction> m_actionAverageAroundSecondBody;
    boost::shared_ptr<QAction> m_actionAverageAroundReset;
    boost::shared_ptr<QAction> m_actionAverageAroundShowRotation;

    boost::shared_ptr<QAction> m_actionContextDisplayBody;
    boost::shared_ptr<QAction> m_actionContextDisplayReset;

    boost::shared_ptr<QAction> m_actionInfoPoint;
    boost::shared_ptr<QAction> m_actionInfoEdge;
    boost::shared_ptr<QAction> m_actionInfoFace;
    boost::shared_ptr<QAction> m_actionInfoBody;
    boost::shared_ptr<QAction> m_actionInfoFoam;
    boost::shared_ptr<QAction> m_actionInfoOpenGL;
    boost::shared_ptr<QAction> m_actionInfoSelectedBodies;

    boost::shared_ptr<QAction> m_actionShowNeighbors;
    boost::shared_ptr<QAction> m_actionShowDeformation;
    boost::shared_ptr<QAction> m_actionShowVelocity;
    boost::shared_ptr<QAction> m_actionShowReset;

    boost::shared_ptr<QAction> m_actionEditColorMap;
    boost::shared_ptr<QAction> m_actionColorBarClampClear;
    boost::shared_ptr<QAction> m_actionEditOverlayMap;
    boost::shared_ptr<QAction> m_actionOverlayBarClampClear;
    boost::shared_ptr<QAction> m_actionOverlayBarCopyVelocityMagnitude;
    boost::shared_ptr<QAction> m_actionOverlayBarClampHighMinimum;
    boost::array<boost::shared_ptr<QAction>, 
		 ViewNumber::COUNT> m_actionCopySelection;
    boost::shared_ptr<QSignalMapper> m_signalMapperCopySelection;
    boost::array<boost::shared_ptr<QAction>, 
		 ViewNumber::COUNT> m_actionCopyPaletteClamping;
    boost::shared_ptr<QSignalMapper> m_signalMapperCopyPaletteClamping;
    boost::shared_ptr<SelectBodiesById> m_selectBodiesByIdList;
    QLabel *m_labelStatusBar;
    bool m_t1sShown;
    bool m_t1sAllTimesteps;
    double m_t1sSize;
    size_t m_highlightLineWidth;
    bool m_averageAroundMarked;
    bool m_contextBoxShown;
    bool m_barLabelsShown;
    ShowType m_showType;
    size_t m_showBodyId;
    boost::array<
	boost::shared_ptr<ViewAverage>, ViewNumber::COUNT> m_viewAverage;
    boost::array<GLuint, ViewNumber::COUNT> m_listBubblePaths;
    boost::array<GLuint, ViewNumber::COUNT> m_listFacesNormal;
    boost::array<GLuint, ViewNumber::COUNT> m_colorBarTexture;
    boost::array<GLuint, ViewNumber::COUNT> m_overlayBarTexture;
    boost::array<bool, DuplicateDomain::COUNT> m_duplicateDomain;
    boost::array<vtkSmartPointer<vtkPolyData>, 
                 ViewNumber::COUNT> m_streamline;
    boost::array<vtkSmartPointer<vtkPolyData>, 
                 ViewNumber::COUNT> m_streamlineSeeds;
    vtkSmartPointer<vtkRungeKutta4> m_rungeKutta;
    vtkSmartPointer<vtkStreamTracer> m_streamer;
};


#endif //__WIDGETGL_H__

// Local Variables:
// mode: c++
// End:
