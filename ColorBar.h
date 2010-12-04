/**
 * @file   ColorBar.h
 * @author Dan R. Lipsa
 * @date 24 August 2010
 *
 * Declaration of the ColorBar class
 */

#ifndef __COLOR_BAR_H__
#define __COLOR_BAR_H__

class ColorBarModel;

class ColorBar : public QwtScaleWidget
{
public:
    ColorBar (QWidget* parent = 0);

public Q_SLOTS:
    void ShowEditColorMap ();
    void ClampClear ();
    void ColorBarModelChangedSlot (
	boost::shared_ptr<ColorBarModel> colorBarModel);
    void SetDefaultFont ();

Q_SIGNALS:
    void EditColorMap ();
    void ColorBarModelChanged (
	boost::shared_ptr<ColorBarModel> colorBarModel);

protected:
    virtual void contextMenuEvent (QContextMenuEvent *event);

private:
    void createActions ();
    void setTitleDefaultFont (const char* s = 0);

private:
    Q_OBJECT
    boost::shared_ptr<ColorBarModel> m_model;
    boost::scoped_ptr<QAction> m_actionEditColorMap;
    boost::scoped_ptr<QAction> m_actionClampClear;
};

#endif //__COLOR_BAR_H__

// Local Variables:
// mode: c++
// End:
