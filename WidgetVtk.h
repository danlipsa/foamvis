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

class vtkActor;
class vtkDataSetMapper;
class vtkRenderer;
class Foam;

class WidgetVtk : public QVTKWidget
{
public:
    WidgetVtk (QWidget* parent);
    void UpdateAverage (const Foam& foam);
			       
public Q_SLOTS:
    
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
