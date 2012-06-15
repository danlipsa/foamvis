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
class FoamProperties;
class OrientedFace;
class OrientedEdge;
class OOBox;
class Vertex;

/**
 * A body is a list of oriented faces
 */
class Body : public Element
{
public:
    typedef vector<boost::shared_ptr<OrientedFace> > OrientedFaces;
    typedef struct {
	G3D::Vector3int16 m_translation;
	boost::shared_ptr<Body> m_body;
	G3D::Vector3 m_centerReflection;
    } Neighbor;


public:
    /**
     * Creates a body for a bubble
     */
    Body(const vector<int>& faceIndexes,
	 const vector< boost::shared_ptr<Face> >& faces,
	 size_t id, const FoamProperties& foamParameters,
	 ElementStatus::Enum duplicateStatus = ElementStatus::ORIGINAL);
    /**
     * Creates a body for a constraint
     */
    Body (boost::shared_ptr<Face> face, size_t id, 
	  const FoamProperties& foamParameters);

    const vector<Neighbor>& GetNeighbors () const
    {
	return m_neighbors;
    }
    
    const Face& GetFace (size_t i) const;
    Face& GetFace (size_t i);
    void GetFaceSet (FaceSet* faceSet) const;
    const OrientedFace& GetOrientedFace (size_t i) const
    {
	return *m_orientedFaces[i];
    }
    boost::shared_ptr<OrientedFace> GetOrientedFacePtr (size_t i) const
    {
	return m_orientedFaces[i];
    }
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
    

    size_t size () const
    {
	return m_orientedFaces.size ();
    }
    /**
     * Calculates the center
     */
    void CalculateCenter ();
    void CalculateBoundingBox ();
    /**
     * Gets the center
     * @return the center of the body
     */
    const G3D::Vector3& GetCenter () const
    {
	return m_center;
    }
    void UpdateAdjacentBody (const boost::shared_ptr<Body>& body);
    string ToString () const;
    void GetVertexSet (VertexSet* vertexSet) const;    
    VertexSet GetVertexSet () const
    {
	VertexSet set;
	GetVertexSet (&set);
	return set;
    }
    void GetEdgeSet (EdgeSet* edgeSet) const;
    EdgeSet GetEdgeSet () const
    {
	EdgeSet set;
	GetEdgeSet (&set);
	return set;
    }
    double GetPropertyValue (BodyProperty::Enum property) const;
    void SetPressureValue (double value);
    void SetPressureDeduced ()
    {
	m_pressureDeduced = true;
    }
    void SetTargetVolumeDeduced ()
    {
	m_targetVolumeDeduced = true;
    }
    void SetActualVolumeDeduced ()
    {
	m_actualVolumeDeduced = true;
    }
    bool ExistsPropertyValue (BodyProperty::Enum property, 
			      bool* deduced = 0) const;

    bool operator< (const Body& other) const;
    bool operator< (size_t otherBodyId) const;

    G3D::Vector3 GetVelocity () const
    {
	return m_velocity;
    }
    void SetVelocity (const G3D::Vector3& velocity)
    {
	m_velocity = velocity;
    }
    const G3D::AABox& GetBoundingBox () const
    {
	return m_boundingBox;
    }
    
    size_t GetSidesPerBody () const;
    double GetPerimeterOverSqrtArea () const
    {
	return m_perimeterOverSqrtArea;
    }
    void CalculatePerimeterOverSqrtArea ();
    static const char* GetAttributeKeywordString (BodyProperty::Enum bp);
    void CalculateNeighbors2D (const OOBox& originalDomain);
    void CalculateDeformationTensor (const OOBox& originalDomain);
    float GetDeformationEigen () const;
    /**
     * The eigen values are sorted decreasing.
     */
    float GetDeformationEigenValue (size_t i) const
    {
	return m_deformationEigenValues[i];
    }
    G3D::Vector3 GetDeformationEigenVector (size_t i) const
    {
	return m_deformationEigenVectors[i];
    }
    bool IsConstraint () const
    {
	return m_constraint;
    }
    size_t GetConstraintIndex () const;

private:
    /**
     * Caches edges and vertices
     */
    void calculatePhysicalVertices (
	vector< boost::shared_ptr<Vertex> >* physicalVertices);

private:
    /**
     * Splits a  set of  vertices in  physical and
     * tesselation vertices
     * @param src source for the objects
     * @param destTessellation where we store tessellation objects
     * @param destPhysical where we store physical objects
     */
    void splitTessellationPhysical (
	const VertexSet& src,
	vector< boost::shared_ptr<Vertex> >* destTessellation,
	vector< boost::shared_ptr<Vertex> >* destPhysical);

private:
    /**
     * Oriented faces that are part of this body.
     */
    OrientedFaces m_orientedFaces;
    vector<Neighbor> m_neighbors;
    boost::array <G3D::Vector3, 3> m_deformationEigenVectors;
    boost::array <float, 3> m_deformationEigenValues;
    /**
     * Center of the body
     */
    G3D::Vector3 m_center;
    G3D::AABox m_boundingBox;
    G3D::Vector3 m_velocity;
    double m_perimeterOverSqrtArea;
    bool m_pressureDeduced;
    bool m_targetVolumeDeduced;
    bool m_actualVolumeDeduced;
    bool m_constraint;
    const FoamProperties& m_foamParameters;
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

template <int index>
class getBodyDeformationEigenValue
{
public:
    float operator () (const boost::shared_ptr<Body>& body)
    {
	return body->GetDeformationEigenValue (index);
    }
};


#endif //__BODY_H__

// Local Variables:
// mode: c++
// End:
