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
class Foam;
class Edge;
class FoamAlongTime;
class OrientedFace;
class OrientedEdge;
class OOBox;

/**
 * Widget for displaying foam bubbles using OpenGL
 */
class GLWidget : public QGLWidget
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
    float GetContextAlpha () const
    {
	return m_contextAlpha;
    }

    const QColor& GetEndTranslationColor (const G3D::Vector3int16& di) const;    

    /**
     * Displays the center of the bodies
     */
    void displayCenterOfBodies ();
    void DataChanged () {ValueChangedSliderData (m_timeStep);}

    GLUquadricObj* GetQuadricObject () const 
    {
	return m_quadric;
    }

    void UpdateDisplayList ();
    float GetArrowBaseRadius () const {return m_arrowBaseRadius;}
    float GetArrowHeight () const {return m_arrowHeight;}
    float GetEdgeRadius () const {return m_edgeRadius;}

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
    void ResetTransformations ();
    void SetColorMap (QwtLinearColorMap* colorMap,
		      QwtDoubleInterval* colorMapInterval)
    {
	m_colorMap = colorMap;
	m_colorMapInterval = colorMapInterval;
    }
    QColor MapScalar (float value)
    {
	if (m_colorMap == 0)
	    return Qt::black;
	else
	    return m_colorMap->color (*m_colorMapInterval, value);
    }
    

    // Slot like methods
    // ======================================================================
    /**
     * Signals a change in data displayed
     * @param newIndex the new index for the data object to be displayed
     */
    void ValueChangedSliderData (int newIndex);
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
    void ToggledFacesTorus (bool checked);
    /**
     * Shows center paths
     * param checked true for showing the center paths false otherwise
     */
    void ToggledCenterPath (bool checked);
    void ValueChangedCenterPathColor (int value);

public Q_SLOTS:
    void CurrentIndexChangedInteractionMode (int index);
    void ToggledBodies (bool checked);
    void ToggledCenterPathDisplayBody (bool checked);

    void ToggledEdgesBodyCenter (bool checked);
    void ToggledEdgesTessellation (bool checked);
    void ToggledEdgesTorusTubes (bool checked);

    void ToggledFacesNormal (bool checked);
    void ToggledFacesTorusTubes (bool checked);

    void ToggledTorusOriginalDomainDisplay (bool checked);
    void ToggledTorusOriginalDomainClipped (bool checked);
    void ToggledBoundingBox (bool checked);
    void ValueChangedAngleOfView (int newIndex);

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

private:
    /**
     * WHAT kind of objects do we display
     */
    enum ViewType {
        EDGES,
	EDGES_TORUS,

        FACES,
        FACES_LIGHTING,
	FACES_TORUS,

	CENTER_PATHS,
        VIEW_TYPE_COUNT
    };

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
	GLuint (GLWidget::* m_displayList) ();
	boost::function<Lighting ()> m_lighting;
    };

private:
    bool edgesTorusTubes ()
    {
	return m_edgesTorusTubes;
    }

    bool facesTorusTubes ()
    {
	return m_facesTorusTubes;
    }

    void view (bool checked, ViewType view);

    void projectionTransformation () const;
    void viewingTransformation () const;
    void modelingTransformation () const;
    G3D::AABox calculateCenteredViewingVolume () const;
    void calculateCameraDistance ();
    /**
     * Generates a display list for a certain kind of objects
     * @param type the type of object that we want displayed.
     * @return the display list
     */
    GLuint displayList (ViewType type);
    /**
     * Generates a display list for edges
     * @return the display list
     */
    template<typename displayEdge>
    GLuint displayListEdges ();


    GLuint displayListEdgesNormal ();
    template<typename displayEdge>
    void displayStandaloneEdges () const;
    GLuint displayListEdgesTorus ()
    {
	if (m_edgesTorusTubes)
	    return displayListEdgesTorusTubes ();
	else
	    return displayListEdgesTorusLines ();
    }
    GLuint displayListEdgesTorusTubes ();
    GLuint displayListEdgesTorusLines ();

    /**
     * Generates a display list for faces
     * @return the display list
     */
    GLuint displayListFacesNormal ();
    GLuint displayListFacesTorus ()
    {
	if (m_facesTorusTubes)
	    return displayListFacesTorusTubes ();
	else
	    return displayListFacesTorusLines ();
    }
    GLuint displayListFacesTorusTubes ();
    GLuint displayListFacesTorusLines ();
    /**
     * Generates a display list for bodies
     * @return the display list
     */
    GLuint displayListFacesLighting ();
    /**
     * Generates a display list for center paths
     */
    GLuint displayListCenterPaths ();
    void displayOriginalDomain ();
    void displayAABox ();
    void displayCenterPaths ();
    void display (const G3D::AABox& aabox) const;
    void display (const OOBox& oobox) const;

    /**
     * Rotates the foam around an axis with a certain angle
     * @param axis can be 0, 1 or 2 for X, Y or Z
     * @param angle angle we rotate the foam with
     */
    void setRotation (int axis, float angle);
    /**
     * Displays   the   contour   of   faces.   Used   together   with
     * displayFacesOffet   and  with  GL_POLYGON_OFFSET_FILL   to  get
     * continuous contours  for polygons.
     * @param bodies displays all the faces in these bodies
     */
    void displayFacesContour (
	const vector<boost::shared_ptr<Body> >& bodies) const;
    /**
     * Displays   the   content   of   faces.   Used   together   with
     * displayFacesContour  and  with  GL_POLYGON_OFFSET_FILL  to  get
     * continuous contours for polygons.
     * @param bodies displays all the faces in these bodies
     */
    void displayFacesOffset (
	const vector<boost::shared_ptr<Body> >& bodies) const;
    /**
     * Setup lighting for shaded bodies
     */
    void enableLighting ();
    float ratioFromCenter (const QPoint& p);
    void rotate (const QPoint& position);
    void translateViewport (const QPoint& position);
    void scaleViewport (const QPoint& position);
    void initViewTypeDisplay ();

    bool doesSelectBody () const;
    bool doesSelectFace () const;
    bool doesSelectEdge () const;
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
    ViewType m_viewType;
    bool m_torusOriginalDomainDisplay;
    bool m_torusOriginalDomainClipped;
    InteractionMode::Enum m_interactionMode;
    /**
     * The current DMP file as a OpenGL display list.
     */
    GLuint m_object;
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
    float m_contextAlpha;

    G3D::Matrix3 m_rotate;    
    G3D::Rect2D m_viewport;
    /**
     * Distance from the camera to the center of the AABox for the foam.
     */
    float m_cameraDistance;
    float m_angleOfView;

    EndLocationColor m_endTranslationColor;
    GLUquadricObj* m_quadric;    
    /**
     * For displaying Torus Model edges as cylinders
     */
    float m_edgeRadius;
    /**
     * For displaying arrows in the Torus Model edges
     */
    float m_arrowBaseRadius;
    float m_arrowHeight;

    bool m_edgesTorusTubes;
    bool m_facesTorusTubes;
    bool m_edgesBodyCenter;
    bool m_edgesTessellation;
    bool m_centerPathDisplayBody;
    bool m_boundingBox;
    boost::array<ViewTypeDisplay, VIEW_TYPE_COUNT> VIEW_TYPE_DISPLAY;
    QwtLinearColorMap* m_colorMap;
    QwtDoubleInterval* m_colorMapInterval;
    CenterPathColor::Enum m_centerPathColor;
};

#endif //__GLWIDGET_H__

// Local Variables:
// mode: c++
// End:
