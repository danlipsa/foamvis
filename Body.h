/**
 * @file   Body.h
 * @author Dan R. Lipsa
 *
 * Declaration of the Body class
 */

#ifndef __BODY_H__
#define __BODY_H__

#include "Comparisons.h"
#include "Element.h"

class AttributesInfo;
class Edge;
class Face;
class OrientedFace;
class OrientedEdge;
class Vertex;

/**
 * A body is a list of oriented faces
 */
class Body : public Element
{
public:
    typedef vector<boost::shared_ptr<OrientedFace> > OrientedFaces;
public:
    /**
     * Creates a new body
     * @param id the original index for this body
     * @param faceIndexes 0 based indexes into a vector of Face objects
     * @param faces vector of Face objects
     */
    Body(const vector<int>& faceIndexes,
	 const vector< boost::shared_ptr<Face> >& faces,
	 size_t id, 
	 ElementStatus::Enum duplicateStatus = ElementStatus::ORIGINAL);

    /**
     * Returns the  vector of oriented faces this body is made of
     * @return a vector of oriented faces
     */
    OrientedFaces& GetOrientedFaces ()
    {
	return m_orientedFaces;
    }
    const OrientedFaces& GetOrientedFaces () const
    {
	return m_orientedFaces;
    }

    boost::shared_ptr<OrientedFace> GetOrientedFace (size_t i) const
    {
	return m_orientedFaces[i];
    }
    boost::shared_ptr<Face> GetFace (size_t i) const;
    size_t size () const
    {
	return m_orientedFaces.size ();
    }
    /**
     * Calculates the center
     */
    void CalculateCenter (size_t dimension, bool isQuadratic);
    /**
     * Gets the center
     * @return the center of the body
     */
    const G3D::Vector3& GetCenter () const
    {
	return m_center;
    }
    void UpdatePartOf (const boost::shared_ptr<Body>& body);
    bool HasWrap () const;
    string ToString () const;
    void GetVertexSet (VertexSet* vertexSet) const;
    void GetEdgeSet (EdgeSet* edgeSet) const;
    void GetFaceSet (FaceSet* faceSet) const;
    double GetPressure () const
    {
	return GetRealAttribute (PRESSURE_INDEX);
    }
    double GetPropertyValue (BodyProperty::Enum property) const;
    bool ExistsPropertyValue (BodyProperty::Enum property) const;
    bool operator< (const Body& other) const;
    G3D::Vector3 GetVelocity () const
    {
	return m_velocity;
    }
    void SetVelocity (const G3D::Vector3& velocity)
    {
	m_velocity = velocity;
    }
    
public:
    static const size_t PRESSURE_INDEX = 0;
    static const size_t VOLUME_INDEX = 1;

private:
    /**
     * Caches edges and vertices
     */
    void calculatePhysicalVertices (
	size_t dimension, bool isQuadratic,
	vector< boost::shared_ptr<Vertex> >* physicalVertices);

private:
    /**
     * Splits a  set of  vertices in  physical and
     * tesselation vertices
     * @param src source for the objects
     * @param destTessellation where we store tessellation objects
     * @param destPhysical where we store physical objects
     */
    static void splitTessellationPhysical (
	const VertexSet& src,
	vector< boost::shared_ptr<Vertex> >* destTessellation,
	vector< boost::shared_ptr<Vertex> >* destPhysical,
	size_t dimension, bool isQuadratic);

private:
    /**
     * Oriented faces that are part of this body.
     */
    OrientedFaces m_orientedFaces;
    /**
     * Center of the body
     */
    G3D::Vector3 m_center;
    G3D::AABox m_AABox;
    G3D::Vector3 m_velocity;
};

/**
 * Prety prints a Body
 * @param ostr where to print
 * @param b what to print
 * @return the stream where we printed.
 */
inline ostream& operator<< (ostream& ostr, const Body& b)
{
    return ostr << b.ToString ();
}
/**
 * Pretty prints a boost::shared_ptr<Body> 
 * @param ostr where to print
 * @param b what to print
 * @return where to print something else
 */
inline ostream& operator<< (ostream& ostr, const boost::shared_ptr<Body>& b)
{
    return ostr << *b << " useCount=" << b.use_count ();;
}

#endif //__BODY_H__

// Local Variables:
// mode: c++
// End:
