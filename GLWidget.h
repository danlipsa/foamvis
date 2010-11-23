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
class BodySelector;
class ColorBarModel;
class DisplayBlend;
class DisplayFaceAverage;
class Foam;
class Edge;
class EditColorMap;
class FoamAlongTime;
class OrientedFace;
class OrientedEdge;
class OOBox;

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
    void SetBodySelector (boost::shared_ptr<BodySelector> bodySelector)
    {
	m_bodySelector = bodySelector;
	updateGL ();
    }
    const BodySelector& GetBodySelector () const
    {
	return *m_bodySelector;
    }

    StatisticsType::Enum GetStatisticsType () const
    {
	return m_statisticsType;
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
    double GetTimeDisplacement () const
    {
	return m_timeDisplacement;
    }
    
    bool AreAllBodiesDisplayed () const
    {
	return m_displayedBodyIndex == DISPLAY_ALL;
    }
    size_t GetDisplayedBodyId () const;
    boost::shared_ptr<Body>  GetDisplayedBody () const;
    /**
     * Gets the face number up to which faces are displayed
     * @return face number up to which faces are displayed or UINT_MAX for all
     */
    size_t GetDisplayedFaceIndex () const
    {
	return m_displayedFaceIndex;
    }
    size_t GetDisplayedFaceId () const;
    boost::shared_ptr<Face>  GetDisplayedFace () const;

    size_t GetDisplayedEdgeIndex () const
    {
	return m_displayedEdgeIndex;
    }
    size_t GetDisplayedEdgeId () const;
    boost::shared_ptr<Edge>  GetDisplayedEdge () const;
    /**
     * Increment displayed body
     */
    void IncrementDisplayedBody ();
    /**
     * Increment displayed face
     */
    void IncrementDisplayedFace ();
    void IncrementDisplayedEdge ();

    /**
     * Decrement displayed body
     */
    void DecrementDisplayedBody ();
    /**
     * Decrement displayed face
     */
    void DecrementDisplayedFace ();
    void DecrementDisplayedEdge ();
    double GetContextAlpha () const
    {
	return m_contextAlpha;
    }

    const QColor& GetEndTranslationColor (const G3D::Vector3int16& di) const;
    const QColor& GetCenterPathNotAvailableColor () const
    {
	return m_notAvailableCenterPathColor;
    }
    const QColor& GetNotAvailableFaceColor () const
    {
	return m_notAvailableFaceColor;
    }
    QColor GetCenterPathContextColor () const;
    QColor GetCenterPathDefaultColor () const
    {
	return Qt::black;
    }

    GLUquadricObj* GetQuadricObject () const 
    {
	return m_quadric;
    }

    double GetArrowBaseRadius () const {return m_arrowBaseRadius;}
    double GetArrowHeight () const {return m_arrowHeight;}
    double GetEdgeRadius () const {return m_edgeRadius;}

    bool IsDisplayedBody (const boost::shared_ptr<Body>  body) const;
    bool IsDisplayedBody (size_t bodyId) const;
    bool IsDisplayedFace (size_t faceI) const;
    bool IsDisplayedEdge (size_t edgeI) const;
    bool IsEdgesTessellation () const
    {
	return m_edgesTessellation;
    }
    bool IsCenterPathBodyShown () const
    {
	return m_centerPathBodyShown;
    }
    bool OnlyPathsWithSelectionShown () const
    {
	return m_onlyPathsWithSelectionShown;
    }

    double TexCoord (double value) const;

    boost::shared_ptr<QAction> GetActionResetTransformation ()
    {
	return m_actionResetTransformation;
    }
    void SetActionSelectAll (boost::shared_ptr<QAction> actionSelectAll);
    void SetActionDeselectAll (boost::shared_ptr<QAction> actionDeselectAll);
    void SetActionInfo (boost::shared_ptr<QAction> actionInfo);
    GLuint GetColorBarTexture () const
    {
	return m_colorBarTexture;
    }
    bool IsPlayMovie () const
    {
	return m_playMovie;
    }
    void SetPlayMovie (bool playMovie);
    
    /**
     * Calculates and does the viewport transform.
     * @param viewport stores the viewport.
     */
    void ViewportTransform (int width, int height, double scale = 1,
			    G3D::Rect2D* viewport = 0) const;
    void ModelViewTransformNoRotation () const;
    void RenderFromFbo (QGLFramebufferObject& fbo) const;
    /**
     * Displays the foam in various way
     * @param type the type of object that we want displayed.
     */
    void DisplayViewType () const;
    double GetSrcAlphaBlend () const
    {
	return m_srcAlphaBlend;
    }
    BodyProperty::Enum GetFacesColor () const
    {
	return m_facesColor;
    }
    void toggledLightingEnabled (bool checked);

Q_SIGNALS:
    void PaintedGL ();

public Q_SLOTS:
    /*
     * Global options
     */
    void ToggledBoundingBoxShown (bool checked);
    void ToggledAxesShown (bool checked);
    void ToggledTorusOriginalDomainShown (bool checked);
    void ToggledLightPositionShown (bool checked);
    void ToggledDirectionalLightEnabled (bool checked);
    void ToggledFullColorBarShown (bool checked);


    /**
     * Shows edges
     * @param checked true for showing edges false otherwise
     */
    void ToggledEdgesNormal (bool checked);
    void ToggledEdgesTorus (bool checked);
    /**
     * Shows faces
     * @param checked true for showing faces false otherwise
     */
    void ToggledFacesNormal (bool checked);
    void ToggledFaceEdgesTorus (bool checked);
    void ToggledFacesAverage (bool checked);

    /**
     * Shows center paths
     * param checked true for showing the center paths false otherwise
     */
    void ToggledCenterPath (bool checked);
    void ToggledCenterPathBodyShown (bool checked);
    void ToggledOnlyPathsWithSelectionShown (bool checked);
    void BodyPropertyChanged (
	boost::shared_ptr<ColorBarModel> colorBarModel,
	BodyProperty::Enum bodyProperty, ViewType::Enum viewType);
    void ColorBarModelChanged (
	boost::shared_ptr<ColorBarModel> colorBarModel);
    void ResetTransformation ();
    void SelectAll ();
    void DeselectAll ();
    void Info ();
    void CurrentIndexChangedInteractionMode (int index);
    void CurrentIndexChangedStatisticsType (int index);
    void CurrentIndexChangedViewportTransformType (int index);
    void CurrentIndexChangedAxesOrder (int index);

    void ToggledEdgesBodyCenter (bool checked);
    void ToggledEdgesTessellation (bool checked);
    void ToggledFacesShowEdges (bool checked);


    void ToggledTorusOriginalDomainClipped (bool checked);

    void ValueChangedAngleOfView (int newIndex);
    void ValueChangedBlend (int index);
    void ValueChangedTimeDisplacement (int timeDisplacement);
    void ValueChangedEdgesRadius (int sliderValue);
    void ShowOpenGLInfo ();
    /**
     * Signals a change in data displayed
     * @param timeStep the new index for the Foam to be displayed
     */
    void ValueChangedSliderTimeSteps (int timeStep);
    void ButtonClickedLightPosition (int lightPosition);

public:
    const static  size_t DISPLAY_ALL;
    const static size_t QUADRIC_SLICES = 5;
    const static size_t QUADRIC_STACKS = 1;
    const static size_t LIGHTS_COUNT = 4;

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

    typedef boost::unordered_map<G3D::Vector3int16, QColor,
				 Vector3int16Hash> EndLocationColor;
    struct ViewTypeDisplay
    {
	void (GLWidget::* m_display) () const;
	boost::function<Lighting ()> m_lightingEnabled;
    };

private:
    static double getMinimumEdgeRadius ();
    void setEdgeRadius ();
    static void calculateEdgeRadius (
	double edgeRadiusMultiplier, double* edgeRadius,
	double* arrowBaseRadius, double* arrowHeight, bool* edgeTubes = 0);
    void initStepDisplayAverage ();
    void display () const;
    string getFoamsInfo () const;

    /**
     * Displays the center of the bodies
     */
    void displayCenterOfBodies (bool useZPos = false) const;
    void displayTextureColorBar () const;
    bool areEdgesTubes () const
    {
	return m_edgesTubes;
    }
    bool edgeLighting () const;
    bool hasLighting () const
    {
	return m_lightingEnabled;
    }

    void view (bool checked, ViewType::Enum view);
    /**
     * Setup the viewing volume first centered around origin and then translated
     * toward negative Z with m_cameraDistance. 
     * It is only done on initializeGL ();
     */
    void projectionTransform () const;
    /**
     * First translate the data to be centered around origin, then
     * rotate and then translate toward negative Z with
     * m_cameraDistance
     */
    void modelViewTransform () const;
    void positionLight ();
    void viewingVolumeCalculations (
	int width, int height,
	G3D::Rect2D* vv2dScreen, G3D::Rect2D* windowWorld) const;
    void boundingBoxCalculations (
	int width, int height, 
	const G3D::Rect2D& windowWorld, G3D::Rect2D* bb2dScreen, 
	double* change) const;
    G3D::AABox calculateCenteredViewingVolume () const;
    void initializeTextures ();
    void initQuadrics ();
    void calculateCameraDistance ();
    /**
     * Generates a display list for edges
     * @return the display list
     */
    template<typename displayEdge>
    void displayEdges () const;


    void displayEdgesNormal () const;
    template<typename displayEdge>
    void displayStandaloneEdges (bool useZPos = false, double zPos = 0) const;
    template<typename displaySameEdges>
    void displayStandaloneFaces () const;
    void displayEdgesTorus () const;
    void displayEdgesTorusTubes () const;
    void displayEdgesTorusLines () const;

    /**
     * @todo display concave filled polygons using the stencil buffer.
     * @see Chapter 14, OpenGL Programming Guide, version 1.1
     */
    void displayFacesNormal () const;
    void displayFacesTorus () const;
    void displayFacesAverage () const;
    void displayFacesTorusTubes () const;
    void displayFacesTorusLines () const;

    /**
     * Generates a display list for center paths
     */
    void displayCenterPathsWithBodies () const;
    void displayOriginalDomain () const;
    void displayBoundingBox () const;
    void displayCenterPaths () const;
    void displayBox (const G3D::AABox& aabb, const QColor& color, 
		     GLenum polygonMode) const;
    void displayBox (const OOBox& oobox) const;
    void displayAxes () const;
    void setInitialLightPosition ();
    G3D::Vector3 getInitialLightPosition (size_t i) const;

    /**
     * Rotates the foam or the light around an axis with a certain angle
     * @param axis can be 0, 1 or 2 for X, Y or Z
     * @param angle angle we rotate the foam with
     */
    static void setRotation (int axis, double angle, G3D::Matrix3* rotate);
    /**
     * Displays   the   contour   of   faces.   Used   together   with
     * displayFacesOffet   and  with  GL_POLYGON_OFFSET_FILL   to  get
     * continuous contours  for polygons.
     * @param bodies displays all the faces in these bodies
     */
    template<typename displaySameEdges>
    void displayFacesContour (
	const vector< boost::shared_ptr<Body> >& bodies) const;
    template<typename displaySameEdges>
    void displayFacesContour (
	const vector< boost::shared_ptr<Face> >& faces) const;
    /**
     * Displays   the   content   of   faces.   Used   together   with
     * displayFacesContour  and  with  GL_POLYGON_OFFSET_FILL  to  get
     * continuous contours for polygons.
     * @param bodies displays all the faces in these bodies
     */
    template<typename displaySameEdges>
    void displayFacesInterior (
	const vector<boost::shared_ptr<Body> >& bodies) const;
    template<typename displaySameEdges>
    void displayFacesInterior (
	const vector<boost::shared_ptr<Face> >& faces) const;

    /**
     * Setup lighting for shaded bodies
     * See OpenGL FAQ 21.030 Why doesn't lighting work when I turn on 
     * texture mapping? (use GL_MODULATE instead of GL_DECAL or GL_REPLACE for
     * glTexEnv)
     * See OpenGL FAQ 21.040 Lighting and texture mapping work pretty well, 
     * but why don't I see specular highlighting? (use
     * glLightModel (GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR))
     */
    void initializeLighting ();
    double ratioFromCenter (const QPoint& p);
    void rotate (const QPoint& position, G3D::Matrix3* rotate);
    void translateViewport (const QPoint& position);
    void translateLight (const QPoint& position);
    void scaleViewport (const QPoint& position);
    void initViewTypeDisplay ();

    bool doesSelectBody () const;
    bool doesSelectFace () const;
    bool doesSelectEdge () const;
    void createActions ();
    /**
     * Rotates the view so that we get the same image as in Surface Evolver
     */
    void rotate3D () const;
    void rotate2DTimeDisplacement () const;
    void rotate2DRight90 () const;
    void toggledLights ();

private:
    static void displayOpositeFaces (G3D::Vector3 origin,
				     G3D::Vector3 faceFirst,
				     G3D::Vector3 faceSecond,
				     G3D::Vector3 translation);

    /**
     * Setup lighting for displaying faces edges and vertices
     */
    static void quadricErrorCallback (GLenum errorCode);
    
private:
    Q_OBJECT

    /**
     * The elements displayed from a DMP file: vertices, edges, faces or bodies.
     */
    ViewType::Enum m_viewType;
    bool m_torusOriginalDomainDisplay;
    bool m_torusOriginalDomainClipped;
    InteractionMode::Enum m_interactionMode;
    StatisticsType::Enum m_statisticsType;
    ViewportTransformType::Enum m_viewportTransformType;
    AxesOrder::Enum m_axesOrder;

    /**
     * Foam to be displayd. Each element coresponds to a DMP file
     */
    FoamAlongTime* m_foamAlongTime;
    /**
     * Index into m_foam that shows the current DMP file displayed
     */
    size_t m_timeStep;
    /**
     * Used for rotating the view
     */
    QPoint m_lastPos;
    /**
     * Used to display one body at a time
     */
    size_t m_displayedBodyIndex;
    /**
     * Used to display one face at a time from the m_displayedBodyIndex.
     */
    size_t m_displayedFaceIndex;
    size_t m_displayedEdgeIndex;

    double m_contextAlpha;

    G3D::Matrix3 m_rotationMatrixModel;
    G3D::Rect2D m_viewport;
    double m_scalingFactorModel;
    /**
     * Distance from the camera to the center of the bounding box for the foam.
     */
    double m_cameraDistance;

    bool m_lightingEnabled;
    bool m_directionalLightEnabled;
    G3D::Matrix3 m_rotationMatrixLight;
    double m_lightPositionRatio;
    bool m_showLightPosition;
    double m_angleOfView;

    EndLocationColor m_endTranslationColor;
    GLUquadricObj* m_quadric;    
    /**
     * For displaying edges as tubes
     */
    double m_edgeRadius;
    /**
     * For displaying arrows in the Torus Model edges
     */
    double m_arrowBaseRadius;
    double m_arrowHeight;
    double m_edgeRadiusMultiplier;

    bool m_edgesTubes;
    bool m_facesShowEdges;
    bool m_edgesBodyCenter;
    bool m_edgesTessellation;
    bool m_centerPathBodyShown;
    bool m_onlyPathsWithSelectionShown;
    bool m_boundingBoxShown;
    bool m_axesShown;
    bool m_textureColorBarShown;
    boost::array<ViewTypeDisplay, ViewType::COUNT> m_viewTypeDisplay;
    BodyProperty::Enum m_centerPathColor;
    BodyProperty::Enum m_facesColor;
    QColor m_notAvailableCenterPathColor;
    QColor m_notAvailableFaceColor;
    boost::shared_ptr<const BodySelector> m_bodySelector;

    // owned by MainWindows
    boost::shared_ptr<QAction> m_actionSelectAll;
    boost::shared_ptr<QAction> m_actionDeselectAll;
    boost::shared_ptr<QAction> m_actionInfo;
    // owned by GLWidget
    boost::shared_ptr<QAction> m_actionResetTransformation;
    bool m_useColorMap;
    boost::shared_ptr<ColorBarModel> m_colorBarModel;
    GLuint m_colorBarTexture;
    double m_srcAlphaBlend;
    double m_timeDisplacement;
    /**
     * True if the program displays data in a loop, false
     * otherwise
     */
    bool m_playMovie;
    boost::scoped_ptr<DisplayBlend> m_displayBlend;
    boost::scoped_ptr<DisplayFaceAverage> m_displayFaceAverage;
    bitset<LIGHTS_COUNT> m_lightEnabled;
};

#endif //__GLWIDGET_H__

// Local Variables:
// mode: c++
// End:
