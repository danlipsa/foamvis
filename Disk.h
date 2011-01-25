/**
 * @file   Disk.h
 * @author Bob Laramee, Dan R. Lipsa
 *
 * Class that creates vertices and normals for a circle (aproximated
 * with an octogon) with arbitrary center and radius.
 */

#ifndef _DISK_H_
#define _DISK_H_

/**
 * Disk is intended to provide methods to perform common operations on 
 * a polygonal disk in 3D space.  Disk creates a polygonal disk without 
 * a hole in the center.  The disk has zero height.  What's great about
 * this disk object is that a programmer only needs to specify the 
 * center coordinate, the 12 o'clock coordinate (any other coordinate on
 * the disk edge), and the normal in order to instantiate it.  It is similar 
 * to Plane and Triangle.
 * Future versions may provide support for specifying the radial and
 * circumferential resolution of the polygonal representation.
 * For now, the polygonal representation is fixed as an octagon 
 * as follows (something like this):
 * <PRE>
 *                    V0 -12 o'clock coordinate
 *                ---o---
 *           /                 \
 *          /                   \
 *      V7 o                     o V1
 *        /                       \
 *       /                         \
 *
 *     |                            |
 *  V6 o              o (center)    o V2
 *     |                            |
 *
 */
class Disk 
{

public:

    /**
     * constructor -Any triangle can be used to instantiate a disk.  
     *              The center coordinate and the 12 o'clock vertex
     * determine the radius of the disk.
     * The parameters that used to be here have been removed so we can
     * create an ListArray of Disk e.g. stream tubes
     *
     * @see Disk::Initialize()
     */
    Disk();

    Disk (const G3D::Vector3& centerCoord, 
	  const G3D::Vector3& twelveOclockCoord, 
	  const G3D::Vector3& threeOclockCoord,
	  double radius)
    {
	Initialize (centerCoord, twelveOclockCoord, threeOclockCoord, radius);
    }


    
    /** 
     * destructor 
     */
    ~Disk();

    /** This enumerated type defines the disk/octagon vertices. */

    enum DISK_VERTEX {
	/** see ASCII image for position of each disk vertex */
	VERTEX0 = 0,
	VERTEX1 = 1,
	VERTEX2 = 2,
	VERTEX3 = 3,
	VERTEX4 = 4,
	VERTEX5 = 5,
	VERTEX6 = 6,
	VERTEX7 = 7,
	COUNT = 8,
    };

    /**
     * Unfortunately, we have to take the initialization out
     * of the constructor if we want an ListArray of Disks.
     * Otherwise we have to use an ListArray of pointers to
     * Disks.  This sort of thing never happens in Java.
     *
     * @param  centerCoord        -the center coordinate of the disk
     * @param  twelveOclockCoord  -the top coordinate of the disk (VERTEX0)
     * @param  threeOclockCoord   -the right coordinate of the disk (VERTEX2)   
     * @param  radius             -radius of the disk
     */
    void Initialize (const G3D::Vector3& centerCoord, 
		     const G3D::Vector3& twelveOclockCoord, 
		     const G3D::Vector3& threeOclockCoord, 
		     double radius);
    /**
     * @param  vertexNumber an identifier for the vertex either VERTEX0, VERTEX1,
     *         VERTEX2, VERTEX3, etc.
     * @return one of this disks's vertex coordinates
     */
    G3D::Vector3 GetVertex(int vertexNumber) const;

    /**
     * This method returns the next disk vertex index defined in the
     * DISK_INDEX enumeration.  Some examples:
     * <LI>  GetNextVertexIndex(DISK_INDEX 1) returns 2
     * <LI>  GetNextVertexIndex(DISK_INDEX 7) returns 0  <BR>
     * This is very handy for procedures that cycle around disk objects.   
     *
     * @from   OglStreamComet::RenderAsQuadStrips()
     * @param  thisVertexIndex
     * @return the next vertex index
     */
    static DISK_VERTEX GetNextVertexIndex(int thisVertexIndex);

    /**
     * This method returns the normal for the entire disk, i.e.
     * the normal to the plane that the disk is sitting in.
     * The normal to the disk is defined as the cross product
     * of the horizontal component vector (pointing towards
     * 3 o'clock) and the vertical component vector (pointing
     * towards 12 o'clcok).
     *
     * @return the normalized normal vector to the disk.
     */
    G3D::Vector3 GetNormal() const;

    /**
     * Note: unless otherwise specified, the normals at the vertices
     *       point radially outward from the center of the disk.
     *
     * @param  vertexNumber an identifier for the vertex either VERTEX0, VERTEX1,
     *         VERTEX2, VERTEX3, etc.
     * @return one of this disks's normals at the given vertex
     */
    G3D::Vector3 GetVertexNormal(int vertexNumber) const;

    /**
     * Returns the center coordinate of this disk.
     * <P>
     * @see    www.kitware.com, vtkDiskSource.h
     * @return the disk center as an x,y,z coordinate
     */
    G3D::Vector3 GetCenter() const;
    G3D::Vector3 GetTwelveOclock () const
    {
	return m_twelveOclock;
    }
    G3D::Vector3 GetThreeOclock () const
    {
	return m_threeOclock;
    }

    /**
     * @return the length of the line around this disk
     */
    float GetDiskCircumference() const;

    /**
     * Print a string representation of this triangle (for debugging).
     * Bob does not believe in operator overloading.
     */
    void Print() const;

    size_t size () const
    {
	return COUNT;
    }

    float GetRadius () const
    {
	return m_radius;
    }


private:
    double m_radius;

    /** the center disk coordinate */
    G3D::Vector3 m_center;
    /** the vertical component (from the center) of the disk to the outer rim */
    G3D::Vector3 m_twelveOclock;
    /** the horizontal component (from the center) of the disk to the outer rim */
    G3D::Vector3 m_threeOclock;
};

/**
 *
 */
inline bool operator==(const Disk& diskA, const Disk& diskB) { 

    return 
    (diskA.GetCenter () == diskB.GetCenter ()) &&
    (diskA.GetTwelveOclock() == diskB.GetTwelveOclock ()) &&
    (diskA.GetThreeOclock () == diskB.GetThreeOclock ());
}

#endif /* DISK */
