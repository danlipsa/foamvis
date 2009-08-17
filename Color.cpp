#include <QColor>
#include "Color.h"

float colors[][4] = 
{
    {0.0,0.0,0.0,1.},
    {0.0,0.0,1.,1.},
    {0.0,1.,0.0,1.},
    {0.0,1.,1.,1.}, 
    {1.,0.0,0.0,1.},
    {1.,0.0,1.,1.},
    {1.,0.5,0.,1.},
    {.6f,.6f,.6f,1.},
    {.3f,.3f,.3f,1.},
    {.3f,.8f,1.,1.}, 
    {.5,1.,.5,1.},
    {.5,1.,1.,1.},
    {1.,.5,.5,1.},
    {1.,.5,1.,1.},
    {1.,1.,.0,1.},
    {1.,1.,1.,1.},
    {0.,0.,0.,0.}
};


const float* Color::GetValue(Color::Name color)
{
    return colors[color];
}
