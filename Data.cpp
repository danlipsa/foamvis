#include <functional>
#include <algorithm>
#include <iostream>
#include "Data.h"

using namespace std;

template <class E>
struct deleteElement : public unary_function<E*, void>
{void operator() (E* e) {delete e;}};

template <class E>
struct PrintElement : public unary_function<E*, void>
{
public:
    PrintElement (ostream& ostr) : 
	m_ostr(ostr), m_index(0) {
    }
    void operator() (E* e) 
    {
	m_ostr << m_index << ": ";
	if (e != 0)
	    m_ostr << *e;
	else
	    m_ostr << "NULL\n";
	m_index++;
    }
private:
    ostream& m_ostr;
    int m_index;
};

Data::~Data()
{
    for_each(m_bodies.begin(), m_bodies.end(), deleteElement<Body>());
    for_each(m_faces.begin(), m_faces.end(), deleteElement<Face>());
    for_each(m_edges.begin(), m_edges.end(), deleteElement<Edge>());
    for_each(m_vertices.begin(), m_vertices.end(), deleteElement<Point>());
}

ostream& operator<< (ostream& ostr, Data& d)
{
    ostr << "Data:" << endl;
    ostr << d.m_vertices.size() << " vertices:" << endl;
    for_each(d.m_vertices.begin (), d.m_vertices.end (), 
	     PrintElement<Point>(ostr));
    ostr << endl;
    ostr << d.m_edges.size() << " edges:" << endl;
    for_each(d.m_edges.begin (), d.m_edges.end (), 
	     PrintElement<Edge>(ostr));
    return ostr;
}

void Data::SetPoint (unsigned int i, Point* p) 
{
    if (i > m_vertices.size ())
	m_vertices.resize (i);
    m_vertices[i - 1] = p;
}

void Data::SetEdge (unsigned int i, Edge* e) 
{
    if (i > m_edges.size ())
	m_edges.resize (i); 
    m_edges[i - 1] = e;
}

void Data::SetFace (unsigned int i, std::vector<int>& edges)
{
    if (i > m_faces.size ())
	m_faces.resize (i);
    m_faces[i - 1] = new Face (edges, m_edges);
}

void Data::SetBody (unsigned int i, std::vector<int>& faces)
{
    if (i > m_bodies.size ())
	m_bodies.resize (i);
    m_bodies[i - 1] = new Body (faces, m_faces);
}

