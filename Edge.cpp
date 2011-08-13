/**
 * @file   Edge.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the Edge class
 */
#include "Attribute.h"
#include "AttributeCreator.h"
#include "AttributeInfo.h"
#include "Debug.h"
#include "DebugStream.h"
#include "Edge.h"
#include "EvolverData_yacc.h"
#include "Foam.h"
#include "FoamParameters.h"
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
	    size_t id, Type type, ElementStatus::Enum duplicateStatus):
    Element(id, duplicateStatus),
    m_begin (begin), m_end (end),
    m_endTranslation (endTranslation), 
    m_torusClipped (0),
    m_type (type)
{
}

Edge::Edge (const boost::shared_ptr<Vertex>& begin, size_t id, Type type) :
    Element (id, ElementStatus::ORIGINAL),
    m_begin (begin),
    m_torusClipped (0),
    m_type (type)
{
}

Edge::Edge (const Edge& o) : 
    Element (o),
    m_begin (o.GetBeginPtr ()), m_end (o.GetEndPtr ()),
    m_endTranslation (o.GetEndTranslation ()),
    m_adjacentOrientedFaces (o.m_adjacentOrientedFaces),
    m_torusClipped (0),
    m_type (o.GetType ())
{
}

boost::shared_ptr<Edge> Edge::Clone () const
{
    return boost::shared_ptr<Edge> (new Edge(*this));
}

G3D::Vector3 Edge::GetTranslatedBegin (const G3D::Vector3& newEnd) const
{
    return newEnd + (GetBeginVector () - GetEndVector ());
}

void Edge::UpdateAdjacentEdge (const boost::shared_ptr<Edge>& edge)
{
    GetBeginPtr ()->AddAdjacentEdge (edge);
    GetEndPtr ()->AddAdjacentEdge (edge);
}

bool Edge::operator< (const Edge& other) const
{
    return 
	GetId () < other.GetId () ||

	(GetId () == other.GetId () && GetBegin () < other.GetBegin ());
}

bool Edge::operator== (const Edge& other) const
{
    return GetId () == other.GetId () && GetBegin () == other.GetBegin ();
}

bool Edge::fuzzyEq (const Edge& other) const
{
    return GetId () == other.GetId () &&
	IsFuzzyZero (GetBeginVector () - other.GetBeginVector ());
}


G3D::Vector3 Edge::GetTorusClippedBegin (size_t index) const
{
    if (m_torusClipped == 0)
    {
	RuntimeAssert (index == 0, "index should be 0 and is ",
		       index);
	return m_begin->GetVector ();
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
	return m_end->GetVector ();
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
    Vector3int16 beginLocation = periods.GetLocation (m_begin->GetVector ());
    Vector3int16 endLocation = periods.GetLocation (m_end->GetVector ());
    Vector3int16 translation = endLocation - beginLocation;
    size_t intersectionCount = OOBox::CountIntersections (translation);
    vector<Vector3> intersections(2);
    if (intersectionCount == 0)
    {
	if (beginLocation == Vector3int16Zero)
	    return;
	intersections[0] = m_begin->GetVector ();
	intersections[1] = m_end->GetVector ();
    }
    else
	intersections = periods.Intersect (
	    m_begin->GetVector (), m_end->GetVector (), 
	    beginLocation, endLocation);

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
    Vector3int16 beginLocation = periods.GetLocation (m_begin->GetVector ());
    Vector3int16 endLocation = periods.GetLocation (m_end->GetVector ());
    Vector3int16 translation = endLocation - beginLocation;
    size_t intersectionCount = OOBox::CountIntersections (translation);
    return intersectionCount + 1;
}


void Edge::AddAdjacentFace (
    boost::shared_ptr<OrientedFace> orientedFace, size_t edgeIndex)
{
    m_adjacentOrientedFaces.insert (
	AdjacentOrientedFace (orientedFace, edgeIndex));
}


string Edge::AdjacentFacesToString () const
{
    ostringstream ostr;
    const AdjacentOrientedFaces& aofs = GetAdjacentFaces ();
    size_t facePartOfSize = aofs.size ();
    ostr << "Edge " << GetStringId () << " is part of " 
	 << facePartOfSize << " faces: ";
    ostream_iterator<AdjacentOrientedFace> output (ostr, " ");
    copy (aofs.begin (), aofs.end (), output);
    return ostr.str ();
}

bool Edge::IsPhysical (const FoamParameters& foamParameters) const
{
    if (foamParameters.Is2D ())
    {
	if (foamParameters.IsQuadratic ())
	    return true;
	else
	    return false;
    }
    else
    {
	if (m_adjacentOrientedFaces.size () < 4)
	    return false;
	AdjacentOrientedFaces::const_iterator end = 
	    m_adjacentOrientedFaces.end ();
	AdjacentOrientedFaces::const_iterator begin;
	AdjacentOrientedFaces::const_iterator next = 
	    m_adjacentOrientedFaces.begin ();
	size_t facesPartOfSize = 0;
	do
	{
	    begin = next;
	    next = m_adjacentOrientedFaces.equal_range (*begin).second;
	    ++facesPartOfSize;
	} while (next != end);
	return facesPartOfSize == 3;
    }
}

string Edge::ToString () const
{
    ostringstream ostr;
    ostr << "Edge " << GetStringId () << " "
	 << GetDuplicateStatus () << " "
	 << *m_begin << ", " 
	 << *m_end << " "
	 << " Adjacent faces(" << m_adjacentOrientedFaces.size () << ")";
    if (HasAttributes ())
    {
	ostr << " Edge attributes: ";
	PrintAttributes (ostr);
    }
    return ostr.str ();
}

boost::shared_ptr<Edge> Edge::GetDuplicate (
    const OOBox& periods,
    const G3D::Vector3& newBegin,
    VertexSet* vertexSet, EdgeSet* edgeSet) const
{
    boost::shared_ptr<Edge> searchDummy = 
	boost::make_shared<Edge>(
	    boost::make_shared<Vertex> (
		newBegin, GetBegin ().GetId ()), GetId ());
    EdgeSet::iterator it = edgeSet->find (searchDummy);
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
	GetBeginVector (), newBegin);
    boost::shared_ptr<Vertex> beginDuplicate = GetBegin ().GetDuplicate (
	periods, translation, vertexSet);
    boost::shared_ptr<Vertex> endDuplicate = GetEnd ().GetDuplicate (
	periods, translation, vertexSet);
    boost::shared_ptr<Edge> duplicate = Clone ();
    duplicate->setBegin (beginDuplicate);
    duplicate->SetEnd (endDuplicate);
    duplicate->SetDuplicateStatus (ElementStatus::DUPLICATE);
    return duplicate;
}

void Edge::GetVertexSet (VertexSet* vertexSet) const
{
    vertexSet->insert (GetBeginPtr ());
    vertexSet->insert (GetEndPtr ());
}

double Edge::GetLength () const
{
    size_t pointCount = GetPointCount ();
    double length = 0;
    G3D::Vector3 prev = GetPoint (0);
    for (size_t i = 1; i < pointCount; ++i)
    {
	G3D::Vector3 p = GetPoint (i);
	length += (p - prev).length ();
	prev = p;
    }
    return length;
}

G3D::Vector3 Edge::GetPoint (size_t i) const
{
    if (i == 0)
	return GetBeginVector ();
    else
	return GetEndVector ();
}

size_t Edge::GetConstraintIndex (size_t i) const
{
    return GetAttribute<IntegerArrayAttribute, 
	IntegerArrayAttribute::value_type> (
	    EdgeAttributeIndex::CONSTRAINTS)[i] - 1;
}

bool Edge::HasConstraints () const
{
    return HasAttribute (EdgeAttributeIndex::CONSTRAINTS);
}

QColor Edge::GetColor (const QColor& defaultColor) const
{
    if (HasAttribute (EdgeAttributeIndex::COLOR))
	return Color::GetValue (
	    GetAttribute<ColorAttribute, ColorAttribute::value_type> (
		EdgeAttributeIndex::COLOR));
    else
	return defaultColor;
}

G3D::Vector3 Edge::GetBeginVector () const
{
    return GetBegin ().GetVector ();
}

G3D::Vector3 Edge::GetEndVector () const
{
    return GetEnd ().GetVector ();
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
    for (int i = 0; i < DIMENSIONS; i++)
    {
        // we store -1, 0 or 1
	result[i] = (value % DOMAIN_INCREMENT_POSSIBILITIES) - 1; 
	value /= DOMAIN_INCREMENT_POSSIBILITIES;
    }
    return result;
}

