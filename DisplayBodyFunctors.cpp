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
#include "Debug.h"
#include "DisplayBodyFunctors.h"
#include "DisplayFaceFunctors.h"
#include "DisplayEdgeFunctors.h"
#include "Simulation.h"
#include "Settings.h"
#include "OpenGLUtils.h"
#include "ViewSettings.h"
#include "VectorAverage.h"

// Private Classes/Functions
// ======================================================================


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

// returns multiplier for velocity
float clampMax (float length, float maxLength, bool* clamped)
{
    if (length > maxLength)
    {
	*clamped = true;
	return maxLength;
    }
    else
    {
	*clamped = false;
	return length;
    }
}



// DisplayBodyBase
// ======================================================================

template <typename PropertySetter>
DisplayBodyBase<PropertySetter>::
DisplayBodyBase (const Settings& settings, const BodySelector& bodySelector,
		 PropertySetter propertySetter, bool useZPos, double zPos) :

    DisplayElementProperty<PropertySetter> (
        settings, propertySetter, useZPos, zPos),
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

template <typename PropertySetter>
void DisplayBodyBase<PropertySetter>::
operator () (boost::shared_ptr<Body> b)
{
    bool focus = m_bodySelector (b);
    if (focus)
	display (b);
    else
    {
	BeginContext ();
	display (b);
	EndContext ();
    }
}



// DisplayBodyDeformation
// ======================================================================

DisplayBodyDeformation::DisplayBodyDeformation (
    const Settings& settings, ViewNumber::Enum viewNumber, bool is2D,
    const BodySelector& bodySelector, float deformationSizeInitialRatio,
    bool useZPos, double zPos):
    
    DisplayBodyBase<> (
	settings, bodySelector,
	SetterTextureCoordinate(settings, viewNumber, is2D), useZPos, zPos),
    m_deformationSizeInitialRatio (deformationSizeInitialRatio)
{}

void DisplayBodyDeformation::display (boost::shared_ptr<Body> body)
{
    if (body->IsObject ())
	return;
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = m_settings.GetViewSettings (viewNumber);
    float size = m_deformationSizeInitialRatio * vs.GetDeformationSize ();
    float lineWidth = vs.GetDeformationLineWidth ();
    if (GetFocusContext (body) == FOCUS)
	glColor (m_settings.GetHighlightColor (viewNumber, HighlightNumber::H0));
    else
	glColor (QColor::fromRgbF (0, 0, 0, vs.GetContextAlpha ()));

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
    const Settings& settings, ViewNumber::Enum viewNumber, bool is2D,
    const BodySelector& bodySelector, float bubbleSize, 
    float velocitySizeInitialRatio, float onePixelInObjectSpace,
    GLUquadricObj* quadric,
    bool sameSize, bool clampingShown,
    bool useZPos, double zPos):
    
    DisplayBodyBase<> (
	settings, bodySelector,
	SetterTextureCoordinate(settings, viewNumber, is2D), useZPos, zPos),
    m_bubbleDiameter (bubbleSize),
    m_onePixelInObjectSpace (onePixelInObjectSpace),
    m_velocitySizeInitialRatio (velocitySizeInitialRatio),
    m_sameSize (sameSize),
    m_clampingShown (clampingShown),
    m_is2D (is2D),
    m_quadric (quadric)
{}

void DisplayBodyVelocity::display (boost::shared_ptr<Body> body)
{
    if (body->IsObject ())
	return;
    ViewNumber::Enum viewNumber = GetViewNumber ();
    ViewSettings& vs = m_settings.GetViewSettings (viewNumber);
    bool clamped = false;
    G3D::Vector3 displayVelocity;
    G3D::Vector3 velocity = body->GetVelocity (); 
    float velocityLength = velocity.length ();
    if (m_sameSize)
    {
        // set all velocity magnitudes to be bubbleDiameter
	clamped = true;
	displayVelocity = velocity;
	displayVelocity *= m_bubbleDiameter / displayVelocity.length ();
    }
    else
    {
        // size = bubbleDiameter / (clampInterval.max () - interval.min ())
	float size = 
            m_velocitySizeInitialRatio * vs.GetVelocityInverseClampMaxRatio ();
	displayVelocity = velocity * 
            (clampMax (velocityLength * size, m_bubbleDiameter, &clamped) / 
             velocityLength);
    }
    if (GetFocusContext (body) == FOCUS)
    {
	float texCoord = vs.GetOverlayBarModel ()->TexCoord (velocityLength);
	glTexCoord1f (texCoord); 
	glColor (
	    m_settings.GetHighlightColor (viewNumber, HighlightNumber::H0));
    }
    else
	glColor (QColor::fromRgbF (0, 0, 0, vs.GetContextAlpha ()));
    if (m_is2D)
        DisplaySegmentArrow2D (
            body->GetCenter ().xy () - displayVelocity.xy () / 2, 
            displayVelocity.xy (), vs.GetVelocityLineWidth (), 
            m_onePixelInObjectSpace, clamped && m_clampingShown);
    else
    {
        DisplayVtkArrow (m_quadric);
    }
}

// DisplayBodyCenter
// ======================================================================

DisplayBodyCenter::DisplayBodyCenter (
    const Settings& settings, const BodySelector& bodySelector,
    bool useZPos, double zPos):
    
    DisplayBodyBase<> (
	settings, bodySelector,
        // the setter is not used
	SetterTextureCoordinate(settings, ViewNumber::VIEW0, true), 
        useZPos, zPos)
{}


void DisplayBodyCenter::display (boost::shared_ptr<Body> b)
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
    const Settings& settings, bool is2D, const BodySelector& bodySelector,
    typename DisplayElement::ContextType contextDisplay, 
    ViewNumber::Enum viewNumber, bool useZPos, double zPos) :

    DisplayBodyBase<PropertySetter> (
	settings, bodySelector, 
        PropertySetter (settings, viewNumber, is2D), useZPos, zPos),
    m_contextDisplay (contextDisplay)
{
}

template<typename displayFace, typename PropertySetter>
DisplayBody<displayFace, PropertySetter>::
DisplayBody (
    const Settings& settings, 
    const BodySelector& bodySelector,
    PropertySetter setter,
    typename DisplayElement::ContextType contextDisplay,
    bool useZPos, double zPos) :

    DisplayBodyBase<PropertySetter> (
        settings, bodySelector, setter, useZPos, zPos),
    m_contextDisplay (contextDisplay)
{
}


template<typename displayFace, typename PropertySetter>
void DisplayBody<displayFace, PropertySetter>::
display (boost::shared_ptr<Body> b)
{
    ViewSettings& vs = this->m_settings.GetViewSettings (this->GetViewNumber ());
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
	    this->m_settings, this->m_propertySetter, bodyFc,
	    this->m_useZPos, this->m_zPos));
}


// DisplayBubblePaths
// ======================================================================

template<typename PropertySetter, typename DisplaySegment>
DisplayBubblePaths<PropertySetter, DisplaySegment>::
DisplayBubblePaths (
    const Settings& settings, ViewNumber::Enum view, bool is2D,
    const BodySelector& bodySelector, GLUquadricObj* quadric,
    const Simulation& simulation, size_t timeBegin, size_t timeEnd,
    bool useTimeDisplacement,
    double timeDisplacement) :

    DisplayBodyBase<PropertySetter> (
	settings, bodySelector, 
        PropertySetter (settings, view, is2D),
	useTimeDisplacement, timeDisplacement),
    m_displaySegment (quadric,
		      this->m_settings.IsBubblePathsLineUsed () ?
		      this->m_settings.GetEdgeWidth () :
		      this->m_settings.GetEdgeRadius ()),
    m_simulation (simulation),
    m_timeBegin (timeBegin),
    m_timeEnd (timeEnd)
{
}


template<typename PropertySetter, typename DisplaySegment>
void DisplayBubblePaths<PropertySetter, DisplaySegment>::
operator () (size_t bodyId)
{
    m_focusTextureSegments.resize (0);
    m_focusColorSegments.resize (0);
    m_contextSegments.resize (0);
    const BodyAlongTime& bat = m_simulation.GetBodyAlongTime (bodyId);
    StripIterator it = bat.GetStripIterator (m_simulation);
    it.ForEachSegment (
	boost::bind (&DisplayBubblePaths::valueStep, this, _1, _2, _3, _4),
        m_timeBegin, m_timeEnd);
    displaySegments ();
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayBubblePaths<PropertySetter, DisplaySegment>::
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
    ViewSettings& vs = this->m_settings.GetViewSettings (
	this->GetViewNumber ());
    halfValueStep (
	begin,
	Segment (
	    beforeBegin.IsEmpty () ? SegmentPerpendicularEnd::BEGIN_END_SEGMENT :
	    SegmentPerpendicularEnd::END_SEGMENT,
	    getPoint (beforeBegin), pointBegin, middle, G3D::Vector3 (),
	    vs.IsContextDisplayBody (begin.m_body->GetId ())));
    halfValueStep (
	end,
	Segment (
	    afterEnd.IsEmpty () ? SegmentPerpendicularEnd::BEGIN_END_SEGMENT :
	    SegmentPerpendicularEnd::BEGIN_SEGMENT,
	    G3D::Vector3 (), middle, pointEnd, getPoint (afterEnd),
	    vs.IsContextDisplayBody (end.m_body->GetId ())));
}


 template<typename PropertySetter, typename DisplaySegment>
 void DisplayBubblePaths<PropertySetter, DisplaySegment>::
 halfValueStep (const StripIteratorPoint& p, const Segment& segment)
 {
    ViewNumber::Enum viewNumber = this->GetViewNumber ();
    ViewSettings& vs = this->m_settings.GetViewSettings (viewNumber);
     bool focus = this->IsFocus (p.m_body);
     if (focus)
     {
	 BodyScalar::Enum property = BodyScalar::FromSizeT (
	     this->m_propertySetter.GetBodyOrFaceScalar ());
	 bool deduced;
	 bool exists = p.m_body->HasScalarValue (property, &deduced);
	 if (exists && 
	     (! deduced || 
	      (deduced && this->m_settings.IsMissingPropertyShown (property))))
	     storeFocusSegment (
		 p.m_body->GetScalarValue (
                     property, 
                     this->m_propertySetter.Is2D ()), segment);
	else
	    storeFocusSegment (
		this->m_settings.GetHighlightColor (
		    this->GetViewNumber (),
		    HighlightNumber::H0), segment);
    }
    else
	storeContextSegment (vs.GetBubblePathsContextColor (), segment);
}


template<typename PropertySetter, typename DisplaySegment>
void DisplayBubblePaths<PropertySetter, DisplaySegment>::
displaySegments ()
{
    ViewSettings& vs = this->m_settings.GetViewSettings (
        this->GetViewNumber ());
    
    // focus segments
    size_t focusSegmentsSize = 
        m_focusTextureSegments.size () + m_focusColorSegments.size();
    for_each (
	m_focusTextureSegments.begin (), m_focusTextureSegments.end (),
	boost::bind (&DisplayBubblePaths<PropertySetter, DisplaySegment>::
		     displayFocusTextureSegment, this, _1));
    if (m_focusColorSegments.size () > 0)
    {
	glDisable (GL_TEXTURE_1D);
	for_each (
	    m_focusColorSegments.begin (), m_focusColorSegments.end (),
	    boost::bind (&DisplayBubblePaths<PropertySetter, DisplaySegment>::
			 displayFocusColorSegment, this, _1));
	glEnable (GL_TEXTURE_1D);
    }

    // context segments
    if (m_contextSegments.size () > 0 &&
        ((focusSegmentsSize == 0 && vs.IsSelectionContextShown ()) ||
         (focusSegmentsSize != 0 && ! vs.IsPartialPathHidden ())))
    {
	glDisable (GL_TEXTURE_1D);
	DisplayBodyBase<>::BeginContext ();
	for_each (
	    m_contextSegments.begin (), m_contextSegments.end (),
	    boost::bind (&DisplayBubblePaths<PropertySetter, DisplaySegment>::
			 displayContextSegment, this, _1));
	DisplayBodyBase<>::EndContext ();
	glEnable (GL_TEXTURE_1D);
    }
}

template<typename PropertySetter, typename DisplaySegment>
G3D::Vector3 DisplayBubblePaths<PropertySetter, DisplaySegment>::
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
void DisplayBubblePaths<PropertySetter, DisplaySegment>::
storeFocusSegment (double value, const Segment& segment)
{
    double textureCoordinate = 
	this->m_settings.GetViewSettings (
	    this->GetViewNumber ()).GetColorBarModel ()
	->TexCoord (value);
    boost::shared_ptr<FocusTextureSegment> fs = 
	boost::make_shared<FocusTextureSegment> (textureCoordinate, segment);
    m_focusTextureSegments.push_back (fs);
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayBubblePaths<PropertySetter, DisplaySegment>::
storeFocusSegment (const QColor& color, const Segment& segment)
{
    boost::shared_ptr<FocusColorSegment> fs = 
	boost::make_shared<FocusColorSegment> (color, segment);
    m_focusColorSegments.push_back (fs);
}



template<typename PropertySetter, typename DisplaySegment>
void DisplayBubblePaths<PropertySetter, DisplaySegment>::
storeContextSegment (
    const QColor& color, const Segment& segment)
{
    boost::shared_ptr<ContextSegment> cs = boost::make_shared<ContextSegment> (
	color, segment);
    m_contextSegments.push_back (cs);
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayBubblePaths<PropertySetter, DisplaySegment>::
displayContextSegment (
    const boost::shared_ptr<ContextSegment>& contextSegment)
{
    glColor (contextSegment->m_color);
    m_displaySegment (*contextSegment);
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayBubblePaths<PropertySetter, DisplaySegment>::
displayFocusTextureSegment (
    const boost::shared_ptr<FocusTextureSegment>& segment)
{
    glColor (Qt::white);
    glTexCoord1f (segment->m_textureCoordinate);
    m_displaySegment (*segment);
}

template<typename PropertySetter, typename DisplaySegment>
void DisplayBubblePaths<PropertySetter, DisplaySegment>::
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
    DisplayFaceHighlightColor<(HighlightNumber::Enum)0, 
                              DisplayFaceEdges<
                                  DisplayEdgePropertyColor<(DisplayElement::TessellationEdgesDisplay)0> >, SetterTextureCoordinate>, SetterTextureCoordinate>;

template class DisplayBody<
    DisplayFaceHighlightColor<(HighlightNumber::Enum)0, 
                              DisplayFaceEdges<
                                  DisplayEdgePropertyColor<(DisplayElement::TessellationEdgesDisplay)1> >, SetterTextureCoordinate>, SetterTextureCoordinate>;


template class DisplayBody<
    DisplayFaceHighlightColor<(HighlightNumber::Enum)0, 
                              DisplayFaceEdges<
                                  DisplayEdge>, SetterTextureCoordinate>, SetterTextureCoordinate>;


template class DisplayBody<
    DisplayFaceHighlightColor<HighlightNumber::H0, DisplayFaceLineStrip, SetterTextureCoordinate>, SetterTextureCoordinate>;


template class DisplayBody<
    DisplayFaceHighlightColor<HighlightNumber::H0, DisplayFaceTriangleFan, SetterTextureCoordinate>, SetterTextureCoordinate>;
template class DisplayBody<
    DisplayFaceBodyScalarColor<SetterTextureCoordinate>, SetterTextureCoordinate>;
template class DisplayBody<DisplayFaceBodyScalarColor<SetterVelocity>, SetterVelocity>;
template class DisplayBody<
    DisplayFaceBodyScalarColor<SetterVertexAttribute>,
    SetterVertexAttribute>;
template class DisplayBody<DisplayFaceBodyScalarColor<SetterNop>, SetterNop>;

template class DisplayBody<DisplayFaceHighlightColor<HighlightNumber::H1, DisplayFaceLineStrip, SetterTextureCoordinate>, SetterTextureCoordinate>;

template class DisplayBody<DisplayFaceBodyScalarColor<SetterDeformation>, SetterDeformation>;

// DisplayBubblePaths
// ======================================================================

template class DisplayBubblePaths<SetterTextureCoordinate, DisplaySegmentTube>;
template class DisplayBubblePaths<SetterTextureCoordinate, 
                                  DisplaySegmentQuadric>;
template class DisplayBubblePaths<SetterTextureCoordinate, DisplaySegmentLine>;
