/**
 * @file OrientedFace.h
 * @author Dan R. Lipsa
 * @brief An oriented face. Allows using a Face in direct or reversed order.
 * @ingroup data model
 */
#ifndef __ORIENTED_FACE_H__
#define __ORIENTED_FACE_H__

#include "AdjacentBody.h"
#include "Comparisons.h"
#include "OrientedElement.h"

class Face;
class Vertex;
class OrientedEdge;

/**
 * @brief An oriented face. Allows using a Face in direct or reversed order.
 *
 * An oriented face is a face  (list of edges) that can have its edges
 * read in direct or reversed order. If 0, 1, 2, ..., n-1 are the
 * edges of the face in direct order, n-1, n-2, ..., 1, 0 are the
 * edges of the face in reverse order.
 */
class OrientedFace : public OrientedElement
{
public:
    /**
     * Constructs a OrientedFace object
     */
    OrientedFace () : OrientedElement() {}
    OrientedFace(const boost::shared_ptr<Face>& face, bool reversed);
    /**
     * Gets the face associated with this oriented face
     * @return the face associated with this oriented face
     */
    boost::shared_ptr<Face> GetFace () const;
    void SetFace (const boost::shared_ptr<Face>& face);

    void AddAdjacentBody (const boost::shared_ptr<Body>& body, size_t ofIndex);
    const AdjacentBody& GetAdjacentBody (bool faceReversed = false) const;
    size_t GetAdjacentBodySize () const;
    void UpdateAdjacentFace (const boost::shared_ptr<OrientedFace>& of);

    /**
     * Gets the begin vertex for an edge in this oriented face
     * @param edgeIndex what edge we are interested in
     * @return the begin vertex
     */
    boost::shared_ptr<Vertex> GetBeginVertex (size_t edgeIndex) 
    {
	return getBeginVertex (edgeIndex);
    }
    boost::shared_ptr<const Vertex> GetBeginVertex (size_t edgeIndex) const
    {
	return getBeginVertex (edgeIndex);
    }
    /**
     * Gets the end vertex for an edge in this oriented face
     * @param edgeIndex what edge we are interested in
     * @return the end vertex
     */
    boost::shared_ptr<Vertex> GetEndVertex (size_t edgeIndex)
    {
	return getEndVertex (edgeIndex);
    }
    boost::shared_ptr<const Vertex> GetEndVertex (size_t edgeIndex) const
    {
	return getEndVertex (edgeIndex);
    }
    /**
     * Gets the oriented edge at edgeIndex in face order.
     */
    OrientedEdge GetOrientedEdge (size_t edgeIndex) const;

    size_t GetNextValidIndex (size_t index) const;
    size_t GetPreviousValidIndex (size_t index) const;
    G3D::Vector3 GetNormal () const;
    G3D::Plane GetPlane () const;

    size_t size () const;
    bool IsClosed () const;
    const G3D::Vector3& GetCenter () const;
    double GetPerimeter () const;
    float GetArea () const;
    void CalculatePerimeter ();
    string ToString (const AttributesInfo* ai = 0) const;
    void GetVertexSet (VertexSet* vertexSet) const;
    static void GetVertexSetV (
	const vector<boost::shared_ptr<OrientedFace> >& vof,
	VertexSet* vertexSet);
    void GetEdgeSet (EdgeSet* edgeSet) const;

    bool IsStandalone () const;
    QColor GetColor (const QColor& defaultColor) const;
    bool HasConstraints () const;
    size_t GetConstraintIndex (size_t i = 0) const;
    static pair< G3D::Vector3, boost::shared_ptr<OrientedFace> > 
    MakeNormalFacePair (boost::shared_ptr<OrientedFace> face)
    {
	return pair<G3D::Vector3, boost::shared_ptr<OrientedFace> > (
	    face->GetNormal (), face);
    }
    static vtkSmartPointer<vtkPolyData> GetPolyData (
	const vector<boost::shared_ptr<OrientedFace> >& vof);


private:
    boost::shared_ptr<Vertex> getBeginVertex (size_t edgeIndex) const;
    boost::shared_ptr<Vertex> getEndVertex (size_t edgeIndex) const;
    static void getPolyPoints (
	const vector<boost::shared_ptr<OrientedFace> >& vof,
	vtkSmartPointer<vtkPoints>* polyPoints,
	vector<boost::shared_ptr<Vertex> >* sortedPoints);
    static void createPolyCells (
	const vector<boost::shared_ptr<OrientedFace> >& vof,
	vtkSmartPointer<vtkPolyData> polyData, 
	const vector<boost::shared_ptr<Vertex> >& sortedPoints);


};

/**
 * Pretty print for the OrientedFace object
 */
inline ostream& operator<< (ostream& ostr, const OrientedFace& of)
{
    return ostr << of.ToString ();
}

/**
 * Pretty prints a pointer to an oriented face
 */
inline ostream& operator<< (ostream& ostr, 
			    const boost::shared_ptr<OrientedFace>& of)
{
    return ostr << *of;
}


#endif //__ORIENTED_FACE_H__

// Local Variables:
// mode: c++
// End:
