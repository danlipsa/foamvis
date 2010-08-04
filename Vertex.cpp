/**
 * @file Vertex.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Vertex class
 */
#include "AttributeInfo.h"
#include "AttributeCreator.h"
#include "Body.h"
#include "Edge.h"
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
    return Vector3Hash () (v);
}

// Methods
// ======================================================================
Vertex::Vertex(float x, float y, float z, 
	       size_t id, ElementStatus::Enum duplicateStatus) :
    G3D::Vector3 (x, y, z),
    Element(id, duplicateStatus)
{}

Vertex::Vertex (const G3D::Vector3& position) : 
    G3D::Vector3 (position),
    Element (Element::INVALID_INDEX, ElementStatus::ORIGINAL)
{}


void Vertex::AddEdgePartOf (const boost::shared_ptr<Edge>& edge) 
{
    m_edgesPartOf.push_back (edge);
}

G3D::Vector3int16 Vertex::GetDomain () const
{
    return Vector3int16Zero;
}

bool Vertex::operator< (const Vertex& other) const
{
    return x < other.x ||
	(x == other.x && y < other.y) ||
	(x == other.x && y == other.y && z < other.z);
}

bool Vertex::operator== (const Vertex& other) const
{
    return static_cast<const G3D::Vector3&>(*this) ==
	static_cast<const G3D::Vector3&>(other);
}

bool Vertex::IsPhysical (size_t dimension, bool isQuadratic) const 
{
    if (dimension == 3)
	return count_if (m_edgesPartOf.begin (), m_edgesPartOf.end (),
			 boost::bind (&Edge::IsPhysical, _1, 
				      dimension, isQuadratic)) == 4;
    else
	if (isQuadratic)
	    return true;
	else
	    return m_edgesPartOf.size () >= 3;
}

string Vertex::ToString () const
{
    ostringstream ostr;
    ostr << "Vertex " << GetStringId () << " "
	 << static_cast<const G3D::Vector3&>(*this) << " "
	 << GetDuplicateStatus ()
	 << " Vertex attributes: ";
    PrintAttributes (ostr);
    return ostr.str ();
}

void Vertex::torusTranslate (
    const OOBox& periods,
    const G3D::Vector3int16& translation)
{
    *static_cast<G3D::Vector3*>(this) = 
	periods.TorusTranslate (*this, translation);
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
	periods.TorusTranslate (*this, translation));
    VertexSet::iterator it = fuzzyFind 
	<VertexSet, VertexSet::iterator, VertexSet::key_type> (
	    *vertexSet, searchDummy);
    if (it != vertexSet->end ())
	return *it;
    boost::shared_ptr<Vertex> duplicate = this->createDuplicate (
	periods, translation);
    vertexSet->insert (duplicate);
    return duplicate;
}
