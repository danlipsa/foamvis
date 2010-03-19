/**
 * @file   TriangleFit.h
 * @author Dan R. Lipsa
 * @date 18 March 2010
 *
 * Declaration of the TriangleFit class
 */

#include "FaceEdgeIndex.h"


class TriangleFit
{
public:
    enum Margin
    {
	BEFORE_FIRST,
	AFTER_FIRST
    };

public:
    TriangleFit (
	Margin margin,
	const FaceEdgeIndex& first,
	const FaceEdgeIndex& second) :
    m_margin (margin), m_first(first), m_second(second) {}
    TriangleFit () :
    m_margin (BEFORE_FIRST) {}
    void SetMargin (Margin margin)
    {
	m_margin = margin;
    }
    Margin GetMargin () const
    {
	return m_margin;
    }
    const FaceEdgeIndex& GetFirst () const
    {
	return m_first;
    }
    const FaceEdgeIndex& GetSecond () const
    {
	return m_second;
    }
    const OrientedFace& GetFirstFace () const
    {
	return *m_first.m_face;
    }
    const OrientedFace& GetSecondFace () const
    {
	return *m_second.m_face;
    }

    void AddQueue (list<TriangleFit>* queue, 
		   TriangleFit* src, OrientedFace* fit);
    OrientedFace* FitAndDuplicateFace (Body* body) const;

public:
    void GetPoints (G3D::Vector3* points) const;

public:
    static void Initialize (list<TriangleFit>* queue,
			    Body* body);
    static Margin FlipMargin (const Margin margin);
    static void GetTriangle (
	const FaceEdgeIndex& first, const FaceEdgeIndex& second,
	G3D::Vector3* triangle);
    friend ostream& operator<< (ostream& ostr, const TriangleFit& fit);

private:
    Margin m_margin;
    FaceEdgeIndex m_first;
    FaceEdgeIndex m_second;
};
