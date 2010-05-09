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
Edge::Edge (Vertex* begin, Vertex* end, G3D::Vector3int16& domainIncrement, 
	    size_t originalIndex, Data* data, ElementStatus::Name status):
    ColoredElement(originalIndex, data, status),
    m_begin (begin), m_end (end),
    m_domainIncrement (domainIncrement), 
    m_physical (false),
    m_torusWrapped (0)
{
    if (m_data->IsTorus () && GetStatus () == ElementStatus::ORIGINAL)
    {
	if (m_domainIncrement == G3D::Vector3int16(0, 0, 0))
	    return;
	m_end = m_data->GetVertexDuplicate (m_end, m_domainIncrement);
    }
}

Edge::Edge (Vertex* begin, size_t originalIndex) :
    ColoredElement (originalIndex, 0, ElementStatus::ORIGINAL),
    m_begin (begin),
    m_end (0),
    m_physical (false),
    m_torusWrapped (0)
{
}

Edge::Edge (const Edge& o) : 
    ColoredElement (o.GetOriginalIndex (), o.GetData (), 
		    ElementStatus::DUPLICATE),
    m_begin (o.GetBegin ()), m_end (o.GetEnd ()),
    m_domainIncrement (o.GetDomainIncrement ()),
    m_physical (false),
    m_torusWrapped (0)
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
    G3D::Vector3int16 domainIncrement = m_data->GetDomainIncrement (
	*GetBegin (), newBegin);
    Vertex* beginDuplicate = m_data->GetVertexDuplicate (
	GetBegin (), domainIncrement);
    Vertex* endDuplicate = m_data->GetVertexDuplicate (
	GetEnd (), domainIncrement);

    Edge* duplicate = new Edge (*this);
    duplicate->SetBegin (beginDuplicate);
    duplicate->SetEnd (endDuplicate);
    return duplicate;
}


G3D::Vector3 Edge::GetTorusWrappedBegin (size_t index) const
{
    if (m_torusWrapped == 0)
    {
	RuntimeAssert (index == 0, "index should be 0 and is ", index);
	return *m_begin;
    }
    else
    {
	size_t n = m_torusWrapped->size ();
	RuntimeAssert (index < n, 
		       "index should be less than ", n, " and is ", index);
	return (*m_torusWrapped)[index].point (0);
    }
}

G3D::Vector3 Edge::GetTorusWrappedEnd (size_t index) const
{
    if (m_torusWrapped == 0)
    {
	RuntimeAssert (index == 0, "index should be 0 and is ", index);
	return *m_end;
    }
    else
    {
	size_t n = m_torusWrapped->size ();
	RuntimeAssert (index < n, 
		       "index should be less than ", n, " and is ", index);
	return (*m_torusWrapped)[index].point (1);
    }
}

size_t Edge::GetTorusWrappedSize () const
{
    if (m_domainIncrement == G3D::Vector3int16(0, 0, 0))
	return 1;
    else
	return 2;
}

void Edge::CalculateTorusWrapped ()
{
    using G3D::Vector3int16;
    using G3D::Vector3;
    using G3D::LineSegment;
    Vector3int16 domainIncrement = GetDomainIncrement ();
    if (domainIncrement == Vector3int16 (0, 0, 0))
    {
	Vector3int16 torusLocation = m_begin->GetTorusLocation ();
	if (torusLocation == Vector3int16 (0, 0, 0))
	    return;
	else
	{
	    domainIncrement = Vector3int16 (0, 0, 0) - torusLocation;
	    const OOBox& periods = m_data->GetPeriods ();
	    m_torusWrapped = new vector<LineSegment>(1);
	    (*m_torusWrapped)[0] = LineSegment::fromTwoPoints (
		periods.TorusTranslate (*m_begin, domainIncrement),
		periods.TorusTranslate (*m_end, domainIncrement));
	}
    }
    else
    {
	const OOBox& periods = m_data->GetPeriods ();
	Vector3 intersection = periods.Intersect (
	    *m_begin, *m_end, domainIncrement);
	domainIncrement = Vector3int16 (0, 0, 0) - domainIncrement;
	m_torusWrapped = new vector<LineSegment> (2);
	(*m_torusWrapped)[0] = LineSegment::fromTwoPoints (
	    *m_begin, intersection);
	(*m_torusWrapped)[1] = LineSegment::fromTwoPoints (
	    periods.TorusTranslate (intersection, domainIncrement),
	    periods.TorusTranslate (*m_end, domainIncrement));
    }
}



size_t Edge::CountIntersections () const
{
    const G3D::Vector3int16& domainIncrement = GetDomainIncrement ();
    return ((domainIncrement.x != 0) + 
	    (domainIncrement.y != 0) + (domainIncrement.z != 0));
}

// Static and Friends Methods
// ======================================================================

short Edge::DomainIncrementCharToNumber (char sign)
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

G3D::Vector3int16 Edge::IntToDomainIncrement (int value)
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
