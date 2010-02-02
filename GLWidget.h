/**
 * @file   GLWidget.h
 * @author Dan R. Lipsa
 *
 * Widget for displaying foam bubbles using OpenGL
 */
#ifndef __GLWIDGET_H__
#define __GLWIDGET_H__

class Body;
class Data;
class DataFiles;

/**
 * Widget for displaying foam bubbles using OpenGL
 */
class GLWidget : public QGLWidget
{
    Q_OBJECT
    
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
     * Specifies the minimum size of the GLWidget
     * @return minimum size of the GLWidget
     */
    QSize minimumSizeHint() const;
    /**
     * Specifies the prefered size
     * @return the prefered size
     */
    QSize sizeHint() const;
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
     * @return a DataFiles object
     */
    DataFiles& GetDataFiles () {return *m_dataFiles;}
    /**
     * Gets the currently displayed data
     * @return currently displayed data
     */
    Data& GetCurrentData ();
    /**
     * Gets the currently displayed body
     * @return the currrently displayed body or UINT_MAX for all bodies
     */
    unsigned int GetDisplayedBody ();
    /**
     * Gets the face number up to which faces are displayed
     * @return face number up to which faces are displayed or UINT_MAX for all
     */
    unsigned int GetDisplayedFace ();
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
    float GetPhysicalObjectsWidth () 
    {
	return OBJECTS_WIDTH[m_physicalObjectsWidth];
    }
    /**
     * Returns the actual size of tessellation objects
     */
    float GetTessellationObjectsWidth () 
    {
	return OBJECTS_WIDTH[m_tessellationObjectsWidth];
    }
public Q_SLOTS:
    /**
     * Shows vertices
     * @param checked true for showing vertices false otherwise
     */
    void ViewVertices (bool checked);
    /**
     * Shows edges
     * @param checked true for showing edges false otherwise
     */
    void ViewEdges (bool checked);
    /**
     * Shows faces
     * @param checked true for showing faces false otherwise
     */
    void ViewFaces (bool checked);
    /**
     * Shows bodies
     * @param checked true for showing bodies false otherwise
     */
    void ViewBodies (bool checked);
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
    void PhysicalObjectsWidthChanged (int value);
    /**
     * Signals a change in the size of the tessellation objects
     * @param value the new size
     */
    void TessellationObjectsWidthChanged (int value);
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
     * Mapping between the index in  the slider and an actual size for
     * physical and tessellation objects.
     */
    static const float OBJECTS_WIDTH[];
    /**
     * WHAT kind of objects do we display
     */
    enum ViewType {
        VERTICES,
        EDGES,
        FACES,
        BODIES,
        COUNT
    };
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
    /**
     * Generates a display list for edges
     * @return the display list
     */
    GLuint displayEdges ();
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
    /**
     * Generates a display list for bodies
     * @return the display list
     */
    GLuint displayBodies ();
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
    void displayFacesContour (const vector<Body*>& bodies);
    /**
     * Displays   the   content   of   faces.   Used   together   with
     * displayFacesContour  and  with  GL_POLYGON_OFFSET_FILL  to  get
     * continuous contours for polygons.
     * @param bodies displays all the faces in these bodies
     */
    void displayFacesOffset (const vector<Body*>& bodies);
    /**
     * Setup lighting for shaded bodies
     */
    void initLightBodies ();
    /**
     * Setup lighting for displaying faces edges and vertices
     */
    static void initLightFlat ();
    /**
     * The elements displayed from a DMP file: vertices, edges, faces or bodies.
     */
    ViewType m_viewType;
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
    int m_dataIndex;
    /**
     * Used for rotating the view
     */
    QPoint m_lastPos;
    /**
     * Used to display one body at a time
     */
    unsigned int m_displayedBody;
    /**
     * Used to display one face at a time from the m_displayedBody.
     */
    unsigned int m_displayedFace;
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
    int m_physicalObjectsWidth;
    /**
     * Stores the size of tessellation objects
     */
    int m_tessellationObjectsWidth;
};

#endif //__GLWIDGET_H__

// Local Variables:
// mode: c++
// End:
