/**
 * @file   Edge.cpp
 * @author Dan R. Lipsa
 * 
 * Implementation of the Edge class
 */
#include "AttributeCreator.h"
#include "AttributeInfo.h"
#include "Edge.h"
#include "EvolverData_yacc.h"
#include "Data.h"
#include "Debug.h"
#include "ParsingDriver.h"
#include "Vertex.h"


// Static Fields
// ======================================================================
AttributesInfo* Edge::m_infos;


// Methods
// ======================================================================
Edge::Edge (Vertex* begin, Vertex* end, G3D::Vector3int16& endTranslation, 
	    size_t originalIndex, Data* data, ElementStatus::Name status):
    ColoredElement(originalIndex, data, status),
    m_begin (begin), m_end (end),
    m_endTranslation (endTranslation), 
    m_physical (false),
    m_torusClipped (0)
{
    if (m_data->IsTorus () && GetStatus () == ElementStatus::ORIGINAL)
    {
	if (m_endTranslation == G3D::Vector3int16(0, 0, 0))
	    return;
	m_end = m_data->GetVertexDuplicate (m_end, m_endTranslation);
    }
}

Edge::Edge (Vertex* begin, size_t originalIndex) :
    ColoredElement (originalIndex, 0, ElementStatus::ORIGINAL),
    m_begin (begin),
    m_end (0),
    m_physical (false),
    m_torusClipped (0)
{
}

Edge::Edge (const Edge& o) : 
    ColoredElement (o.GetOriginalIndex (), o.GetData (), 
		    ElementStatus::DUPLICATE),
    m_begin (o.GetBegin ()), m_end (o.GetEnd ()),
    m_endTranslation (o.GetEndTranslation ()),
    m_physical (false),
    m_torusClipped (0)
{
}


vector<Face*>& Edge::GetAdjacentFaces () 
{
    return m_adjacentFaces;
}

G3D::Vector3 Edge::GetBegin (const G3D::Vector3* end) const
{
    return *end + (*GetBegin () - *GetEnd ());
}

void Edge::UpdateVerticesAdjacency ()
{
    this->GetBegin ()->AddAdjacentEdge (this);
    this->GetEnd ()->AddAdjacentEdge (this);
}

bool Edge::operator< (const Edge& other) const
{
    return GetOriginalIndex () < other.GetOriginalIndex () ||
	(GetOriginalIndex () == other.GetOriginalIndex () &&
	 *GetBegin () < *other.GetBegin ());
}

bool Edge::operator== (const Edge& other) const
{
    return GetOriginalIndex () == other.GetOriginalIndex () &&
	*GetBegin () == *other.GetBegin ();
}

bool Edge::fuzzyEq (const Edge& other) const
{
    return GetOriginalIndex () == other.GetOriginalIndex () &&
	GetBegin ()->fuzzyEq(*other.GetBegin ());
}

bool Edge::IsZero () const
{
    return (*GetEnd () - *GetBegin ()).isZero ();
}

Edge* Edge::CreateDuplicate (const G3D::Vector3& newBegin)
{
    SetStatus (ElementStatus::DUPLICATE_MADE);
    G3D::Vector3int16 translation = m_data->GetPeriods ().GetTranslation (
	*GetBegin (), newBegin);
    Vertex* beginDuplicate = m_data->GetVertexDuplicate (
	GetBegin (), translation);
    Vertex* endDuplicate = m_data->GetVertexDuplicate (GetEnd (), translation);
    Edge* duplicate = new Edge (*this);
    duplicate->SetBegin (beginDuplicate);
    duplicate->SetEnd (endDuplicate);
    return duplicate;
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


void Edge::CalculateTorusClipped ()
{
    using G3D::Vector3int16;using G3D::Vector3;using G3D::LineSegment;
    const OOBox& periods = m_data->GetPeriods ();
    Vector3int16 beginLocation = m_begin->GetTorusLocation ();
    Vector3int16 endLocation = m_end->GetTorusLocation ();
    Vector3int16 translation = endLocation - beginLocation;
    size_t intersectionCount = OOBox::CountIntersections (translation);
    vector<Vector3> intersections(2);
    if (intersectionCount == 0)
    {
	if (beginLocation == Vector3int16 (0, 0, 0))
	    return;
	intersections[0] = *m_begin;
	intersections[1] = *m_end;
    }
    else
	intersections = periods.Intersect (
	    *m_begin, *m_end, beginLocation, endLocation);

    m_torusClipped = new vector<LineSegment> (intersectionCount + 1);
    for (size_t i = 0; i < intersections.size () - 1; i++)
    {
	translation = Vector3int16 (0, 0, 0) - periods.GetTorusLocation (
	    (intersections[i] + intersections[i+1]) / 2);
	(*m_torusClipped)[i] = LineSegment::fromTwoPoints (
	    periods.TorusTranslate (intersections[i], translation),
	    periods.TorusTranslate (intersections[i + 1], translation));
    }
}

size_t Edge::GetTorusClippedSize () const
{
    using G3D::Vector3int16;
    Vector3int16 beginLocation = m_begin->GetTorusLocation ();
    Vector3int16 endLocation = m_end->GetTorusLocation ();
    Vector3int16 translation = endLocation - beginLocation;
    size_t intersectionCount = OOBox::CountIntersections (translation);
    return intersectionCount + 1;
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

void Edge::StoreDefaultAttributes (AttributesInfo* infos)
{
    using EvolverData::parser;
    m_infos = infos;
    ColoredElement::StoreDefaultAttributes (infos);
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL),
        new IntegerAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::CONSTRAINTS),
        new IntegerVectorAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::DENSITY),
        new RealAttributeCreator());
}

ostream& operator<< (ostream& ostr, const Edge& e)
{
    ostr << "Edge " << e.GetOriginalIndex () << " "
	 << e.GetStatus ()
	 << ": "
	 << static_cast<G3D::Vector3>(*e.m_begin) << ", " 
	 << static_cast<G3D::Vector3>(*e.m_end)
	 << " Adjacent faces(" << e.m_adjacentFaces.size () << ")"
	 << " Edge attributes: ";
    return e.PrintAttributes (ostr);
}
