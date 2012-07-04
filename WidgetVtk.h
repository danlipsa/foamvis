/**
 * @file   WidgetVtk.h
 * @author Dan R. Lipsa
 * @date 6 June 2012
 * 
 * Widget for displaying foam bubbles using VTK
 * 
 */
#ifndef __WIDGETVTK_H__
#define __WIDGETVTK_H__

#include "Enums.h"

class vtkActor;
class vtkDataSetMapper;
class vtkRenderer;
class Foam;
class BodySelector;

class WidgetVtk : public QVTKWidget
{
public:
    WidgetVtk (QWidget* parent);
    void UpdateColorTransferFunction (
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction);
    void UpdateRenderStructured (
	const Foam& foam, const BodySelector& bodySelector,
	vtkSmartPointer<vtkMatrix4x4> modelView,
	BodyProperty::Enum bodyProperty);

public:
    QSize sizeHint ()
    {
	return QSize (128, 128);
    }

    QSize minimumSizeHint ()
    {
	return QSize (128, 128);
    }

Q_SIGNALS:
    void Paint ();


protected:
    void paintEvent (QPaintEvent * event);
    
private:
    Q_OBJECT
    vtkSmartPointer<vtkDataSetMapper> m_mapper;
};


#endif //__WIDGETVTK_H__

// Local Variables:
// mode: c++
// End:
