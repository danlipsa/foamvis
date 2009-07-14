#include <algorithm>
#include "Face.h"
#include "Element.h"

using namespace std;

Face::~Face()
{
    for_each(m_edges.begin(), m_edges.end(), DeleteElement<OrientedEdge>());
}

class IndexToOrientedEdge : public unary_function<int, OrientedEdge*>
{
public:
    IndexToOrientedEdge(vector<Edge*>& edges): m_edges(edges) {}

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
    vector<Edge*>& m_edges;
};


Face::Face(const vector<int>& edge_indexes, vector<Edge*>& edges)
{
    transform (edge_indexes.begin(), edge_indexes.end(), m_edges.begin(), 
	       IndexToOrientedEdge(edges));
}
