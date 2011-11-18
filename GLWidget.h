/**
 * @file   GLWidget.h
 * @author Dan R. Lipsa
 *
 * Widget for displaying foam bubbles using OpenGL
 */
#ifndef __GLWIDGET_H__
#define __GLWIDGET_H__

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
class Force;
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
class GLWidget : public QGLWidget, public QGLFunctions
{
public:
    /**
     * Constructor
     * @param parent parent widget
     */
    GLWidget(QWidget *parent = 0);
    /**
     * Destructor
     */
    ~GLWidget();
    /**
     * Gets the minimum size of the GLWidget
     */
    QSize minimumSizeHint() ;
    /**
     * Gets the prefered size
     */
    QSize sizeHint() ;
    /**
     * Sets the data displayed by the GLWidget
     * @param dataAlongTime data displayed by the GLWidget
     */
    void SetSimulationGroup (const SimulationGroup* dataAlongTime);
    const SimulationGroup& GetSimulationGroup ()
    {
	return *m_simulationGroup;
    }
    /**
     * Gets the data displayed by the GLWidget
     */
    const Simulation& GetSimulation (size_t index) const;
    const Simulation& GetSimulation (ViewNumber::Enum viewNumber) const;
    const Simulation& GetSimulation () const;
    ViewNumber::Enum GetViewNumber () const
    {
	return m_viewNumber;
    }
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

    bool IsMissingPropertyShown (BodyProperty::Enum bodyProperty) const;

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

    bool IsEdgesTessellation () const
    {
	return m_edgesTessellation;
    }
    bool IsCenterPathBodyShown () const
    {
	return m_centerPathBodyShown;
    }

    boost::shared_ptr<QAction> GetActionResetTransformAll () const
    {
	return m_actionResetTransformAll;
    }
    boost::shared_ptr<QAction> GetActionEditColorMap () const
    {
	return m_actionEditColorMap;
    }

    
    /**
     * Calculates and does the viewport transform.
     */
    void ModelViewTransform (ViewNumber::Enum viewNumber, 
			     size_t timeStep) const;
    /**
     * Setup the viewing volume first centered around origin and then 
     * translated toward negative Z with m_cameraDistance.
     */
    void ProjectionTransform (
	ViewNumber::Enum viewNumber, 
	ViewingVolumeOperation::Enum enclose = 
	ViewingVolumeOperation::DONT_ENCLOSE2D) const;
    size_t GetBodyOrFaceProperty () const
    {
	return GetBodyOrFaceProperty (GetViewNumber ());
    }

    size_t GetBodyOrFaceProperty (ViewNumber::Enum viewNumber) const;

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
    double GetCellLength (ViewNumber::Enum viewNumber) const;
    float GetDeformationSizeInitialRatio (ViewNumber::Enum viewNumber) const;
    float GetVelocitySizeInitialRatio (ViewNumber::Enum viewNumber) const;
    G3D::AABox CalculateViewingVolume (
	ViewNumber::Enum viewNumber, ViewingVolumeOperation::Enum enclose) const;
    void RotateAndTranslateAverageAround (
	ViewNumber::Enum viewNumber,
	size_t timeStep, int direction) const;
    void DisplayT1Quad (ViewNumber::Enum view, 
			size_t timeStep, size_t t1Index) const;
    pair<float, float> GetRange (ViewNumber::Enum viewNumber) const;
    pair<float, float> GetRangeCount (ViewNumber::Enum viewNumber) const;
    pair<float, float> GetRangeCount () const;
    pair<float, float> GetRangeT1sPDE (ViewNumber::Enum viewNumber) const;
    pair<float, float> GetRangeT1sPDE () const
    {
	return GetRangeT1sPDE (GetViewNumber ());
    }
    static ColorBarType::Enum GetColorBarType (
	ViewType::Enum viewType, size_t property, 
	StatisticsType::Enum statisticsType);
    ColorBarType::Enum GetColorBarType (ViewNumber::Enum viewNumber) const;
    ColorBarType::Enum GetColorBarType () const;
    void SetBodyOrFaceProperty (
	ViewNumber::Enum viewNumber,
	boost::shared_ptr<ColorBarModel> colorBarModel,
	size_t property);
    float LinkedTimeStepStretch (ViewNumber::Enum viewNumber) const;
    float LinkedTimeStepStretch (size_t max,
				    ViewNumber::Enum viewNumber) const;
    pair<size_t, ViewNumber::Enum> LinkedTimeMaxInterval () const;
    pair<size_t, ViewNumber::Enum> LinkedTimeMaxSteps () const;
    TimeLinkage::Enum GetTimeLinkage () const
    {
	return m_timeLinkage;
    }
    TransformLinkage::Enum GetTransformLinkage () const
    {
	return m_transformLinkage;
    }
    void SetTransformLinkage (TransformLinkage::Enum transformLinkage);
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

Q_SIGNALS:
    void PaintedGL ();
    void ViewChanged ();
    void ColorBarModelChanged (boost::shared_ptr<ColorBarModel> colorBarModel);

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
    void ToggledBoundingBoxShown (bool checked);
    void ToggledBodiesBoundingBoxesShown (bool checked);
    void ToggledAverageAroundMarked (bool checked);
    void ToggledViewFocusShown (bool checked);
    /**
     * Shows center paths
     * param checked true for showing the center paths false otherwise
     */
    void ToggledCenterPathBodyShown (bool checked);
    void ToggledCenterPathLineUsed (bool checked);
    void ToggledCenterPathTubeUsed (bool checked);
    void ToggledContextView (bool checked);
    void ToggledForceNetworkShown (bool checked);
    void ToggledForcePressureShown (bool checked);
    void ToggledForceResultShown (bool checked);
    void ToggledDirectionalLightEnabled (bool checked);
    void ToggledBodyCenterShown (bool checked);
    void ToggledBodyNeighborsShown (bool checked);
    void ToggledFaceCenterShown (bool checked);
    void ToggledEdgesTessellation (bool checked);

