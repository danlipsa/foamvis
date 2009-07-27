/**
 * @file   Data.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Data object
 */
#include <algorithm>
#include <iostream>
#include "Data.h"
#include "Element.h"
#include "ParsingData.h"

/**
 * Prints a 4x4 matrix element. Used in for_each algorithm.
 */
struct printMatrixElement : public unary_function<float, void>
{
    /**
     * Constructs the object
     * @param ostr stream where to print the matrix element
     */
    printMatrixElement(ostream& ostr) : m_ostr(ostr), m_index (0) {}
    /**
     * Prints the matrix element to the output stream
     * @param f element to be printed.
     */
    void operator() (float f)
    {
	if (m_index != 0)
	{
	    if ((m_index % 4) == 0)
		m_ostr << endl;
	    else
		m_ostr << ", ";
	}
	m_ostr << f;
	m_index++;
    }
private:
    /**
     * Output stream where to print the matrix element
     */
    ostream& m_ostr;
    /**
     * Index of the element currently printed.
     */
    int m_index;
};


Data::Data () :m_parsingData (new ParsingData ()) {}

Data::~Data()
{
    for_each(m_bodies.begin (), m_bodies.end (), DeleteElementPtr<Body>);
    for_each(m_faces.begin (), m_faces.end (), DeleteElementPtr<Face>);
    for_each(m_edges.begin (), m_edges.end (), DeleteElementPtr<Edge>);
    for_each(m_vertices.begin (), m_vertices.end (), DeleteElementPtr<Point>);
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

void Data::SetFace (unsigned int i, const vector<int>& edges)
{
    if (i >= m_faces.size ())
	m_faces.resize (i + 1);
    m_faces[i] = new Face (edges, m_edges);
}

void Data::SetBody (unsigned int i, const vector<int>& faces)
{
    if (i >= m_bodies.size ())
	m_bodies.resize (i + 1);
    m_bodies[i] = new Body (faces, m_faces);
}

ostream& operator<< (ostream& ostr, Data& d)
{
    ostr << "Data:" << endl;
    PrintElementPtrs<Point> (ostr, d.m_vertices, "vertices", true);
    PrintElementPtrs<Edge> (ostr, d.m_edges, "edges", true);
    PrintElementPtrs<Face> (ostr, d.m_faces, "faces", true);
    PrintElementPtrs<Body> (ostr, d.m_bodies, "bodies", true);
    ostr << "view matrix:" << endl;
    for_each (d.m_viewMatrix, 
	      d.m_viewMatrix + 
	      sizeof(d.m_viewMatrix)/sizeof(d.m_viewMatrix[0]), 
	      printMatrixElement (ostr));
    ostr << endl;
    return ostr;
}
