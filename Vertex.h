/**
 * @file   Vertex.h
 * @author Dan R. Lipsa
 * 
 * Vertex represented in a DMP file. Is part of edges, faces and bodies.
 */
#ifndef __VERTEX_H__
#define __VERTEX_H__

#include "Element.h"

class AttributesInfo;
class Edge;
class Body;
class Data;

/**
 * Vertex represented in a DMP file. Is part of edges, faces and bodies.
 */
class Vertex : public G3D::Vector3, public Element
{
public:    
    struct Hash
    {
	size_t operator() (const G3D::Vector3& v) const
	{
	    std::size_t seed = 0;
	    boost::hash_combine (seed, v.x);
	    boost::hash_combine (seed, v.y);
	    boost::hash_combine (seed, v.z);
	    return seed;
	}
    };

public:
    /**
     * Creates a Vertex object
     * @param originalIndex original index for this vertex 
     *    (before invoque 'compact')
     * @param x the X coordinate
     * @param y the Y coordinate
     * @param z the Z coordinate
     */
    Vertex(float x, float y, float z,
	   size_t originalIndex, Data* data,
	   ElementStatus::Name status = ElementStatus::ORIGINAL);
    Vertex (const G3D::Vector3* position, Data* data);
    Vertex (const G3D::Vector3* position, Data* data,
	    const G3D::Vector3int16& domainIncrement);
    /**
     * Is this a physical (not tesselation) vertex
     * @return true if it is physical, false otherwise
     */
    bool IsPhysical () const 
    {
	return m_physical || (m_adjacentPhysicalEdgesCount == 4);
    }
    void SetPhysical ()
    {
	m_physical = true;
    }
    /**
     * Adds an edge that is adjacent to this vertex
     * @param edge edge touched by this vertex
     */
    void AddAdjacentEdge (Edge* edge);
    G3D::Vector3int16 GetDomain () const;
    bool operator< (const Vertex& other) const;
    bool operator== (const Vertex& other) const;
    Vertex* CreateDuplicate (const G3D::Vector3int16& domainIncrement);
    G3D::Vector3int16 GetLocation () const;
    bool InsideTorusOriginalDomain () const
    {
	return GetLocation () == G3D::Vector3int16 (0, 0, 0);
    }
    ostream& PrintAttributes (ostream& ostr) const
    {
	return printAttributes (ostr, *Vertex::m_infos);
    }
    void TorusTranslate (const G3D::Vector3int16& domainIncrement);

public:
    template <typename Vertices>
    static ostream& PrintDomains (ostream& ostr, Vertices vertices)
    {
	map< G3D::Vector3int16, list<Vertex*>, lessThanVector3int16 > 
	    domainVerticesMap;
	for_each (vertices.begin (), vertices.end (),
		  storeByDomain (domainVerticesMap));
	ostr << domainVerticesMap.size () << " domains:" << endl;
	for_each (domainVerticesMap.begin (), domainVerticesMap.end (),
		  printDomainVertices (ostr));
	return ostr;
    }

public:
    /**
     * Accumulates along X, Y or Z
     * @param result where we accumulate
     * @param v the vertex
     * @return result + the value of the vertex along the specified axis
     */
    static G3D::Vector3 Accumulate (Vector3 result, const Vertex* v)
    {
	return result + *v;
    }

    /**
     * Specifies the default attributes for the Vertex object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    static void StoreDefaultAttributes (AttributesInfo* info);
    /**
     * Pretty print for a Vertex object
     * @param ostr where to print
     * @param v what to print
     * @return output stream used to print the object to
     */
    static const G3D::Vector3int16& UnitVector3int16 (size_t direction)
    {
	return m_unitVector3int16[direction];
    }
    friend ostream& operator<< (ostream& ostr, const Vertex& v);

private:
    struct lessThanVector3int16
    {
	bool operator () (const G3D::Vector3int16& first, 
			  const G3D::Vector3int16& second)
	{
	    return 
		first.x < second.x ||
		(first.x == second.x && first.y < second.y) ||
		(first.x == second.x && first.y == second.y && first.z < second.z);
	}
    };

    class storeByDomain
    {
    public:
	storeByDomain (map< G3D::Vector3int16, list< Vertex*>,
		       lessThanVector3int16 >& 
		       domainVerticesMap) : 
	    m_domainVerticesMap (domainVerticesMap)
	{}
	void operator() (Vertex* v)
	{
	    m_domainVerticesMap[v->GetDomain ()].push_back (v);
	}
    private:
	map< G3D::Vector3int16, list< Vertex*>, 
	     lessThanVector3int16 >& m_domainVerticesMap;
    };

    class printVertexIndex
    {
    public:
	printVertexIndex (ostream& ostr) : m_ostr(ostr) {}
	void operator() (Vertex* v)
	{
	    m_ostr << (v->GetOriginalIndex () + 1) << " ";
	}
    private:
	ostream& m_ostr;
    };

    class printDomainVertices
    {
    public:
	printDomainVertices (ostream& ostr) : m_ostr(ostr) {}

	void operator() (pair<const G3D::Vector3int16, list< Vertex*> >& pair)
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
    /**
     * Edges adjacent to this vertex
     */
    vector<Edge*> m_adjacentEdges;
    size_t m_adjacentPhysicalEdgesCount;
    bool m_physical;

private:
    /**
     * Stores information about all vertex attributes
     */
    static AttributesInfo* m_infos;
    static const G3D::Vector3int16 m_unitVector3int16[3];
};
/**
 * Pretty prints a Vertex* by calling the operator<< for a Vertex.
 * @param ostr where to print
 * @param pv what to print
 * @return where to print
 */
inline ostream& operator<< (ostream& ostr, const Vertex* pv)
{
    return ostr << *pv;
}

size_t hash_value (Vertex const& v);

#endif //__VERTEX_H__

// Local Variables:
// mode: c++
// End:
