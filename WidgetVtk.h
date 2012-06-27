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
    void UpdateRenderUnstructured (
	const Foam& foam, const BodySelector& bodySelector,
	vtkSmartPointer<vtkMatrix4x4> modelView,
	BodyProperty::Enum bodyProperty,
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction);
    void UpdateRenderStructured (
	const Foam& foam, const BodySelector& bodySelector,
	vtkSmartPointer<vtkMatrix4x4> modelView,
	BodyProperty::Enum bodyProperty,
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction);

public:
    QSize sizeHint ()
    {
	return QSize (128, 128);
    }

    QSize minimumSizeHint ()
    {
	return QSize (128, 128);
    }
    
private:
    Q_OBJECT

    vtkSmartPointer<vtkDataSetMapper> m_mapper;
    vtkSmartPointer<vtkActor> m_actor;
    vtkSmartPointer<vtkRenderer> m_renderer;
};


#endif //__WIDGETVTK_H__

// Local Variables:
// mode: c++
// End:
