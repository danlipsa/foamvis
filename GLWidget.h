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
class FoamAlongTime;
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
    void SetFoamAlongTime (FoamAlongTime* dataAlongTime);
    /**
     * Gets the data displayed by the GLWidget
     */
    const FoamAlongTime& GetFoamAlongTime () const
    {
	return *m_foamAlongTime;
    }
    FoamAlongTime& GetFoamAlongTime ()
    {
	return *m_foamAlongTime;
    }
    ViewNumber::Enum GetViewNumber () const
    {
	return m_viewNumber;
    }
    ViewSettings& GetViewSettings (ViewNumber::Enum viewNumber) const
    {
	return *m_viewSettings[viewNumber];
    }

    ViewSettings& GetViewSettings () const
    {
	return GetViewSettings (GetViewNumber ());
    }

    QColor GetHighlightColor (ViewNumber::Enum view, 
			      HighlightNumber::Enum highlight) const;
    size_t GetHighlightLineWidth () const
    {
	return m_highlightLineWidth;
    }
    double GetForceLength () const
    {
	return m_forceLength;
    }

    const BodiesAlongTime& GetBodiesAlongTime () const;
    const BodyAlongTime& GetBodyAlongTime (size_t bodyId) const;
    /**
     * Gets the currently displayed data
     */
    const Foam& GetCurrentFoam () const;
    Foam& GetCurrentFoam ();

    /**
     * Gets the index of the currently displayed data.
     */
    size_t GetTimeStep () const
    {
	return m_timeStep;
    }
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
    bool IsContextHidden () const
    {
	return m_contextHidden;
    }

    boost::shared_ptr<QAction> GetActionResetTransformation ()
    {
	return m_actionResetTransformation;
    }
    boost::shared_ptr<QAction> GetActionResetSelectedLightNumber ()
    {
	return m_actionResetSelectedLightNumber;
    }
    
    /**
     * Calculates and does the viewport transform.
     */
    void ModelViewTransform (ViewNumber::Enum viewNumber, 
			     size_t timeStep) const;
    BodyProperty::Enum GetBodyProperty () const
    {
	return GetBodyProperty (GetViewNumber ());
    }

    BodyProperty::Enum GetBodyProperty (ViewNumber::Enum viewNumber) const;

    G3D::Rect2D GetViewRect (ViewNumber::Enum viewNumber) const;
    G3D::Rect2D GetViewRect () const
    {
	return GetViewRect (GetViewNumber ());
    }

    void SetStatus (QLabel* labelStatusBar)
    {
	m_labelStatusBar = labelStatusBar;
    }

Q_SIGNALS:
    void PaintedGL ();
    void EditColorMap ();
    void ViewChanged ();
    void ColorBarModelChanged (boost::shared_ptr<ColorBarModel> colorBarModel);

public Q_SLOTS:
    /*
     * Global options
     */
    void ToggledTimeStepShown (bool checked);
    void ToggledStandaloneElementsShown (bool checked);
    void ToggledAxesShown (bool checked);
    void ToggledBoundingBoxShown (bool checked);
    void ToggledBodiesBoundingBoxesShown (bool checked);
    void ToggledAverageAroundBody (bool checked);
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
    void ToggledFaceCenterShown (bool checked);
    void ToggledEdgesTessellation (bool checked);
    void ButtonClickedViewType (int id);

    void ToggledFacesShowEdges (bool checked);
    void ToggledLightNumberShown (bool checked);
    void ToggledLightEnabled (bool checked);
    void ToggledIsContextHidden (bool checked);
    void ToggledTorusOriginalDomainShown (bool checked);
    void ToggledT1sShown (bool checked);
    void ToggledTitleShown (bool checked);
    void ToggledT1sShiftLower (bool checked);
    void ToggledTorusOriginalDomainClipped (bool checked);
    void ToggledMissingPressureShown (bool checked);
    void ToggledMissingVolumeShown (bool checked);

    void SetBodyProperty (
	boost::shared_ptr<ColorBarModel> colorBarModel,
	BodyProperty::Enum property);
    void SetColorBarModel (boost::shared_ptr<ColorBarModel> colorBarModel);
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
    void ValueChangedForceLength (int index);
    void ValueChangedHighlightLineWidth (int newWidth);
    // Actions
    void ResetTransformation ();
    void ResetSelectedLightNumber ();
    void SelectBodiesByIdList ();
    void SelectAll ();
    void DeselectAll ();
    void AverageAroundBody ();
    void AverageAroundConstraint ();
    void AverageAroundReset ();
    void ContextDisplayBody ();
    void ContextDisplayReset ();
    void ContextStationaryFoam ();
    void ContextStationaryReset ();
    void InfoPoint ();
    void InfoEdge ();
    void InfoFace ();
    void InfoBody ();
    void InfoFoam ();
    void InfoOpenGL ();
    // Actions color bar
    void EditColorMapDispatch ();
    void ColorBarClampClear ();
    void CopyTransformationFrom (int viewNumber);
    void CopySelectionFrom (int viewNumber);
    void CopyColorBarFrom (int viewNumber);

public:
    const static  size_t DISPLAY_ALL;
    const static size_t QUADRIC_SLICES;
    const static size_t QUADRIC_STACKS;

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
    bool isColorBarUsed (ViewNumber::Enum view) const;


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

    typedef boost::unordered_map<G3D::Vector3int16, QColor,
				 Vector3int16Hash> EndLocationColor;
    typedef void (GLWidget::* ViewTypeDisplay) (ViewNumber::Enum view) const;

private:
    void setLight (int sliderValue, int maximumValue, 
		   LightType::Enum lightType, ColorNumber::Enum colorNumber);
    void viewportTransform (ViewNumber::Enum viewNumber) const;
    void setView (const G3D::Vector2& clickedPoint);
    void selectView (const G3D::Vector2& clickedPoint);
    double getViewXOverY () const;
    static G3D::Rect2D getViewColorBarRect (const G3D::Rect2D& viewRect);
    double getMinimumEdgeRadius () const;
    void calculateEdgeRadius (
	double edgeRadiusMultiplier,
	double* edgeRadius, double* arrowBaseRadius, 
	double* arrowHeight, double* edgeWidth) const;
    void displayContextMenuPos (ViewNumber::Enum viewNumber) const;
    void displayBodyCenters (ViewNumber::Enum viewNumber, 
			     bool useZPos = false) const;
    void displayFaceCenters (ViewNumber::Enum viewNumber) const;
    void displayViewDecorations (ViewNumber::Enum view);
    void displayViewGrid ();
    void displayTextureColorBar (ViewNumber::Enum viewNumber, 
				 const G3D::Rect2D& viewRect);
    void displayViewTitle (const G3D::Rect2D& viewRect, ViewNumber::Enum view);
    void displayViewTimeStep (const G3D::Rect2D& viewRect);
    void displayViewGrid () const;

    ViewType::Enum changeViewType (bool checked, ViewType::Enum newViewType);
    /**
     * Setup the viewing volume first centered around origin and then 
     * translated toward negative Z with m_cameraDistance.
     */
    void projectionTransform (ViewNumber::Enum viewNumber) const;
    /**
     * First translate the data to be centered around origin, then
     * rotate and then translate toward negative Z with
     * m_cameraDistance
     */
    void displayLightDirection (
	ViewNumber::Enum viewNumber, LightNumber::Enum light) const;
    void displayLightDirection (ViewNumber::Enum viewNumber) const;
    G3D::AABox calculateCenteredViewingVolume (
	double xOverY, double scaleRatio) const;
    G3D::AABox calculateViewingVolume (
	ViewNumber::Enum viewNumber, double xOverY, double scaleRatio) const;

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
	bool adjustForContextStationaryFoam = false) const;
    void displayAverageAroundBody (ViewNumber::Enum view) const;
    void displayAverageAroundConstraint (
	ViewNumber::Enum view, 
	bool adjustForContextStationaryFoam = false) const;

    void displayEdgesNormal (ViewNumber::Enum view) const;
    template<typename displayEdge>
    void displayStandaloneEdges (bool useZPos = false, double zPos = 0) const;
    void displayStandaloneFaces () const;
    void displayEdgesTorus (ViewNumber::Enum view) const;
    void displayEdgesTorusTubes () const;
    void displayEdgesTorusLines () const;

    /**
     * @todo display concave filled polygons using the stencil buffer.
     * @see Chapter 14, OpenGL Programming Guide, version 1.1
     */
    void displayFacesNormal (ViewNumber::Enum view) const;
    void displayFacesTorus (ViewNumber::Enum view) const;
    void displayFacesStatistics (ViewNumber::Enum view) const;
    void displayFacesTorusTubes () const;
    void displayFacesTorusLines () const;

    /**
     * Generates a display list for center paths
     */
    void displayCenterPathsWithBodies (ViewNumber::Enum view) const;
    void displayOriginalDomain () const;
    void displayT1s (ViewNumber::Enum view) const;
    void displayT1s (ViewNumber::Enum view, size_t timeStep) const;
    void displayT1sGlobal (ViewNumber::Enum view) const;
    void displayCenterPaths (ViewNumber::Enum view) const;
    void compileCenterPaths (ViewNumber::Enum view) const;
    void compile (ViewNumber::Enum view) const;
    void labelCompileUpdate ();

    void displayBoundingBox (ViewNumber::Enum viewNumber) const;
    void displayFocusBox (ViewNumber::Enum viewNumber) const;
    void displayAxes ();
    G3D::Vector3 getInitialLightPosition (
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
    double ratioFromCenter (const QPoint& p);
    G3D::Matrix3 rotate (ViewNumber::Enum viewNumber, 
			 const QPoint& position, const G3D::Matrix3& rotate);
    void translate (ViewNumber::Enum viewNumber,
	const QPoint& position,
	G3D::Vector3::Axis screenXTranslation,
	G3D::Vector3::Axis screenYTranslation);
    void translateLight (ViewNumber::Enum viewNumber, const QPoint& position);
    void scale (ViewNumber::Enum viewNumber, const QPoint& position);
    void scaleContext (ViewNumber::Enum viewNumber, const QPoint& position);
    void translateAndScale (double scaleRatio, const G3D::Vector3& translation,
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
    
    void initViewTypeDisplay ();
    void createActions ();
    /**
     * Rotates the view so that we get the same image as in Surface Evolver
     */
    void rotate3D () const;
    void rotate2DTimeDisplacement () const;
    void rotate2DRight90 () const;
    string getBodySelectorLabel ();
    string getContextLabel ();
    string getContextStationaryLabel ();
    string getAverageAroundBody ();
    void setLabel ();
    void transformFoamStationary (
	ViewNumber::Enum viewNumber, size_t timeStep) const;
    void initCopy (
	boost::array<boost::shared_ptr<QAction>, 
	ViewNumber::COUNT>& actionCopyTransformation,
	boost::shared_ptr<QSignalMapper>& signalMapperCopyTransformation);
    void rotateAverageAroundConstraint (size_t timeStep, int direction) const;
    void valueChanged (
	double* dest, const pair<double,double>& minMax, int index);
    string infoSelectedBody () const;
    string infoSelectedBodies () const;

private:
    /**
     * Setup lighting for displaying faces edges and vertices
     */
    static void quadricErrorCallback (GLenum errorCode);
    
private:
    const static double ENCLOSE_ROTATION_RATIO;
    // Min, max values for T1s, Context alpha, force length
    const static pair<double,double> T1_SIZE;
    const static pair<double,double> CONTEXT_ALPHA;
    const static pair<double,double> FORCE_LENGTH;
    const static GLfloat HIGHLIGHT_LINE_WIDTH;

private:
    Q_OBJECT

    /**
     * What do we display
     */
    bool m_torusOriginalDomainDisplay;
    bool m_torusOriginalDomainClipped;
    InteractionMode::Enum m_interactionMode;

    /**
     * Foam to be displayd. Each element coresponds to a DMP file
     */
    FoamAlongTime* m_foamAlongTime;
    /**
     * Index into m_foam that shows the current DMP file displayed
     */
    size_t m_timeStep;
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
    bool m_faceCenterShown;
    bool m_edgesTessellation;
    bool m_centerPathBodyShown;
    bool m_contextHidden;
    bool m_boundingBoxShown;
    bool m_bodiesBoundingBoxesShown;
    bool m_axesShown;
    bool m_standaloneElementsShown;
    boost::array<ViewTypeDisplay, ViewType::COUNT> m_viewTypeDisplay;

    boost::shared_ptr<QAction> m_actionSelectAll;
    boost::shared_ptr<QAction> m_actionDeselectAll;
    boost::shared_ptr<QAction> m_actionResetTransformation;
    boost::shared_ptr<QAction> m_actionResetSelectedLightNumber;
    boost::shared_ptr<QAction> m_actionSelectBodiesById;

    boost::shared_ptr<QAction> m_actionAverageAroundBody;
    boost::shared_ptr<QAction> m_actionAverageAroundConstraint;
    boost::shared_ptr<QAction> m_actionAverageAroundReset;

    boost::shared_ptr<QAction> m_actionContextDisplayBody;
    boost::shared_ptr<QAction> m_actionContextDisplayReset;
    boost::shared_ptr<QAction> m_actionContextStationaryFoam;
    boost::shared_ptr<QAction> m_actionContextStationaryReset;

    boost::shared_ptr<QAction> m_actionInfoPoint;
    boost::shared_ptr<QAction> m_actionInfoEdge;
    boost::shared_ptr<QAction> m_actionInfoFace;
    boost::shared_ptr<QAction> m_actionInfoBody;
    boost::shared_ptr<QAction> m_actionInfoFoam;
    boost::shared_ptr<QAction> m_actionInfoOpenGL;
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
    double m_t1Size;
    double m_contextAlpha;
    double m_forceLength;
    size_t m_highlightLineWidth;
    bool m_missingPressureShown;
    bool m_missingVolumeShown;
    bool m_titleShown;
    bool m_timeStepShown;
    bool m_averageAroundBody;

    // View related variables
    ViewCount::Enum m_viewCount;
    ViewLayout::Enum m_viewLayout;
    ViewNumber::Enum m_viewNumber;
    boost::array<
	boost::shared_ptr<ViewSettings>, ViewNumber::COUNT> m_viewSettings;
};

#endif //__GLWIDGET_H__

// Local Variables:
// mode: c++
// End:
