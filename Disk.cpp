/**
 * @file   Disk.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Disk class
 */
#include "Disk.h"
#include "Debug.h"

Disk::Disk()
               /* explicit initialization */
    :m_radius (0),
     m_center (),
     m_twelveOclock (),
     m_threeOclock () 
{
    
    if (false)
	cdbg << "Disk::Disk() new" << endl;
}

Disk::~Disk() 
{
}

void Disk::Initialize(const G3D::Vector3& centerCoord,
		      const G3D::Vector3& twelveOclockCoord, 
		      const G3D::Vector3& threeOclockCoord,
		      double radius)
{
    m_radius         = radius;
    m_center         = centerCoord;
    m_twelveOclock   = twelveOclockCoord * radius;
    m_threeOclock = threeOclockCoord * radius;
}

G3D::Vector3 Disk::GetVertex(int vertexNumber) const 
{
    const float cos45 = 0.707106781;
    switch(vertexNumber)
    {
	
    case VERTEX0:  return (m_center + m_twelveOclock);

    case VERTEX1:  return (m_center + 
			   (cos45 * m_twelveOclock) +
			   (cos45 * m_threeOclock));

    case VERTEX2:  return (m_center + m_threeOclock);

    case VERTEX3:  return (m_center -
			   (cos45 * m_twelveOclock) +
			   (cos45 * m_threeOclock));

    case VERTEX4:  return (m_center - m_twelveOclock);

    case VERTEX5:  return (m_center -
			   (cos45 * m_twelveOclock) -
			   (cos45 * m_threeOclock));

    case VERTEX6:  return (m_center - m_threeOclock);

    case VERTEX7:  return (m_center +
			   (cos45 * m_twelveOclock) -
			   (cos45 * m_threeOclock));

    default:
	cdbg << "*** Error, Disk::GetVertex() unrecognized vertex number"  
	     << endl;
	return m_center;

    } /* end switch */
}

Disk::DISK_VERTEX Disk::GetNextVertexIndex(int thisVertexIndex) 
{
    return static_cast<Disk::DISK_VERTEX> ((thisVertexIndex + 1) % COUNT);
}

G3D::Vector3 Disk::GetNormal() const 
{
    return m_threeOclock.cross (m_twelveOclock).unit ();
}

G3D::Vector3 Disk::GetVertexNormal(int vertexNumber) const 
{
    return (GetVertex (vertexNumber) - GetCenter ()).unit ();
}

G3D::Vector3 Disk::GetCenter() const 
{

    bool debug = false;   
    if (debug) cdbg << "Disk::GetCenter()" << endl;

    return  ( (this->GetVertex(VERTEX0) + 
	       this->GetVertex(VERTEX4) ) / 2.0f);
}

float Disk::GetDiskCircumference() const 
{
    /* Distance() method in Coord3D.h */
    float circumference = 
	(GetVertex(VERTEX0) -  GetVertex(VERTEX1)).length () +
	(GetVertex(VERTEX1) - GetVertex(VERTEX2)).length () +
	(GetVertex(VERTEX2) - this->GetVertex(VERTEX3)).length () +
	(GetVertex(VERTEX3) - GetVertex(VERTEX4)).length () +
	(GetVertex(VERTEX4) - GetVertex(VERTEX5)).length () +
	(GetVertex(VERTEX5) - GetVertex(VERTEX6)).length () +
	(GetVertex(VERTEX6) - this->GetVertex(VERTEX7)).length () +
	(GetVertex(VERTEX7) - this->GetVertex(VERTEX0)).length ();

    RuntimeAssert (
	circumference > 0, 
	"*** Warning Disk::GetDiskCircumference() circumference <= 0");
    return circumference;
}

/*
 * @see Disk.h for comments
 */
void Disk::Print() const {

   cdbg << "this disk: " << endl;
   cdbg << " vertex0: " << this->GetVertex(VERTEX0) << endl;
   cdbg << " vertex1: " << this->GetVertex(VERTEX1) << endl;
   cdbg << " vertex2: " << this->GetVertex(VERTEX2) << endl;
}

