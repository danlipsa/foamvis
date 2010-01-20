/**
 * @file   GLWidget.h
 * @author Dan R. Lipsa
 *
 * Widget for displaying foam bubbles using OpenGL
 */
#ifndef __GLWIDGET_H__
#define __GLWIDGET_H__

class Data;
class Body;
/*
 * Widget for displaying foam bubbles using OpenGL
 */
class GLWidget : public QGLWidget
{
    Q_OBJECT

    public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    void SetData (std::vector<Data*>& data) 
    {
        m_data = &data;
    }
    std::vector<Data*>& GetData () {return *m_data;}
    Data& GetCurrentData () {return *(*m_data)[m_dataIndex];}
    unsigned int GetDisplayedBody ();
    unsigned int GetDisplayedFace ();
    void IncrementDisplayedBody ();
    void DecrementDisplayedBody ();
    void IncrementDisplayedFace ();
    void DecrementDisplayedFace ();

public Q_SLOTS:
    void ViewVertices (bool checked);
    void ViewEdges (bool checked);
    void ViewFaces (bool checked);
    void ViewBodies (bool checked);
    void DataChanged (int newIndex);
    void SaveMovie (bool checked);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    enum ViewType {
        VERTICES,
        EDGES,
        FACES,
        BODIES,
        COUNT
    };

    GLuint display (ViewType type);
    GLuint displayVertices ()
    {return displayEV (VERTICES);}
    GLuint displayEdges ()
    {return displayEV (EDGES);}
    GLuint displayEV (ViewType type);
    GLuint displayFaces ();
    GLuint displayBodies ();
    void setRotation (int axis, float angle);
    void displayFacesContour (const std::vector<Body*>& bodies);
    void displayFacesOffset (const std::vector<Body*>& bodies);
    void emitRotationChanged (int axis, int angle);
    static void initLightBodies ();
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
     * Vector of data to be displayd. Each element coresponds to a DMP file
     */
    std::vector<Data*>* m_data;
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
    int m_currentFrame;
};

#endif //__GLWIDGET_H__

// Local Variables:
// mode: c++
// End:
