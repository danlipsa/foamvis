/**
 * @file   Body.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Body class
 */
#include "Body.h"
#include "AttributeInfo.h"
#include "ParsingDriver.h"
#include "Data.h"
#include "Debug.h"

/**
 * Functor that caches an edge and its vertices
 */
class cacheEdgeVertices
{
public:
    /**
     * Constructor
     * @param body where to cache the edge and vertices
     */
    cacheEdgeVertices (Body& body) : m_body (body) {}
    /**
     * Functor that caches an edge and its vertices
     * @param oe the edge to cache
     */
    void operator () (OrientedEdge* oe)
    {
	Edge *e = oe->GetEdge ();
	m_body.CacheEdge (e);
	m_body.CacheVertex (e->GetBegin ());
	m_body.CacheVertex (e->GetEnd ());
    }
private:
    /**
     * Where to cache the edge and vertices
     */
    Body& m_body;
};

/**
 * Functor that caches edges and vertices in vectors stored in the Body
 */
class cacheEdgesVertices
{
public:
    /**
     * Constructor
     * @param body object we work on
     */
    cacheEdgesVertices (Body& body) : m_body(body) {}
    /**
     * Functor that caches edges and vertices in vectors stored in the Body
     * @param of cache all edges and vertices for this OrientedFace
     */
    void operator() (OrientedFace* of)
    {
	vector<OrientedEdge*> oev = of->GetFace ()->GetOrientedEdges ();
	for_each (oev.begin (), oev.end (), cacheEdgeVertices (m_body));
    }
private:
    /**
     * Where to we store edges and vertices
     */
    Body& m_body;
};

/**
 * STL unary  function that converts a  signed index into  a vector of
 * Face  objects into a  OrientedFace object.  A negative  index means
 * that the Face object is listed  in reverse order in the Body object
 * than in the vector of Face objects.
 */
class indexToOrientedFace : public unary_function<int, OrientedFace*>
{
public:
    /**
     * Constructor
     * @param faces vector of Face pointers. This is where the indexes point to.
     */
    indexToOrientedFace(vector<Face*>& faces): m_faces(faces) {}
    /**
     * Converts a 1-based index into an OrientedFace
     * @param i index into a vector of Face pointers
     * @return an OrientedFace pointer
     */
    OrientedFace* operator() (int i)
    {
        bool reversed = false;
        if (i < 0)
        {
            i = -i;
            reversed = true;
        }
        i--;
        return new OrientedFace(m_faces[i], reversed);
    }
private:
    /**
     * Vector of Face pointers
     */
    vector<Face*>& m_faces;
};

ostream& operator<< (ostream& ostr, const Body& b)
{
    PrintElements<OrientedFace*> (ostr, b.m_faces, 
				  "faces part of the body", true);
    ostr << " Body attributes: ";
    return b.PrintAttributes (ostr);
}


ostream& operator<< (
    ostream& ostr,
    const Body::FaceIntersectionMargin::FaceEdgeIndex& fei)
{
    ostr << "face " << fei.m_face->GetFace ()->GetOriginalIndex () 
	 << " " << fei.m_face->GetFace ()->GetColor () << ":index "
	 << fei.m_edgeIndex;
    return ostr;
}

ostream& operator<< (
    ostream& ostr, const Body::FaceIntersectionMargin& fim)
{
    ostr << "FaceIntersectionMargin: " 
	 << ((fim.m_margin == 
	      Body::FaceIntersectionMargin::BEFORE_FIRST_AFTER_SECOND) ? 
	     "BEFORE_FIRST_AFTER_SECOND" : "AFTER_FIRST_BEFORE_SECOND") << endl;
    ostr << fim.m_first << " - " << fim.m_second << endl;
    return ostr;
}


void Body::ProcessForTorus::Initialize ()
{
    FaceIntersectionMargin firstMargin, secondMargin;
    // start with two faces, mark them as placed
    m_body->m_faces[0]->SetPlaced (true);
    m_body->m_faces[1]->SetPlaced (true);
    GetFaceIntersectionMargins (
	*m_body->m_faces[0], *m_body->m_faces[1], &firstMargin, &secondMargin);
    m_queue.push_back (firstMargin);
    m_queue.push_back (secondMargin);
}

void Body::ProcessForTorus::End ()
{
    for_each (m_body->m_faces.begin (), m_body->m_faces.end (),
	      bind (&OrientedFace::SetPlaced, _1, false));
}


bool Body::ProcessForTorus::Step ()
{
    if (m_queue.empty ())
	return false;
    cdbg << "Queue " << m_queue.size () << " elements" << endl;
    ostream_iterator<FaceIntersectionMargin> output (cdbg, "\n");
    copy (m_queue.begin (), m_queue.end (), output);


    FaceIntersectionMargin firstMargin, secondMargin;
    FaceIntersectionMargin margin = m_queue.front ();
    m_queue.pop_front ();

    // place the face that fits over that margin (this might
    // create a duplicate face)
    OrientedFace* face = m_body->fitAndDuplicateFace (margin);

    // if the face was not placed before
    // add two more angles in the queue
    if (! face->IsPlaced ())
    {
	if (face->GetFace ()->IsDuplicate ())
	    cdbg << "Fitted face: " << endl << *face << endl;
	else
	{
	    cdbg << "Fitted face " << face->GetFace ()->GetOriginalIndex () 
		 << " " << face->GetFace ()->GetColor () << " "
		 << " not a DUPLICATE" << endl;
	}
	face->SetPlaced (true);
	GetFaceIntersectionMargins (*face, margin.GetFirstFace (),
				    &firstMargin, &secondMargin);
	m_queue.push_back (secondMargin);
	GetFaceIntersectionMargins (*face, margin.GetSecondFace (), 
				    &firstMargin, &secondMargin);
	m_queue.push_back (firstMargin);
    }
    else
    {
	cdbg << "Fitted face " << face->GetFace ()->GetOriginalIndex () 
	     << " " << face->GetFace ()->GetColor () << " "
	     << " already PLACED" << endl;
    }
    return true;
}


AttributesInfo* Body::m_infos;

Body::Body(vector<int>& faceIndexes, vector<Face*>& faces,
	   size_t originalIndex, Data* data,
	   bool duplicate) :
    Element(originalIndex, data, duplicate)
{
    using boost::bind;
    m_faces.resize (faceIndexes.size ());
    transform (faceIndexes.begin(), faceIndexes.end(), m_faces.begin(), 
               indexToOrientedFace(faces));
    //if (m_data->IsTorus () && m_data->GetSpaceDimension () == 3)
    if (false)
    {
	ProcessForTorus processForTorus (this);
	processForTorus.Initialize ();
	while (processForTorus.Step ());
	processForTorus.End ();
    }
}

Body::~Body()
{
    using boost::bind;
    for_each(m_faces.begin(), m_faces.end(), 
	     bind (DeletePointer<OrientedFace>(), _1));
}

void Body::StoreDefaultAttributes (AttributesInfo* infos)
{
    using EvolverData::parser;
    m_infos = infos;
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL),
        new IntegerAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::LAGRANGE_MULTIPLIER),
        new RealAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::VOLUME),
        new RealAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::VOLCONST),
        new RealAttributeCreator());
}

void Body::CacheEdgesVertices ()
{
    for_each (m_faces.begin (), m_faces.end(), cacheEdgesVertices(*this));
    split (m_vertices, m_tessellationVertices, m_physicalVertices);
    split (m_edges, m_tessellationEdges, m_physicalEdges);
}

template <typename T>
void Body::split (
    set<T*>& src,
    vector<T*>& destTessellation,
    vector<T*>& destPhysical)
{
    destTessellation.resize (src.size ());
    copy (src.begin (), src.end (), destTessellation.begin ());
    typename vector<T*>::iterator bp;
    bp = partition (destTessellation.begin (),destTessellation.end (), 
		    not1(mem_fun(&T::IsPhysical)));
    destPhysical.resize (destTessellation.end () - bp);
    copy (bp, destTessellation.end (), 
	  destPhysical.begin ());
    destTessellation.resize (
	bp - destTessellation.begin ());
}


void Body::CalculateCenter ()
{
    using G3D::Vector3;
    size_t size = m_physicalVertices.size ();
    RuntimeAssert (
	size != 0, 
	"Call Body::CacheEdgesVertices before calling this function");
    m_center = accumulate (
	m_physicalVertices.begin (), m_physicalVertices.end (), m_center, 
	&Vertex::Accumulate);
    m_center /= Vector3(size, size, size);
}

void Body::UpdateFacesAdjacency ()
{
    using boost::bind;
    vector<OrientedFace*>& of = GetOrientedFaces ();
    for_each (of.begin (), of.end (),
	      bind(&OrientedFace::AddAdjacentBody, _1, this));
}

void Body::GetFaceIntersectionMargins (
    const OrientedFace& firstFace, const OrientedFace& secondFace,
    FaceIntersectionMargin* firstMargin, FaceIntersectionMargin* secondMargin)
{
    for (size_t i = 0; i < firstFace.GetEdgeCount (); i++)
    {
	for (size_t j = 0; j < secondFace.GetEdgeCount (); j++)
	{
	    if (*(firstFace.GetOrientedEdge (i)->GetEdge ()) == 
		*(secondFace.GetOrientedEdge (j)->GetEdge ()))
	    {
		*firstMargin = FaceIntersectionMargin (
		    FaceIntersectionMargin::BEFORE_FIRST_AFTER_SECOND,
		    FaceIntersectionMargin::FaceEdgeIndex (&firstFace, i),
		    FaceIntersectionMargin::FaceEdgeIndex (&secondFace, j));
		*secondMargin = *firstMargin;
		secondMargin->SetMargin ( 
		    FaceIntersectionMargin::AFTER_FIRST_BEFORE_SECOND);
		return;
	    }
	}
    }
    RuntimeAssert (false, "No face intersection found");
}


void Body::FaceIntersectionMargin::GetTriangle (
    const FaceEdgeIndex& first, const FaceEdgeIndex& second,
    boost::array<G3D::Vector3, 3>* t)
{
    boost::array<G3D::Vector3, 3>& triangle = *t;
    size_t afterIndex = second.m_face->GetNextValidIndex (second.m_edgeIndex);
    triangle[0] = *second.m_face->GetEnd (afterIndex);
    triangle[1] = *second.m_face->GetBegin (afterIndex);
    size_t beforeIndex = first.m_face->GetPreviousValidIndex (
	first.m_edgeIndex);
    G3D::Vector3 otherTwo = *first.m_face->GetEnd (beforeIndex);
    RuntimeAssert (triangle[1] == otherTwo,
		   "Invalid triangle: ", triangle[1], ", ", otherTwo);
    triangle[2] = *first.m_face->GetBegin (beforeIndex);
}


OrientedFace* Body::fitAndDuplicateFace (
    const FaceIntersectionMargin& faceIntersectionMargin)
{
    using G3D::Vector3;
    boost::array<Vector3, 3> triangle;
    if (faceIntersectionMargin.GetMargin () == 
	FaceIntersectionMargin::BEFORE_FIRST_AFTER_SECOND)
	FaceIntersectionMargin::GetTriangle (
	    faceIntersectionMargin.GetFirst (), 
	    faceIntersectionMargin.GetSecond (), &triangle);
    else
	FaceIntersectionMargin::GetTriangle (
	    faceIntersectionMargin.GetSecond (),
	    faceIntersectionMargin.GetFirst (), &triangle);
    Vector3 translation;
    bool found = false;
    OrientedFace* face = 0;
    BOOST_FOREACH (face, m_faces)
	if (fitFace (*face, triangle, &translation))
	{
	    // you  only need  to consider  one match  because  of the
	    // orientation of the face.
	    found = true;
	    break;
	}
    RuntimeAssert (found,
		   "No face was fitted for this face intersection margin");
    if (! translation.isZero ())
    {
	//found a possible fit
	face->SetFace (
	    m_data->GetFaceDuplicate (
		*face->GetFace (),
		*(face->GetOrientedEdge(0)->GetBegin ()) + translation));
    }
    return face;
}

bool Body::fitFace (
    const OrientedFace& candidate,
    const boost::array<G3D::Vector3, 3>& triangle,
    G3D::Vector3* translation)
{
    using G3D::Vector3;
    size_t currentFitPoint = 0;
    bool found = false;

    for (size_t start = 0; 
	 ! found && start < (candidate.GetEdgeCount () + triangle.size () - 1);
	 start++)
    {
	size_t i = start % candidate.GetEdgeCount ();
	Vector3 current = *candidate.GetEnd (i);
	switch (currentFitPoint)
	{
	case 0:
	    *translation = (triangle[0] - current);
	    currentFitPoint++;
	    break;

	case 1:
	    if ((triangle[1] - current).fuzzyEq(*translation))
		currentFitPoint++;
	    else
	    {
		start--;
		currentFitPoint = 0;
	    }
	    break;

	case 2:
	    if ((triangle[2] - current).fuzzyEq(*translation))	 
		found = true;	    
	    else
	    {
		start--;
		currentFitPoint = 0;
	    }
	    break;
	}
    }
    return found;
}

