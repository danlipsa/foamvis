/**
 * @file   GLWidget.h
 * @author Dan R. Lipsa
 *
 * Widget for displaying foam bubbles using OpenGL
 */
#ifndef __GLWIDGET_H__
#define __GLWIDGET_H__

#include "Hashes.h"
#include "InteractionMode.h"

class Body;
class Data;
class DataFiles;

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
     * @param dataFiles data displayed by the GLWidget
     */
    void SetDataFiles (DataFiles* dataFiles) 
    {
        m_dataFiles = dataFiles;
    }
    /**
     * Gets the data displayed by the GLWidget
     */
    DataFiles& GetDataFiles () {return *m_dataFiles;}
    /**
     * Gets the currently displayed data
     */
    Data& GetCurrentData ();
    /**
     * Gets the index of the currently displayed data.
     */
    size_t GetCurrentDataIndex () {return m_dataIndex;}
    
    /**
     * Gets the currently displayed body
     * @return the currrently displayed body or UINT_MAX for all bodies
     */
    size_t GetDisplayedBody () const
    {
	return m_displayedBody;
    }
    /**
     * Gets the face number up to which faces are displayed
     * @return face number up to which faces are displayed or UINT_MAX for all
     */
    size_t GetDisplayedFace () const
    {
	return m_displayedFace;
    }

    /**
     * Increment displayed body
     */
    void IncrementDisplayedBody ();
    /**
     * Decrement displayed body
     */
    void DecrementDisplayedBody ();
    /**
     * Increment displayed face
     */
    void IncrementDisplayedFace ();
    /**
     * Decrement displayed face
     */
    void DecrementDisplayedFace ();
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

    const QColor& GetDomainIncrementColor (const G3D::Vector3int16& di) const;
    
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
    void DataChanged () {DataSliderValueChanged (m_dataIndex);}

    GLUquadricObj* GetQuadricObject () const 
    {
	return m_quadric;
    }

    void UpdateDisplay ()
    {
	setObject (&m_object, display(m_viewType));
	updateGL ();
    }

    float GetArrowBaseRadius () const {return m_arrowBaseRadius;}
    float GetArrowHeight () const {return m_arrowHeight;}
    float GetEdgeRadius () const {return m_edgeRadius;}


public Q_SLOTS:
    /**
     * Shows vertices
     * @param checked true for showing vertices false otherwise
     */
    void ViewVertices (bool checked);
    void ViewPhysicalVertices (bool checked);
    void ViewRawVertices (bool checked);
    /**
     * Shows edges
     * @param checked true for showing edges false otherwise
     */
    void ViewEdges (bool checked);
    void ViewRawEdges (bool checked);
    void ViewPhysicalEdges (bool checked);
    /**
     * Shows faces
     * @param checked true for showing faces false otherwise
     */
    void ViewFaces (bool checked);
    void ViewRawFaces (bool checked);
    /**
     * Shows bodies
     * @param checked true for showing bodies false otherwise
     */
    void ViewBodies (bool checked);
    void ViewTorusOriginalDomain (bool checked);
    /**
     * Shows center paths
     * param checked true for showing the center paths false otherwise
     */
    void ViewCenterPaths (bool checked);
    /**
     * Signals a change in data displayed
     * @param newIndex the new index for the data object to be displayed
     */
    void DataSliderValueChanged (int newIndex);
    /**
     * Save JPG images of the GLWidget
     * @param checked true for saving images of the GLWidget, false otherwise
     */
    void SaveMovie (bool checked);
    /**
     * Signals a change in the size of the physical objects
     * @param value the new size
     */
    void PhysicalVertexSizeChanged (int value);
    void PhysicalEdgeWidthChanged (int value);
    /**
     * Signals a change in the size of the tessellation objects
     * @param value the new size
     */
    void TessellationVertexSizeChanged (int value);
    void TessellationEdgeWidthChanged (int value);
    void InteractionModeChanged (int index);

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
	RAW_VERTICES,
	PHYSICAL_VERTICES,

        EDGES,
	RAW_EDGES,
	PHYSICAL_EDGES,

        FACES,
	RAW_FACES,
        BODIES,
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
				 Vector3int16Hash> DomainIncrementColor;

private:
    void view (bool checked, ViewType view, Lighting lighting);

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

    void project ();
    void calculateViewingVolume ();
    /**
     * Generates a display list for a certain kind of objects
     * @param type the type of object that we want displayed.
     * @return the display list
     */
    GLuint display (ViewType type);
    /**
     * Generates a display list for vertices
     * @return the display list
     */
    GLuint displayVertices ();
    GLuint displayRawVertices ();
    GLuint displayPhysicalVertices ();
    /**
     * Generates a display list for edges
     * @return the display list
     */
    GLuint displayEdges ();
    GLuint displayRawEdges ();
    GLuint displayPhysicalEdges ();

    /**
     * Generates a display list for vertices or edges
     * @param type specifies vertices or edges
     * @return the display list
     */
    GLuint displayEV (ViewType type);
    /**
     * Generates a display list for faces
     * @return the display list
     */
    GLuint displayFaces ();
    GLuint displayRawFaces ();

    /**
     * Generates a display list for bodies
     * @return the display list
     */
    GLuint displayBodies ();
    /**
     * Generates a display list for center paths
     */
    GLuint displayCenterPaths ();
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

private:
    /**
     * Setup lighting for displaying faces edges and vertices
     */
    static void disableLighting ();
    static void materialProperties ();
    static void quadricErrorCallback (GLenum errorCode);
    
private:
    Q_OBJECT

    /**
     * The elements displayed from a DMP file: vertices, edges, faces or bodies.
     */
    ViewType m_viewType;
    bool m_viewTorusOriginalDomain;
    InteractionMode::Name m_interactionMode;
    /**
     * The current DMP file as a OpenGL display list.
     */
    GLuint m_object;
    /**
     * Data to be displayd. Each element coresponds to a DMP file
     */
    DataFiles* m_dataFiles;
    /**
     * Index into m_data that shows the current DMP file displayed
     */
    size_t m_dataIndex;
    /**
     * Used for rotating the view
     */
    QPoint m_lastPos;
    /**
     * Used to display one body at a time
     */
    size_t m_displayedBody;
    /**
     * Used to display one face at a time from the m_displayedBody.
     */
    size_t m_displayedFace;
    /**
     * Save a jpeg of the current image.
     */
    bool m_saveMovie;
    /**
     * Keeps track of the current frame saved in a file.
     */
    int m_currentFrame;
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
    DomainIncrementColor m_domainIncrementColor;
    GLUquadricObj* m_quadric;    
    float m_arrowBaseRadius;
    float m_arrowHeight;
    float m_edgeRadius;
private:
    /**
     * Mapping between the index in  the slider and an actual size for
     * physical and tessellation objects.
     */
    static  float OBJECTS_WIDTH[];
};

#endif //__GLWIDGET_H__

// Local Variables:
// mode: c++
// End:
