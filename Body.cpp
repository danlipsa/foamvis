/**
 * @file   Body.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Body class
 */
#include "Attribute.h"
#include "AttributeCreator.h"
#include "AttributeInfo.h"
#include "Body.h"
#include "Edge.h"
#include "Foam.h"
#include "FoamProperties.h"
#include "Debug.h"
#include "DebugStream.h"
#include "Utils.h"
#include "Face.h"
#include "OrientedEdge.h"
#include "OrientedFace.h"
#include "ParsingData.h"
#include "ParsingDriver.h"
#include "ProcessBodyTorus.h"
#include "Vertex.h"

//#define __LOG__(code) code
#define __LOG__(code)


// Private Classes/Functions
// ======================================================================

/**
 * STL unary  function that converts a  signed index into  a vector of
 * Face  objects into a  OrientedFace object.  A negative  index means
 * that the Face object is listed  in reverse order in the Body object
 * than in the vector of Face objects.
 */
class indexToOrientedFace : 
    public unary_function<int, boost::shared_ptr<OrientedFace> >
{
public:
    /**
     * Constructor
     * @param faces vector of Face pointers. This is where the indexes point to.
     */
    indexToOrientedFace(const vector< boost::shared_ptr<Face> >& faces): 
	m_faces(faces) {}
    /**
     * Converts a 1-based index into an OrientedFace
     * @param i index into a vector of Face pointers
     * @return an OrientedFace pointer
     */
    boost::shared_ptr<OrientedFace> operator() (int i)
    {
        bool reversed = false;
        if (i < 0)
        {
            i = -i;
            reversed = true;
        }
        i--;
        return boost::make_shared<OrientedFace> (m_faces[i], reversed);
    }
private:
    /**
     * Vector of Face pointers
     */
    const vector<boost::shared_ptr<Face> >& m_faces;
};

class SymmetricMatrixEigen
{
public:
    SymmetricMatrixEigen (size_t size) : SIZE (size)
    {
	m_m = gsl_matrix_alloc (SIZE, SIZE);	
	m_eval = gsl_vector_alloc (SIZE);
	m_evec = gsl_matrix_alloc (SIZE, SIZE);     
	m_w = gsl_eigen_symmv_alloc (SIZE);
    }

    ~SymmetricMatrixEigen ()
    {
	gsl_eigen_symmv_free (m_w);
	gsl_matrix_free (m_evec);
	gsl_vector_free (m_eval);
	gsl_matrix_free (m_m);
    }

    void Calculate (
	const G3D::Matrix3& tensor,
	float eigenValues[3], G3D::Vector3 eigenVectors[3])
    {
	for (size_t i = 0; i < SIZE; ++i)
	    for (size_t j = 0; j < SIZE; ++j)
		gsl_matrix_set (m_m, i, j, tensor[i][j]);
	
	gsl_eigen_symmv (m_m, m_eval, m_evec, m_w);     
	gsl_eigen_symmv_sort (m_eval, m_evec, GSL_EIGEN_SORT_ABS_DESC);
       
	for (size_t i = 0; i < SIZE; ++i)
	{
	    eigenValues[i] = gsl_vector_get (m_eval, i);
	    for (size_t j = 0; j < SIZE; ++j)
	    {
		gsl_vector_view evec_i = gsl_matrix_column (m_evec, i);
		eigenVectors[i][j] = gsl_vector_get (&evec_i.vector, j);
	    }
	}
    }

private:
    const size_t SIZE;
    gsl_matrix* m_m;
    gsl_vector* m_eval;
    gsl_matrix* m_evec;
    gsl_eigen_symmv_workspace* m_w;
};


// Methods
// ======================================================================

Body::Body(
    const vector<int>& faceIndexes,
    const vector<boost::shared_ptr<Face> >& faces,
    size_t id, const FoamProperties& foamParameters, 
    ElementStatus::Enum duplicateStatus) :
    Element(id, duplicateStatus),
    m_perimeterOverSqrtArea (0),
    m_pressureDeduced (false),
    m_targetVolumeDeduced (false),
    m_actualVolumeDeduced (false),
    m_constraint (false),
    m_foamParameters (foamParameters)
{
    m_orientedFaces.resize (faceIndexes.size ());
    transform (faceIndexes.begin(), faceIndexes.end(), m_orientedFaces.begin(), 
               indexToOrientedFace(faces));
}

Body::Body (boost::shared_ptr<Face> face, size_t id, 
	    const FoamProperties& foamParameters) :
    Element (id, ElementStatus::ORIGINAL),
    m_perimeterOverSqrtArea (0),
    m_pressureDeduced (false),
    m_targetVolumeDeduced (false),
    m_actualVolumeDeduced (false),
    m_constraint (true),
    m_foamParameters (foamParameters)
{
    m_orientedFaces.resize (1);
    m_orientedFaces[0].reset (new OrientedFace (face, false));
}

void Body::calculatePhysicalVertices (
    vector< boost::shared_ptr<Vertex> >* physicalVertices)
{
    VertexSet vertices;
    vector< boost::shared_ptr<Vertex> > tessellationVertices;

    GetVertexSet (&vertices);
    splitTessellationPhysical (
	vertices, &tessellationVertices, physicalVertices);
}

void Body::splitTessellationPhysical (
    const VertexSet& src,
    vector< boost::shared_ptr<Vertex> >* destTessellation,
    vector< boost::shared_ptr<Vertex> >* destPhysical)
{
    destTessellation->resize (src.size ());
    copy (src.begin (), src.end (), destTessellation->begin ());
    vector< boost::shared_ptr<Vertex> >::iterator bp;
    bp = partition (destTessellation->begin (), destTessellation->end (), 
		    !boost::bind(&Vertex::IsPhysical, _1, 
				 m_foamParameters));
    destPhysical->resize (destTessellation->end () - bp);
    copy (bp, destTessellation->end (), destPhysical->begin ());
    destTessellation->resize (bp - destTessellation->begin ());
}


void Body::CalculateCenter ()
{
    if (m_foamParameters.Is2D ())
    {
	m_center = GetFace (0).GetCenter ();
	return;
    }
    vector< boost::shared_ptr<Vertex> > physicalVertices;
    calculatePhysicalVertices (&physicalVertices);
    size_t size = physicalVertices.size ();
    if (size >= 3)
    {	
	m_center = accumulate (
	    physicalVertices.begin (), physicalVertices.end (),
	    G3D::Vector3::zero (), 
	    boost::bind (plus<G3D::Vector3> (),
			 _1, boost::bind (&Vertex::GetVector, _2)));
    }
    else
    {
	VertexSet vertices;
	GetVertexSet (&vertices);
	size = vertices.size ();
	m_center = accumulate (
	    vertices.begin (), vertices.end (), G3D::Vector3::zero (), 
	    boost::bind (plus<G3D::Vector3> (),
			 _1, boost::bind (&Vertex::GetVector, _2)));
    }
    m_center /= G3D::Vector3(size, size, size);
}


void Body::UpdateAdjacentBody (const boost::shared_ptr<Body>& body)
{
    for (size_t i = 0; i < m_orientedFaces.size (); i++)
    {
	boost::shared_ptr<OrientedFace> of = m_orientedFaces[i];
	of->AddAdjacentBody (body, i);
	of->UpdateAdjacentFace (of);
    }
}


const Face& Body::GetFace (size_t i) const
{
    return *GetOrientedFace (i).GetFace ();
}

Face& Body::GetFace (size_t i)
{
    return *GetOrientedFace (i).GetFace ();
}

string Body::ToString () const
{
    ostringstream ostr;
    ostr << "Body " << GetId () << ":" << endl;
    ostr << m_orientedFaces.size () << " faces part of the body\n";
    BOOST_FOREACH (boost::shared_ptr<OrientedFace> of, m_orientedFaces)
	ostr << of->GetStringId ();
    if (HasAttributes ())
    {
	ostr << " Body attributes: ";
	PrintAttributes (ostr);
    }
    ostr << "\nBody center: " << m_center;
    if (IsConstraint ())
	ostr << " constraint";
    ostr << "\nEigen values: " 
	 << GetDeformationEigenValue (0) << ", "
	 << GetDeformationEigenValue (1)
	 << "\nEigen vectors: "
	 << GetDeformationEigenVector (0) << ", "
	 << GetDeformationEigenVector (1);
    return ostr.str ();
}

bool Body::operator< (const Body& other) const
{
    return GetId () < other.GetId ();
}

bool Body::operator< (size_t otherBodyId) const
{
    return GetId () < otherBodyId;
}

void Body::GetVertexSet (VertexSet* vertexSet) const
{
    const OrientedFaces& orientedFaces = GetOrientedFaces ();
    for_each (orientedFaces.begin (), orientedFaces.end (),
	      boost::bind (&OrientedFace::GetVertexSet, _1, vertexSet));
}

void Body::GetEdgeSet (EdgeSet* edgeSet) const
{
    const OrientedFaces& orientedFaces = GetOrientedFaces ();
    for_each (orientedFaces.begin (), orientedFaces.end (),
	      boost::bind (&OrientedFace::GetEdgeSet, _1, edgeSet));
}

void Body::GetFaceSet (FaceSet* faceSet) const
{
    const OrientedFaces& orientedFaces = GetOrientedFaces ();
    BOOST_FOREACH (boost::shared_ptr<OrientedFace> of, orientedFaces)
	faceSet->insert (of->GetFace ());
}

bool Body::ExistsPropertyValue (BodyProperty::Enum property, 
				bool* deduced) const
{
    if (deduced != 0)
	*deduced = false;
    switch (property)
    {
    case BodyProperty::DEFORMATION_P_OVER_SQRTA:
	return HasAttribute (BodyProperty::TARGET_VOLUME - 
			     BodyProperty::DMP_BEGIN);
    case BodyProperty::DEFORMATION_EIGEN:
	return ! IsConstraint ();
    case BodyProperty::PRESSURE:
	if (deduced != 0)
	    *deduced = m_pressureDeduced;
	return HasAttribute (property - BodyProperty::DMP_BEGIN);
    case BodyProperty::TARGET_VOLUME:
	if (deduced != 0)
	    *deduced = m_targetVolumeDeduced;
	return HasAttribute (property - BodyProperty::DMP_BEGIN);
    case BodyProperty::ACTUAL_VOLUME:
	if (deduced != 0)
	    *deduced = m_actualVolumeDeduced;
	return HasAttribute (property - BodyProperty::DMP_BEGIN);
    default:
	return true;
    }
}

double Body::GetPropertyValue (BodyProperty::Enum property) const
{
    switch (property)
    {
    case BodyProperty::VELOCITY_X:
	return GetVelocity ().x;
    case BodyProperty::VELOCITY_Y:
	return GetVelocity ().y;
    case BodyProperty::VELOCITY_Z:
	return GetVelocity ().z;
    case BodyProperty::VELOCITY_MAGNITUDE:
	return GetVelocity ().length ();
    case BodyProperty::SIDES_PER_BODY:
	return GetSidesPerBody ();
    case BodyProperty::DEFORMATION_P_OVER_SQRTA:
	return GetPerimeterOverSqrtArea ();
    case BodyProperty::DEFORMATION_EIGEN:
	return GetDeformationEigen ();
    case BodyProperty::COUNT:
	ThrowException ("Invalid BodyProperty: ", property);
    default:
	return GetAttribute<RealAttribute, double> (
	    property - BodyProperty::DMP_BEGIN);
    }
}

float Body::GetDeformationEigen () const
{
    float deformationEigen = 1. - 
	GetDeformationEigenValue (1) / GetDeformationEigenValue (0);
    return deformationEigen;
}

size_t Body::GetSidesPerBody () const
{
    size_t ofSize = m_orientedFaces.size ();
    if (ofSize == 1)
	return GetOrientedFace (0).GetFace ()->GetEdgesPerFace (
	    m_foamParameters);
    else
	return ofSize;
}


void Body::SetPressureValue (double value)
{
    SetAttribute<RealAttribute, double> (
	BodyProperty::PRESSURE - BodyProperty::DMP_BEGIN, value);
}

void Body::CalculateBoundingBox ()
{
    m_boundingBox = ::CalculateBoundingBox (*this);
}

void Body::CalculatePerimeterOverSqrtArea ()
{
    if (m_orientedFaces.size () == 1 && 
	ExistsPropertyValue (BodyProperty::TARGET_VOLUME))
    {
	boost::shared_ptr<OrientedFace> of = GetOrientedFacePtr (0);
	of->CalculatePerimeter ();
	m_perimeterOverSqrtArea = of->GetPerimeter () / 
	    sqrt (GetPropertyValue (BodyProperty::TARGET_VOLUME));
    }
}

const char* Body::GetAttributeKeywordString (BodyProperty::Enum bp)
{
    using EvolverData::parser;
    switch (bp)
    {
    case BodyProperty::PRESSURE:
	return ParsingDriver::GetKeywordString(
	    parser::token::LAGRANGE_MULTIPLIER);
    case BodyProperty::TARGET_VOLUME:
	return ParsingDriver::GetKeywordString(parser::token::VOLUME);	
    case BodyProperty::ACTUAL_VOLUME:
	return ParsingDriver::GetKeywordString(parser::token::ACTUAL);
    default:
	return 0;
    }
}

void Body::CalculateNeighbors2D (const OOBox& originalDomain)
{
    const OrientedFace& of = GetOrientedFace (0);
    m_neighbors.resize (of.size ());
    size_t j = 0;
    for (size_t i = 0; i < of.size (); ++i, ++j)
    {
	OrientedEdge oe = of.GetOrientedEdge (i);
	const AdjacentOrientedFaces& aofs = oe.GetAdjacentFaces ();
	if (oe.HasConstraints ())
	{	    
	    G3D::Vector3 b = oe.GetBeginVector ();
	    G3D::Vector3 e = oe.GetEndVector ();
	    G3D::Vector3 m = (b + e) / 2;	    
	    /*
	     * Another way to do the reflection.
	    size_t i = oe.GetPointCount () / 2;
	    G3D::Vector3 m = oe.GetPoint (i);
	    */
	    G3D::Vector3 c = GetCenter ();
	    m_neighbors[j].m_centerReflection = c + 2 * (m - c);
	}
	else
	{
	    AdjacentOrientedFaces::const_iterator it = aofs.begin ();
	    while (it != aofs.end () && (it->IsStandalone () ||
					 it->GetBodyId () == GetId ()))
		++it;
	    if (it == aofs.end ())
	    {
		--j;
		continue;
	    }
	    boost::shared_ptr<Body> neighbor = it->GetBody ();
	    m_neighbors[j].m_body = neighbor;
	    G3D::Vector3int16 translation;
	    originalDomain.IsWrap (GetCenter (), neighbor->GetCenter (),
				   &translation);
	    m_neighbors[j].m_translation = Vector3int16Zero - translation;
	}
	RuntimeAssert (aofs.size () <= 2, 
		       "AdjacentOrientedFaces size > 2: ", aofs.size ());
    }
    m_neighbors.resize (j);
}

void Body::CalculateDeformationTensor (const OOBox& originalDomain)
{
    if (IsConstraint ())
	return;
    size_t bubbleNeighborsCount = 0;
    G3D::Matrix3 textureTensor = G3D::Matrix3::zero ();
    const vector<Neighbor>& neighbors = GetNeighbors ();
    BOOST_FOREACH (Body::Neighbor neighbor, neighbors)
    {
	G3D::Vector3 s;
	if (neighbor.m_body)
	{
	    ++bubbleNeighborsCount;
	    s = neighbor.m_body->GetCenter ();
	}
	else 
	{
	    // debug: no reflection used in average computation.
	    // continue;
	    s = neighbor.m_centerReflection;
	}
	G3D::Vector3 first = GetCenter ();
	G3D::Vector3 second = 
	    originalDomain.TorusTranslate (s, neighbor.m_translation);
	G3D::Vector3 l = second - first;
	textureTensor += G3D::Matrix3 (l.x * l.x, l.x * l.y, l.x * l.z,
				       l.y * l.x, l.y * l.y, l.y * l.z,
				       l.z * l.x, l.z * l.y, l.z * l.z);
    }
    textureTensor /= neighbors.size ();
    SymmetricMatrixEigen(3).Calculate (textureTensor, 
				       &m_deformationEigenValues[0], 
				       &m_deformationEigenVectors[0]);
    //textureTensor.eigenSolveSymmetric (
    //&m_deformationEigenValues[0], &m_deformationEigenVectors[0]);

    __LOG__(
	ostream_iterator<float> of(cdbg, " ");
	copy (m_deformationEigenValues.begin (), 
	      m_deformationEigenValues.end (), of);
	cdbg << endl;
	ostream_iterator<G3D::Vector3> ov (cdbg, "\n");
	copy (m_deformationEigenVectors.begin (), 
	      m_deformationEigenVectors.end (), ov);
	);
    if (bubbleNeighborsCount == 0)
	m_constraint = true;
}

size_t Body::GetConstraintIndex () const
{
    return GetFace (0).GetOrientedEdge (0).GetConstraintIndex ();
}
