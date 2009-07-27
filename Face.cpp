/**
 * @file   Face.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Face class
 */
#include <algorithm>
#include "Face.h"
#include "Element.h"

/**
 * Unary function that converts an index into a vector of Edge objects
 * into a oriented edge.
 */
class indexToOrientedEdge : public unary_function<int, OrientedEdge*>
{
public:
    /**
     * Constructs this function object
     * @param edges vector of Edge objects
     */
    indexToOrientedEdge(vector<Edge*>& edges): m_edges(edges) {}
    /**
     * Converts an index (signed integer) into a oriented edge
     * @param i index into the  vector of edges. A negative sign means
     *        that the edege  apears in the face in  the reverse order
     *        than it appears in the vector of edges.
     * @return an OrientedEdge which is  like an Edge and a boolean that
     *        specifies the order of the edge.
     */
    OrientedEdge* operator() (int i)
    {
	bool reversed = false;
	if (i < 0)
	{
	    i = -i;
	    reversed = true;
	}
	return new OrientedEdge(m_edges[i], reversed);
    }
private:
    /**
     * Vector of edges
     */
    vector<Edge*>& m_edges;
};

Face::Face(const vector<int>& edgeIndexes, vector<Edge*>& edges)
{
    m_edges.resize (edgeIndexes.size ());
    transform (edgeIndexes.begin(), edgeIndexes.end(), m_edges.begin(), 
	       indexToOrientedEdge(edges));
}

Face::~Face()
{
    for_each(m_edges.begin(), m_edges.end(), DeleteElementPtr<OrientedEdge>);
}

void Face::ReversePrint (ostream& ostr)
{
    if (this == 0)
	ostr << "NULL";
    else
	ReversePrintElementPtrs<OrientedEdge> (
	    ostr, m_edges, "edges part of the face", true);
}

ostream& operator<< (ostream& ostr, Face& f)
{
    if (&f == 0)
	ostr << "NULL";
    else
	PrintElementPtrs<OrientedEdge> (ostr, f.m_edges, 
					"edges part of the face", true);
    return ostr;
}
