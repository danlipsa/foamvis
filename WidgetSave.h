/**
 * @file   WidgetSave.h
 * @author Dan R. Lipsa
 * @date 02 August 2010 
 *
 * Declarations for the WidgetSave class
 */
#ifndef __WIDGET_SAVE_H__
#define __WIDGET_SAVE_H__

/**
 * This window is saved as JPGs as the user interacts with the program.
 */
class WidgetSave : public QWidget
{
public:
    WidgetSave (QWidget * parent = 0);

public Q_SLOTS:
    /**
     * Save JPG images of the widgetDisplay
     * @param checked true for saving images, false otherwise
     */
    void ToggledSaveMovie (bool checked);
    void SaveFrame ();


protected:
    void paintEvent ( QPaintEvent * event );

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
};

#endif //__WIDGET_SAVE_H__

// Local Variables:
// mode: c++
// End:
