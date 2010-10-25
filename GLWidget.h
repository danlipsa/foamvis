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
    
    bool IsDisplayedAllBodies () const
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

    void DataChanged () 
    {
	ValueChangedSliderTimeSteps (m_timeStep);
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
    bool IsCenterPathDisplayBody () const
    {
	return m_centerPathDisplayBody;
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
    
public Q_SLOTS:
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
    void ToggledFacesTorus (bool checked);
    /**
     * Shows center paths
     * param checked true for showing the center paths false otherwise
     */
    void ToggledCenterPath (bool checked);
    void ToggledAverage (bool checked);


    void BodyPropertyChanged (
	boost::shared_ptr<ColorBarModel> colorBarModel,
	BodyProperty::Enum bodyProperty, ViewType::Enum viewType);
    void ColorBarModelChanged (
	boost::shared_ptr<ColorBarModel> colorBarModel);
    void ResetTransformation ();
    void ChangePalette ();
    void SelectAll ();
    void DeselectAll ();
    void Info ();
    void CurrentIndexChangedInteractionMode (int index);
    void ToggledBodies (bool checked);
    void ToggledCenterPathDisplayBody (bool checked);

    void ToggledEdgesBodyCenter (bool checked);
    void ToggledEdgesTessellation (bool checked);
    void ToggledEdgesTorusTubes (bool checked);

    void ToggledFacesTorusTubes (bool checked);
    void ToggledFacesShowEdges (bool checked);

    void ToggledTorusOriginalDomainDisplay (bool checked);
    void ToggledTorusOriginalDomainClipped (bool checked);
    void ToggledShowBoundingBox (bool checked);
    void ValueChangedAngleOfView (int newIndex);
    void ValueChangedBlend (int index);
    void ShowOpenGLInfo ();
    /**
     * Signals a change in data displayed
     * @param timeStep the new index for the Foam to be displayed
     */
    void ValueChangedSliderTimeSteps (int timeStep);
    /**
     * Calculates and does the viewport transform.
     * @param viewport stores the viewport. If it is != 0 the function does the
     * viewport transform as well.
     * @return the foam size in screen coordinates.
     */
    QSize ViewportTransform (int width, int height, double scale = 1,
			     G3D::Rect2D* viewport = 0) const;
    void ModelViewTransformNoRotation () const;
    void RenderFromFbo (QGLFramebufferObject& fbo) const;
    /**
     * Displays the foam in various way
     * @param type the type of object that we want displayed.
     */
    void Display () const;
    double GetSrcAlphaBlend () const
    {
	return m_srcAlphaBlend;
    }


public:
    const static  size_t DISPLAY_ALL;
    const static size_t QUADRIC_SLICES;
    const static size_t QUADRIC_STACKS;;

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
	boost::function<Lighting ()> m_lighting;
    };

private:
    /**
     * Displays the center of the bodies
     */
    void displayCenterOfBodies () const;
    void displayTextureColorMap () const;
    bool edgesTorusTubes ()
    {
	return m_edgesTorusTubes;
    }

    bool facesTorusTubes ()
    {
	return m_facesTorusTubes;
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
    void viewingVolumeCalculations (
	int width, int height,
	G3D::Rect2D* vv2dScreen, G3D::Rect2D* windowWorld) const;
    void boundingBoxCalculations (
	int width, int height, 
	const G3D::Rect2D& windowWorld, G3D::Rect2D* bb2dScreen, 
	double* change) const;
    G3D::AABox calculateCenteredViewingVolume () const;
    void initializeTextures ();
    void calculateCameraDistance ();
    /**
     * Generates a display list for edges
     * @return the display list
     */
    template<typename displayEdge>
    void displayEdges () const;


    void displayBlend () const;
    void displayEdgesNormal () const;
    template<typename displayEdge>
    void displayStandaloneEdges () const;
    template<typename displaySameEdges>
    void displayStandaloneFaces () const;
    void displayEdgesTorus () const
    {
	if (m_edgesTorusTubes)
	    displayEdgesTorusTubes ();
	else
	    displayEdgesTorusLines ();
    }
    void displayEdgesTorusTubes () const;
    void displayEdgesTorusLines () const;

    /**
     * Generates a display list for faces
     * @return the display list
     */
    void displayFacesNormal () const;
    void displayFacesTorus () const
    {
	if (m_facesTorusTubes)
	    displayFacesTorusTubes ();
	else
	    displayFacesTorusLines ();
    }
    void displayFacesTorusTubes () const;
    void displayFacesTorusLines () const;
    /**
     * Generates a display list for bodies
     * @return the display list
     */
    void displayFacesLighting () const;
    /**
     * Generates a display list for center paths
     */
    void displayCenterPathsWithBodies () const;
    void displayAverage () const;
    void displayOriginalDomain () const;
    void displayBoundingBox () const;
    void displayCenterPaths () const;
    void displayBox (const G3D::AABox& aabb, const QColor& color, 
		     GLenum polygonMode) const;
    void displayBox (const OOBox& oobox) const;
    void displayAxes () const;

    /**
     * Rotates the foam around an axis with a certain angle
     * @param axis can be 0, 1 or 2 for X, Y or Z
     * @param angle angle we rotate the foam with
     */
    void setRotation (int axis, double angle);
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
     */
    void enableLighting ();
    double ratioFromCenter (const QPoint& p);
    void rotate (const QPoint& position);
    void translateViewport (const QPoint& position);
    void scaleViewport (const QPoint& position);
    void initViewTypeDisplay ();

    bool doesSelectBody () const;
    bool doesSelectFace () const;
    bool doesSelectEdge () const;
    void createActions ();
    void rotateSurfaceEvolverCompatible () const;

private:
    static void displayOpositeFaces (G3D::Vector3 origin,
				     G3D::Vector3 faceFirst,
				     G3D::Vector3 faceSecond,
				     G3D::Vector3 translation);

    /**
     * Setup lighting for displaying faces edges and vertices
     */
    static void disableLighting ();
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

    int m_normalVertexSize;
    int m_normalEdgeWidth;
    double m_contextAlpha;

    G3D::Matrix3 m_rotate;    
    G3D::Rect2D m_viewport;
    double m_scale;
    /**
     * Distance from the camera to the center of the bounding box for the foam.
     */
    double m_cameraDistance;
    double m_angleOfView;

    EndLocationColor m_endTranslationColor;
    GLUquadricObj* m_quadric;    
    /**
     * For displaying Torus Model edges as cylinders
     */
    double m_edgeRadius;
    /**
     * For displaying arrows in the Torus Model edges
     */
    double m_arrowBaseRadius;
    double m_arrowHeight;

    bool m_edgesTorusTubes;
    bool m_facesTorusTubes;
    bool m_facesShowEdges;
    bool m_edgesBodyCenter;
    bool m_edgesTessellation;
    bool m_centerPathDisplayBody;
    bool m_boundingBox;
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
    /**
     * True if the program displays data in a loop, false
     * otherwise
     */
    bool m_playMovie;
    boost::scoped_ptr<DisplayBlend> m_displayBlend;
    /**
     * FBO used in averaging.
     */
    boost::scoped_ptr<QGLFramebufferObject> m_new;
    /**
     * FBO used in averaging.
     */
    boost::scoped_ptr<QGLFramebufferObject> m_old;
};

#endif //__GLWIDGET_H__

// Local Variables:
// mode: c++
// End:
