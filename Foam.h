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
#include "Enums.h"
#include "Hashes.h"
#include "OOBox.h"
#include "Statistics.h"

class AttributeCreator;
class Body;
class Edge;
class Face;
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


public:
    /**
     * Constructs a Foam object.
     */
    Foam ();

    void GetVertexSet (VertexSet* vertexSet) const;
    void GetEdgeSet (EdgeSet* edgeSet) const;
    void GetFaceSet (FaceSet* faceSet) const;

    /**
     * Gets ith body
     * @param i index of the body to be returned
     * @return the body
     */
    boost::shared_ptr<Body>  GetBody (size_t i) const
    {
	return m_bodies[i];
    }
    /**
     * Gets all bodies from the Foam
     * @return a vector of Body pointers
     */
    Bodies& GetBodies ()
    {
	return m_bodies;
    }
    const Bodies& GetBodies () const
    {
	return m_bodies;
    }

    /**
     * Stores a Body object in the Foam object
     * @param i index where to store the Body object
     * @param  faces vector of  faces that  form the  body. A  face is
     *         specified  using  an  index  of the  face  that  should
     *         already be *  stored in the Foam object.   If the index
     *         is negative, the face * that  is part of the Body is in
     *         reverse order  than the  Face that *  is stored  in the
     *         Foam object.
     * @param attributes the list of attributes
     */
    void SetBody (size_t i,  vector<int>& faces,
                  vector<NameSemanticValue*>& attributes);
    /**
     * Stores an element of the 4x4 view matrix.
     * @param i index where to store the elment
     * @param f the value to be stored
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
        auto_ptr<AttributeCreator> creator);
    /**
     * Gets a AABox of this Foam object
     * @return an AABox of this Foam object
     */
    const G3D::AABox& GetBoundingBox () const
    {
	return m_AABox;
    }
    /**
     * Gets the low point of the AABox of this Foam object
     */
    const G3D::Vector3& GetBoundingBoxLow () const
    {
	return m_AABox.low ();
    }
    /**
     * Gets the high point of the AABox of this Foam object
     */
    const G3D::Vector3& GetBoundingBoxHigh () const
    {
	return m_AABox.high ();
    }
    /**
     * Compares the low element of two data objects on the X,Y or Z axes
     * @return  true if  the  first  object is  less  than the  second
     * object, false otherwise.
     */
    void Preprocess ();

    const OOBox& GetOriginalDomain () const 
    {
	return m_originalDomain;
    }
    void SetPeriods (const G3D::Vector3& x, const G3D::Vector3& y,
		     const G3D::Vector3& z)
    {
	m_originalDomain.Set (x, y, z);
    }

    void SetPeriods (const G3D::Vector3& x, const G3D::Vector3& y);

    bool IsTorus () const;
    
    void SetDimension (size_t spaceDimension) 
    {
	m_spaceDimension = spaceDimension;
    }
    size_t GetDimension () const 
    {
	return m_spaceDimension;
    }
    bool IsQuadratic () const
    {
	return m_quadratic;
    }
    void SetQuadratic ()
    {
	m_quadratic = true;
    }
    const AttributesInfo& GetAttributesInfo (
	DefineAttribute::Enum attributeType) const;
    const Edges& GetStandaloneEdges () const
    {
	return m_standaloneEdges;
    }
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
    Bodies::iterator BodyInsideOriginalDomainStep (
	Bodies::iterator begin,
	VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet);
    /**
     * Specifies the default attributes for the Vertex object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    void AddDefaultVertexAttributes ();
    /**
     * Specifies the default attributes for an Edge object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    void AddDefaultEdgeAttributes ();
    /**
     * Specifies the default attributes for an Face object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    void AddDefaultFaceAttributes ();
    /**
     * Specifies the default attributes for the Body object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    void AddDefaultBodyAttributes ();
    void CalculateHistogramStatistics (BodyProperty::Enum property,
				       double min, double max);

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
    
    template <typename Accumulator>
    void Accumulate (Accumulator* acc, BodyProperty::Enum property) const;

public:
    /**
     * Pretty print the Foam object
     */
    friend ostream& operator<< (ostream& ostr, const Foam& d);

private:
    void adjustPressure ();
    void copyStandaloneElements ();
    /**
     * The vectors of vertices, edges, faces and bodies may have holes.
     * This function compacts the elements in those vectors so that it
     * eliminates the holes.
     */
    void compact ();
    /**
     * Calculate the bounding box for all vertices in this Foam
     */
    void calculateAABox ();
    /**
     * Calculate centers for all bodies.
     */
    void calculateBodiesCenters ();
    void calculateTorusClipped ();
    /**
     * Calculate faces part of a body, edges part of a face, ...
     */
    void updatePartOf ();
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
    void calculateAABoxForTorus (G3D::Vector3* low, G3D::Vector3* high);
    void calculateMinMaxStatistics ();
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


private:
    Edges m_standaloneEdges;
    Faces m_standaloneFaces;

    /**
     * A vector of bodies.
     */
    Bodies m_bodies;
    /**
     * View matrix for displaying vertices, edges, faces and bodies.
     */
    boost::shared_ptr<G3D::Matrix4> m_viewMatrix;
    OOBox m_originalDomain;
    /**
     * Vector of maps between the name of an attribute and information about it.
     * The indexes in the vector are for vertices, edges, faces, ...
     */
    boost::array<AttributesInfo, DefineAttribute::COUNT> m_attributesInfo;
    /**
     * Foam used in parsing the DMP file.
     */
    boost::scoped_ptr<ParsingData> m_parsingData;
    /**
     * The axially aligned bounding box for all vertices.
     */
    G3D::AABox m_AABox;
    size_t m_spaceDimension;
    bool m_quadratic;
    double m_min[BodyProperty::PROPERTY_END];
    double m_max[BodyProperty::PROPERTY_END];
    vector<HistogramStatistics> m_histogram;
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
