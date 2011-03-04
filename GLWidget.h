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
class DisplayFaceStatistics;
class Foam;
class Edge;
class EditColorMap;
class FoamAlongTime;
class OrientedFace;
class OrientedEdge;
class OOBox;
class SelectBodiesById;
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
    ViewType::Enum GetViewType () const
    {
	return m_viewType;
    }

    QColor GetHighlightColor (size_t i) const;
    void SetBodySelector (boost::shared_ptr<PropertyValueBodySelector> selector);
    void SetBodySelector (boost::shared_ptr<IdBodySelector> bodySelector);
    void SetBodySelector (boost::shared_ptr<AllBodySelector> selector, 
			  BodySelectorType::Enum type);
    void UnionBodySelector (const vector<size_t>& bodyIds);
    void DifferenceBodySelector (const vector<size_t>& bodyIds);

    const BodySelector& GetBodySelector () const
    {
	return *m_bodySelector;
    }

    StatisticsType::Enum GetStatisticsType () const
    {
	return m_statisticsType;
    }
    pair<double, double> getStatisticsMinMax () const;

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
    size_t GetStationaryBodyId () const
    {
	return m_stationaryBodyId;
    }
    bool IsStationaryBodyContext (size_t bodyId) const
    {
	return m_stationaryBodyContext.find (bodyId) != 
	    m_stationaryBodyContext.end ();
    }

    bool IsTimeDisplacementUsed () const;

    double GetTimeDisplacement () const
    {
	return m_timeDisplacement;
    }
    
    bool AllBodiesSelected () const
    {
	return m_selectedBodyIndex == DISPLAY_ALL;
    }
    size_t GetSelectedBodyId () const;
    boost::shared_ptr<Body>  GetSelectedBody () const;
    /**
     * Gets the face number up to which faces are displayed
     * @return face number up to which faces are displayed or UINT_MAX for all
     */
    size_t GetSelectedFaceIndex () const
    {
	return m_selectedFaceIndex;
    }
    size_t GetSelectedFaceId () const;
    boost::shared_ptr<Face>  GetSelectedFace () const;

    size_t GetSelectedEdgeIndex () const
    {
	return m_selectedEdgeIndex;
    }
    size_t GetSelectedEdgeId () const;
    boost::shared_ptr<Edge>  GetSelectedEdge () const;
    /**
     * Increment displayed body
     */
    void IncrementSelectedBodyIndex ();
    /**
     * Increment displayed face
     */
    void IncrementSelectedFaceIndex ();
    void IncrementSelectedEdgeIndex ();

    /**
     * Decrement displayed body
     */
    void DecrementSelectedBodyIndex ();
    /**
     * Decrement displayed face
     */
    void DecrementSelectedFaceIndex ();
    void DecrementSelectedEdgeIndex ();
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
    double GetArrowHeight () const {return m_arrowHeight;}
    double GetEdgeRadius () const {return m_edgeRadius;}

    bool IsDisplayedBody (const boost::shared_ptr<Body>  body) const;
    bool IsDisplayedBody (size_t bodyId) const;
    bool IsDisplayedFace (size_t faceI) const;
    bool IsDisplayedEdge (size_t edgeI) const;
    bool IsLightEnabled (LightPosition::Enum i)
    {
	return m_lightEnabled[i];
    }
    bool IsDirectionalLightEnabled (LightPosition::Enum i)
    {
	return m_directionalLightEnabled [i];
    }
    bool IsLightPositionShown (LightPosition::Enum i)
    {
	return m_lightPositionShown[i];
    }
    const boost::array<GLfloat, 4> GetLightAmbient (LightPosition::Enum i)
    {
	return m_lightAmbient[i];
    }
    const boost::array<GLfloat, 4> GetLightDiffuse (LightPosition::Enum i)
    {
	return m_lightDiffuse[i];
    }
    const boost::array<GLfloat, 4> GetLightSpecular (LightPosition::Enum i)
    {
	return m_lightSpecular[i];
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
    boost::shared_ptr<QAction> GetActionResetSelectedLightPosition ()
    {
	return m_actionResetSelectedLightPosition;
    }

    GLuint GetColorBarTexture () const
    {
	return m_colorBarTexture;
    }

    const ColorBarModel& GetColorBarModel () const
    {
	return *m_colorBarModel;
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
    void ViewportTransform ();
    void ModelViewTransform (size_t timeStep) const;
    void RenderFromFbo (QGLFramebufferObject& fbo) const;
    /**
     * Displays the foam in various way
     * @param type the type of object that we want displayed.
     */
    void DisplayViewType () const;
    BodyProperty::Enum GetBodyProperty () const
    {
	return m_bodyProperty;
    }

Q_SIGNALS:
    void PaintedGL ();
    void EditColorMap ();
    void ColorBarModelChanged (boost::shared_ptr<ColorBarModel> colorBarModel);

public Q_SLOTS:
    /*
     * Global options
     */
    void ToggledAxesShown (bool checked);
    void ToggledBoundingBoxShown (bool checked);
    void ToggledBodiesBoundingBoxesShown (bool checked);
    /**
     * Shows center paths
     * param checked true for showing the center paths false otherwise
     */
    void ToggledCenterPath (bool checked);
    void ToggledCenterPathBodyShown (bool checked);
    void ToggledContextView (bool checked);
    void ToggledDirectionalLightEnabled (bool checked);
    /**
     * Shows edges
     * @param checked true for showing edges false otherwise
     */
    void ToggledEdgesNormal (bool checked);
    void ToggledEdgesTorus (bool checked);
    void ToggledEdgesBodyCenter (bool checked);
    void ToggledEdgesTessellation (bool checked);
    /**
     * Shows faces
     * @param checked true for showing faces false otherwise
     */
    void ToggledFacesNormal (bool checked);
    void ToggledFacesStatistics (bool checked);
    void ToggledFaceEdgesTorus (bool checked);
    void ToggledFacesShowEdges (bool checked);

    void ToggledHideContent (bool checked);
    void ToggledLightPositionShown (bool checked);
    void ToggledLightEnabled (bool checked);
    void ToggledIsContextHidden (bool checked);
    void ToggledTorusOriginalDomainShown (bool checked);
    void ToggledT1sShown (bool checked);
    void ToggledT1sShiftLower (bool checked);
    void ToggledTorusOriginalDomainClipped (bool checked);
    void ToggledTubeCenterPathUsed (bool checked);

    void SetBodyProperty (
	boost::shared_ptr<ColorBarModel> colorBarModel,
	BodyProperty::Enum property);
    void SetColorBarModel (boost::shared_ptr<ColorBarModel> colorBarModel);

    void CurrentIndexChangedInteractionMode (int index);
    void CurrentIndexChangedStatisticsType (int index);
    void CurrentIndexChangedAxesOrder (int index);
    void CurrentIndexChangedSelectedLight (int selectedLight);

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
    void ValueChangedStatisticsHistory (int timeSteps);
    void ValueChangedTimeDisplacement (int timeDisplacement);
    // Actions
    void ResetTransformation ();
    void ResetSelectedLightPosition ();
    void SelectBodiesByIdList ();
    void SelectAll ();
    void DeselectAll ();
    void StationarySet ();
    void StationaryReset ();
    void StationaryContextAdd ();
    void InfoFocus ();
    void InfoFoam ();
    void InfoOpenGL ();
    // Actions color bar
    void ColorBarEdit ();
    void ColorBarClampClear ();

    /**
     * Signals a change in data displayed
     * @param timeStep the new index for the Foam to be displayed
     */
    void SetStatus (QLabel* labelStatusBar)
    {
	m_labelStatusBar = labelStatusBar;
    }

public:
    const static  size_t DISPLAY_ALL;
    const static size_t NONE;
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

    typedef void (GLWidget::* ViewTypeDisplay) () const;

private:
    void enableLighting (bool checked);
    double getMinimumEdgeRadius () const;
    void setEdgeRadius ();
    void calculateEdgeRadius (
	double edgeRadiusMultiplier,
	double* edgeRadius, double* arrowBaseRadius, 
	double* arrowHeight, bool* edgeTubes = 0) const;

    /**
     * Displays the center of the bodies
     */
    void displayCenterOfBodies (bool useZPos = false) const;
    void displayViewDecorations ();
    void displayTextureColorBar ();
    void displayViewTitle ();
    bool areEdgesTubes () const
    {
	return m_edgesTubes;
    }
    bool isLightingEnabled () const
    {
	return m_lightingEnabled;
    }

    void changeView (bool checked, ViewType::Enum view);
    /**
     * Setup the viewing volume first centered around origin and then translated
     * toward negative Z with m_cameraDistance. 
     */
    void projectionTransform ();
    /**
     * First translate the data to be centered around origin, then
     * rotate and then translate toward negative Z with
     * m_cameraDistance
     */
    void positionLight (LightPosition::Enum light);
    void positionLights ();
    void showLightPosition (LightPosition::Enum light) const;
    void showLightPositions () const;
    void viewingVolumeCalculations (
	int width, int height,
	G3D::Rect2D* vv2dScreen, G3D::Rect2D* windowWorld) const;
    void boundingBoxCalculations (
	int width, int height, 
	const G3D::Rect2D& windowWorld, G3D::Rect2D* bb2dScreen, 
	double* change) const;
    G3D::AABox calculateCenteredViewingVolume (double xOverY) const;
    G3D::AABox calculateViewingVolume (double xOverY) const;

    void initializeTextures ();
    void initQuadrics ();
    void initEndTranslationColor ();
    void calculateCameraDistance ();
    /**
     * Generates a display list for edges
     * @return the display list
     */
    template<typename displayEdge>
    void displayEdges () const;
    void displayStationaryBodyAndContext () const;

    void displayEdgesNormal () const;
    template<typename displayEdge>
    void displayStandaloneEdges (bool useZPos = false, double zPos = 0) const;
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
    void displayFacesStatistics () const;
    void displayFacesTorusTubes () const;
    void displayFacesTorusLines () const;

    /**
     * Generates a display list for center paths
     */
    void displayCenterPathsWithBodies () const;
    void displayOriginalDomain () const;
    void displayT1s () const;
    void displayT1s (size_t timeStep) const;
    void displayT1sGlobal () const;
    void displayCenterPaths () const;
    void compileCenterPaths () const;
    void compile () const;

    void displayBoundingBox () const;
    void displayFocusBox () const;
    void displayAxes () const;
    void setInitialLightPosition (LightPosition::Enum i);
    void setInitialLightParameters ();
    G3D::Vector3 getInitialLightPosition (
	LightPosition::Enum lightPosition) const;

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
    template<size_t highlightColorIndex>
    void displayFacesContour (
	const vector< boost::shared_ptr<Body> >& bodies) const;
    template<size_t highlightColorIndex>
    void displayFacesContour (
	const vector< boost::shared_ptr<Face> >& faces) const;
    /**
     * Displays   the   content   of   faces.   Used   together   with
     * displayFacesContour  and  with  GL_POLYGON_OFFSET_FILL  to  get
     * continuous contours for polygons.
     * @param bodies displays all the faces in these bodies
     */
    void displayFacesInterior (
	const vector<boost::shared_ptr<Body> >& bodies) const;
    void displayFacesInterior (
	const vector<boost::shared_ptr<Face> >& faces) const;

    /**
     * Setup lighting for shaded bodies
     */
    void initializeLighting ();
    double ratioFromCenter (const QPoint& p);
    void rotate (const QPoint& position, G3D::Matrix3* rotate);
    void translate (
	const QPoint& position,
	G3D::Vector3::Axis screenXTranslation,
	G3D::Vector3::Axis screenYTranslation);
    void translateLight (const QPoint& position);
    void scale (const QPoint& position);
    void translateAndScale (
	double m_scaleRatio, const G3D::Vector3& translation,
	bool contextView) const;
    void select (const QPoint& position);
    void deselect (const QPoint& position);
    void brushedBodies (const QPoint& position, vector<size_t>* bodies) const;
    
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
    void setBodySelectorLabel (BodySelectorType::Enum type);
    void setStationaryBodyLabel ();
    void translateFoamStationaryBody (size_t timeStep) const;

private:
    /**
     * Setup lighting for displaying faces edges and vertices
     */
    static void quadricErrorCallback (GLenum errorCode);
    
private:
    const static double MIN_CONTEXT_ALPHA;
    const static double MAX_CONTEXT_ALPHA;
    const static double ENCLOSE_ROTATION_RATIO;

private:
    Q_OBJECT

    /**
     * What do we display
     */
    ViewType::Enum m_viewType;
    bool m_torusOriginalDomainDisplay;
    bool m_torusOriginalDomainClipped;
    InteractionMode::Enum m_interactionMode;
    StatisticsType::Enum m_statisticsType;
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
     * Used for rotation, translation and scale
     */
    QPoint m_lastPos;
    /**
     * Used to select a stationary body
     */
    QPoint m_contextMenuPos;
    /**
     * Used to display one body at a time
     */
    size_t m_selectedBodyIndex;
    /**
     * Used to display one face at a time from the m_selectedBodyIndex.
     */
    size_t m_selectedFaceIndex;
    size_t m_selectedEdgeIndex;
    /**
     * Keep this body stationary during the evolution of the foam
     */
    size_t m_stationaryBodyId;
    size_t m_stationaryBodyTimeStep;
    set<size_t> m_stationaryBodyContext;

    double m_contextAlpha;

    G3D::Matrix3 m_rotationModel;
    G3D::Rect2D m_viewport;
    double m_scaleRatio;
    G3D::Vector3 m_translation;
    /**
     * Distance from the camera to the center of the bounding box for the foam.
     */
    double m_cameraDistance;

    bool m_lightingEnabled;
    LightPosition::Enum m_selectedLight;
    bitset<LightPosition::COUNT> m_lightEnabled;
    bitset<LightPosition::COUNT> m_directionalLightEnabled;
    bitset<LightPosition::COUNT> m_lightPositionShown;    
    boost::array<G3D::Matrix3, LightPosition::COUNT> m_rotationLight;
    boost::array<double, LightPosition::COUNT> m_lightPositionRatio;
    boost::array<boost::array<GLfloat, 4>, LightPosition::COUNT> m_lightAmbient;
    boost::array<boost::array<GLfloat, 4>, LightPosition::COUNT> m_lightDiffuse;
    boost::array<boost::array<GLfloat, 4>, LightPosition::COUNT> m_lightSpecular;

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
    bool m_contextHidden;
    bool m_boundingBoxShown;
    bool m_bodiesBoundingBoxesShown;
    bool m_axesShown;
    boost::array<ViewTypeDisplay, ViewType::COUNT> m_viewTypeDisplay;
    BodyProperty::Enum m_bodyProperty;
    boost::shared_ptr<BodySelector> m_bodySelector;

    boost::shared_ptr<QAction> m_actionSelectAll;
    boost::shared_ptr<QAction> m_actionDeselectAll;
    boost::shared_ptr<QAction> m_actionResetTransformation;
    boost::shared_ptr<QAction> m_actionResetSelectedLightPosition;
    boost::shared_ptr<QAction> m_actionSelectBodiesById;
    boost::shared_ptr<QAction> m_actionStationarySet;
    boost::shared_ptr<QAction> m_actionStationaryReset;
    boost::shared_ptr<QAction> m_actionStationaryContextAdd;
    boost::shared_ptr<QAction> m_actionInfoFocus;
    boost::shared_ptr<QAction> m_actionInfoFoam;
    boost::shared_ptr<QAction> m_actionInfoOpenGL;
    boost::scoped_ptr<QAction> m_actionEditColorMap;
    boost::scoped_ptr<QAction> m_actionClampClear;

    
    boost::shared_ptr<ColorBarModel> m_colorBarModel;
    GLuint m_colorBarTexture;
    double m_timeDisplacement;
    /**
     * True if the program displays data in a loop, false
     * otherwise
     */
    bool m_playMovie;
    boost::scoped_ptr<DisplayFaceStatistics> m_displayFaceStatistics;
    boost::shared_ptr<SelectBodiesById> m_selectBodiesById;
    QLabel *m_labelStatusBar;
    bool m_contextView;
    bool m_hideContent;
    bool m_tubeCenterPathUsed;
    GLuint m_listCenterPaths;
    bool m_t1sShown;
    G3D::Rect2D m_colorBarRect;
};

#endif //__GLWIDGET_H__

// Local Variables:
// mode: c++
// End:
