/**
 * @file   Vertex.h
 * @author Dan R. Lipsa
 * 
 * Vertex represented in a DMP file. Is part of edges, faces and bodies.
 */
#ifndef __VERTEX_H__
#define __VERTEX_H__

#include "Element.h"
#include "Comparisons.h"

class Body;
class AttributesInfo;
class Edge;
class OOBox;

/**
 * Vertex represented in a DMP file. Is part of edges, faces and bodies.
 */
class Vertex : public G3D::Vector3, public Element
{
public:
    /**
     * Creates a Vertex object
     * @param id original index for this vertex 
     *    (before invoque 'compact')
     * @param x the X coordinate
     * @param y the Y coordinate
     * @param z the Z coordinate
     */
    Vertex(float x, float y, float z,
	   size_t id, 
	   ElementStatus::Enum duplicateStatus = ElementStatus::ORIGINAL);
    Vertex (const G3D::Vector3& position);

    /**
     * Is this a physical (not tesselation) vertex
     * @return true if it is physical, false otherwise
     */
    bool IsPhysical (size_t dimension, bool isQuadratic) const ;
    /**
     * Adds an edge that is adjacent to this vertex
     * @param edge edge touched by this vertex
     */
    void AddEdgePartOf (const boost::shared_ptr<Edge>& edge);
    G3D::Vector3int16 GetDomain () const;
    bool operator< (const Vertex& other) const;
    bool operator== (const Vertex& other) const;
    string ToString () const;
    boost::shared_ptr<Vertex> GetDuplicate (
	const OOBox& periods,
	const G3D::Vector3int16& translation,
	VertexSet* vertexSet) const;


private:
    class storeByDomain
    {
    public:
	storeByDomain (map< G3D::Vector3int16,
		       list< boost::shared_ptr<Vertex> >,
		       Vector3int16LessThan >& 
		       domainVerticesMap) : 
	    m_domainVerticesMap (domainVerticesMap)
	{}
	void operator() (boost::shared_ptr<Vertex> v)
	{
	    m_domainVerticesMap[v->GetDomain ()].push_back (v);
	}
    private:
	map< G3D::Vector3int16, list< boost::shared_ptr<Vertex> >, 
	     Vector3int16LessThan >& m_domainVerticesMap;
    };

    class printVertexIndex
    {
    public:
	printVertexIndex (ostream& ostr) : m_ostr(ostr) {}
	void operator() (boost::shared_ptr<Vertex> v)
	{
	    m_ostr << (v->GetId () + 1) << " ";
	}
    private:
	ostream& m_ostr;
    };

    class printDomainVertices
    {
    public:
	printDomainVertices (ostream& ostr) : m_ostr(ostr) {}

	void operator() (pair<const G3D::Vector3int16,
			 list< boost::shared_ptr<Vertex> > >& pair)
	{
	    m_ostr << "Domain: " << pair.first
		   << " Vertices: ";
	    for_each (pair.second.begin (), pair.second.end (), 
		      printVertexIndex (m_ostr));
	    m_ostr << endl;
	}
    private:
	ostream& m_ostr;
    };

private:
    void torusTranslate (
	const OOBox& periods,
	const G3D::Vector3int16& translation);
    boost::shared_ptr<Vertex> createDuplicate (
	const OOBox& periods, const G3D::Vector3int16& translation) const;

private:
    /**
     * Edges this vertex is part of
     */
    vector<boost::shared_ptr<Edge> > m_edgesPartOf;
};
/**
 * Pretty print for a Vertex object
 * @param ostr where to print
 * @param v what to print
 * @return output stream used to print the object to
 */
inline ostream& operator<< (ostream& ostr, const Vertex& v)
{
    return ostr << v.ToString ();
}
/**
 * Pretty prints a Vertex* by calling the operator<< for a Vertex.
 * @param ostr where to print
 * @param pv what to print
 * @return where to print
 */
inline ostream& operator<< (ostream& ostr,
			    const boost::shared_ptr<Vertex>& vertex)
{
    return ostr << *vertex << "useCount=" << vertex.use_count ();
}

size_t hash_value (Vertex const& v);

#endif //__VERTEX_H__

// Local Variables:
// mode: c++
// End:
