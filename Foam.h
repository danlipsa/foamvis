/**
 * @file Foam.h
 * @author Dan R. Lipsa
 *
 * Declaration of the Foam class
 */
#ifndef __FOAM_H__
#define __FOAM_H__

#include "AttributeInfo.h"
#include "Comparisons.h"
#include "ParsingEnums.h"
#include "Enums.h"
#include "ForcesOneObject.h"
#include "Hashes.h"
#include "HistogramStatistics.h"
#include "ObjectPosition.h"
#include "OOBox.h"

class AttributeCreator;
class Body;
class ConstraintEdge;
class Edge;
class Face;
class FoamProperties;
class NameSemanticValue;
class ParsingData;

/**
 * Stores information about a foam. A foam is composed of bodies,
 * faces, edges and vertices and is read from a DMP file produced by
 * the Surface Evolver software.
 */
class Foam
{
public:
    typedef vector< boost::shared_ptr<Edge> > Edges;
    typedef vector< boost::shared_ptr<Face> > Faces;
    typedef vector< boost::shared_ptr<Body> > Bodies;
    enum ParametersOperation 
    {
	SET_FOAM_PROPERTIES,
	TEST_FOAM_PROPERTIES
    };


public:
    /**
     * Constructs a Foam object.
     */
    Foam (bool useOriginal, 
	  const DmpObjectInfo& dmpObjectInfo,
	  const vector<ForcesOneObjectNames>& forcesNames,
	  FoamProperties& foamParameters, ParametersOperation paramsOp);

    void GetVertexSet (VertexSet* vertexSet) const;
    VertexSet GetVertexSet () const
    {
	VertexSet vertexSet;
	GetVertexSet (&vertexSet);
	return vertexSet;
    }
    void GetEdgeSet (EdgeSet* edgeSet) const;
    EdgeSet GetEdgeSet () const
    {
	EdgeSet edgeSet;
	GetEdgeSet (&edgeSet);
	return edgeSet;
    }
    size_t GetLastEdgeId (const EdgeSet& edgeSet) const;
    void GetFaceSet (FaceSet* faceSet) const;
    FaceSet GetFaceSet () const
    {
	FaceSet faceSet;
	GetFaceSet (&faceSet);
	return faceSet;
    }
    size_t GetLastFaceId () const;

    /**
     * Gets ith body
     */
    Body& GetBody (size_t index) const
    {
	return *m_bodies[index];
    }
    boost::shared_ptr<Body> GetBodyPtr (size_t index) const
    {
	return m_bodies[index];
    }


    Bodies::const_iterator FindBody (size_t bodyId) const;
    bool ExistsBodyWithValueIn (
	BodyProperty::Enum property, const QwtDoubleInterval& interval) const;

    /**
     * Gets all bodies from the Foam
     * @return a vector of Bodys sorted by ID
     */
    Bodies& GetBodies ()
    {
	return m_bodies;
    }
    const Bodies& GetBodies () const
    {
	return m_bodies;
    }
    size_t GetLastBodyId () const;

    /**
     * Stores a Body object in the Foam object
     */
    void SetBody (size_t i,  vector<int>& faces,
                  vector<NameSemanticValue*>& attributes, 
		  bool useOriginal);
    /**
     * Stores an element of the 4x4 view matrix.
     */
    void SetViewMatrix (
	double r1c1, double r1c2, double r1c3, double r1c4, 
	double r2c1, double r2c2, double r2c3, double r2c4, 
	double r3c1, double r3c2, double r3c3, double r3c4, 
	double r4c1, double r4c2, double r4c3, double r4c4);
    /**
     * Gets the view matrix
     * @return the 4x4 view matrix
     */
    const G3D::Matrix4& GetViewMatrix () const 
    {
	return *m_viewMatrix;
    }
    /**
     * Make the parsing data accessible
     * @return reference to the ParsingData object.
     */
    ParsingData& GetParsingData () 
    {
	return *m_parsingData;
    }
    /**
     * Deletes the parsing data
     */
    void ReleaseParsingData ();
    /**
     * Stores information about an attribute.
     * @param type the type of attribute (@see DefineAttribute)
     * @param name the name of the attribute
     * @param creator knows to create the attribute (@see AttributeCreator)
     */
    void AddAttributeInfo (
        DefineAttribute::Enum type, const char* name,
        boost::shared_ptr<AttributeCreator> creator);
    /**
     * Gets a AABox of this Foam object
     * @return an AABox of this Foam object
     */
    const G3D::AABox& GetBoundingBox () const
    {
	return m_boundingBox;
    }
    const G3D::AABox& GetBoundingBoxTorus () const
    {
	return m_boundingBoxTorus;
    }
    

    /**
     * Compares the low element of two data objects on the X,Y or Z axes
     * @return  true if  the  first  object is  less  than the  second
     * object, false otherwise.
     */
    void Preprocess ();

    const OOBox& GetTorusDomain () const 
    {
	return m_torusDomain;
    }
    void SetTorusDomain (const G3D::Vector3& x, const G3D::Vector3& y,
		     const G3D::Vector3& z)
    {
	m_torusDomain.Set (x, y, z);
    }

    void SetTorusDomain (const G3D::Vector3& x, const G3D::Vector3& y);
    bool IsTorus () const;
    
    const AttributesInfo& GetAttributesInfo (
	DefineAttribute::Enum attributeType) const;
    const Edges& GetStandaloneEdges () const
    {
	return m_standaloneEdges;
    }
    const Edges& GetConstraintEdges (size_t constraintIndex) const
    {
	return *m_constraintEdges[constraintIndex];
    }
    void SortConstraintEdges (size_t constraintIndex);

    const Faces& GetStandaloneFaces () const
    {
	return m_standaloneFaces;
    }
    boost::shared_ptr<Edge> GetStandardEdge () const;

    /**
     * Insures a body's center is inside the original domain.
     * @return true if the body is already inside the original domain,
     *         so no translation is necessary, false otherwise
     */
    Foam::Bodies::iterator BodyInsideOriginalDomainStep (
	Foam::Bodies::iterator begin,
	VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet);
    /**
     * Specifies the default attributes for the Vertex object.
     * These attributes don't appear as a DEFINE in the .DMP file
     */
    void AddDefaultVertexAttributes ();
    /**
     * Specifies the default attributes for an Edge object.
     * These attributes don't appear as a DEFINE in the .DMP file
     */
    void AddDefaultEdgeAttributes ();
    /**
     * Specifies the default attributes for an Face object.
     * These attributes don't appear as a DEFINE in the .DMP file
     */
    void AddDefaultFaceAttributes ();
    /**
     * Specifies the default attributes for the Body object.
     * These attributes don't appear as a DEFINE in the .DMP file
     */
    void AddDefaultBodyAttributes ();
    template <typename Accumulator>
    void AccumulateProperty (
	Accumulator* acc, BodyProperty::Enum property) const;
    template <typename Accumulator, typename GetBodyProperty>
    void Accumulate (Accumulator* acc, GetBodyProperty getBodyProperty) const;

    void AdjustPressure (double adjustment);

    void CalculateHistogramStatistics (BodyProperty::Enum property,
				       double min, double max);
    double CalculateMedian (BodyProperty::Enum property);

    const HistogramStatistics& GetHistogram (BodyProperty::Enum property) const
    {
	return m_histogram[property];
    }

    double GetMin (BodyProperty::Enum property) const
    {
	return m_min[property];
    }

    double GetMax (BodyProperty::Enum property) const
    {
	return m_max[property];
    }

    QwtDoubleInterval GetRange (BodyProperty::Enum property) const
    {
	return QwtDoubleInterval (GetMin (property), GetMax (property));
    }
    
    void CalculateMinMaxStatistics ();
    void SetDmpObjectPosition (const DmpObjectInfo& dmpObjectInfo);
    ObjectPosition GetDmpObjectPosition () const
    {
	return m_dmpObjectPosition;
    }
    G3D::Vector2 GetAverageAroundAxis (size_t bodyId, size_t secondBodyId) const;
    const vector<ForcesOneObject>& GetForces () const
    {
	return m_forces;
    }
    void SetForcesAllObjects ();

    /**
     * Calculate the bounding box for all vertices in this Foam
     */
    void CalculateBoundingBox ();
    void CalculatePerimeterOverArea ();
    void FixConstraintPoints (const Foam* prevFoam);
    void StoreAttribute (Body* body, BodyProperty::Enum property, double r);
    void CalculateBodyNeighbors ();
    void CalculateBodyDeformationTensor ();
    void CreateConstraintBody (size_t constraint);
    bool Is2D () const;
    bool IsQuadratic () const;
    const FoamProperties& GetProperties () const
    {
	return m_properties;
    }
    void SetSpaceDimension (size_t spaceDimension);
    void SetQuadratic (bool quadratic);

public:
    static const double Z_COORDINATE_2D = 0.0;

public:
    /**
     * Pretty print the Foam object
     */
    friend ostream& operator<< (ostream& ostr, const Foam& d);

private:
    void setForcesOneObject (const ForcesOneObjectNames& names, ForcesOneObject* forces);
    void copyStandaloneElements ();
    /**
     * The vectors of vertices, edges, faces and bodies may have holes.
     * This function compacts the elements in those vectors so that it
     * eliminates the holes.
     */
    void compact ();
    /**
     * Calculate centers for all bodies.
     */
    void calculateBodiesCenters ();
    void calculateTorusClipped ();
    /**
     * Calculate faces part of a body, edges part of a face, ...
     */
    void updateAdjacent ();
    bool bodyInsideOriginalDomain (
	const boost::shared_ptr<Body>& body,
	VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet);
    void bodiesInsideOriginalDomain (
	VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet);

    /**
     * Calculates the AABOX for the the foam and the 8 corners of the 
     * original domain.
     * @params low low point of the aabox for the Foam (input/output)
     * @params high high point of the aabox for the foam (input/output)
     */
    void calculateBoundingBoxTorus (G3D::Vector3* low, G3D::Vector3* high);
    void calculateMinMaxStatistics (BodyProperty::Enum property);

    void unwrap (VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet);
    void unwrap (boost::shared_ptr<Edge> edge, VertexSet* vertexSet) const;
    void unwrap (boost::shared_ptr<Face> face, 
		 VertexSet* vertexSet, EdgeSet* edgeSet) const;
    void unwrap (boost::shared_ptr<Body> body,
		 VertexSet* vertexSet, EdgeSet* edgeSet, 
		 FaceSet* faceSet) const;
    void bodyTranslate (
	const boost::shared_ptr<Body>& body,
	const G3D::Vector3int16& translate,
	VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet);
    void setMissingPressureZero ();
    void setMissingVolume ();
    void addConstraintEdges ();
    boost::shared_ptr<ConstraintEdge> calculateConstraintEdge (
	boost::shared_ptr<Vertex> begin, boost::shared_ptr<Vertex> end,
	size_t id, size_t bodyIndex, VertexSet* vertexSet, EdgeSet* edgeSet);
    bool isVectorOnConstraint (const G3D::Vector3& v, 
			       size_t constraintIndex) const;
    G3D::Vector3int16 getVectorOnConstraintTranslation (
	const G3D::Vector3& v, size_t constraintIndex) const;

private:
    // the torus original domain can be different for each time step, as in 
    // shear_160
    OOBox m_torusDomain;
    Edges m_standaloneEdges;
    vector< boost::shared_ptr<Edges> > m_constraintEdges;
    Faces m_standaloneFaces;

    /**
     * A vector of bodies sorted by ID.
     */
    Bodies m_bodies;
    /**
     * View matrix for displaying vertices, edges, faces and bodies.
     */
    boost::shared_ptr<G3D::Matrix4> m_viewMatrix;
    /**
     * Vector of maps between the name of an attribute and information about it.
     * The indexes in the vector are for vertices, edges, faces, ...
     */
    boost::array<AttributesInfo, DefineAttribute::COUNT> m_attributesInfo;
    /**
     * Data used in parsing the DMP file.
     */
    boost::scoped_ptr<ParsingData> m_parsingData;
    /**
     * The axially aligned bounding box for all vertices.
     */
    G3D::AABox m_boundingBox;
    G3D::AABox m_boundingBoxTorus;

    double m_min[BodyProperty::COUNT];
    double m_max[BodyProperty::COUNT];
    vector<HistogramStatistics> m_histogram;
    ObjectPosition m_dmpObjectPosition;
    vector<ForcesOneObject> m_forces;
    /*
     * AdjacentBody, PointIndex for constraint points that need fixing.
     */
    vector< pair<size_t, size_t> > m_constraintPointsToFix;
    FoamProperties& m_properties;    
    ParametersOperation m_parametersOperation;
};

/**
 * Pretty prints a Foam*
 * @param ostr where to print
 * @param d what to print
 * @return where to print something else
 */
inline ostream& operator<< (ostream& ostr, const Foam* d)
{
    return ostr << *d;
}

#endif //__FOAM_H__

// Local Variables:
// mode: c++
// End:
