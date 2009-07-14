#include <algorithm>
#include <iostream>
#include "Data.h"
#include "Element.h"

using namespace std;

template <class E>
struct printElement : public unary_function<E*, void>
{
public:
    printElement (ostream& ostr) : 
	m_ostr(ostr), m_index(0) {
    }
    void operator() (E* e) 
    {
	m_ostr << m_index << ": ";
	if (e != 0)
	    m_ostr << *e;
	else
	    m_ostr << "NULL";
	m_index++;
	m_ostr << endl;
    }
private:
    ostream& m_ostr;
    int m_index;
};

Data::~Data()
{
    for_each(m_bodies.begin(), m_bodies.end(), DeleteElement<Body>());
    for_each(m_faces.begin(), m_faces.end(), DeleteElement<Face>());
    for_each(m_edges.begin(), m_edges.end(), DeleteElement<Edge>());
    for_each(m_vertices.begin(), m_vertices.end(), DeleteElement<Point>());
}

ostream& operator<< (ostream& ostr, Data& d)
{
    ostr << "Data:" << endl;
    ostr << d.m_vertices.size() << " vertices:" << endl;
    for_each(d.m_vertices.begin (), d.m_vertices.end (), 
	     printElement<Point>(ostr));
    ostr << endl;
    ostr << d.m_edges.size() << " edges:" << endl;
    for_each(d.m_edges.begin (), d.m_edges.end (), 
	     printElement<Edge>(ostr));
    return ostr;
}

void Data::SetPoint (unsigned int i, float x, float y, float z) 
{
    if (i >= m_vertices.size ())
	m_vertices.resize (i + 1);
    m_vertices[i] = new Point (x, y ,z);
}

void Data::SetEdge (unsigned int i, unsigned int begin, unsigned int end) 
{
    if (i >= m_edges.size ())
	m_edges.resize (i + 1); 
    m_edges[i] = new Edge (data.GetPoint(begin), data.GetPoint(end));
}

void Data::SetFace (unsigned int i, const std::vector<int>& edges)
{
    if (i >= m_faces.size ())
	m_faces.resize (i + 1);
    m_faces[i] = new Face (edges, m_edges);
}

void Data::SetBody (unsigned int i, const std::vector<int>& faces)
{
    if (i >= m_bodies.size ())
	m_bodies.resize (i + 1);
    m_bodies[i] = new Body (faces, m_faces);
}

