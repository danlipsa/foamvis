/**
 * @file   FoamvisInteractorStyle.h
 * @author modified from vtkInteractorStyleTrackballCamera by Dan Lipsa
 * @date 3 Sept. 2012
 * 
 * Interactor for WidgetVtk
 * 
 */

#include "FoamvisInteractorStyle.h"
#include "DebugStream.h"

vtkStandardNewMacro(FoamvisInteractorStyle);

//----------------------------------------------------------------------------
FoamvisInteractorStyle::FoamvisInteractorStyle() 
{
}

//----------------------------------------------------------------------------
FoamvisInteractorStyle::~FoamvisInteractorStyle() 
{
}

//----------------------------------------------------------------------------
void FoamvisInteractorStyle::OnLeftButtonDown() 
{ 
    this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
                            this->Interactor->GetEventPosition()[1]);
    if (this->CurrentRenderer == NULL)
    {
        return;
    }
  
    this->GrabFocus(this->EventCallbackCommand);

    switch (m_getInteractionMode ())
    {
    case InteractionMode::ROTATE:
        if (this->Interactor->GetShiftKey () &&
            this->Interactor->GetControlKey ())
            this->StartSpin ();
        else
            this->StartRotate ();
        break;
    case InteractionMode::SCALE:
        this->StartDolly ();
        break;
    case InteractionMode::TRANSLATE:
        this->StartPan ();
        break;

    case InteractionMode::SELECT:
    case InteractionMode::DESELECT:
    case InteractionMode::COUNT:
        break;
    }
}

//----------------------------------------------------------------------------
void FoamvisInteractorStyle::OnLeftButtonUp()
{
  switch (this->State) 
    {
    case VTKIS_DOLLY:
      this->EndDolly();
      break;

    case VTKIS_PAN:
      this->EndPan();
      break;

    case VTKIS_SPIN:
      this->EndSpin();
      break;

    case VTKIS_ROTATE:
      this->EndRotate();
      break;
    }

  if ( this->Interactor )
    {
    this->ReleaseFocus();
    }
}

//----------------------------------------------------------------------------
void FoamvisInteractorStyle::OnMiddleButtonDown() 
{
    vtkInteractorStyle::OnMiddleButtonDown ();
}

//----------------------------------------------------------------------------
void FoamvisInteractorStyle::OnMiddleButtonUp()
{
    vtkInteractorStyle::OnMiddleButtonUp ();
}

//----------------------------------------------------------------------------
void FoamvisInteractorStyle::OnRightButtonDown() 
{
    vtkInteractorStyle::OnRightButtonDown ();
}

//----------------------------------------------------------------------------
void FoamvisInteractorStyle::OnRightButtonUp()
{
    vtkInteractorStyle::OnRightButtonUp ();
}

