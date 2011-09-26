#include "FoamProperties.h"
#include "OOBox.h"

bool FoamProperties::IsTorus () const
{
    return GetOriginalDomain ().IsTorusDomain ();
}

void FoamProperties::SetPeriods (const G3D::Vector3& x, const G3D::Vector3& y)
{
    using G3D::Vector3;
    Vector3 third = x.cross (y).unit ();
    double thirdLength = min (x.length (), y.length ());
    SetPeriods (x, y, thirdLength * third);
}
