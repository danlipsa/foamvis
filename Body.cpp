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
#include "Debug.h"
#include "DebugStream.h"
#include "Utils.h"
#include "Face.h"
#include "OrientedEdge.h"
#include "OrientedFace.h"
#include "ParsingDriver.h"
#include "ProcessBodyTorus.h"
#include "Vertex.h"

// Private Classes
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

// Private functions
// ======================================================================
/**
 * Accumulates along X, Y or Z
 * @param result where we accumulate
 * @param v the vertex
 * @return result + the value of the vertex along the specified axis
 */
G3D::Vector3 vertexAccumulate (G3D::Vector3 result,
			       const boost::shared_ptr<Vertex>& v)
{
    return result + *v;
}


// Methods
// ======================================================================

Body::Body(
    const vector<int>& faceIndexes,
    const vector<boost::shared_ptr<Face> >& faces,
    size_t id, ElementStatus::Enum duplicateStatus) :
    Element(id, duplicateStatus)
{
    m_orientedFaces.resize (faceIndexes.size ());
    transform (faceIndexes.begin(), faceIndexes.end(), m_orientedFaces.begin(), 
               indexToOrientedFace(faces));
}

void Body::calculatePhysicalVertices (
    size_t dimension, bool isQuadratic,
    vector< boost::shared_ptr<Vertex> >* physicalVertices)
{
    VertexSet vertices;
    vector< boost::shared_ptr<Vertex> > tessellationVertices;

    GetVertexSet (&vertices);
    splitTessellationPhysical (
	vertices, &tessellationVertices, physicalVertices, 
	dimension, isQuadratic);
}

void Body::splitTessellationPhysical (
    const VertexSet& src,
    vector< boost::shared_ptr<Vertex> >* destTessellation,
    vector< boost::shared_ptr<Vertex> >* destPhysical,
    size_t dimension, bool isQuadratic)
{
    destTessellation->resize (src.size ());
    copy (src.begin (), src.end (), destTessellation->begin ());
    vector< boost::shared_ptr<Vertex> >::iterator bp;
    bp = partition (destTessellation->begin (), destTessellation->end (), 
		    !boost::bind(&Vertex::IsPhysical, _1, 
				 dimension, isQuadratic));
    destPhysical->resize (destTessellation->end () - bp);
    copy (bp, destTessellation->end (), destPhysical->begin ());
    destTessellation->resize (bp - destTessellation->begin ());
}


void Body::CalculateCenter (size_t dimension, bool isQuadratic)
{
    using G3D::Vector3;    
    vector< boost::shared_ptr<Vertex> > physicalVertices;
    calculatePhysicalVertices (dimension, isQuadratic, &physicalVertices);
    size_t size = physicalVertices.size ();
    if (size >= 3)
    {	
	m_center = accumulate (
	    physicalVertices.begin (), physicalVertices.end (),
	    G3D::Vector3::zero (), &vertexAccumulate);

    }
    else
    {
	VertexSet vertices;
	GetVertexSet (&vertices);
	size = vertices.size ();
	m_center = accumulate (vertices.begin (), vertices.end (),
			       G3D::Vector3::zero (), &vertexAccumulate);
    }
    m_center /= Vector3(size, size, size);
}


void Body::UpdatePartOf (const boost::shared_ptr<Body>& body)
{
    for (size_t i = 0; i < m_orientedFaces.size (); i++)
    {
	boost::shared_ptr<OrientedFace> of = m_orientedFaces[i];
	of->AddBodyPartOf (body, i);
	of->UpdateFacePartOf (of);
    }
}


bool Body::HasWrap () const
{
    BOOST_FOREACH (boost::shared_ptr<OrientedFace> of, m_orientedFaces)
	if (of->GetFace ()->HasWrap ())
	    return true;
    return false;
}

boost::shared_ptr<Face> Body::GetFace (size_t i) const
{
    return GetOrientedFace (i)->GetFace ();
}

string Body::ToString () const
{
    ostringstream ostr;
    ostr << "Body " << GetId () << ":" << endl;
    ostr << m_orientedFaces.size () << " faces part of the body\n";
    ostream_iterator< boost::shared_ptr<OrientedFace> > ofOutput (ostr, "\n");
    copy (m_orientedFaces.begin (), m_orientedFaces.end (), ofOutput);
    ostr << "Body attributes: ";
    PrintAttributes (ostr);
    ostr << "\nBody center: " << m_center;
    return ostr.str ();
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

