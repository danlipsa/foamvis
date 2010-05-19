/**
 * @file   ProcessBodyTorus.cpp
 * @author Dan R. Lipsa
 * @date 22 March 2010
 *
 * Implementation of the ProcessBodyTorus class
 */

#include "Body.h"
#include "Foam.h"
#include "Debug.h"
#include "DebugStream.h"
#include "ProcessBodyTorus.h"
#include "OrientedFace.h"
#include "Vertex.h"

void ProcessBodyTorus::Initialize ()
{
    OrientedFace* of = m_body->GetOrientedFace (0);
    m_queue.push (QueueElement (of->GetOrientedEdge (0), of));
}

bool ProcessBodyTorus::Step ()
{
    QueueElement element;
    while (m_queue.size () > 0)
    {
	element = m_queue.front ();
	if (element.second->IsTraversed ())
	    m_queue.pop ();
	else
	    break;
    }
    if (m_queue.size () == 0)
	return false;
    OrientedFace* of = element.second;
    const OrientedEdge& edge = element.first;
    G3D::Vector3 translation;
    of->CalculateTranslation (edge, &translation);
    if (! translation.isZero ())
    {
	const G3D::Vector3& begin = *of->GetOrientedEdge (0).GetBegin ();
	Face* f = m_body->GetFoam ()->GetFaceDuplicate (*of->GetFace (), begin);
	of->SetFace (f);
    }

    return true;
}
