/**
 * @file   Edge.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the Edge class
 */
#include "AttributeCreator.h"
#include "AttributeInfo.h"
#include "Debug.h"
#include "DebugStream.h"
#include "Edge.h"
#include "EvolverData_yacc.h"
#include "Foam.h"
#include "OrientedFace.h"
#include "OrientedEdge.h"
#include "ParsingDriver.h"
#include "Utils.h"
#include "Vertex.h"


// Methods
// ======================================================================
Edge::Edge (const boost::shared_ptr<Vertex>& begin,
	    const boost::shared_ptr<Vertex>& end,
	    const G3D::Vector3int16& endTranslation, 
	    size_t id, ElementStatus::Enum duplicateStatus):
    ColoredElement(id, duplicateStatus),
    m_begin (begin), m_end (end),
    m_endTranslation (endTranslation), 
    m_torusClipped (0)
{
}

Edge::Edge (const boost::shared_ptr<Vertex>& begin, size_t id) :
    ColoredElement (id, ElementStatus::ORIGINAL),
    m_begin (begin),
    m_torusClipped (0)
{
}

Edge::Edge (const Edge& o) : 
    ColoredElement (o),
    m_begin (o.GetBegin ()), m_end (o.GetEnd ()),
    m_endTranslation (o.GetEndTranslation ()),
    m_facesPartOf (o.m_facesPartOf),
    m_torusClipped (0)
{
}

boost::shared_ptr<Edge> Edge::Clone () const
{
    return boost::shared_ptr<Edge> (new Edge(*this));
}

G3D::Vector3 Edge::GetTranslatedBegin (const G3D::Vector3& newEnd) const
{
    return newEnd + (*GetBegin () - *GetEnd ());
}

void Edge::UpdateEdgePartOf (const boost::shared_ptr<Edge>& edge)
{
    GetBegin ()->AddEdgePartOf (edge);
    GetEnd ()->AddEdgePartOf (edge);
}

bool Edge::operator< (const Edge& other) const
{
    return GetId () < other.GetId () ||
	(GetId () == other.GetId () &&
	 *GetBegin () < *other.GetBegin ());
}

bool Edge::operator== (const Edge& other) const
{
    return GetId () == other.GetId () &&
	*GetBegin () == *other.GetBegin ();
}

bool Edge::fuzzyEq (const Edge& other) const
{
    return GetId () == other.GetId () &&
	GetBegin ()->fuzzyEq(*other.GetBegin ());
}

bool Edge::IsZero () const
{
    return (*GetEnd () - *GetBegin ()).isZero ();
}


G3D::Vector3 Edge::GetTorusClippedBegin (size_t index) const
{
    if (m_torusClipped == 0)
    {
	RuntimeAssert (index == 0, "index should be 0 and is ",
		       index);
	return *m_begin;
    }
    else
    {
	size_t n = m_torusClipped->size ();
	RuntimeAssert (index < n, 
		       "index should be less than ", n, " and is ", index);
	return (*m_torusClipped)[index].point (0);
    }
}

G3D::Vector3 Edge::GetTorusClippedEnd (size_t index) const
{
    if (m_torusClipped == 0)
    {
	RuntimeAssert (index == 0, "index should be 0 and is ", index);
	return *m_end;
    }
    else
    {
	size_t n = m_torusClipped->size ();
	RuntimeAssert (index < n, 
		       "index should be less than ", n, " and is ", index);
	return (*m_torusClipped)[index].point (1);
    }
}


void Edge::CalculateTorusClipped (const OOBox& periods)
{
    using G3D::Vector3int16;using G3D::Vector3;using G3D::LineSegment;
    Vector3int16 beginLocation = periods.GetLocation (*m_begin);
    Vector3int16 endLocation = periods.GetLocation (*m_end);
    Vector3int16 translation = endLocation - beginLocation;
    size_t intersectionCount = OOBox::CountIntersections (translation);
    vector<Vector3> intersections(2);
    if (intersectionCount == 0)
    {
	if (beginLocation == Vector3int16Zero)
	    return;
	intersections[0] = *m_begin;
	intersections[1] = *m_end;
    }
    else
	intersections = periods.Intersect (
	    *m_begin, *m_end, beginLocation, endLocation);

    m_torusClipped.reset (new vector<LineSegment> (intersectionCount + 1));
    for (size_t i = 0; i < intersections.size () - 1; i++)
    {
	translation = Vector3int16Zero - periods.GetLocation (
	    (intersections[i] + intersections[i+1]) / 2);
	(*m_torusClipped)[i] = LineSegment::fromTwoPoints (
	    periods.TorusTranslate (intersections[i], translation),
	    periods.TorusTranslate (intersections[i + 1], translation));
    }
}

size_t Edge::GetTorusClippedSize (const OOBox& periods) const
{
    using G3D::Vector3int16;
    Vector3int16 beginLocation = periods.GetLocation (*m_begin);
    Vector3int16 endLocation = periods.GetLocation (*m_end);
    Vector3int16 translation = endLocation - beginLocation;
    size_t intersectionCount = OOBox::CountIntersections (translation);
    return intersectionCount + 1;
}


void Edge::AddFacePartOf (boost::shared_ptr<OrientedFace>  orientedFace, size_t edgeIndex)
{
    m_facesPartOf.insert (OrientedFaceIndex (orientedFace, edgeIndex));
}


void Edge::PrintFacePartOfInformation (ostream& ostr) const
{
    size_t facePartOfSize = GetFacePartOfSize ();
    ostr << "Edge " << GetStringId () << " is part of " 
	 << facePartOfSize << " faces: ";
    ostream_iterator<OrientedFaceIndex> output (ostr, " ");
    copy (GetFacePartOfBegin (), GetFacePartOfEnd (), output);
    ostr << endl;
}

bool Edge::IsPhysical (size_t dimension, bool isQuadratic) const
{
    if (dimension == 3)
    {
	if (m_facesPartOf.size () < 4)
	    return false;
	OrientedFaceIndexList::const_iterator end = m_facesPartOf.end ();
	OrientedFaceIndexList::const_iterator begin;
	OrientedFaceIndexList::const_iterator next = m_facesPartOf.begin ();
	size_t facesPartOfSize = 0;
	do
	{
	    begin = next;
	    next = m_facesPartOf.equal_range (*begin).second;
	    ++facesPartOfSize;
	} while (next != end);
	return facesPartOfSize == 3;
    }
    else
    {
	if (isQuadratic)
	    return true;
	else
	    return false;
    }
}

string Edge::ToString () const
{
    ostringstream ostr;
    ostr << "Edge " << GetStringId () << " "
	 << static_cast<G3D::Vector3>(*m_begin) << ", " 
	 << static_cast<G3D::Vector3>(*m_end) << " "
	 << GetDuplicateStatus () << " "
	 << " Adjacent faces(" << m_facesPartOf.size () << ")"
	 << " Edge attributes: ";
    PrintAttributes (ostr);
    return ostr.str ();
}

void Edge::Display (Color::Enum defaultColor, float alpha) const
{
    Color::Enum color = GetColor (defaultColor);
    glColor (G3D::Color4 (Color::GetValue(color), alpha));
    glBegin(GL_LINE_STRIP);
    for (size_t i = 0; i < PointCount (); i++)
	glVertex (GetPoint (i));
    glEnd ();
}

boost::shared_ptr<Edge> Edge::GetDuplicate (
    const OOBox& periods,
    const G3D::Vector3& newBegin,
    VertexSet* vertexSet, EdgeSet* edgeSet) const
{
    boost::shared_ptr<Edge> searchDummy = 
	boost::make_shared<Edge>(
	    boost::make_shared<Vertex> (newBegin), GetId ());
    EdgeSet::iterator it = 
	fuzzyFind <EdgeSet, EdgeSet::iterator, EdgeSet::key_type> (
	    *edgeSet, searchDummy);
    if (it != edgeSet->end ())
	return *it;
    boost::shared_ptr<Edge> duplicate = createDuplicate (
	periods, newBegin, vertexSet);
    edgeSet->insert (duplicate);
    return duplicate;
}

boost::shared_ptr<Edge> Edge::createDuplicate (
    const OOBox& periods,
    const G3D::Vector3& newBegin, VertexSet* vertexSet) const
{
    G3D::Vector3int16 translation = periods.GetTranslation (
	*GetBegin (), newBegin);
    boost::shared_ptr<Vertex> beginDuplicate = GetBegin ()->GetDuplicate (
	periods, translation, vertexSet);
    boost::shared_ptr<Vertex> endDuplicate = GetEnd ()->GetDuplicate (
	periods, translation, vertexSet);
    boost::shared_ptr<Edge> duplicate = Clone ();
    duplicate->SetBegin (beginDuplicate);
    duplicate->SetEnd (endDuplicate);
    duplicate->SetDuplicateStatus (ElementStatus::DUPLICATE);
    return duplicate;
}

void Edge::GetVertexSet (VertexSet* vertexSet) const
{
    vertexSet->insert (GetBegin ());
    vertexSet->insert (GetEnd ());
}


// Static and Friends Methods
// ======================================================================

short Edge::LocationCharToNumber (char sign)
{
    switch (sign)
    {
    case '*':
	return 0;
    case '+':
	return 1;
    case '-':
	return -1;
    default:
	RuntimeAssert (false, "Invalid sign: ", sign);
	return 0;
    }
}

G3D::Vector3int16 Edge::IntToLocation (int value)
{
    G3D::Vector3int16 result;
    const int DIMENSIONS = 3;
    const int POSSIBILITIES = 3; // *, -, +
    for (int i = 0; i < DIMENSIONS; i++)
    {
	result[i] = (value % POSSIBILITIES) - 1; // we store -1, 0 or 1
	value /= POSSIBILITIES;
    }
    return result;
}

size_t Edge::PointCount () const
{
    return 2;
}

G3D::Vector3 Edge::GetPoint (size_t i) const
{
    if (i == 0)
	return *GetBegin ();
    else
	return *GetEnd ();
}

