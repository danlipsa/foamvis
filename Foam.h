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

class AttributeCreator;
class Body;
class Edge;
class Face;
class NameSemanticValue;
class ParsingData;

/**
 * Stores information  read from  a DMP file  produced by  the Surface
 * Evolver software.
 */
class Foam
{
public:
    typedef vector< boost::shared_ptr<Vertex> > Vertices;
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
    void SetViewMatrixElement (size_t i, float f)
    {
	m_viewMatrix[i] = f;
    }
    /**
     * Gets the view matrix
     * @return the 4x4 view matrix
     */
    const boost::array<float,16>& GetViewMatrix () const 
    {
	return m_viewMatrix;
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
    const G3D::AABox& GetAABox () const
    {
	return m_AABox;
    }
    /**
     * Gets the low point of the AABox of this Foam object
     */
    const G3D::Vector3& GetAABoxLow () const
    {
	return m_AABox.low ();
    }
    /**
     * Gets the high point of the AABox of this Foam object
     */
    const G3D::Vector3& GetAABoxHigh () const
    {
	return m_AABox.high ();
    }
    /**
     * Compares the low element of two data objects on the X,Y or Z axes
     * @return  true if  the  first  object is  less  than the  second
     * object, false otherwise.
     */
    void PostProcess ();

    const OOBox& GetOriginalDomain () const 
    {
	return m_originalDomain;
    }
    void SetPeriods (const G3D::Vector3& x, const G3D::Vector3& y,
		     const G3D::Vector3& z)
    {
	m_originalDomain.Set (x, y, z);
    }

    bool IsTorus () const;
    
    void SetSpaceDimension (size_t spaceDimension) 
    {
	m_spaceDimension = spaceDimension;
    }
    size_t GetSpaceDimension () const 
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

public:
    /**
     * Pretty print the Foam object
     */
    friend ostream& operator<< (ostream& ostr, const Foam& d);

private:
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
    boost::array<float, 16> m_viewMatrix;
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
