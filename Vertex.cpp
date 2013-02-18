/**
 * @file Vertex.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Vertex class
 */
#include "Attribute.h"
#include "AttributeInfo.h"
#include "AttributeCreator.h"
#include "Body.h"
#include "Edge.h"
#include "Foam.h"
#include "DataProperties.h"
#include "EvolverData_yacc.h"
#include "DebugStream.h"
#include "Debug.h"
#include "Hashes.h"
#include "OOBox.h"
#include "ParsingDriver.h"
#include "Utils.h"
#include "Vertex.h"

// Private Functions
// ======================================================================
size_t hash_value (Vertex const& v)
{
    return Vector3Hash () (v.GetVector ());
}

// Methods
// ======================================================================
Vertex::Vertex(double x, double y, double z, 
	       size_t id, ElementStatus::Enum duplicateStatus) :
    Element(id, duplicateStatus),
    m_vector (x, y, z)
{}

Vertex::Vertex (const G3D::Vector3& position, size_t id) : 
    Element (id, ElementStatus::ORIGINAL),
    m_vector (position)
{}


void Vertex::AddAdjacentEdge (const boost::shared_ptr<Edge>& edge) 
{
    m_adjacentEdges.push_back (edge);
}

G3D::Vector3int16 Vertex::GetDomain () const
{
    return Vector3int16Zero;
}

bool Vertex::operator< (const Vertex& other) const
{
    return 
	GetId () < other.GetId () ||

	(GetId () == other.GetId () &&
	 G3D::fuzzyLt (GetVector ().x, other.GetVector ().x)) ||

	(GetId () == other.GetId () &&
	 G3D::fuzzyEq (GetVector ().x, other.GetVector ().x) && 
	 G3D::fuzzyLt (GetVector ().y, other.GetVector ().y)) ||

	(GetId () == other.GetId () &&
	 G3D::fuzzyEq (GetVector ().x, other.GetVector ().x) && 
	 G3D::fuzzyEq (GetVector ().y, other.GetVector ().y) && 
	 G3D::fuzzyLt (GetVector ().z, other.GetVector ().z));
}

bool Vertex::operator== (const Vertex& other) const
{
    return GetVector () == other.GetVector ();
}

bool Vertex::fuzzyEq (const Vertex& other) const
{
    return IsFuzzyZero (GetVector () - other.GetVector ());
}


bool Vertex::IsPhysical (bool is2D) const 
{
    if (IsStandalone ())
        return true;
    else
    {
        if (is2D)
        {
            if (adjacentQuadraticEdge ())
                return true;
            else
                return m_adjacentEdges.size () >= 3;
        }
        else
            return 
                count_if (m_adjacentEdges.begin (), m_adjacentEdges.end (),
                          boost::bind (&Edge::IsPhysical, _1)) 
                == 4;
    }
}

bool Vertex::adjacentQuadraticEdge () const
{
    return count_if (m_adjacentEdges.begin (), m_adjacentEdges.end (),
                     boost::bind (&Edge::IsQuadratic, _1)) >= 1;
}




string Vertex::ToString (const AttributesInfo* ai) const
{
    ostringstream ostr;
    ostr << "Vertex " << GetStringId () << " "
	 << GetVector () << " "
	 << GetDuplicateStatus ();
    if (HasAttributes ())
    {
	ostr << endl << "Vertex attributes: ";
	PrintAttributes (ostr, ai);
    }
    return ostr.str ();
}

void Vertex::torusTranslate (
    const OOBox& periods,
    const G3D::Vector3int16& translation)
{
    m_vector = periods.TorusTranslate (GetVector (), translation);
}

boost::shared_ptr<Vertex> Vertex::createDuplicate (
    const OOBox& periods,
    const G3D::Vector3int16& translation) const
{
    boost::shared_ptr<Vertex> duplicate = boost::make_shared<Vertex> (
	*this);
    duplicate->SetDuplicateStatus (ElementStatus::DUPLICATE);
    duplicate->torusTranslate (periods, translation);
    return duplicate;
}

boost::shared_ptr<Vertex> Vertex::GetDuplicate (
    const OOBox& periods,
    const G3D::Vector3int16& translation,
    VertexSet* vertexSet) const
{
    boost::shared_ptr<Vertex> searchDummy = boost::make_shared<Vertex>(
	periods.TorusTranslate (GetVector (), translation), GetId ());
    VertexSet::iterator it = vertexSet->find (searchDummy);
    if (it != vertexSet->end ())
	return *it;
    boost::shared_ptr<Vertex> duplicate = this->createDuplicate (
	periods, translation);
    vertexSet->insert (duplicate);
    return duplicate;
}

size_t Vertex::GetConstraintIndex (size_t i) const
{
    return GetAttribute<IntegerArrayAttribute, 
	    IntegerArrayAttribute::value_type> (
		VertexAttributeIndex::CONSTRAINTS) [i] - 1;
}

size_t FindVertex (const vector<boost::shared_ptr<Vertex> >& sortedPoints,
		   const boost::shared_ptr<Vertex>& point)
{
    vector<boost::shared_ptr<Vertex> >::const_iterator centerIt = 
	lower_bound (sortedPoints.begin (), sortedPoints.end (), point, 
		     VertexPtrLessThan ());
    return centerIt - sortedPoints.begin ();
}
