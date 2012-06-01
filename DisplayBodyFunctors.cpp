/**
 * @file   DisplayBodyFunctors.h
 * @author Dan R. Lipsa
 * @date  26 Oct. 2010
 *
 * Implementation for functors to display a body
 */

#include "Body.h"
#include "BodySelector.h"
#include "ColorBarModel.h"
#include "DebugStream.h"
#include "DisplayBodyFunctors.h"
#include "DisplayFaceFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "Simulation.h"
#include "GLWidget.h"
#include "OpenGLUtils.h"
#include "ViewSettings.h"
#include "VectorAverage.h"

struct ContextSegment : public Segment
{
    ContextSegment () : Segment ()
    {
    }
    
    ContextSegment (const QColor& color, const Segment& segment) :
	Segment (segment), m_color (color)
    {
    }
    QColor m_color;
};

struct FocusTextureSegment : public Segment
{
    FocusTextureSegment () : Segment (), m_textureCoordinate (0)
    {
    }

    FocusTextureSegment (GLfloat textureCoordinate,
			 const Segment& segment) :
	Segment (segment), m_textureCoordinate (textureCoordinate)
    {
    }
    GLfloat m_textureCoordinate;
};

struct FocusColorSegment : public Segment
{
    FocusColorSegment () : Segment ()
    {
    }

    FocusColorSegment (const QColor& color,
		       const Segment& segment) :
	Segment (segment), m_color (color)
    {
    }
    QColor m_color;
};




// DisplayBodyBase
// ======================================================================

template <typename PropertySetter>
DisplayBodyBase<PropertySetter>::
DisplayBodyBase (const GLWidget& widget, const Foam& foam,
		 const BodySelector& bodySelector,
		 PropertySetter propertySetter, bool useZPos, double zPos) :

    DisplayElementProperty<PropertySetter> (
	widget, foam, propertySetter, useZPos, zPos),
    m_bodySelector (bodySelector)
{
}

template <typename PropertySetter>
void DisplayBodyBase<PropertySetter>::
BeginContext ()
{
    glEnable (GL_BLEND);
    glDepthMask (GL_FALSE);
}

template <typename PropertySetter>
void DisplayBodyBase<PropertySetter>::
EndContext ()
{
    glDepthMask (GL_TRUE);
    glDisable (GL_BLEND);
}


// DisplayBodyDeformation
// ======================================================================

DisplayBodyDeformation::DisplayBodyDeformation (
    const GLWidget& widget, ViewNumber::Enum viewNumber, 
    const Foam& foam,
    const BodySelector& bodySelector,
    bool useZPos, double zPos):
    
    DisplayBodyBase<> (
	widget, foam, bodySelector,
	SetterTextureCoordinate(widget, viewNumber), useZPos, zPos)
{}

void DisplayBodyDeformation::operator () (boost::shared_ptr<Body> body)
{
    if (body->IsConstraint ())
	return;
    ViewNumber::Enum viewNumber = m_propertySetter.GetViewNumber ();
    ViewSettings& vs = m_glWidget.GetViewSettings (viewNumber);
    float size = m_glWidget.GetDeformationSizeInitialRatio (viewNumber) * 
	vs.GetDeformationSize ();
    float lineWidth = vs.GetDeformationLineWidth ();
    if (GetFocusContext (body) == FOCUS)
	glColor (m_glWidget.GetHighlightColor (viewNumber, HighlightNumber::H0));
    else
	glColor (QColor::fromRgbF (
		     0, 0, 0, this->m_glWidget.GetContextAlpha ()));

    G3D::Matrix3 rotation = MatrixFromColumns (
	body->GetDeformationEigenVector (0),
	body->GetDeformationEigenVector (1),
	body->GetDeformationEigenVector (2));
    G3D::CoordinateFrame cf (rotation, body->GetCenter ());
    glPushMatrix ();
    glMultMatrix (cf);
    drawEllipsis2D (body->GetDeformationEigenValue (0), 
		    body->GetDeformationEigenValue (1), size, lineWidth);
    glPopMatrix ();
}

