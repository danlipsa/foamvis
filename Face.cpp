/**
 * @file   Face.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Face class
 */
#include "Face.h"
#include "ElementUtils.h"
#include "AttributeInfo.h"
#include "ParsingDriver.h"


/**
 * Unary function that  creates an oriented edge from  an index into a
 * vector of Edge objects.
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
     * Creates an oriented edge from an 1-based index (signed integer).
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
        i--;
        return new OrientedEdge(m_edges[i], reversed);
    }
private:
    /**
     * Vector of edges
     */
    vector<Edge*>& m_edges;
};

ostream& operator<< (ostream& ostr, Face& f)
{
    if (&f == 0)
        ostr << "NULL";
    else
        PrintElements<OrientedEdge*> (ostr, f.m_edges, 
				      "edges part of the face", true);
    ostr << " Face attributes: ";
    return f.PrintAttributes (ostr, *Face::m_infos);
}

AttributesInfo* Face::m_infos;

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
        ReversePrintElements<OrientedEdge*> (
            ostr, m_edges, "edges part of the face", true);
    ostr << " Face attributes: ";
    PrintAttributes (ostr, *Face::m_infos);
}

void Face::StoreDefaultAttributes (AttributesInfo& infos)
{
    using EvolverData::parser;
    m_infos = &infos;
    const char* colorString = 
        ParsingDriver::GetKeywordString(parser::token::COLOR);
    // load the color attribute and nothing else
    infos.Load (colorString);

    infos.AddAttributeInfo (colorString, new ColorAttributeCreator());
    infos.AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL),
        new IntegerAttributeCreator());
}

Color::Name Face::GetColor () const
{
    return dynamic_cast<const ColorAttribute*>(
        (*m_attributes)[COLOR_INDEX])->GetColor ();
}

