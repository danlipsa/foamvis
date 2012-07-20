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

class Foam;
class SendPaintEnd;
class Settings;

class WidgetVtk : public QVTKWidget
{
    friend class SendPaintEnd;
    
public:
    WidgetVtk (QWidget* parent);
    void UpdateColorTransferFunction (
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction);
    void UpdateThreshold (QwtDoubleInterval interval);
    void UpdateRenderStructured (
	const Foam& foam, vtkSmartPointer<vtkMatrix4x4> modelView,
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction,
	QwtDoubleInterval interval, BodyScalar::Enum bodyScalar);
    void SetSettings (boost::shared_ptr<Settings> settings)
    {
	m_settings = settings;
    }
    void UpdateOpacity ();

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
    void PaintEnd ();
    
private:
    Q_OBJECT
    vtkSmartPointer<vtkDataSetMapper> m_mapper;
    vtkSmartPointer<vtkThreshold> m_threshold;
    vector< vtkSmartPointer<vtkActor> > m_constraintSurface;
    boost::shared_ptr<Settings> m_settings;
};


#endif //__WIDGETVTK_H__

// Local Variables:
// mode: c++
// End:
