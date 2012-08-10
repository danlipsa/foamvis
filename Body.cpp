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
#include "DataProperties.h"
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

Body::Neighbor pairToNeighbor (const pair<size_t, Body::Neighbor>& p)
{
    return p.second;
}


// Methods Body::Neighbor
// ======================================================================
Body::Neighbor::Neighbor ()
{
}

Body::Neighbor::Neighbor (G3D::Vector3 centerOfReflection) :
    m_centerReflection (centerOfReflection)
{
}

Body::Neighbor::Neighbor (
    boost::shared_ptr<Body> body, G3D::Vector3int16 translation) :

    m_body (body),
    m_translation (translation), 
    m_centerReflection (G3D::Vector3::zero ())
{
}

// Methods Body
// ======================================================================

Body::Body(
    const vector<int>& faceIndexes,
    const vector<boost::shared_ptr<Face> >& faces,
    size_t id,
    ElementStatus::Enum duplicateStatus) :
    Element(id, duplicateStatus),
    m_area (0),
    m_growthRate (0),
    m_deformationSimple (0),
    m_pressureDeduced (false),
    m_targetVolumeDeduced (false),
    m_actualVolumeDeduced (false),
    m_object (false)
{
    m_orientedFaces.resize (faceIndexes.size ());
    transform (faceIndexes.begin(), faceIndexes.end(), m_orientedFaces.begin(), 
               indexToOrientedFace(faces));
}

Body::Body (boost::shared_ptr<Face> face, size_t id) :
    Element (id, ElementStatus::ORIGINAL),
    m_deformationSimple (0),
    m_pressureDeduced (false),
    m_targetVolumeDeduced (false),
    m_actualVolumeDeduced (false),
    m_object (true)
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
		    !boost::bind(&Vertex::IsPhysical, _1));
    destPhysical->resize (destTessellation->end () - bp);
    copy (bp, destTessellation->end (), destPhysical->begin ());
    destTessellation->resize (bp - destTessellation->begin ());
}


void Body::CalculateCenter ()
{
    if (DATA_PROPERTIES.Is2D ())
    {
	m_center = GetFace (0).GetCenter ();
    }
    else
    {
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

void Body::GetFaceSet (FaceSet* faceSet) const
{
    const OrientedFaces& orientedFaces = GetOrientedFaces ();
    BOOST_FOREACH (boost::shared_ptr<OrientedFace> of, orientedFaces)
	faceSet->insert (of->GetFace ());
}

string Body::ToString (const AttributesInfo* ai) const
{
    ostringstream ostr;
    ostr << "Body " << GetId ();
    if (IsObject ())
	ostr << " Object (" << GetConstraintIndex () << "):";
    else 
	ostr << " Bubble:";
    ostr << endl;
    ostr << m_orientedFaces.size () << " faces part of the body\n";
    BOOST_FOREACH (boost::shared_ptr<OrientedFace> of, m_orientedFaces)
	ostr << of->GetStringId () << " ";
    if (HasAttributes ())
    {
	ostr << "\nBody attributes: ";
	PrintAttributes (ostr, ai);
    }
    ostr << "\nBody center: " << m_center;
    ostr << "\nEigen values: " 
	 << GetDeformationEigenValues ()
	 << "\nEigen vectors: "
	 << GetDeformationEigenVector (0) << ", "
	 << GetDeformationEigenVector (1) << ", " 
	 << GetDeformationEigenVector (2);
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
    return OrientedFace::GetVertexSetV (GetOrientedFaces (), vertexSet);
}

void Body::GetEdgeSet (EdgeSet* edgeSet) const
{
    const OrientedFaces& orientedFaces = GetOrientedFaces ();
    for_each (orientedFaces.begin (), orientedFaces.end (),
	      boost::bind (&OrientedFace::GetEdgeSet, _1, edgeSet));
}

bool Body::HasScalarValue (BodyScalar::Enum property, 
				bool* deduced) const
{
    setPointerValue (deduced, false);
    if (IsObject ())
    {
	if (property == BodyScalar::VELOCITY_X ||
	    property == BodyScalar::VELOCITY_Y ||
	    property == BodyScalar::VELOCITY_MAGNITUDE)
	{
	    setPointerValue (deduced, true);
	    return true;
	}
	else
	    return false;
    }
    switch (property)
    {
    case BodyScalar::DEFORMATION_SIMPLE:
	return HasAttribute (BodyScalar::TARGET_VOLUME - 
			     BodyScalar::DMP_BEGIN);
    case BodyScalar::PRESSURE:
	setPointerValue (deduced, m_pressureDeduced);
	return HasAttribute (property - BodyScalar::DMP_BEGIN);
    case BodyScalar::TARGET_VOLUME:
	setPointerValue (deduced, m_targetVolumeDeduced);
	return HasAttribute (property - BodyScalar::DMP_BEGIN);
    case BodyScalar::ACTUAL_VOLUME:
	setPointerValue (deduced, m_actualVolumeDeduced);
	return HasAttribute (property - BodyScalar::DMP_BEGIN);
    default:
	return true;
    }
}

float Body::GetScalarValue (BodyScalar::Enum property) const
{
    switch (property)
    {
    case BodyScalar::VELOCITY_X:
	return GetVelocity ().x;
    case BodyScalar::VELOCITY_Y:
	return GetVelocity ().y;
    case BodyScalar::VELOCITY_Z:
	return GetVelocity ().z;
    case BodyScalar::VELOCITY_MAGNITUDE:
	return GetVelocity ().length ();
    case BodyScalar::SIDES_PER_BUBBLE:
	return GetSidesPerBody ();
    case BodyScalar::DEFORMATION_SIMPLE:
	return GetDeformationSimple ();
    case BodyScalar::DEFORMATION_EIGEN:
	return GetDeformationEigenScalar ();
    case BodyScalar::PRESSURE:
    case BodyScalar::TARGET_VOLUME:
    case BodyScalar::ACTUAL_VOLUME:
	return GetAttribute<RealAttribute, double> (
	    property - BodyScalar::DMP_BEGIN);
    case BodyScalar::GROWTH_RATE:
	return GetGrowthRate ();
    case BodyScalar::COUNT:
	ThrowException ("Invalid BodyScalar: ", property);
	return 0;
    }
    return 0;
}

void Body::GetAttributeValue (size_t attribute, float* value)
{
    if (BodyAttribute::IsScalar (attribute))
    {
	BodyScalar::Enum bodyScalar = BodyScalar::FromSizeT(attribute);
	float v = HasScalarValue (bodyScalar) ? GetScalarValue (bodyScalar) : 0;
	*value = v;
    }
    else if (BodyAttribute::IsVector (attribute))
    {
	G3D::Vector3 v = GetVelocity ();
	value[0] = v.x;
	value[1] = v.y;
	value[2] = v.z;
    }
    else
	GetDeformationTensor (value, G3D::Matrix3::identity ());
}

float Body::GetDeformationSimple () const
{
    return m_deformationSimple;
}

G3D::Matrix3 Body::GetDeformationTensor (
    const G3D::Matrix3& additionalRotation) const
{
    // Practical Linear Algebra, A Geometry Toolbox, 
    // Gerald Farin, Dianne Hansford, Sec 7.5
    G3D::Matrix3 l = G3D::Matrix3::fromDiagonal (GetDeformationEigenValues ());
    G3D::Matrix3 r = 
	additionalRotation *
	MatrixFromColumns (GetDeformationEigenVector (0),
			   GetDeformationEigenVector (1),
			   GetDeformationEigenVector (2));
    return r * l * r.transpose ();
}

float Body::GetDeformationEigenScalar () const
{
    size_t maxIndex = 0;
    size_t minIndex = DATA_PROPERTIES.Is2D () ? 1 : 2;
    float deformationEigen = 1. - GetDeformationEigenValue (minIndex) / 
	GetDeformationEigenValue (maxIndex);
    return deformationEigen;
}

size_t Body::GetSidesPerBody () const
{
    size_t ofSize = m_orientedFaces.size ();
    if (ofSize == 1)
	return GetOrientedFace (0).GetFace ()->GetEdgesPerFace ();
    else
	return ofSize;
}


void Body::SetPressureValue (double value)
{
    SetAttribute<RealAttribute, double> (
	BodyScalar::PRESSURE - BodyScalar::DMP_BEGIN, value);
}

void Body::CalculateBoundingBox ()
{
    m_boundingBox = ::CalculateBoundingBox (*this);
}

void Body::calculateArea ()
{
    m_area = 0;
    BOOST_FOREACH (boost::shared_ptr<OrientedFace> of, GetOrientedFaces ())
	m_area += (of->GetArea ());
}


void Body::CalculateDeformationSimple ()
{
    if (! HasScalarValue (BodyScalar::TARGET_VOLUME))
	return;    
    calculateArea ();
    if (DATA_PROPERTIES.Is2D ())
    {
	boost::shared_ptr<OrientedFace> of = GetOrientedFacePtr (0);
	of->CalculatePerimeter ();
	m_deformationSimple = of->GetPerimeter () / 
	    sqrt (GetScalarValue (BodyScalar::TARGET_VOLUME));
    }
    else
	m_deformationSimple = 
	    GetArea () / 
	    pow (GetScalarValue (BodyScalar::TARGET_VOLUME), 
		 static_cast<float>(2.0 / 3.0));
}

const char* Body::GetAttributeKeywordString (BodyScalar::Enum bp)
{
    using EvolverData::parser;
    switch (bp)
    {
    case BodyScalar::PRESSURE:
	return ParsingDriver::GetKeywordString(
	    parser::token::LAGRANGE_MULTIPLIER);
    case BodyScalar::TARGET_VOLUME:
	return ParsingDriver::GetKeywordString(parser::token::VOLUME);	
    case BodyScalar::ACTUAL_VOLUME:
	return ParsingDriver::GetKeywordString(parser::token::ACTUAL);
    default:
	return 0;
    }
}

void Body::CalculateNeighborsAndGrowthRate (const OOBox& originalDomain)
{
    if (DATA_PROPERTIES.Is2D ())
	calculateNeighbors2D (originalDomain);
    else
	calculateNeighbors3D (originalDomain);
}


void Body::calculateNeighbors3D (const OOBox& originalDomain)
{
    set<size_t> neighborId;
    BOOST_FOREACH (boost::shared_ptr<OrientedFace> of, GetOrientedFaces ())
    {
	// wall faces do not create neighbors (have only this as adjacent body)
	if (of->HasConstraints ())
	{
	    // reflect
	}
	else if (of->GetAdjacentBodySize () == 2)
	{
	    const AdjacentBody& ab = of->GetAdjacentBody (true);
	    boost::shared_ptr<Body> body = ab.GetBody ();
	    G3D::Vector3int16 translation;
	    originalDomain.IsWrap (GetCenter (), body->GetCenter (),
				   &translation);
	    Neighbor neighbor (body, Vector3int16Zero - translation);
	    // The calculations can be executed on original pressure
	    // (medians are not aligned) yielding the same results.
	    m_growthRate += (GetScalarValue (BodyScalar::PRESSURE) - 
			     body->GetScalarValue (BodyScalar::PRESSURE)) * 
		of->GetArea ();
	    // insert the neighbor into the map
	    pair< set<size_t>::iterator, bool> p = 
		neighborId.insert (body->GetId ());
	    if (p.second)
		// Neighbor that has not been seen before.
		// As a physical face can have several tessellation faces
		// a neighbor appears several times.
		m_neighbors.push_back (neighbor);
	}
    }
}

void Body::calculateNeighbors2D (const OOBox& originalDomain)
{
    const OrientedFace& of = GetOrientedFace (0);
    m_neighbors.resize (of.size ());
    size_t j = 0;
    for (size_t i = 0; i < of.size (); ++i, ++j)
    {
	OrientedEdge oe = of.GetOrientedEdge (i);
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
	    m_neighbors[j] = Neighbor (c + 2 * (m - c));
	}
	else
	{
	    const AdjacentOrientedFaces& aofs = oe.GetAdjacentOrientedFaces ();
	    RuntimeAssert (aofs.size () <= 2, 
			   "AdjacentOrientedFaces size > 2: ", aofs.size ());
	    AdjacentOrientedFaces::const_iterator it = aofs.begin ();
	    while (it != aofs.end () && (it->IsStandalone () ||
					 it->GetBodyId () == GetId ()))
		++it;
	    if (it == aofs.end ())
	    {
		--j;
		continue;
	    }
	    boost::shared_ptr<Body> body = it->GetBody ();
	    G3D::Vector3int16 translation;
	    originalDomain.IsWrap (GetCenter (), body->GetCenter (),
				   &translation);


	    m_neighbors[j] = Neighbor (body,
				       Vector3int16Zero - translation);
	    m_growthRate += (GetScalarValue (BodyScalar::PRESSURE) - 
			     body->GetScalarValue (BodyScalar::PRESSURE)) * 
		oe.GetLength ();

	}
    }
    m_neighbors.resize (j);
}

void Body::CalculateDeformationTensor (const OOBox& originalDomain)
{
    if (IsObject ())
	return;
    size_t bubbleNeighborsCount = 0;
    G3D::Matrix3 textureTensor = G3D::Matrix3::zero ();
    const vector<Neighbor>& neighbors = GetNeighbors ();
    BOOST_FOREACH (Body::Neighbor neighbor, neighbors)
    {
	G3D::Vector3 s;
	if (neighbor.GetBody ())
	{
	    ++bubbleNeighborsCount;
	    s = neighbor.GetBody ()->GetCenter ();
	}
	else 
	{
	    // debug: no reflection used in average computation.
	    // continue;
	    s = neighbor.GetCenterReflection ();
	}
	G3D::Vector3 first = GetCenter ();
	G3D::Vector3 second = 
	    originalDomain.TorusTranslate (s, neighbor.GetTranslation ());
	G3D::Vector3 l = second - first;
	textureTensor += G3D::Matrix3 (l.x * l.x, l.x * l.y, l.x * l.z,
				       l.y * l.x, l.y * l.y, l.y * l.z,
				       l.z * l.x, l.z * l.y, l.z * l.z);
    }
    textureTensor /= neighbors.size ();
    SymmetricMatrixEigen().Calculate (textureTensor, 
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
	m_object = true;
}

size_t Body::GetConstraintIndex () const
{
    return GetFace (0).GetOrientedEdge (0).GetConstraintIndex ();
}

vtkSmartPointer<vtkPolyData> Body::GetPolyData () const
{
    return OrientedFace::GetPolyData (GetOrientedFaces ());
}