// DisplayBodyVelocity
// ======================================================================

DisplayBodyVelocity::DisplayBodyVelocity (
    const GLWidget& widget, ViewNumber::Enum viewNumber, 
    const Foam& foam,
    const BodySelector& bodySelector,
    bool useZPos, double zPos):
    
    DisplayBodyBase<> (
	widget, foam, bodySelector,
	SetterTextureCoordinate(widget, viewNumber), useZPos, zPos)
{}

G3D::Vector2 clamp (G3D::Vector2 v, float maxLength, bool* clamped)
{
    float length = v.length ();
    if (length > maxLength)
    {
	*clamped = true;
	return v * (maxLength / length);
    }
    else
    {
	*clamped = false;
	return v;
    }
}

void DisplayBodyVelocity::operator () (boost::shared_ptr<Body> body)
{
    if (body->IsConstraint ())
	return;
    ViewNumber::Enum viewNumber = m_propertySetter.GetViewNumber ();
    ViewSettings& vs = m_glWidget.GetViewSettings (viewNumber);
    bool clamped = false;
    G3D::Vector2 displayVelocity;
    G3D::Vector2 velocity = body->GetVelocity ().xy (); 
    if (vs.GetVelocityAverage ().IsSameSize ())
    {
	clamped = true;
	displayVelocity = velocity;
	displayVelocity *= 
	    m_glWidget.GetBubbleSize (viewNumber) / displayVelocity.length ();
    }
    else
    {
	float size = m_glWidget.GetVelocitySizeInitialRatio (viewNumber) * 
	    vs.GetVelocityClampingRatio ();
	displayVelocity = clamp (
	    velocity * size, 
	    m_glWidget.GetBubbleSize (viewNumber), &clamped);
    }
    if (GetFocusContext (body) == FOCUS)
    {
	double value = velocity.length ();
	float texCoord = vs.GetOverlayBarModel ()->TexCoord (value);
	glTexCoord1f (texCoord); 
	glColor (
	    m_glWidget.GetHighlightColor (viewNumber, HighlightNumber::H0));
    }
    else
	glColor (QColor::fromRgbF (0, 0, 0, m_glWidget.GetContextAlpha ()));
    DisplaySegmentArrow (
	body->GetCenter ().xy () - displayVelocity / 2, displayVelocity, 
	vs.GetVelocityLineWidth (),
	m_glWidget.GetOnePixelInObjectSpace (), 
	clamped && vs.GetVelocityAverage ().IsClampingShown ());
}

// DisplayBodyCenter
// ======================================================================

DisplayBodyCenter::DisplayBodyCenter (
    const GLWidget& widget, const Foam& foam,
    const BodySelector& bodySelector,
    bool useZPos, double zPos):
    
    DisplayBodyBase<> (
	widget, foam, bodySelector,
	SetterTextureCoordinate(widget, ViewNumber::VIEW0), useZPos, zPos)
{}


void DisplayBodyCenter::operator () (boost::shared_ptr<Body> b)
{
    if (GetFocusContext (b) == FOCUS)
    {
	G3D::Vector3 v = b->GetCenter ();
	v = (m_useZPos ? G3D::Vector3 (v.xy (), m_zPos) : v);
	glBegin(GL_POINTS);
	::glVertex(v);
	glEnd ();
    }
}

// DisplayBody
// ======================================================================

template<typename displayFace, typename PropertySetter>
DisplayBody<displayFace, PropertySetter>::
DisplayBody (
    const GLWidget& widget, const Foam& foam,
    const BodySelector& bodySelector,
    typename DisplayElement::ContextType contextDisplay, 
    ViewNumber::Enum view, bool useZPos, double zPos) :

    DisplayBodyBase<PropertySetter> (
	widget, foam, bodySelector, PropertySetter (widget, view), useZPos, zPos),
    m_contextDisplay (contextDisplay)
{
}

template<typename displayFace, typename PropertySetter>
DisplayBody<displayFace, PropertySetter>::
DisplayBody (
    const GLWidget& widget, const Foam& foam,
    const BodySelector& bodySelector,
    PropertySetter setter,
    typename DisplayElement::ContextType contextDisplay,
    bool useZPos, double zPos) :

    DisplayBodyBase<PropertySetter> (
	widget, foam, bodySelector, setter, useZPos, zPos),
    m_contextDisplay (contextDisplay)
{
}


template<typename displayFace, typename PropertySetter>
void DisplayBody<displayFace, PropertySetter>::
operator () (boost::shared_ptr<Body> b)
{
    ViewSettings& vs = this->m_glWidget.GetViewSettings (
	this->m_propertySetter.GetViewNumber ());
    DisplayElement::FocusContext bodyFc = 
	DisplayBodyBase<PropertySetter>::GetFocusContext (b);
    if (bodyFc == DisplayElement::CONTEXT &&
	(m_contextDisplay == DisplayElement::INVISIBLE_CONTEXT ||
	 (m_contextDisplay == DisplayElement::USER_DEFINED_CONTEXT && 
	  ! vs.IsSelectionContextShown ()))
	)
	return;
    vector<boost::shared_ptr<OrientedFace> > v = b->GetOrientedFaces ();
    for_each (
	v.begin (), v.end (),
	displayFace (
	    this->m_glWidget, this->m_foam, 
	    this->m_propertySetter, bodyFc,
	    this->m_useZPos, this->m_zPos));
}


// DisplayCenterPath
// ======================================================================

template<typename PropertySetter, typename DisplaySegment>
DisplayCenterPath<PropertySetter, DisplaySegment>::
DisplayCenterPath (
    const GLWidget& widget, const Foam& foam,
    ViewNumber::Enum view,
    const BodySelector& bodySelector,
    bool useTimeDisplacement,
    double timeDisplacement,
    boost::shared_ptr<ofstream> output) :

    DisplayBodyBase<PropertySetter> (
	widget, foam, bodySelector, PropertySetter (widget, view),
	 useTimeDisplacement, timeDisplacement),
     m_displaySegment (this->m_glWidget.GetQuadricObject (),
		       this->m_glWidget.IsCenterPathLineUsed () ?
		       this->m_glWidget.GetEdgeWidth () :
		       this->m_glWidget.GetEdgeRadius ()),
     m_output (output),
     m_index (0)
{
}


template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
operator () (size_t bodyId)
{
    m_focusTextureSegments.resize (0);
    m_focusColorSegments.resize (0);
    m_contextSegments.resize (0);
    const BodyAlongTime& bat = this->m_glWidget.GetBodyAlongTime (bodyId);
    StripIterator it = bat.GetStripIterator (
	this->m_glWidget.GetSimulation ());
    it.ForEachSegment (
	boost::bind (&DisplayCenterPath::valueStep, this, _1, _2, _3, _4));
    displaySegments ();
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
valueStep (
    const StripIteratorPoint& beforeBegin,
    const StripIteratorPoint& begin,
    const StripIteratorPoint& end,
    const StripIteratorPoint& afterEnd)
{
    static_cast<void>(beforeBegin);
    static_cast<void>(afterEnd);
    G3D::Vector3 pointBegin = getPoint (begin);
    G3D::Vector3 pointEnd = getPoint (end);
    G3D::Vector3 middle = (pointBegin + pointEnd) / 2;
    ViewSettings& vs = this->m_glWidget.GetViewSettings (
	this->m_propertySetter.GetViewNumber ());
    halfValueStep (
	begin,
	Segment (
	    beforeBegin.IsEmpty () ? SegmentPerpendicularEnd::BEGIN_END :
	    SegmentPerpendicularEnd::END,
	    getPoint (beforeBegin), pointBegin, middle, G3D::Vector3 (),
	    vs.IsContextDisplayBody (begin.m_body->GetId ())));
    halfValueStep (
	end,
	Segment (
	    afterEnd.IsEmpty () ? SegmentPerpendicularEnd::BEGIN_END :
	    SegmentPerpendicularEnd::BEGIN,
	    G3D::Vector3 (), middle, pointEnd, getPoint (afterEnd),
	    vs.IsContextDisplayBody (end.m_body->GetId ())));
}


 template<typename PropertySetter, typename DisplaySegment>
 void DisplayCenterPath<PropertySetter, DisplaySegment>::
 halfValueStep (const StripIteratorPoint& p, const Segment& segment)
 {
     bool focus = this->IsFocus (p.m_body);
     if (focus)
     {
	 BodyProperty::Enum property = BodyProperty::FromSizeT (
	     this->m_propertySetter.GetBodyOrFaceProperty ());
	 bool deduced;
	 bool exists = p.m_body->ExistsPropertyValue (property, &deduced);
	 if (exists && 
	     (! deduced || 
	      (deduced && this->m_glWidget.IsMissingPropertyShown (property))))
	     storeFocusSegment (
		 p.m_body->GetPropertyValue (property), segment);
	else
	    storeFocusSegment (
		this->m_glWidget.GetHighlightColor (
		    this->m_propertySetter.GetViewNumber (),
		    HighlightNumber::H0), segment);
    }
    else
	storeContextSegment (
	    this->m_glWidget.GetCenterPathContextColor (), segment);
}


template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
displaySegments ()
{
     ViewSettings& vs = this->m_glWidget.GetViewSettings (
	 this->m_propertySetter.GetViewNumber ());
    for_each (
	m_focusTextureSegments.begin (), m_focusTextureSegments.end (),
	boost::bind (&DisplayCenterPath<PropertySetter, DisplaySegment>::
		     displayFocusTextureSegment, this, _1));
    if (vs.IsSelectionContextShown () && 
	! vs.IsCenterPathHidden () && m_contextSegments.size () > 0)
    {
	glDisable (GL_TEXTURE_1D);
	DisplayBodyBase<>::BeginContext ();
	for_each (
	    m_contextSegments.begin (), m_contextSegments.end (),
	    boost::bind (&DisplayCenterPath<PropertySetter, DisplaySegment>::
			 displayContextSegment, this, _1));
	DisplayBodyBase<>::EndContext ();
	glEnable (GL_TEXTURE_1D);
    }
    if (m_focusColorSegments.size () > 0)
    {
	glDisable (GL_TEXTURE_1D);
	for_each (
	    m_focusColorSegments.begin (), m_focusColorSegments.end (),
	    boost::bind (&DisplayCenterPath<PropertySetter, DisplaySegment>::
			 displayFocusColorSegment, this, _1));
	glEnable (GL_TEXTURE_1D);
    }
}

template<typename PropertySetter, typename DisplaySegment>
G3D::Vector3 DisplayCenterPath<PropertySetter, DisplaySegment>::
getPoint (StripIteratorPoint p) const
{
    if (this->m_useZPos)
    {
	G3D::Vector3 v = p.m_point;
	v.z = p.m_timeStep * this->m_zPos;
	return v;
    }
    else
	return p.m_point;
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
storeFocusSegment (double value, const Segment& segment)
{
    double textureCoordinate = 
	this->m_glWidget.GetViewSettings (
	    this->m_propertySetter.GetViewNumber ()).GetColorBarModel ()
	->TexCoord (value);
    boost::shared_ptr<FocusTextureSegment> fs = 
	boost::make_shared<FocusTextureSegment> (textureCoordinate, segment);
    m_focusTextureSegments.push_back (fs);
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
storeFocusSegment (const QColor& color, const Segment& segment)
{
    boost::shared_ptr<FocusColorSegment> fs = 
	boost::make_shared<FocusColorSegment> (color, segment);
    m_focusColorSegments.push_back (fs);
}



template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
storeContextSegment (
    const QColor& color, const Segment& segment)
{
    boost::shared_ptr<ContextSegment> cs = boost::make_shared<ContextSegment> (
	color, segment);
    m_contextSegments.push_back (cs);
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
displayContextSegment (
    const boost::shared_ptr<ContextSegment>& contextSegment)
{
    glColor (contextSegment->m_color);
    m_displaySegment (*contextSegment);
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
displayFocusTextureSegment (
    const boost::shared_ptr<FocusTextureSegment>& segment)
{
    glColor (Qt::white);
    glTexCoord1f (segment->m_textureCoordinate);
    m_displaySegment (*segment);
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayCenterPath<PropertySetter, DisplaySegment>::
displayFocusColorSegment (const boost::shared_ptr<FocusColorSegment>& segment)
{
    glColor (segment->m_color);
    m_displaySegment (*segment);
}



// Template instantiations
// ======================================================================

// DisplayBodyBase
// ======================================================================

template class DisplayBodyBase<SetterVertexAttribute>;
template class DisplayBodyBase<SetterTextureCoordinate>;
template class DisplayBodyBase<SetterDeformation>;
template class DisplayBodyBase<SetterNop>;
template class DisplayBodyBase<SetterVelocity>;


// DisplayBody
// ======================================================================

template class DisplayBody<
    DisplayFaceHighlightColor<HighlightNumber::H0,
	DisplayFaceEdges<
	    DisplayEdgePropertyColor<DisplayElement::DONT_DISPLAY_TESSELLATION_EDGES> >,
	SetterTextureCoordinate>, SetterTextureCoordinate>;
template class DisplayBody<
    DisplayFaceHighlightColor<HighlightNumber::H0,
	DisplayFaceEdges<
	    DisplayEdgePropertyColor<DisplayElement::DISPLAY_TESSELLATION_EDGES> >,
	SetterTextureCoordinate>, SetterTextureCoordinate>;
template class DisplayBody<
    DisplayFaceHighlightColor<HighlightNumber::H0,
	DisplayFaceEdges<DisplayEdgeTorusClipped>,
	SetterTextureCoordinate>,
    SetterTextureCoordinate>;
template class DisplayBody<
    DisplayFaceHighlightColor<HighlightNumber::H0, DisplayFaceLineStrip, SetterTextureCoordinate>, SetterTextureCoordinate>;


template class DisplayBody<
    DisplayFaceHighlightColor<HighlightNumber::H0, DisplayFaceTriangleFan, SetterTextureCoordinate>, SetterTextureCoordinate>;
template class DisplayBody<
    DisplayFaceBodyPropertyColor<SetterTextureCoordinate>, SetterTextureCoordinate>;
template class DisplayBody<DisplayFaceBodyPropertyColor<SetterVelocity>, SetterVelocity>;
template class DisplayBody<
    DisplayFaceBodyPropertyColor<SetterVertexAttribute>,
    SetterVertexAttribute>;
template class DisplayBody<DisplayFaceBodyPropertyColor<SetterNop>, SetterNop>;

template class DisplayBody<DisplayFaceHighlightColor<HighlightNumber::H1, DisplayFaceLineStrip, SetterTextureCoordinate>, SetterTextureCoordinate>;

template class DisplayBody<DisplayFaceBodyPropertyColor<SetterDeformation>, SetterDeformation>;

// DisplayCenterPath
// ======================================================================

template class DisplayCenterPath<SetterTextureCoordinate, DisplaySegmentTube>;
template class DisplayCenterPath<SetterTextureCoordinate, DisplaySegmentQuadric>;
template class DisplayCenterPath<SetterTextureCoordinate, DisplaySegment>;
