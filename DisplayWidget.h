/**
 * @file   DisplayWidget.h
 * @author Dan R. Lipsa
 * @date 02 August 2010 
 *
 * Declarations for the DisplayWidget class
 */
#ifndef __DISPLAY_WIDGET_H__
#define __DISPLAY_WIDGET_H__

/**
 * This window is saved as JPGs as the user interacts with the program.
 */
class DisplayWidget : public QWidget
{
public:
    DisplayWidget (QWidget * parent = 0);

public Q_SLOTS:
    /**
     * Save JPG images of the widgetDisplay
     * @param checked true for saving images, false otherwise
     */
    void ToggledSaveMovie (bool checked);
    void SaveFrame ();


private:
    Q_OBJECT
    /**
     * Save a jpeg of the current image.
     */
    bool m_saveMovie;
    /**
     * Keeps track of the current frame saved in a file.
     */
    int m_currentFrame;
    boost::scoped_ptr<QTimer> m_saveTimer;
};

#endif //__DISPLAY_WIDGET_H__

// Local Variables:
// mode: c++
// End:
