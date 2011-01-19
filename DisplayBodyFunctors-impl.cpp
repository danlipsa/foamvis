/**
 * @file   DisplayBodyFunctors-impl.cpp
 * @author Dan R. Lipsa
 * @date  19 Jan. 2011
 *
 * Template instantiations for functors to display a body
 */


#include "DisplayBodyFunctors.cpp"

// DisplayBodyBase
// ======================================================================

template class DisplayBodyBase<VertexAttributeSetter>;
template class DisplayBodyBase<TexCoordSetter>;

// DisplayBody
// ======================================================================

template class DisplayBody<
    DisplayFace<
	DisplayEdges<
	    DisplayEdgeWithColor<DisplayElement::DONT_DISPLAY_TESSELLATION> >, 
	TexCoordSetter>, TexCoordSetter>;
template class DisplayBody<
    DisplayFace<
	DisplayEdges<
	    DisplayEdgeWithColor<DisplayElement::TEST_DISPLAY_TESSELLATION> >, 
	TexCoordSetter>, TexCoordSetter>;

template class DisplayBody<
    DisplayFace<
	DisplayEdges<DisplayEdgeTorusClipped>, 
	TexCoordSetter>, 
    TexCoordSetter>;
template class DisplayBody<
    DisplayFace<DisplaySameEdges, TexCoordSetter>, TexCoordSetter>;
template class DisplayBody<
    DisplayFace<DisplaySameTriangles, TexCoordSetter>, TexCoordSetter>;

template class DisplayBody<
    DisplayFaceWithColor<DisplaySameEdges, TexCoordSetter>, TexCoordSetter>;
template class DisplayBody<
    DisplayFaceWithColor<DisplaySameTriangles, TexCoordSetter>, TexCoordSetter>;
template class DisplayBody<
    DisplayFaceWithColor<DisplaySameEdges, VertexAttributeSetter>, 
    VertexAttributeSetter>;
template class DisplayBody<
    DisplayFaceWithColor<DisplaySameTriangles, VertexAttributeSetter>, 
    VertexAttributeSetter>;


// DisplayCenterPath
// ======================================================================

template class DisplayCenterPath<TexCoordSetter, DisplayEdgeCylinder>;
template class DisplayCenterPath<TexCoordSetter, DisplayEdge>;
