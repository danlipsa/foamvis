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
    void SetModel (boost::shared_ptr<ColorBarModel> model);

public Q_SLOTS:
    void ShowEditTransferFunction ();

Q_SIGNALS:
    void EditTransferFunction ();

protected:
    virtual void contextMenuEvent (QContextMenuEvent *event);

private:
    void createActions ();

private:
    Q_OBJECT

    boost::shared_ptr<ColorBarModel> m_model;
    boost::shared_ptr<QAction> m_actionEditTransferFunction;
};

#endif //__COLOR_BAR_H__

// Local Variables:
// mode: c++
// End:
