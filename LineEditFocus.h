/**
 * @file   LineEditFocus.h
 * @author Dan R. Lipsa
 * @date 20 August 2010
 *
 * Declaration of the LineEditFocus class
 */

#ifndef __LINE_EDIT_FOCUS_H__
#define __LINE_EDIT_FOCUS_H__

class LineEditFocus : public QLineEdit
{
public:
    LineEditFocus (QWidget* parent = 0) :
	QLineEdit (parent)
    {
    }
Q_SIGNALS:
    void focusIn ();

protected:
    virtual void focusInEvent ( QFocusEvent * event )
    {
	QWidget::focusInEvent (event);
	Q_EMIT focusIn ();
    }
private:
    Q_OBJECT
};

#endif //__LINE_EDIT_FOCUS_H__

// Local Variables:
// mode: c++
// End:
