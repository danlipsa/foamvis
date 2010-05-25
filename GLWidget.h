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
    FoamAlongTime& GetFoamAlongTime () 
    {
	return *m_foamAlongTime;
    }
    BodiesAlongTime& GetBodiesAlongTime ();
    BodyAlongTime& GetBodyAlongTime (size_t id);
    /**
     * Gets the currently displayed data
     */
    Foam& GetCurrentFoam () const;
    /**
     * Gets the index of the currently displayed data.
     */
    size_t GetTimeStep () {return m_timeStep;}
    
    /**
     * Gets the currently displayed body
     * @return the currrently displayed body or UINT_MAX for all bodies
     */
    size_t GetDisplayedBodyIndex () const
    {
	return m_displayedBodyIndex;
    }
    size_t GetDisplayedBodyId () const;
    Body* GetDisplayedBody () const;
    /**
     * Gets the face number up to which faces are displayed
     * @return face number up to which faces are displayed or UINT_MAX for all
     */
    size_t GetDisplayedFaceIndex () const
    {
	return m_displayedFaceIndex;
    }
    size_t GetDisplayedFaceId () const;
    Face* GetDisplayedFace () const;

    size_t GetDisplayedEdgeIndex () const
    {
	return m_displayedEdgeIndex;
    }
    size_t GetDisplayedEdgeId () const;
    Edge* GetDisplayedEdge () const;
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
    /**
     * Returns the actual size of physical objects
     */
    float GetPhysicalVertexSize () const
    {
	return OBJECTS_WIDTH[m_physicalVertexSize];
    }
    float GetPhysicalEdgeWidth () const
    {
	return OBJECTS_WIDTH[m_physicalEdgeWidth];
    }
    /**
     * Returns the actual size of tessellation objects
     */
    float GetTessellationVertexSize () const
    {
	return OBJECTS_WIDTH[m_tessellationVertexSize];
    }
    float GetTessellationEdgeWidth () const
    {
	return OBJECTS_WIDTH[m_tessellationEdgeWidth];
    }
    const QColor& GetTessellationVertexColor () const
    {
	return m_tessellationVertexColor;
    }
    const QColor& GetTessellationEdgeColor () const
    {
	return m_tessellationVertexColor;
    }

    const QColor& GetEndTranslationColor (const G3D::Vector3int16& di) const;
    
    const QColor& GetPhysicalVertexColor () const
    {
	return m_physicalVertexColor;
    }
    const QColor& GetPhysicalEdgeColor () const
    {
	return m_physicalEdgeColor;
    }


    /**
     * Displays the center of the bodies
     */
    void displayCenterOfBodies ();
    void DataChanged () {ValueChangedSliderData (m_timeStep);}

    GLUquadricObj* GetQuadricObject () const 
    {
	return m_quadric;
    }

    void UpdateDisplay ()
    {
	setObject (&m_object, displayList (m_viewType));
	updateGL ();
    }

    float GetArrowBaseRadius () const {return m_arrowBaseRadius;}
    float GetArrowHeight () const {return m_arrowHeight;}
    float GetEdgeRadius () const {return m_edgeRadius;}

    // Slot like methods
    // ======================================================================
    /**
     * Signals a change in data displayed
     * @param newIndex the new index for the data object to be displayed
     */
    void ValueChangedSliderData (int newIndex);
    void ToggledVerticesPhysical (bool checked);
    /**
     * Shows edges
     * @param checked true for showing edges false otherwise
     */
    void ToggledEdgesNormal (bool checked);
    void ToggledEdgesPhysical (bool checked);
    void ToggledEdgesTorus (bool checked);
    void ToggledFacesTorus (bool checked);

public Q_SLOTS:
    /**
     * Shows vertices
     * @param checked true for showing vertices false otherwise
     */
    void ToggledVerticesNormal (bool checked);
    void ToggledVerticesTorus (bool checked);
    /**
     * Shows faces
     * @param checked true for showing faces false otherwise
     */
    void ToggledFacesNormal (bool checked);
    /**
     * Shows bodies
     * @param checked true for showing bodies false otherwise
     */
    void ToggledBodies (bool checked);
    void ToggledTorusOriginalDomainDisplay (bool checked);
    void ToggledTorusOriginalDomainClipped (bool checked);
    /**
     * Shows center paths
     * param checked true for showing the center paths false otherwise
     */
    void ToggledCenterPath (bool checked);
    /**
     * Signals a change in the size of the physical objects
     * @param value the new size
     */
    void ValueChangedVerticesPhysical (int value);
    void ValueChangedEdgesPhysical (int value);
    /**
     * Signals a change in the size of the tessellation objects
     * @param value the new size
     */
    void ValueChangedVerticesTessellation (int value);
    void ValueChangedEdgesTessellation (int value);
    void currentIndexChangedInteractionMode (int index);
    void ToggledEdgesTorusTubes (bool checked);
    void ToggledFacesTorusTubes (bool checked);
    void ToggledEdgesBodyCenter (bool checked);

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
        VERTICES,
	VERTICES_PHYSICAL,
	VERTICES_TORUS,

        EDGES,
	EDGES_PHYSICAL,
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

    void project ();
    void calculateViewingVolume ();
    /**
     * Generates a display list for a certain kind of objects
     * @param type the type of object that we want displayed.
     * @return the display list
     */
    GLuint displayList (ViewType type);
    /**
     * Generates a display list for vertices
     * @return the display list
     */
    GLuint displayListVerticesNormal ();
    GLuint displayListVerticesTorus ();
    GLuint displayListVerticesPhysical ();
    /**
     * Generates a display list for edges
     * @return the display list
     */
    GLuint displayListEdges (
	boost::function<void (Edge*)> displayEdge,
	boost::function<bool (Edge*)> shouldDisplayEdge);


    GLuint displayListEdgesNormal ();
    GLuint displayListEdgesTorus ()
    {
	if (m_edgesTorusTubes)
	    return displayListEdgesTorusTubes ();
	else
	    return displayListEdgesTorusLines ();
    }
    GLuint displayListEdgesTorusTubes ();
    GLuint displayListEdgesTorusLines ();
    GLuint displayListEdgesPhysical ();

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
    void displayFacesContour (vector<Body*>& bodies);
    /**
     * Displays   the   content   of   faces.   Used   together   with
     * displayFacesContour  and  with  GL_POLYGON_OFFSET_FILL  to  get
     * continuous contours for polygons.
     * @param bodies displays all the faces in these bodies
     */
    void displayFacesOffset (vector<Body*>& bodies);
    /**
     * Setup lighting for shaded bodies
     */
    void enableLighting ();
    float ratioFromCenter (const QPoint& p);
    void rotate (const QPoint& position);
    void translateViewport (const QPoint& position);
    void scale (const QPoint& position);
    void scaleViewport (const QPoint& position);
    void initViewTypeDisplay ();

private:
    /**
     * Dealocates the space occupied by  an old OpenGL object and stores a
     * newObject
     *
     * @param object address where the  old object is stored and where the
     * new object will be stored
     * @param newObject the new object that will be stored
     */
    static void setObject (GLuint* object, GLuint newObject)
    {
	glDeleteLists(*object, 1);
	*object = newObject;
    }
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
    InteractionMode::Name m_interactionMode;
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
    /**
     * Stores the size of physical objects
     */
    int m_physicalVertexSize;
    int m_physicalEdgeWidth;
    QColor m_physicalVertexColor;
    QColor m_physicalEdgeColor;
    /**
     * Stores the size of tessellation objects
     */
    int m_tessellationVertexSize;
    int m_tessellationEdgeWidth;

    int m_normalVertexSize;
    int m_normalEdgeWidth;


    QColor m_tessellationVertexColor;
    QColor m_tessellationEdgeColor;
    QColor m_centerPathColor;
    G3D::AABox m_viewingVolume;
    /**
     * Rotations and translations
     */
    G3D::CoordinateFrame m_transform;
    G3D::Rect2D m_viewport;
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
    boost::array<ViewTypeDisplay, VIEW_TYPE_COUNT> VIEW_TYPE_DISPLAY;
private:
    /**
     * Mapping between the index in  the slider and an actual size for
     * physical and tessellation objects.
     */
    const static  float OBJECTS_WIDTH[];
};

#endif //__GLWIDGET_H__

// Local Variables:
// mode: c++
// End:
