/**
 * @file   DisplayEdgeFunctors-impl.cpp
 * @author Dan R. Lipsa
 * @date  19 Jan. 2011
 *
 * Template instantiations for functors that display an edge
 */

#include "DisplayEdgeFunctors.cpp"

// DisplayEdgeTorus
// ======================================================================

template class DisplayEdgeTorus<DisplayEdgeCylinder, DisplayArrowTube, false>;
template class DisplayEdgeTorus<DisplayEdgeCylinder, DisplayArrowTube, true>;
template class DisplayEdgeTorus<DisplayEdge, DisplayArrow, false>;
template class DisplayEdgeTorus<DisplayEdge, DisplayArrow, true>;


// DisplayEdgeWithColor
// ======================================================================

template class DisplayEdgeWithColor <DisplayElement::TEST_DISPLAY_TESSELLATION>;
template class DisplayEdgeWithColor <DisplayElement::DONT_DISPLAY_TESSELLATION>;


// DisplayEdgeTorus
// ======================================================================

template class DisplayEdges<
    DisplayEdgeTorus <DisplayEdge, DisplayArrow, true> >;
template class DisplayEdges<
    DisplayEdgeTorus<DisplayEdgeCylinder, DisplayArrowTube, true> >;
template class DisplayEdges<
    DisplayEdgeWithColor<DisplayElement::TEST_DISPLAY_TESSELLATION> >;
template class DisplayEdges<
    DisplayEdgeWithColor<DisplayElement::DONT_DISPLAY_TESSELLATION> >;
template class DisplayEdges<DisplayEdgeTorusClipped>;
