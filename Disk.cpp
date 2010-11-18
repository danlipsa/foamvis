/**
 * @file   Disk.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Disk class
 */
#include "Disk.h"
#include "Debug.h"
#include "DebugStream.h"

Disk::Disk()
               /* explicit initialization */
    :m_CenterCoord(),
     m_VerticalComponent(),
     m_HorizontalComponent() 
{
    
    if (false)
	cdbg << "Disk::Disk() new" << endl;
}

Disk::~Disk() 
{
}

bool Disk::SetVertices(const G3D::Vector3& centerCoord,
		       const G3D::Vector3& twelveOclockCoord, 
		       const G3D::Vector3& threeOclockCoord) 
{

    if (false) 
	cdbg << "Disk::SetVertices()" << endl;

    m_CenterCoord         = centerCoord;
    m_VerticalComponent   = twelveOclockCoord - centerCoord;
    m_HorizontalComponent = threeOclockCoord  - centerCoord;

    return true;
}

G3D::Vector3 Disk::GetDiskCoord(int vertexNumber) const 
{
    switch(vertexNumber)
    {
	
    case VERTEX0:  return (m_CenterCoord + m_VerticalComponent);

    case VERTEX1:  return (m_CenterCoord + 
			   (0.75f * m_VerticalComponent) +
			   (0.75f * m_HorizontalComponent));

    case VERTEX2:  return (m_CenterCoord + m_HorizontalComponent);

    case VERTEX3:  return (m_CenterCoord -
			   (0.75f * m_VerticalComponent) +
			   (0.75f * m_HorizontalComponent));

    case VERTEX4:  return (m_CenterCoord - m_VerticalComponent);

    case VERTEX5:  return (m_CenterCoord -
			   (0.75f * m_VerticalComponent) -
			   (0.75f * m_HorizontalComponent));

    case VERTEX6:  return (m_CenterCoord - m_HorizontalComponent);

    case VERTEX7:  return (m_CenterCoord +
			   (0.75f * m_VerticalComponent) -
			   (0.75f * m_HorizontalComponent));

    case CENTER:        return m_CenterCoord;
    case TWELVEOCLOCK:  return (m_CenterCoord + m_VerticalComponent);
    case THREEOCLOCK:   return (m_CenterCoord + m_HorizontalComponent);

    default:
	cdbg << "*** Error, Disk::GetDiskCoord() unrecognized vertex number"  
	     << endl;
	return m_CenterCoord;

    } /* end switch */
}

Disk::DISK_VERTEX Disk::GetNextVertexIndex(int thisVertexIndex) 
{

    switch(thisVertexIndex) 
    {

    case VERTEX0:  return VERTEX1;
    case VERTEX1:  return VERTEX2;
    case VERTEX2:  return VERTEX3;
    case VERTEX3:  return VERTEX4;
    case VERTEX4:  return VERTEX5;
    case VERTEX5:  return VERTEX6;
    case VERTEX6:  return VERTEX7;
    case VERTEX7:  return VERTEX0;

    default:
	cdbg << 
	    "*** Error, Disk::GetNextVertexIndex() unrecognized vertex number"
	     << endl;
	return VERTEX0;
    } /* end switch */
}

G3D::Vector3 Disk::GetDiskNormal() const 
{
    return m_HorizontalComponent.cross (m_VerticalComponent).unit ();
}

G3D::Vector3 Disk::GetVertexNormal(int vertexNumber) const 
{

    G3D::Vector3 normalVector;
    switch(vertexNumber)
    {
    case VERTEX0:
	normalVector = this->GetDiskCoord(VERTEX0) - this->GetDiskCenter();
	break;
         
    case VERTEX1:
	normalVector = this->GetDiskCoord(VERTEX1) - this->GetDiskCenter();
	break;

    case VERTEX2:
	normalVector = this->GetDiskCoord(VERTEX2) - this->GetDiskCenter();
	break;

    case VERTEX3:
	normalVector = this->GetDiskCoord(VERTEX3) - this->GetDiskCenter();
	break;
         
    case VERTEX4:
	normalVector = this->GetDiskCoord(VERTEX4) - this->GetDiskCenter();
	break;

    case VERTEX5:
	normalVector = this->GetDiskCoord(VERTEX5) - this->GetDiskCenter();
	break;

    case VERTEX6:
	normalVector = this->GetDiskCoord(VERTEX6) - this->GetDiskCenter();
	break;

    case VERTEX7:
	normalVector = this->GetDiskCoord(VERTEX7) - this->GetDiskCenter();
	break;

    default:
	cdbg << "*** Error, Disk::GetDiskNormal() unrecognized vertex number"  
	     << endl;
	normalVector = this->GetDiskCoord(VERTEX0) - this->GetDiskCenter();
	break;
    } /* end switch */

    return normalVector.unit ();
}

void Disk::setDiskCoord(DISK_VERTEX vertexNumber, 
			const G3D::Vector3& newCoord) 
{

    bool debug = true;
    if (debug) cdbg << "Disk::setDiskCoord() newCoord: " << newCoord << endl;
   
    switch(vertexNumber) {

    case VERTEX0:  /* m_Vertex0 = newCoord; */  return;
    case VERTEX1:  /* m_Vertex1 = newCoord; */  return;
    case VERTEX2:  /* m_Vertex2 = newCoord; */  return;
    case VERTEX3:  /* m_Vertex3 = newCoord; */  return;
    case VERTEX4:  /* m_Vertex4 = newCoord; */  return;
    case VERTEX5:  /* m_Vertex5 = newCoord; */  return;
    case VERTEX6:  /* m_Vertex6 = newCoord; */  return;
    case VERTEX7:  /* m_Vertex7 = newCoord; */  return;

    default:
	cdbg << "*** Error, Disk::setDiskCoord(): " 
	     << (int)vertexNumber << endl;
	return;
    }
}

G3D::Vector3 Disk::GetDiskCenter() const 
{

    bool debug = false;   
    if (debug) cdbg << "Disk::GetDiskCenter()" << endl;

    return  ( (this->GetDiskCoord(VERTEX0) + 
	       this->GetDiskCoord(VERTEX4) ) / 2.0f);
}

float Disk::GetDiskCircumference() const 
{
    /* Distance() method in Coord3D.h */
    float circumference = 
	(GetDiskCoord(VERTEX0) -  GetDiskCoord(VERTEX1)).length () +
	(GetDiskCoord(VERTEX1) - GetDiskCoord(VERTEX2)).length () +
	(GetDiskCoord(VERTEX2) - this->GetDiskCoord(VERTEX3)).length () +
	(GetDiskCoord(VERTEX3) - GetDiskCoord(VERTEX4)).length () +
	(GetDiskCoord(VERTEX4) - GetDiskCoord(VERTEX5)).length () +
	(GetDiskCoord(VERTEX5) - GetDiskCoord(VERTEX6)).length () +
	(GetDiskCoord(VERTEX6) - this->GetDiskCoord(VERTEX7)).length () +
	(GetDiskCoord(VERTEX7) - this->GetDiskCoord(VERTEX0)).length ();

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
   cdbg << " vertex0: " << this->GetDiskCoord(VERTEX0) << endl;
   cdbg << " vertex1: " << this->GetDiskCoord(VERTEX1) << endl;
   cdbg << " vertex2: " << this->GetDiskCoord(VERTEX2) << endl;
}

