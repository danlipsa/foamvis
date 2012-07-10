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

class Body;
class BodyAlongTime;
class BodiesAlongTime;
class ColorBarModel;
class Foam;
class Edge;
class EditColorMap;
class Simulation;
class SimulationGroup;
class ForcesOneObject;
class OrientedFace;
class OrientedEdge;
class OOBox;
class SelectBodiesById;
class ViewSettings;
class PropertyValueBodySelector;
class IdBodySelector;
class AllBodySelector;

/**
 * Widget for displaying foam bubbles using OpenGL
 */
class WidgetGl : public QGLWidget, public QGLFunctions
{
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
    /**
     * Sets the data displayed by the WidgetGl
     * @param dataAlongTime data displayed by the WidgetGl
     */
    void SetSimulationGroup (SimulationGroup* dataAlongTime);
    const SimulationGroup& GetSimulationGroup ()
    {
	return *m_simulationGroup;
    }
    /**
     * Gets the data displayed by the WidgetGl
     */
    const Simulation& GetSimulation (size_t index) const;
    Simulation& GetSimulation (size_t index);
    const Simulation& GetSimulation (ViewNumber::Enum viewNumber) const;
    Simulation& GetSimulation (ViewNumber::Enum viewNumber);
    const Simulation& GetSimulation () const;
    ViewNumber::Enum GetViewNumber () const
    {
	return m_viewNumber;
    }
    vector<ViewNumber::Enum> GetConnectedViewNumbers (
	ViewNumber::Enum viewNumber = ViewNumber::COUNT) const;
    void SetViewNumber (ViewNumber::Enum viewNumber);
    ViewSettings& GetViewSettings (ViewNumber::Enum viewNumber) const
    {
	return *m_viewSettings[viewNumber];
    }
    ViewSettings& GetViewSettings () const
    {
	return GetViewSettings (GetViewNumber ());
    }
    size_t GetViewSettingsSize ()
    {
	return m_viewSettings.size ();
    }

    QColor GetHighlightColor (ViewNumber::Enum view, 
			      HighlightNumber::Enum highlight) const;
    size_t GetHighlightLineWidth () const
    {
	return m_highlightLineWidth;
    }

    const BodiesAlongTime& GetBodiesAlongTime () const;
    const BodyAlongTime& GetBodyAlongTime (size_t bodyId) const;

    size_t GetCurrentTime () const
    {
	return GetCurrentTime (GetViewNumber ());
    }
    size_t GetCurrentTime (ViewNumber::Enum viewNumber) const;
    void SetCurrentTime (size_t time, bool setLastStep = false);
    size_t GetTimeSteps () const
    {
	return GetTimeSteps (GetViewNumber ());
    }
    size_t GetTimeSteps (ViewNumber::Enum viewNumber) const;

    bool IsMissingPropertyShown (BodyScalar::Enum bodyProperty) const;

    bool IsTimeDisplacementUsed () const;

    double GetTimeDisplacement () const
    {
	return m_timeDisplacement;
    }
    

    double GetContextAlpha () const
    {
	return m_contextAlpha;
    }

    const QColor& GetEndTranslationColor (const G3D::Vector3int16& di) const;
    QColor GetCenterPathContextColor () const;

    GLUquadricObj* GetQuadricObject () const 
    {
	return m_quadric;
    }

    double GetArrowBaseRadius () const {return m_arrowBaseRadius;}
    double GetArrowHeight () const 
    {
	return m_arrowHeight;
    }
    double GetEdgeRadius () const 
    {
	return m_edgeRadius;
    }
    double GetEdgeWidth () const 
    {
	return m_edgeWidth;
    }

    bool IsCenterPathLineUsed () const
    {
	return m_centerPathLineUsed;
    }

    bool EdgesShown () const
    {
	return m_edgesShown;
    }

    bool EdgesTessellationShown () const
    {
	return m_edgesTessellationShown;
    }
    bool IsCenterPathBodyShown () const
    {
	return m_centerPathBodyShown;
    }

    bool ConstraintsShown () const
    {
	return m_constraintsShown;
    }

    bool ConstraintPointsShown () const
    {
	return m_constraintPointsShown;
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
     * Calculates and does the viewport transform.
     */
    void ModelViewTransform (ViewNumber::Enum viewNumber, 
			     size_t timeStep) const;
    vtkSmartPointer<vtkMatrix4x4> GetModelViewMatrix (
	ViewNumber::Enum viewNumber, size_t timeStep) const;

    /**
     * Setup the viewing volume first centered around origin and then 
     * translated toward negative Z with m_cameraDistance.
     */
    void ProjectionTransform (
	ViewNumber::Enum viewNumber, 
	ViewingVolumeOperation::Enum enclose = 
	ViewingVolumeOperation::DONT_ENCLOSE2D) const;
    size_t GetFaceScalar () const
    {
	return GetFaceScalar (GetViewNumber ());
    }

    size_t GetFaceScalar (ViewNumber::Enum viewNumber) const;

    G3D::Rect2D GetViewRect (ViewNumber::Enum viewNumber) const;
    G3D::Rect2D GetViewRect () const
    {
	return GetViewRect (GetViewNumber ());
    }

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
    float GetOnePixelInObjectSpace () const;
    float GetBubbleSize (ViewNumber::Enum viewNumber) const;
    float GetDeformationSizeInitialRatio (ViewNumber::Enum viewNumber) const;
    float GetVelocitySizeInitialRatio (ViewNumber::Enum viewNumber) const;
    G3D::AABox CalculateViewingVolume (
	ViewNumber::Enum viewNumber, 
	ViewingVolumeOperation::Enum enclose  = 
	ViewingVolumeOperation::DONT_ENCLOSE2D) const;
    void RotateAndTranslateAverageAround (
	ViewNumber::Enum viewNumber,
	size_t timeStep, int direction) const;
    void DisplayT1Quad (ViewNumber::Enum view, 
			size_t timeStep, size_t t1Index) const;
    pair<float, float> GetRange (ViewNumber::Enum viewNumber) const;
    pair<float, float> GetVelocityMagnitudeRange (
	ViewNumber::Enum viewNumber) const;
    pair<float, float> GetRangeCount (ViewNumber::Enum viewNumber) const;
    pair<float, float> GetRangeCount () const;
    pair<float, float> GetRangeT1sPDE (ViewNumber::Enum viewNumber) const;
    pair<float, float> GetRangeT1sPDE () const
    {
	return GetRangeT1sPDE (GetViewNumber ());
    }
    static ColorBarType::Enum GetColorBarType (
	ViewType::Enum viewType, size_t property, 
	ComputationType::Enum statisticsType);
    ColorBarType::Enum GetColorBarType (ViewNumber::Enum viewNumber) const;
    ColorBarType::Enum GetColorBarType () const;
    float LinkedTimeStepStretch (ViewNumber::Enum viewNumber) const;
    float LinkedTimeStepStretch (size_t max,
				    ViewNumber::Enum viewNumber) const;
    pair<size_t, ViewNumber::Enum> LinkedTimeMaxInterval () const;
    pair<size_t, ViewNumber::Enum> LinkedTimeMaxSteps () const;
    TimeLinkage::Enum GetTimeLinkage () const
    {
	return m_timeLinkage;
    }
    bool IsReflectedHalfView () const
    {
	return m_reflectedHalfView;
    }
    void SetReflectedHalfView (bool reflectedHalfView);
    void SetForceDifferenceShown (bool forceDifference);
    size_t GetLinkedTime () const
    {
	return m_linkedTime;
    }
    ViewCount::Enum GetViewCount () const
    {
	return m_viewCount;
    }
    ViewLayout::Enum GetViewLayout () const
    {
	return m_viewLayout;
    }
    template<typename T>
    void SetOneOrTwoViews (T* t,void (T::*f) (ViewNumber::Enum));


Q_SIGNALS:
    void PaintEnd ();
    void ViewChanged ();
    void ColorBarModelChanged (
	ViewNumber::Enum viewNumber,
	boost::shared_ptr<ColorBarModel> colorBarModel);
    void OverlayBarModelChanged (
	ViewNumber::Enum viewNumber,
	boost::shared_ptr<ColorBarModel> colorBarModel);

public Q_SLOTS:
    void ButtonClickedTimeLinkage (int id);
    void ButtonClickedViewType (int id);
    void ButtonClickedInteractionObject (int id);
    void ClickedEnd ();

    /*
     * Global options
     */
    void ToggledStandaloneElementsShown (bool checked);
    void ToggledAxesShown (bool checked);
    void ToggledBoundingBoxSimulation (bool checked);
    void ToggledBoundingBoxFoam (bool checked);
    void ToggledBoundingBoxBody (bool checked);
    void ToggledAverageAroundMarked (bool checked);
    void ToggledViewFocusShown (bool checked);
    /**
     * Shows center paths
     * param checked true for showing the center paths false otherwise
     */
    void ToggledConstraintsShown (bool checked);
    void ToggledConstraintPointsShown (bool checked);
    void ToggledCenterPathBodyShown (bool checked);
    void ToggledCenterPathLineUsed (bool checked);
    void ToggledCenterPathTubeUsed (bool checked);
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
    void ToggledCenterPathHidden (bool checked);
    void ToggledTorusDomainShown (bool checked);
    void ToggledT1sShown (bool checked);
    void ToggledTitleShown (bool checked);
    void ToggledT1sShiftLower (bool checked);
    void ToggledTorusOriginalDomainClipped (bool checked);
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

    void SetBodyOrFaceScalar (
	ViewNumber::Enum viewNumber,
	boost::shared_ptr<ColorBarModel> colorBarModel,
	size_t property);
    void SetColorBarModel (ViewNumber::Enum viewNumber, 
			   boost::shared_ptr<ColorBarModel> colorBarModel);
    void SetOverlayBarModel (ViewNumber::Enum viewNumber, 
			     boost::shared_ptr<ColorBarModel> colorBarModel);
    void CurrentIndexChangedSimulation (int index);
    void CurrentIndexChangedInteractionMode (int index);
    void CurrentIndexChangedComputationType (int index);
    void CurrentIndexChangedAxesOrder (int index);
    void CurrentIndexChangedSelectedLight (int selectedLight);
    void CurrentIndexChangedViewCount (int index);
    void CurrentIndexChangedViewLayout (int index);

    void ValueChangedNoiseStart (int i);
    void ValueChangedNoiseAmplitude (int i);
    void ValueChangedNoiseFrequency (int i);

    void ValueChangedAngleOfView (int newIndex);
    void ValueChangedContextAlpha (int sliderValue);
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


    void ValueChangedSliderTimeSteps (int timeStep);
    void ValueChangedAverageTimeWindow (int timeSteps);
    void ValueChangedT1sTimeWindow (int timeSteps);
    void ValueChangedTimeDisplacement (int timeDisplacement);
    void ValueChangedT1Size (int index);
    void ValueChangedT1sKernelIntervalPerPixel (int index);
    void ValueChangedT1sKernelSigma (int index);
    void ValueChangedT1sKernelTextureSize (int index);
    void ToggledT1sKernelTextureSizeShown (bool checked);
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
    void LinkedTimeBegin ();
    void LinkedTimeEnd ();
    void SelectAll ();
    void DeselectAll ();
    void SelectBodiesByIdList ();
    void SelectThisBodyOnly ();
    void AverageAroundBody ();
    void AverageAroundSecondBody ();
    void AverageAroundReset ();
    void ContextDisplayBody ();
    void ContextDisplayReset ();
    void ToggledAverageAroundAllowRotation (bool checked);
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
    void CopyColorBarFrom (int viewNumber);

public:
    const static  size_t DISPLAY_ALL;
    const static size_t QUADRIC_SLICES;
    const static size_t QUADRIC_STACKS;
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

    typedef boost::unordered_map<G3D::Vector3int16, QColor,
				 Vector3int16Hash> EndLocationColor;
    typedef void (WidgetGl::* ViewTypeDisplay) (ViewNumber::Enum view) const;

private:
    void setSimulation (int i, ViewNumber::Enum viewNumber);
    void initTransformViewport ();
    void cleanupTransformViewport ();
    void valueChangedT1sKernelTextureSize (ViewNumber::Enum viewNumber);
    void toggledT1sKernelTextureSizeShown (ViewNumber::Enum viewNumber);
    void valueChangedT1sKernelSigma (ViewNumber::Enum viewNumber);
    void valueChangedT1sKernelIntervalPerPixel (ViewNumber::Enum viewNumber);
    void setScaleCenter (ViewNumber::Enum viewNumber);
    G3D::Vector2 calculateScaleCenter (ViewNumber::Enum viewNumber, 
				       const G3D::Rect2D& rect) const;
    bool linkedTimesValid (size_t timeBegin, size_t timeEnd);
    bool linkedTimesValid ();
    void contextMenuEventColorBar (QMenu* menu) const;
    void contextMenuEventOverlayBar (QMenu* menu) const;
    void contextMenuEventView (QMenu* menu) const;
    void activateViewShader (
	ViewNumber::Enum viewNumber, 
	ViewingVolumeOperation::Enum enclose, G3D::Rect2D& srcRect,
	G3D::Vector2 rotationCenter, float angleDegrees) const;
    G3D::Vector2 toTexture (ViewNumber::Enum viewNumber, 
			    G3D::Vector2 object) const;
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
    void setView (const G3D::Vector2& clickedPoint);
    void selectView (const G3D::Vector2& clickedPoint);
    double getXOverY (ViewNumber::Enum viewNumber) const;
    double getXOverY () const;
    static G3D::Rect2D getViewColorBarRect (const G3D::Rect2D& viewRect);
    static G3D::Rect2D getViewOverlayBarRect (const G3D::Rect2D& viewRect);
    void calculateEdgeRadius (
	double edgeRadiusMultiplier,
	double* edgeRadius, double* arrowBaseRadius, 
	double* arrowHeight, double* edgeWidth) const;
    void displayContextMenuPos (ViewNumber::Enum viewNumber) const;
    void displayBodyCenters (ViewNumber::Enum viewNumber, 
			     bool useZPos = false) const;
    void displayRotationCenter (ViewNumber::Enum viewNumber) const;
    void displayFaceCenters (ViewNumber::Enum viewNumber) const;
    void displayViewDecorations (ViewNumber::Enum viewNumber);
    void displayViewFocus (ViewNumber::Enum viewNumber);
    void displayViewsGrid ();
    void displayTextureColorBar (GLuint texture,
				 ViewNumber::Enum viewNumber, 
				 const G3D::Rect2D& barRect);
    void displayOverlayBar (
	ViewNumber::Enum viewNumber, const G3D::Rect2D& barRect);
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
    G3D::AABox calculateCenteredViewingVolume (
	ViewNumber::Enum viewNumber) const;
    G3D::Vector3 calculateViewingVolumeScaledExtent (
	ViewNumber::Enum viewNumber) const;
    void initQuadrics ();
    void initEndTranslationColor ();
    void initViewSettings ();
    void calculateCameraDistance (ViewNumber::Enum viewNumber);
    /**
     * Generates a display list for edges
     * @return the display list
     */
    template<typename displayEdge>
    void displayEdges (ViewNumber::Enum viewNumber) const;
    void displayView (ViewNumber::Enum view);
    void displayViews ();
    void displayContextBodies (ViewNumber::Enum view) const;
    void displayContextBox (
	ViewNumber::Enum view,
	bool adjustForAverageAroundMovementRotation = false) const;
    void displayAverageAroundBodies (
	ViewNumber::Enum view, 
	bool adjustForAverageAroundMovementRotation = false) const;

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
    void displayFacesTorus (ViewNumber::Enum view) const;
    void displayFacesAverage (ViewNumber::Enum view) const;
    void displayFacesTorusTubes () const;
    void displayFacesTorusLines () const;

    /**
     * Generates a display list for center paths
     */
    void displayCenterPathsWithBodies (ViewNumber::Enum view) const;
    void displayTorusDomain (ViewNumber::Enum viewNumber) const;
    void displayBodyNeighbors (ViewNumber::Enum viewNumber) const;
    void displayBodiesNeighbors () const;
    void displayBodyDeformation (ViewNumber::Enum viewNumber) const;
    void displayBodyVelocity (ViewNumber::Enum viewNumber) const;
    void displayDeformation (ViewNumber::Enum viewNumber) const;
    void displayVelocity (ViewNumber::Enum viewNumber) const;
    void displayT1sDot (ViewNumber::Enum view) const;
    void displayT1sDot (ViewNumber::Enum view, size_t timeStep) const;
    void displayCenterPaths (ViewNumber::Enum view) const;
    void compileCenterPaths (ViewNumber::Enum view) const;
    void compile (ViewNumber::Enum view) const;
    void compileUpdate ();

    void displayBoundingBox (ViewNumber::Enum viewNumber) const;
    void displayFocusBox (ViewNumber::Enum viewNumber) const;
    void displayAxes (ViewNumber::Enum viewNumber);
    G3D::Vector3 getInitialLightPosition (ViewNumber::Enum viewNumber,
					  LightNumber::Enum lightPosition) const;

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
    template<HighlightNumber::Enum highlightColorIndex>
    void displayFacesContour (
	const vector< boost::shared_ptr<Body> >& bodies, 
	ViewNumber::Enum viewNumber, GLfloat lineWidth) const;
    void displayFacesContour (
	const vector< boost::shared_ptr<Body> >& bodies, 
	ViewNumber::Enum viewNumber) const;
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
	const QPoint& position, vector<size_t>* bodies, 
	bool selected = true) const;
    G3D::Vector3 brushedBodies (
	const QPoint& position, 
	vector< boost::shared_ptr<Body> >* bodies, bool selected = true) const;
    G3D::Vector3 brushedFace (const OrientedFace** of) const;
    OrientedEdge brushedEdge () const;
    void brushedFace (const QPoint& position, vector<size_t>* bodies) const;
    G3D::Vector3 toObjectTransform (const QPoint& position) const;
    G3D::Vector3 toObject (const QPoint& position) const;
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
    void transformFoamAverageAround (
	ViewNumber::Enum viewNumber, size_t timeStep) const;
    void initCopy (
	boost::array<boost::shared_ptr<QAction>, 
	ViewNumber::COUNT>& actionCopyTransformation,
	boost::shared_ptr<QSignalMapper>& signalMapperCopyTransformation);
    string infoSelectedBody () const;
    string infoSelectedBodies () const;

private:
    /**
     * Setup lighting for displaying faces edges and vertices
     */
    static void quadricErrorCallback (GLenum errorCode);
    
private:
    // Min, max values for T1s, Context alpha, force length
    const static pair<float,float> T1S_SIZE;
    const static pair<float,float> CELL_LENGTH_EXP2;
    const static pair<float,float> CONTEXT_ALPHA;
    const static GLfloat HIGHLIGHT_LINE_WIDTH;

private:
    Q_OBJECT

    /**
     * What do we display
     */
    bool m_torusDomainShown;
    bool m_torusOriginalDomainClipped;
    InteractionMode::Enum m_interactionMode;
    InteractionObject::Enum m_interactionObject;

    /**
     * Foam to be displayd. Each element coresponds to a DMP file
     */
    SimulationGroup* m_simulationGroup;
    /**
     * Used for rotation, translation and scale
     */
    QPoint m_lastPos;
    QPoint m_contextMenuPosScreen;
    G3D::Vector3 m_contextMenuPosObject;
    EndLocationColor m_endTranslationColor;
    GLUquadricObj* m_quadric;    
    /**
     * For displaying edges as tubes
     */
    double m_edgeRadius;
    double m_edgeWidth;
    double m_edgeRadiusRatio;
    /**
     * For displaying arrows in the Torus Model edges
     */
    double m_arrowBaseRadius;
    double m_arrowHeight;

    bool m_edgesShown;
    bool m_edgesTessellationShown;
    bool m_bodyCenterShown;
    bool m_bodyNeighborsShown;
    bool m_faceCenterShown;
    bool m_centerPathBodyShown;
    bool m_boundingBoxSimulationShown;    
    bool m_boundingBoxFoamShown;
    bool m_boundingBoxBodyShown;
    bool m_axesShown;
    bool m_standaloneElementsShown;
    boost::array<ViewTypeDisplay, ViewType::COUNT> m_display;


    boost::shared_ptr<QAction> m_actionLinkedTimeBegin;
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
    boost::shared_ptr<QAction> m_actionOverlayBarClampHighMinimum;
    boost::array<boost::shared_ptr<QAction>, 
		 ViewNumber::COUNT> m_actionCopyTransformation;
    boost::shared_ptr<QSignalMapper> m_signalMapperCopyTransformation;
    boost::array<boost::shared_ptr<QAction>, 
		 ViewNumber::COUNT> m_actionCopySelection;
    boost::shared_ptr<QSignalMapper> m_signalMapperCopySelection;
    boost::array<boost::shared_ptr<QAction>, 
		 ViewNumber::COUNT> m_actionCopyColorMap;
    boost::shared_ptr<QSignalMapper> m_signalMapperCopyColorBar;
    double m_timeDisplacement;
    boost::shared_ptr<SelectBodiesById> m_selectBodiesByIdList;
    QLabel *m_labelStatusBar;
    bool m_centerPathTubeUsed;
    bool m_centerPathLineUsed;
    bool m_t1sShown;
    double m_t1sSize;
    double m_contextAlpha;
    size_t m_highlightLineWidth;
    bool m_missingPressureShown;
    bool m_missingVolumeShown;
    bool m_objectVelocityShown;
    bool m_titleShown;
    bool m_averageAroundMarked;
    bool m_viewFocusShown;
    bool m_constraintsShown;
    bool m_constraintPointsShown;
    bool m_contextBoxShown;

    // View related variables
    ViewCount::Enum m_viewCount;
    ViewLayout::Enum m_viewLayout;
    ViewNumber::Enum m_viewNumber;
    boost::array<
	boost::shared_ptr<ViewSettings>, ViewNumber::COUNT> m_viewSettings;
    TimeLinkage::Enum m_timeLinkage;
    bool m_reflectedHalfView;
    /**
     * Used to keep trak of time for TimeLinkage::LINKED.
     * It has the resolution of the view that has the maximum interval and the 
     * range of the view that has the maximum range.
     * @see LinkedTimeMaxInterval, @see LinkedTimeMaxSteps
     */
    size_t m_linkedTime;
    ShowType m_showType;
    size_t m_showBodyId;
};


#endif //__WIDGETGL_H__

// Local Variables:
// mode: c++
// End: