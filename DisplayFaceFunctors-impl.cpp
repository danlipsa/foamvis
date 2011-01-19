/**
 * @file   DisplayFaceFunctors-impl.cpp
 * @author Dan R. Lipsa
 * @date  19 Jan. 2011
 *
 * Template instantiations for functors that display a face
 */

#include "DisplayFaceFunctors.cpp"

// DisplayFace
// ======================================================================

template class DisplayFace<
    DisplayEdges<
	DisplayEdgeTorus<DisplayEdgeCylinder, DisplayArrowTube, true> >, 
    TexCoordSetter>;
template class DisplayFace<
    DisplayEdges<
	DisplayEdgeTorus<DisplayEdge, DisplayArrow, true> >, TexCoordSetter>;
template class DisplayFace<
    DisplayEdges<
	DisplayEdgeTorusClipped>, TexCoordSetter>;
template class DisplayFace<
    DisplayEdges<
	DisplayEdgeWithColor<DisplayElement::TEST_DISPLAY_TESSELLATION> >, 
    TexCoordSetter>;
template class DisplayFace<
    DisplayEdges<
	DisplayEdgeWithColor<DisplayElement::DONT_DISPLAY_TESSELLATION> >, 
    TexCoordSetter>;
template class DisplayFace<DisplaySameEdges, TexCoordSetter>;
template class DisplayFace<DisplaySameTriangles, TexCoordSetter>;
template class DisplayFace<DisplaySameTriangles, VertexAttributeSetter>;
template class DisplayFace<DisplaySameEdges, VertexAttributeSetter>;

// DisplayFaceWithColor
// ======================================================================

template class DisplayFaceWithColor<DisplaySameEdges, TexCoordSetter>;
template class DisplayFaceWithColor<DisplaySameTriangles, TexCoordSetter>;
template class DisplayFaceWithColor<DisplaySameTriangles, VertexAttributeSetter>;
template class DisplayFaceWithColor<DisplaySameEdges, VertexAttributeSetter>;
