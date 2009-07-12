#include <functional>
#include <algorithm>
#include <iostream>
#include "Data.h"

using namespace std;

struct deleteBody : public unary_function<Body*, void>
{void operator() (Body* b) {delete b;}};

struct deleteFace : public unary_function<Face*, void>
{void operator() (Face* f) {delete f;}};

struct deleteEdge : public unary_function<Edge*, void>
{void operator() (Edge* e) {delete e;}};

struct deletePoint : public unary_function<Point*, void>
{void operator() (Point* p) {delete p;}};

struct PrintPoint : public unary_function<Point*, void>
{
public:
    PrintPoint (ostream& ostr) : 
	m_ostr(ostr) {
    }
    void operator() (Point* p) {
	m_ostr << *p;}
private:
    ostream& m_ostr;
};

Data::~Data()
{
    for_each(m_bodies.begin(), m_bodies.end(), deleteBody());
    for_each(m_faces.begin(), m_faces.end(), deleteFace());
    for_each(m_edges.begin(), m_edges.end(), deleteEdge());
    for_each(m_vertices.begin(), m_vertices.end(), deletePoint());
}

ostream& operator<< (ostream& ostr, Data& d)
{
    ostr << "Data:" << endl;
    ostr << d.m_vertices.size() << " vertices:" << endl;
    for_each(d.m_vertices.begin (), d.m_vertices.end (), PrintPoint(ostr));
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