    void ToggledFacesShowEdges (bool checked);
    void ToggledLightNumberShown (bool checked);
    void ToggledLightEnabled (bool checked);
    void ToggledContextHidden (bool checked);
    void ToggledCenterPathHidden (bool checked);
    void ToggledTorusOriginalDomainShown (bool checked);
    void ToggledT1sShown (bool checked);
    void ToggledTitleShown (bool checked);
    void ToggledT1sShiftLower (bool checked);
    void ToggledTorusOriginalDomainClipped (bool checked);
    void ToggledMissingPressureShown (bool checked);
    void ToggledMissingVolumeShown (bool checked);
    void ToggledShowDeformation (bool checked);
    void ToggledShowDeformationGrid (bool checked);
    void ToggledShowVelocity (bool checked);
    void ToggledShowVelocityGrid (bool checked);
    void ToggledShowDeformationGridCellCenter (bool checked);


    void SetBodyOrFaceProperty (
	boost::shared_ptr<ColorBarModel> colorBarModel,
	size_t property);
    void SetColorBarModel (boost::shared_ptr<ColorBarModel> colorBarModel);
    void CurrentIndexChangedSimulation (int index);
    void CurrentIndexChangedInteractionMode (int index);
    void CurrentIndexChangedStatisticsType (int index);
    void CurrentIndexChangedAxesOrder (int index);
    void CurrentIndexChangedSelectedLight (int selectedLight);
    void CurrentIndexChangedViewCount (int index);
    void CurrentIndexChangedViewLayout (int index);

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
    void ValueChangedStatisticsTimeWindow (int timeSteps);
    void ValueChangedTimeDisplacement (int timeDisplacement);
    void ValueChangedT1Size (int index);
    void ValueChangedT1sKernelIntervalPerPixel (int index);
    void ValueChangedT1sKernelSigma (int index);
    void ValueChangedT1sKernelTextureSize (int index);
    void ToggledT1sKernelTextureSizeShown (bool checked);
    void ValueChangedDeformationSizeExp (int index);
    void ValueChangedDeformationLineWidthExp (int index);
    void ValueChangedVelocitySizeExp (int index);
    void ValueChangedVelocityLineWidthExp (int index);
    void ValueChangedForceSizeExp (int index);
    void ValueChangedForceLineWidthExp (int index);
    void ValueChangedHighlightLineWidth (int newWidth);
    // Actions
    void ResetTransformAll ();
    void ResetTransformFocus ();
    void ResetTransformContext ();
    void ResetTransformLight ();    
    void ResetTransformGrid ();
    void SelectBodiesByIdList ();
    void LinkedTimeBegin ();
    void LinkedTimeEnd ();
    void SelectAll ();
    void DeselectAll ();
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
    void ShowNeighbors ();
    void ShowDeformation ();
    void ShowVelocity ();
    void ShowReset ();
    // Actions color bar
    void ColorBarClampClear ();
    void CopyTransformationFrom (int viewNumber);
    void CopySelectionFrom (int viewNumber);
    void CopyColorBarFrom (int viewNumber);

public:
    const static  size_t DISPLAY_ALL;
    const static size_t QUADRIC_SLICES;
    const static size_t QUADRIC_STACKS;
    const static pair<float,float> SIZE_EXP2;
    const static pair<float,float> LINE_WIDTH_EXP2;
    const static pair<float,float> FORCE_SIZE_EXP2;

protected:
    /**
     * Initializes OpenGL
     */
    void initializeGL();
    /**
     * Called when the GLWidget needs repainted
     */
    void paintGL();
    /**
     * Called when the GLWidget is resized
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
    typedef void (GLWidget::* ViewTypeDisplay) (ViewNumber::Enum view) const;

private:
    void initTransformViewport ();
    void cleanupTransformViewport ();
    void setOneOrTwoViews (void (GLWidget::*f) (ViewNumber::Enum));
    void valueChangedT1sKernelTextureSize (ViewNumber::Enum viewNumber);
    void toggledT1sKernelTextureSizeShown (
	ViewNumber::Enum viewNumber, bool checked);
    void valueChangedT1sKernelSigma (ViewNumber::Enum viewNumber);
    void valueChangedT1sKernelIntervalPerPixel (ViewNumber::Enum viewNumber);
    void setScaleCenter (ViewNumber::Enum viewNumber);
    G3D::Vector3 getScaleCenterTranslation (ViewNumber::Enum viewNumber) const;
    G3D::Vector2 getScaleCenter (ViewNumber::Enum viewNumber, 
				 const G3D::Rect2D& rect) const;
    bool linkedTimesValid (size_t timeBegin, size_t timeEnd);
    bool linkedTimesValid ();
    void contextMenuEventColorBar (QMenu* menu) const;
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
    void mouseMoveRotate (QMouseEvent *event);
    void mouseMoveTranslate (QMouseEvent *event);
    void mouseMoveScale (QMouseEvent *event);
    void allTransform (ViewNumber::Enum viewNumber) const;
    void viewportTransform (ViewNumber::Enum viewNumber) const;
    void setLight (int sliderValue, int maximumValue, 
		   LightType::Enum lightType, ColorNumber::Enum colorNumber);
    void setView (const G3D::Vector2& clickedPoint);
    void selectView (const G3D::Vector2& clickedPoint);
    double getViewXOverY () const;
    static G3D::Rect2D getViewColorBarRect (const G3D::Rect2D& viewRect);
    void calculateEdgeRadius (
	double edgeRadiusMultiplier,
	double* edgeRadius, double* arrowBaseRadius, 
	double* arrowHeight, double* edgeWidth) const;
    void displayContextMenuPos (ViewNumber::Enum viewNumber) const;
    void displayBodyCenters (ViewNumber::Enum viewNumber, 
			     bool useZPos = false) const;
    void displayFaceCenters (ViewNumber::Enum viewNumber) const;
    void displayViewDecorations (ViewNumber::Enum viewNumber);
    void displayViewFocus (ViewNumber::Enum viewNumber);
    void displayViewsGrid ();
    void displayTextureColorBar (ViewNumber::Enum viewNumber, 
				 const G3D::Rect2D& viewRect);
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
    G3D::AABox calculateViewingVolume (
	ViewNumber::Enum viewNumber,
	double xOverY, double scaleRatio, 
	ViewingVolumeOperation::Enum enclose = 
	ViewingVolumeOperation::DONT_ENCLOSE2D) const;
    G3D::AABox calculateCenteredViewingVolume (
	ViewNumber::Enum viewNumber, double xOverY,
	double scaleRatio) const;

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
    void displayContextStationaryFoam (
	ViewNumber::Enum view,
	bool adjustForAverageAroundMovementRotation = false) const;
    void displayAverageAroundBodies (
	ViewNumber::Enum view, 
	bool adjustForAverageAroundMovementRotation = false) const;

    void displayEdgesNormal (ViewNumber::Enum view) const;
    template<typename displayEdge>
    void displayStandaloneEdges (
	const Foam& foam, bool useZPos = false, double zPos = 0) const;
    void displayStandaloneFaces () const;
    void displayEdgesTorus (ViewNumber::Enum view) const;
    void displayEdgesTorusTubes () const;
    void displayEdgesTorusLines () const;

    void displayFacesNormal (ViewNumber::Enum view) const;
    void displayFacesTorus (ViewNumber::Enum view) const;
    void displayFacesAverage (ViewNumber::Enum view) const;
    void displayT1sPDE (ViewNumber::Enum view) const;
    void displayFacesTorusTubes () const;
    void displayFacesTorusLines () const;

    /**
     * Generates a display list for center paths
     */
    void displayCenterPathsWithBodies (ViewNumber::Enum view) const;
    void displayOriginalDomain (ViewNumber::Enum viewNumber) const;
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
    void labelCompileUpdate ();

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
    G3D::Matrix3 rotate (ViewNumber::Enum viewNumber, 
			 const QPoint& position, const G3D::Matrix3& rotate);
    void translate (ViewNumber::Enum viewNumber,
	const QPoint& position,
	G3D::Vector3::Axis screenXTranslation,
	G3D::Vector3::Axis screenYTranslation);
    void translateGrid (ViewNumber::Enum viewNumber, const QPoint& position);
    void calculateTranslationRatio (
    ViewNumber::Enum viewNumber, const QPoint& position,
    G3D::Vector3::Axis screenXTranslation,
    G3D::Vector3::Axis screenYTranslation, 
    G3D::Vector3* translationRatio, G3D::Vector3* focusBoxExtent);
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
	const QPoint& position, vector<size_t>* bodies) const;
    G3D::Vector3 brushedBodies (const QPoint& position, 
				vector< boost::shared_ptr<Body> >* bodies) const;
    G3D::Vector3 brushedFace (const OrientedFace** of) const;
    OrientedEdge brushedEdge () const;
    void brushedFace (const QPoint& position, vector<size_t>* bodies) const;
    G3D::Vector3 toObjectTransform (const QPoint& position) const;
    G3D::Vector3 toObject (const QPoint& position) const;
    G3D::Vector3 toObjectTransform (const QPoint& position, 
				    ViewNumber::Enum viewNumber) const;
    
    void initDisplayView ();
    void createActions ();
    string getBodySelectorLabel ();
    string getContextLabel ();
    string getAverageAroundMovementShownLabel ();
    string getAverageAroundLabel ();
    string getBodyScaleLabel ();
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
    bool m_torusOriginalDomainDisplay;
    bool m_torusOriginalDomainClipped;
    InteractionMode::Enum m_interactionMode;
    InteractionObject::Enum m_interactionObject;

    /**
     * Foam to be displayd. Each element coresponds to a DMP file
     */
    const SimulationGroup* m_simulationGroup;
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
    double m_minimumEdgeRadius;
    double m_edgeRadiusRatio;
    /**
     * For displaying arrows in the Torus Model edges
     */
    double m_arrowBaseRadius;
    double m_arrowHeight;

    bool m_facesShowEdges;
    bool m_bodyCenterShown;
    bool m_bodyNeighborsShown;
    bool m_faceCenterShown;
    bool m_edgesTessellation;
    bool m_centerPathBodyShown;
    bool m_boundingBoxShown;
    bool m_bodiesBoundingBoxesShown;
    bool m_axesShown;
    bool m_standaloneElementsShown;
    boost::array<ViewTypeDisplay, ViewType::COUNT> m_display;


    boost::shared_ptr<QAction> m_actionLinkedTimeBegin;
    boost::shared_ptr<QAction> m_actionLinkedTimeEnd;
    boost::shared_ptr<QAction> m_actionSelectAll;
    boost::shared_ptr<QAction> m_actionDeselectAll;
    boost::shared_ptr<QAction> m_actionResetTransformAll;
    boost::shared_ptr<QAction> m_actionResetTransformFocus;
    boost::shared_ptr<QAction> m_actionResetTransformContext;
    boost::shared_ptr<QAction> m_actionResetTransformLight;
    boost::shared_ptr<QAction> m_actionResetTransformGrid;
    boost::shared_ptr<QAction> m_actionSelectBodiesById;

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

    boost::shared_ptr<QAction> m_actionShowNeighbors;
    boost::shared_ptr<QAction> m_actionShowDeformation;
    boost::shared_ptr<QAction> m_actionShowVelocity;
    boost::shared_ptr<QAction> m_actionShowReset;

    boost::shared_ptr<QAction> m_actionEditColorMap;
    boost::shared_ptr<QAction> m_actionClampClear;
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
    boost::shared_ptr<SelectBodiesById> m_selectBodiesById;
    QLabel *m_labelStatusBar;
    bool m_centerPathTubeUsed;
    bool m_centerPathLineUsed;
    bool m_t1sShown;
    double m_t1sSize;
    double m_contextAlpha;
    size_t m_highlightLineWidth;
    bool m_missingPressureShown;
    bool m_missingVolumeShown;
    bool m_titleShown;
    bool m_averageAroundMarked;
    bool m_viewFocusShown;

    // View related variables
    ViewCount::Enum m_viewCount;
    ViewLayout::Enum m_viewLayout;
    ViewNumber::Enum m_viewNumber;
    boost::array<
	boost::shared_ptr<ViewSettings>, ViewNumber::COUNT> m_viewSettings;
    TimeLinkage::Enum m_timeLinkage;
    TransformLinkage::Enum m_transformLinkage;
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

#endif //__GLWIDGET_H__

// Local Variables:
// mode: c++
// End:
