/**
 * @file   Body.h
 * @author Dan R. Lipsa
 * @brief A bubble.
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
class OOBox;
class Vertex;

/**
 * @brief A bubble.
 *
 * A body consists of a list of oriented faces
 */
class Body : public Element
{
public:
    typedef vector<boost::shared_ptr<OrientedFace> > OrientedFaces;
    /**
     * Store the neighbor m_body and an eventual translation for the periodic
     * domain (m_translation) or 
     * a point obtained by reflecting against a wall or object m_centerReflection
     */
    class Neighbor {
    public:
	Neighbor ();
	Neighbor (G3D::Vector3 centerOfReflection);
	Neighbor (boost::shared_ptr<Body> body, G3D::Vector3int16 translation);

	boost::shared_ptr<Body> GetBody () const
	{
	    return m_body;
	}
	G3D::Vector3int16 GetTranslation () const
	{
	    return m_translation;
	}
	G3D::Vector3 GetCenterReflection () const
	{
	    return m_centerReflection;
	}

    private:
	boost::shared_ptr<Body> m_body;
	G3D::Vector3int16 m_translation;
	G3D::Vector3 m_centerReflection;
    };


public:
    /**
     * Creates a body for a bubble
     */
    Body(const vector<int>& faceIndexes,
	 const vector< boost::shared_ptr<Face> >& faces,
	 size_t id,
	 ElementStatus::Enum duplicateStatus = ElementStatus::ORIGINAL);
    /**
     * Creates a body determined by a constraint. Usualy this is an
     * object interacting with the foam.
     */
    Body (boost::shared_ptr<Face> face, size_t id);
    
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
    
    bool Is2D () const
    {
        return m_orientedFaces.size () <= 1;
    }
    
    size_t GetFaceCount () const
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
    const G3D::Vector3 GetCenter () const
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
    float GetScalarValue (BodyScalar::Enum property) const;
    bool HasScalarValue (BodyScalar::Enum property, bool* deduced = 0) const;
    /**
     * BodyAttribute::GetNumberOfComponents (attribute) floats have to 
     * be alocated in 'value' for scalars, vectors and tensors
     */
    void GetAttributeValue (size_t attribute, float* value) const;
    G3D::Vector3 GetVelocity () const
    {
	return m_velocity;
    }


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

    bool operator< (const Body& other) const;
    bool operator< (size_t otherBodyId) const;

    void SetVelocity (const G3D::Vector3& velocity)
    {
	m_velocity = velocity;
    }
    const G3D::AABox& GetBoundingBox () const
    {
	return m_boundingBox;
    }
    
    size_t GetSidesPerBody () const;
    float GetDeformationSimple () const;
    float GetArea () const
    {
	return m_area;
    }
    
    void CalculateDeformationSimple ();
    static const char* GetAttributeKeywordString (BodyScalar::Enum bp);
    void CalculateDeformationTensor (const OOBox& originalDomain);
    G3D::Matrix3 GetDeformationTensor (
	const G3D::Matrix3& additionalRotation) const;
    void GetDeformationTensor (float* value, 
			       const G3D::Matrix3& additionalRotation) const;

    float GetDeformationEigenScalar () const;
    /**
     * The eigen values are sorted decreasing.
     */
    float GetDeformationEigenValue (size_t i) const
    {
	return m_deformationEigenValues[i];
    }
    G3D::Vector3 GetDeformationEigenValues () const
    {
	return G3D::Vector3 (
	    m_deformationEigenValues[0], m_deformationEigenValues[1], 
	    m_deformationEigenValues[2]);
    }
    G3D::Vector3 GetDeformationEigenVector (size_t i) const
    {
	return m_deformationEigenVectors[i];
    }

    /**
     * Objects are rigid and they interact with the foam. Objects are
     * defined by constraints.
     */
    bool IsObject () const
    {
	return m_object;
    }
    bool HasConstraints () const
    {
	return IsObject ();
    }
    size_t GetConstraintIndex () const;

    vtkSmartPointer<vtkPolyData> GetPolyData () const;

    /**
     * @pre {CalculateNeighborsAndGrowthRate}
     */
    const vector<Neighbor>& GetNeighbors () const
    {
	return m_neighbors;
    }
    /**
     * @pre {CalculateNeighborsAndGrowthRate}
     */
    float GetGrowthRate () const
    {
	return m_growthRate;
    }
    /**
     * @pre {Face::CalculateCentroidAndArea}
     * /home/dlipsa/data/foam/wetfoam_100_0002.dmp stores all 200 liquid channels
     * in a body.
     * @todo: handle this correctly
     */
    void CalculateNeighborsAndGrowthRate (
        const OOBox& originalDomain, bool is2D);
    /**
     * @pre CalculateNeighborsAndGrowthRate
     */
    bool HasFreeFace () const
    {
	return m_hasFreeFace;
    }
    float CalculateVolume () const;
    static float GetBubbleDiameter (float volume, bool is2D);
    float GetBubbleDiameter () const;


private:
    /**
     * Caches edges and vertices
     */
    void calculatePhysicalVertices (
	vector< boost::shared_ptr<Vertex> >* physicalVertices);
    float calculateArea () const;

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
    void calculateNeighbors2D (const OOBox& originalDomain);
    void calculateNeighbors3D (const OOBox& originalDomain);


private:
    /**
     * Oriented faces that are part of this body.
     */
    OrientedFaces m_orientedFaces;
    bool m_hasFreeFace;
    vector<Neighbor> m_neighbors;
    boost::array <G3D::Vector3, 3> m_deformationEigenVectors;
    boost::array <float, 3> m_deformationEigenValues;
    /**
     * Center of the body
     */
    G3D::Vector3 m_center;
    G3D::AABox m_boundingBox;
    G3D::Vector3 m_velocity;
    float m_area;
    float m_volume;
    float m_growthRate;
    float m_deformationSimple;
    bool m_pressureDeduced;
    bool m_targetVolumeDeduced;
    bool m_actualVolumeDeduced;
    bool m_object;
};


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
