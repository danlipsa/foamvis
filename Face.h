/**
 * @file   Face.h
 * @author Dan R. Lipsa
 *
 * Declaration of Face class
 */
#ifndef __FACE_H__
#define __FACE_H__

#include "OrientedEdge.h"
#include "Color.h"

class AttributesInfo;

/**
 * A Face is a oriented list of edges.
 */
class Face : public Element
{
public:
    /**
     * Constructs a Face object
     * @param edgeIndexes indexes into a vector of Edge objects
     * @param edges vector of Edge objects
     */
    Face(const std::vector<int>& edgeIndexes, std::vector<Edge*>& edges);
    /**
     * Destroys a Face object
     */
    ~Face();
    /**
     * Pretty prints this Face by printing the edges in REVERSE order
     */
    void ReversePrint (std::ostream& ostr);
    /**
     * Gets the list of oriented edges
     * @return vector of oriented edges
     */
    const std::vector<OrientedEdge*>& GetOrientedEdges () const 
    { return m_edges;}
    /**
     * Returns the face color
     */
    Color::Name GetColor () const;
    /**
     * Specifies the default attributes for an Face object.
     * These attributes don't appear as a DEFINE in the .DMP file
     * @param info the object where the default attributes are stored.
     */
    static void StoreDefaultAttributes (AttributesInfo& info);
    /**
     * Pretty prints this Face by printing the edges in DIRECT order
     */
    friend std::ostream& operator<< (std::ostream& ostr, Face& f); 
private:
    /**
     * Index where the color attribute is stored for a face
     */
    const static unsigned int COLOR_INDEX = 0;
    /**
     * Edges that are part of this face
     */
    std::vector<OrientedEdge*> m_edges;
    /**
     * Stores information about all vertex attributes
     */
    static AttributesInfo* m_infos;
};
/**
 * Pretty prints a Face*
 * @param ostr where to print the Face object
 * @param f what to print
 * @return stream where to print other data
 */
inline std::ostream& operator<< (std::ostream& ostr, Face* f)
{
    return ostr << *f;
}


#endif //__FACE_H__

// Local Variables:
// mode: c++
// End:
