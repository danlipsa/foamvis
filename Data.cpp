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
#include "foam_yacc.h"

template <class E>
void compact (vector<E*>& v)
{
    unsigned int step = 0;
    for (unsigned int i = 0; i < v.size (); i++)
    {
	if (v[i] == 0)
	    step++;
	else if (step != 0)
	    v[i - step] = v[i];
    }
    unsigned int resize = v.size () - step;
    cerr << "resizing vector with " << step << endl;
    v.resize (resize);
}



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

Data::Data () : 
    m_attributesInfo(Attribute::ATTRIBUTE_TYPE_COUNT),
    m_parsingData (new ParsingData ())
{
    m_attributesInfo[Attribute::VERTEX_TYPE][KeywordString(ORIGINAL)] = 
	new AttributeInfo (0, new IntegerAttributeCreator());

    m_attributesInfo[Attribute::EDGE_TYPE][KeywordString(ORIGINAL)] = 
	new AttributeInfo (0, new IntegerAttributeCreator());

    m_attributesInfo[Attribute::FACE_TYPE][KeywordString(ORIGINAL)] = 
	new AttributeInfo (0, new IntegerAttributeCreator());
    m_attributesInfo[Attribute::FACE_TYPE][KeywordString(COLOR)] = 
	new AttributeInfo (0, new IntegerAttributeCreator());

    m_attributesInfo[Attribute::BODY_TYPE][KeywordString(ORIGINAL)] = 
	new AttributeInfo (0, new IntegerAttributeCreator());
    m_attributesInfo[Attribute::BODY_TYPE]
	[KeywordString(LAGRANGE_MULTIPLIER)] = 
	new AttributeInfo (0, new RealAttributeCreator());
    m_attributesInfo[Attribute::BODY_TYPE]
	[KeywordString(VOLUME)] = 
	new AttributeInfo (0, new RealAttributeCreator());
}

Data::~Data ()
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

void Data::Compact (void)
{
    compact (m_vertices);
    compact (m_edges);
    compact (m_faces);
    compact (m_bodies);
}

void Data::AddAttributeInfo (
    Attribute::Type type, const char* name, AttributeCreator* creator)
{
    m_attributesInfo[type][name] = new AttributeInfo (
	m_attributesInfo[type].size(), creator);
}
