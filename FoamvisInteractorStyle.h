/**
 * @file   FoamvisInteractorStyle.h
 * @author modified from vtkInteractorStyleTrackballCamera by Dan Lipsa
 * @date 3 Sept. 2012
 * 
 * Interactor for WidgetVtk
 * 
 */

#ifndef __FOAMVIS_INTRACTOR_STYLE_H
#define __FOAMVIS_INTRACTOR_STYLE_H


#include "Enums.h"

class VTK_RENDERING_EXPORT FoamvisInteractorStyle : 
    public vtkInteractorStyleTrackballCamera
{
public:
  static FoamvisInteractorStyle *New();
  vtkTypeMacro(FoamvisInteractorStyle,vtkInteractorStyle);

  void SetInteractionModeQuery (boost::function<InteractionMode::Enum ()> f)
  {
      m_getInteractionMode = f;
  }

  // Description:
  // Event bindings controlling the effects of pressing mouse buttons
  // or moving the mouse.
  virtual void OnLeftButtonDown();
  virtual void OnLeftButtonUp();
  virtual void OnMiddleButtonDown();
  virtual void OnMiddleButtonUp();
  virtual void OnRightButtonDown();
  virtual void OnRightButtonUp();
  
protected:
  FoamvisInteractorStyle();
  ~FoamvisInteractorStyle();

private:
  FoamvisInteractorStyle(const FoamvisInteractorStyle&);  // Not implemented.
  void operator=(const FoamvisInteractorStyle&);  // Not implemented.

private:
  boost::function<InteractionMode::Enum ()> m_getInteractionMode;
};


#endif //__WIDGETVTK_H__

// Local Variables:
// mode: c++
// End:
