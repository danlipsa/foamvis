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
    /**
     * Creates a Vertex object
     * @param originalIndex original index for this vertex 
     *    (before invoque 'compact')
     * @param x the X coordinate
     * @param y the Y coordinate
     * @param z the Z coordinate
     */
    Vertex(float x, float y, float z,
	   unsigned int originalIndex, Data& data,
	   bool duplicate=false);
    /**
     * Is this a physical (not tesselation) vertex
     * @return true if it is physical, false otherwise
     */
    bool IsPhysical ()  {return (m_adjacentPhysicalEdgesCount == 4);}
    /**
     * Adds an edge that is adjacent to this vertex
     * @param edge edge touched by this vertex
     */
    void AddAdjacentEdge (Edge* edge);
    G3D::Vector3int16 GetDomain () ;
    void AdjustPosition (G3D::Vector3int16& domainIncrement);
    /**
     * Specifies the default attributes for the Vertex object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    static void StoreDefaultAttributes (AttributesInfo& info);
    /**
     * Pretty print for a Vertex object
     * @param ostr where to print
     * @param v what to print
     * @return output stream used to print the object to
     */
    friend ostream& operator<< (ostream& ostr,  Vertex& v);
    /**
     * Functor that compares two vertices along X, Y or Z axis
     */
    class LessThanAlong
    {
    public:
	/**
	 * Constructor
	 * Stores the axis we want to do the comparison on.
	 */
	LessThanAlong(G3D::Vector3::Axis axis) : 
	    m_axis(axis) 
	{}
	/**
	 * Compares two vertices
	 * @param first the first vertex
	 * @param second the second vertex
	 * @return true if first is less than second false otherwise
	 */
	bool operator() (Vertex* first,  Vertex* second) 
	{
	    return (*first)[m_axis] < (*second)[m_axis];
	}
    private:
	/**
	 * Axis along which we make the comparison
	 */
	G3D::Vector3::Axis m_axis;
    };

    class LessThan
    {
    public:
	bool operator () (Vertex* first, Vertex* second) const
	{
	    return first->x < second->x ||
		(first->x == second->x && first->y < second->y) ||
		(first->x == second->x && first->y == second->y && 
		 first->z < second->z);
	}
    };

    template <typename Vertices>
    static ostream& PrintDomains (ostream& ostr, Vertices vertices)
    {
	map < G3D::Vector3int16, list< Vertex*>,lessThanVector3int16 > 
	    domainVerticesMap;
	for_each (vertices.begin (), vertices.end (),
		  storeByDomain (domainVerticesMap));
	ostr << domainVerticesMap.size () << " domains:" << endl;
	for_each (domainVerticesMap.begin (), domainVerticesMap.end (),
		  printDomainVertices (ostr));
	return ostr;
    }

    /**
     * Accumulates along X, Y or Z
     * @param result where we accumulate
     * @param v the vertex
     * @return result + the value of the vertex along the specified axis
     */
    static G3D::Vector3 Accumulate (Vector3 result,  Vertex* v)
    {
	return result + *v;
    }

private:
    /**
     * Edges adjacent to this vertex
     */
    vector<Edge*> m_adjacentEdges;
    unsigned int m_adjacentPhysicalEdgesCount;
    /**
     * Stores information about all vertex attributes
     */
    static AttributesInfo* m_infos;

    struct lessThanVector3int16
    {
	bool operator () (const Vector3int16& first, const Vector3int16& second)
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


};
/**
 * Pretty prints a Vertex* by calling the operator<< for a Vertex.
 * @param ostr where to print
 * @param pv what to print
 * @return where to print
 */
inline ostream& operator<< (ostream& ostr,  Vertex* pv)
{
    return ostr << *pv;
}

#endif //__VERTEX_H__

// Local Variables:
// mode: c++
// End:
