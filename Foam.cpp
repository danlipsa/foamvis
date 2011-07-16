/**
 * @file   Foam.cpp
 * @author Dan R. Lipsa
 *
 * Implementation of the Foam object
 */
#include "Attribute.h"
#include "AttributeInfo.h"
#include "AttributeCreator.h"
#include "Body.h"
#include "ConstraintEdge.h"
#include "Debug.h"
#include "DebugStream.h"
#include "Foam.h"
#include "Edge.h"
#include "Face.h"
#include "Utils.h"
#include "OrientedEdge.h"
#include "OrientedFace.h"
#include "ParsingData.h"
#include "ProcessBodyTorus.h"
#include "Vertex.h"

// Private Functions
// ======================================================================

template<typename Element>
void copyStandalone (const vector< boost::shared_ptr<Element> >& source,
		     vector< boost::shared_ptr<Element> >* destination)
{
    BOOST_FOREACH (boost::shared_ptr<Element> e, source)
    {
	if (e->IsStandalone ())
	    destination->push_back (e);
    }
}


G3D::Vector3* Vector3Address (G3D::Vector3& v)
{
    return &v;
}

/**
 * Move elements in a vector toward the begining of the vector so that we
 * eliminate holes.
 * @param v vector of elements
 */
template <typename E>
void compact (vector< boost::shared_ptr<E> >& v)
{
    size_t step = 0;
    for (size_t i = 0; i < v.size (); i++)
    {
        if (v[i] == 0)
            step++;
        else if (step != 0)
            v[i - step] = v[i];
    }
    size_t resize = v.size () - step;
    v.resize (resize);
}

// Methods
// ======================================================================

Foam::Foam (bool useOriginal, 
	    const ConstraintRotationNames& constraintRotationNames,
	    const vector<ForceNames>& forcesNames) :
    m_viewMatrix (new G3D::Matrix4 (G3D::Matrix4::identity ())),
    m_parsingData (new ParsingData (
		       useOriginal, constraintRotationNames, forcesNames)),
    m_spaceDimension (3),
    m_quadratic (false),
    m_histogram (
	BodyProperty::PROPERTY_END, HistogramStatistics (HISTOGRAM_INTERVALS))
{
    m_parsingData->SetVariable ("pi", M_PI);
    AddDefaultVertexAttributes ();
    AddDefaultEdgeAttributes ();
    AddDefaultFaceAttributes ();
    AddDefaultBodyAttributes ();
}

template <typename Accumulator>
void Foam::Accumulate (Accumulator* acc, BodyProperty::Enum property) const
{
    BOOST_FOREACH (const boost::shared_ptr<Body>& body, GetBodies ())
    {
	if (body->ExistsPropertyValue (property))
	    (*acc) (body->GetPropertyValue (property));
    }
}

void Foam::AddDefaultBodyAttributes ()
{
    using EvolverData::parser;
    AttributesInfo* infos = &m_attributesInfo[DefineAttribute::BODY];

    // the order of the attributes should match the order in
    // BodyProperty::Enum
    boost::shared_ptr<AttributeCreator> ac;

    ac.reset (new RealAttributeCreator ());
    size_t index = infos->AddAttributeInfoLoad (
        ParsingDriver::GetKeywordString(
	    parser::token::LAGRANGE_MULTIPLIER), ac);
    RuntimeAssert (index == BodyAttributeIndex::PRESSURE,
		   "Pressure body attribute index is ", index);

    ac.reset (new RealAttributeCreator());
    index = infos->AddAttributeInfoLoad (
        ParsingDriver::GetKeywordString(parser::token::VOLUME), ac);
    RuntimeAssert (index == BodyAttributeIndex::TARGET_VOLUME,
		   "Target volume body attribute index is ", index);

    ac.reset (new RealAttributeCreator());
    index = infos->AddAttributeInfoLoad (
        ParsingDriver::GetKeywordString(parser::token::ACTUAL), ac);
    RuntimeAssert (index == BodyAttributeIndex::ACTUAL_VOLUME,
		   "Actual volume body attribute index is ", index);

    ac.reset (new IntegerAttributeCreator ());
    index = infos->AddAttributeInfoLoad (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL), ac);
    RuntimeAssert (index == BodyAttributeIndex::ORIGINAL,
		   "Volume original attribute index is ", index);


    ac.reset (new RealAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::VOLCONST), ac);

    ac.reset (new RealAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ACTUAL_VOLUME), ac);
}


void Foam::AddDefaultFaceAttributes ()
{
    using EvolverData::parser;
    AttributesInfo* infos = &m_attributesInfo[DefineAttribute::FACE];
    boost::shared_ptr<AttributeCreator> ac;

    ac.reset (new ColorAttributeCreator ());
    size_t index = infos->AddAttributeInfoLoad (
        ParsingDriver::GetKeywordString(parser::token::COLOR), ac);
    RuntimeAssert (
	index == FaceAttributeIndex::COLOR,
	"Color face attribute index is ", FaceAttributeIndex::COLOR);

    ac.reset (new RealAttributeCreator ());
    index = infos->AddAttributeInfoLoad (
        ParsingDriver::GetKeywordString(parser::token::AREA), ac);
    RuntimeAssert (
	index == FaceAttributeIndex::AREA,
	"Face area attribute index is ", FaceAttributeIndex::AREA);

    ac.reset (new IntegerVectorAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::CONSTRAINTS), ac);

    ac.reset (new IntegerAttributeCreator ());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL), ac);

    ac.reset (new RealAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::DENSITY), ac);
}


void Foam::AddDefaultEdgeAttributes ()
{
    using EvolverData::parser;
    AttributesInfo* infos = &m_attributesInfo[DefineAttribute::EDGE];
    boost::shared_ptr<AttributeCreator> ac;

    ac.reset (new ColorAttributeCreator ());
    size_t index = infos->AddAttributeInfoLoad (
	ParsingDriver::GetKeywordString(parser::token::COLOR), ac);
    RuntimeAssert (
	index == EdgeAttributeIndex::COLOR, 
	"Color edge attribute index is ", index);

    ac.reset (new IntegerVectorAttributeCreator());
    index = infos->AddAttributeInfoLoad (
        ParsingDriver::GetKeywordString(parser::token::CONSTRAINTS), ac);
    RuntimeAssert (
	index == EdgeAttributeIndex::CONSTRAINTS, 
	"Constraints edge attribute index is ", index);

    ac.reset (new IntegerAttributeCreator ());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL), ac);

    ac.reset (new RealAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::DENSITY), ac);
}


void Foam::AddDefaultVertexAttributes ()
{
    using EvolverData::parser;
    AttributesInfo* infos = &m_attributesInfo[DefineAttribute::VERTEX];
    boost::shared_ptr<AttributeCreator> ac;

    ac.reset (new IntegerVectorAttributeCreator());
    size_t index = infos->AddAttributeInfoLoad (
        ParsingDriver::GetKeywordString(parser::token::CONSTRAINTS), ac);
    RuntimeAssert (index == VertexAttributeIndex::CONSTRAINTS, 
		   "Constraints vertex attribute index is ", index);

    ac.reset (new IntegerAttributeCreator());
    infos->AddAttributeInfo (
        ParsingDriver::GetKeywordString(parser::token::ORIGINAL), ac);
}

void Foam::SetBody (size_t i, vector<int>& faces,
                    vector<NameSemanticValue*>& attributes, bool useOriginal)
{
    resizeAllowIndex (&m_bodies, i);
    boost::shared_ptr<Body> body = boost::make_shared<Body> (
	faces, GetParsingData ().GetFaces (), i);
    if (&attributes != 0)
        body->StoreAttributes (attributes,
			       m_attributesInfo[DefineAttribute::BODY]);
    if (useOriginal)
	if (body->HasAttribute (BodyAttributeIndex::ORIGINAL))
	{
	    size_t newId = body->GetAttribute<
	    IntegerAttribute, IntegerAttribute::value_type> (
		BodyAttributeIndex::ORIGINAL) - 1;
	    body->SetId (newId);
	}
    m_bodies[i] = body;
}

void Foam::compact (void)
{
    ::compact (GetParsingData ().GetVertices ());
    ::compact (GetParsingData ().GetEdges ());
    ::compact (GetParsingData ().GetFaces ());
    ::compact (m_bodies);
}


void Foam::ReleaseParsingData ()
{
    m_parsingData.reset ();
}

void Foam::updateAdjacent ()
{
    for_each (m_bodies.begin (), m_bodies.end (),
	      boost::bind(&Body::UpdateAdjacentBody, _1, _1));

    ParsingData::Faces faces = GetParsingData ().GetFaces ();
    for_each (faces.begin (), faces.end (),
	      boost::bind (&Face::UpdateAdjacentFaceStandalone, _1, _1));

    ParsingData::Edges edges = GetParsingData ().GetEdges ();
    for_each (edges.begin (), edges.end (),
	      boost::bind (&Edge::UpdateAdjacentEdge, _1, _1));
}

void Foam::CalculateBoundingBox ()
{
    G3D::Vector3 low, high;
    for_each (m_bodies.begin (), m_bodies.end (),
	      boost::bind (&Body::CalculateBoundingBox, _1));

    // using the BB for bodies to calculate BB for the foam does not
    // work when there are no bodies
    VertexSet vertexSet = GetVertexSet ();
    CalculateAggregate <VertexSet, VertexSet::iterator, VertexLessThanAlong>() (
	min_element, vertexSet, &low);
    CalculateAggregate <VertexSet, VertexSet::iterator, VertexLessThanAlong>()(
	max_element, vertexSet, &high);
    if (IsTorus ())
	calculateBoundingBoxForTorus (&low, &high);
    m_boundingBox.set(low, high);
}

void Foam::CalculatePerimeterOverArea ()
{
    for_each (m_bodies.begin (), m_bodies.end (),
	      boost::bind (&Body::CalculatePerimeterOverSqrtArea, _1));
}

void Foam::calculateBoundingBoxForTorus (G3D::Vector3* low, G3D::Vector3* high)
{
    using boost::array;
    using G3D::Vector3;
    Vector3 origin = Vector3::zero ();
    Vector3 first = m_originalDomain[0];
    Vector3 second = m_originalDomain[1];
    Vector3 sum = first + second;
    Vector3 third = m_originalDomain[2];
    array<Vector3, 10> additionalVertices =
    {{
	    *low,
	    origin, first, sum, second,
	    origin + third, first + third, sum + third, second + third,
	    *high
    }};
    vector<Vector3*> v(additionalVertices.size ());
    transform (additionalVertices.begin (), additionalVertices.end (),
	       v.begin (), Vector3Address);
    CalculateAggregate<vector<Vector3*>, vector<Vector3*>::iterator,
	VertexLessThanAlong>() (min_element, v, low);
    CalculateAggregate<vector<Vector3*>, vector<Vector3*>::iterator,
	VertexLessThanAlong>() (max_element, v, high);
}

void Foam::calculateBodiesCenters ()
{
    for_each (m_bodies.begin (), m_bodies.end (),
	      boost::bind(&Body::CalculateCenter, _1,
			  Is2D (), IsQuadratic ()));
}

void Foam::calculateTorusClipped ()
{
    const OOBox& periods = GetOriginalDomain ();
    EdgeSet edgeSet;
    GetEdgeSet (&edgeSet);
    BOOST_FOREACH (boost::shared_ptr<Edge> e, edgeSet)
    {
	if (e->IsClipped ())
	    e->CalculateTorusClipped (periods);
    }
}

void Foam::unwrap (VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet)
{
    BOOST_FOREACH (boost::shared_ptr<Vertex> v, GetParsingData ().GetVertices ())
    {
	vertexSet->insert (v);
    }
    BOOST_FOREACH (boost::shared_ptr<Edge> e, GetParsingData ().GetEdges ())
    {
	unwrap (e, vertexSet);
	edgeSet->insert (e);
    }
    BOOST_FOREACH (boost::shared_ptr<Face> f, GetParsingData ().GetFaces ())
    {
	unwrap (f, vertexSet, edgeSet);
	f->SetNormal ();
	faceSet->insert (f);
    }
    BOOST_FOREACH (boost::shared_ptr<Body> b, GetBodies ())
    {
	unwrap (b, vertexSet, edgeSet, faceSet);
    }
}

void Foam::unwrap (boost::shared_ptr<Edge> edge, VertexSet* vertexSet) const
{
    if (edge->GetEndTranslation () != Vector3int16Zero)
	edge->SetEnd (
	    edge->GetEnd ().GetDuplicate (
		GetOriginalDomain (), edge->GetEndTranslation (), vertexSet));
}

void Foam::unwrap (boost::shared_ptr<Face> face,
		   VertexSet* vertexSet, EdgeSet* edgeSet) const
{
    Face::OrientedEdges& orientedEdges = face->GetOrientedEdges ();
    G3D::Vector3 begin = (*orientedEdges.begin())->GetBeginVector ();
    BOOST_FOREACH (boost::shared_ptr<OrientedEdge> oe, orientedEdges)
    {
	boost::shared_ptr<Edge>  edge = oe->GetEdge ();
	G3D::Vector3 edgeBegin =
	    (oe->IsReversed ()) ? edge->GetTranslatedBegin (begin) : begin;
	oe->SetEdge (
	    edge->GetDuplicate (
		GetOriginalDomain (), edgeBegin, vertexSet, edgeSet));
	begin = oe->GetEndVector ();
    }
    face->CalculateCentroidAndArea ();
}

void Foam::unwrap (
    boost::shared_ptr<Body> body,
    VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet) const
{
    ProcessBodyTorus(*this, body).Unwrap (vertexSet, edgeSet, faceSet);
}




void Foam::copyStandaloneElements ()
{
    copyStandalone (GetParsingData ().GetEdges (), &m_standaloneEdges);
    copyStandalone (GetParsingData ().GetFaces (), &m_standaloneFaces);
}

void Foam::Preprocess ()
{
    VertexSet vertexSet;
    EdgeSet edgeSet;
    FaceSet faceSet;
    const ConstraintRotationNames& constraintRotationNames = 
	GetParsingData ().GetConstraintRotationNames ();
    if (constraintRotationNames.RotationUsed ())
	SetConstraintRotation (constraintRotationNames);
    const vector<ForceNames>& forcesNames = GetParsingData ().GetForcesNames ();
    if (forcesNames.size () > 0)
	SetForces (forcesNames);
    compact ();
    updateAdjacent ();
    copyStandaloneElements ();
    if (IsTorus ())
	unwrap (&vertexSet, &edgeSet, &faceSet);
    else
    {
	GetFaceSet (&faceSet);
	BOOST_FOREACH (const boost::shared_ptr<Face>& f, faceSet)
	    f->SetNormal ();
    }
    calculateBodiesCenters ();
    if (IsTorus ())
    {
	bodiesInsideOriginalDomain (&vertexSet, &edgeSet, &faceSet);
	calculateTorusClipped ();
    }
    sort (m_bodies.begin (), m_bodies.end (), BodyLessThan);
    setMissingPressureZero ();
    setMissingVolume ();
    addConstraintEdges ();
}

void Foam::CalculateBodyNeighbors ()
{
    if (Is2D ())
	for_each (m_bodies.begin (), m_bodies.end (),
		  boost::bind (&Body::CalculateNeighbors2D, _1, 
			       GetOriginalDomain ()));
}

void Foam::CalculateBodyDeformationTensor ()
{

    for_each (m_bodies.begin (), m_bodies.end (),
	      boost::bind (&Body::CalculateDeformationTensor, _1, 
			   GetOriginalDomain ()));
}


void Foam::addConstraintEdges ()
{
    if (! Is2D ())
	return;
    Bodies bodies = GetBodies ();
    for (size_t i = 0; i < bodies.size (); ++i)
    {
	boost::shared_ptr<Body> body = bodies[i];
	Face& face = body->GetFace (0);
	if (! face.IsClosed ())
	{
	    boost::shared_ptr<Vertex> end = 
		face.GetOrientedEdge (0).GetBeginPtr ();
	    boost::shared_ptr<Vertex> begin = 
		face.GetOrientedEdge (face.GetEdgeCount () - 1).GetEndPtr ();
	    //if (body->GetId () == 541)
	    {
		ConstraintEdge* constraintEdge = new ConstraintEdge (
		    &GetParsingData (), begin, end, 
		    &m_constraintPointsToFix, i);
		boost::shared_ptr<Edge> edge (constraintEdge);
		face.AddEdge (edge);
		face.CalculateCentroidAndArea ();
		size_t constraintIndex = constraintEdge->GetConstraintIndex ();
		if ( constraintIndex == GetParsingData ().
		     GetConstraintRotationNames ().m_constraintIndex)
		{
		    resizeAllowIndex (&m_constraintEdges, constraintIndex);
		    if (! m_constraintEdges[constraintIndex])
			m_constraintEdges[constraintIndex].reset (
			    new Edges ());
		    m_constraintEdges[constraintIndex]->push_back (edge);
		}
	    }
	}
    }
}

void Foam::FixConstraintPoints (const Foam* prevFoam)
{
    pair<size_t, size_t> index;
    ConstraintEdge* prevProcessedEdge = 0;
    BOOST_FOREACH (index, m_constraintPointsToFix)
    {
	//cdbg << "process: " << index.first << ", " << index.second << endl;
	bool valid;
	const Face& face = GetBody (index.first).GetFace (0);
	ConstraintEdge& edge = static_cast<ConstraintEdge&> (
	    *face.GetOrientedEdge (face.size () - 1).GetEdge ());
	if (prevFoam == 0)
	    edge.FixPointsConcaveOrConvex ();
	else
	{
	    const Face& prevFace = prevFoam->GetBody (index.first).GetFace (0);
	    ConstraintEdge& prevEdge = static_cast<ConstraintEdge&> (
		*prevFace.GetOrientedEdge (prevFace.size () - 1).GetEdge ());
	    G3D::Vector2 prevPoint = prevEdge.GetPoint (index.second).xy ();
	    G3D::Vector3 newPoint = edge.ComputePointMulti (
		index.second, &valid, prevPoint);
	    if (valid)
		edge.ChoosePoint (index.second, newPoint);
	    if (prevProcessedEdge != &edge)
	    {
		if (prevProcessedEdge != 0)
		    prevProcessedEdge->FixPointsConcaveOrConvex ();
		prevProcessedEdge = &edge;	    
	    }
	}
    }
    if (prevProcessedEdge != 0)
	prevProcessedEdge->FixPointsConcaveOrConvex ();
    m_constraintPointsToFix.clear ();
}



Foam::Bodies::const_iterator
Foam::FindBody (size_t bodyId) const
{
    return lower_bound (m_bodies.begin (), m_bodies.end (), bodyId, 
			BodyLessThanId);
}

void Foam::setMissingVolume ()
{
    if (Is2D ())
    {
	using EvolverData::parser;
	BOOST_FOREACH (const boost::shared_ptr<Body>& body, m_bodies)
	{
	    double area = body->GetOrientedFace (0).GetArea ();
	    if (! body->ExistsPropertyValue (BodyProperty::TARGET_VOLUME))
	    {
		body->SetTargetVolumeDeduced ();
		StoreAttribute (body.get (), BodyProperty::TARGET_VOLUME, area);
	    }
	    /*
	    else
	    {
		cdbg << "area: " << area 
		     << body->GetPropertyValue (BodyProperty::TARGET_VOLUME) 
		     << endl;
	    }
	    */
	    if (! body->ExistsPropertyValue (BodyProperty::ACTUAL_VOLUME))
	    {
		body->SetActualVolumeDeduced ();
		StoreAttribute (body.get (), BodyProperty::ACTUAL_VOLUME, area);
	    }
	}
    }
}

void Foam::setMissingPressureZero ()
{
    using EvolverData::parser;
    BOOST_FOREACH (const boost::shared_ptr<Body>& body, m_bodies)
    {
	if (! body->ExistsPropertyValue (BodyProperty::PRESSURE))
	{
	    StoreAttribute (body.get (), BodyProperty::PRESSURE, 0);
	    body->SetPressureDeduced ();
	}
    }
}

void Foam::AdjustPressure (double adjustment)
{
    BOOST_FOREACH (const boost::shared_ptr<Body>& body, m_bodies)
    {
	if (body->ExistsPropertyValue (BodyProperty::PRESSURE))
	{
	    double newPressure =
		body->GetPropertyValue (BodyProperty::PRESSURE) - adjustment;
	    body->SetPressureValue (newPressure);
	}
    }
    m_min[BodyProperty::PRESSURE] -= adjustment;
    m_max[BodyProperty::PRESSURE] -= adjustment;
}

double Foam::CalculateMedian (BodyProperty::Enum property)
{
    MedianStatistics median;
    BOOST_FOREACH (const boost::shared_ptr<Body>& body, m_bodies)
    {
	if (body->ExistsPropertyValue (property))
	    median (body->GetPropertyValue (property));
    }
    return acc::median (median);
}

bool Foam::IsTorus () const
{
    return GetOriginalDomain ().IsTorusDomain ();
}


void Foam::AddAttributeInfo (
    DefineAttribute::Enum type, const char* name,
    boost::shared_ptr<AttributeCreator> creator)
{
    m_attributesInfo[type].AddAttributeInfo (name, creator);
    m_parsingData->AddAttribute (name);
}

void Foam::bodiesInsideOriginalDomain (
    VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet)
{
    for_each (m_bodies.begin (), m_bodies.end (),
	      boost::bind (&Foam::bodyInsideOriginalDomain, this,
			   _1, vertexSet, edgeSet, faceSet));
}

Foam::Bodies::iterator Foam::BodyInsideOriginalDomainStep (
    Foam::Bodies::iterator begin,
    VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet)
{
    cdbg << "BodyInsideOriginalDomainStep" << endl;
    Bodies::iterator it = begin;
    while (it != m_bodies.end () &&
	   bodyInsideOriginalDomain (*it, vertexSet, edgeSet, faceSet))
	++it;
    if (it == m_bodies.end ())
	return it;
    else
	return ++it;
}


bool Foam::bodyInsideOriginalDomain (
    const boost::shared_ptr<Body>& body,
    VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet)
{
    G3D::Vector3int16 centerLocation =
	GetOriginalDomain ().GetLocation (body->GetCenter ());
    if (centerLocation == Vector3int16Zero)
	return true;
    G3D::Vector3int16 translation = Vector3int16Zero - centerLocation;
    bodyTranslate (body, translation, vertexSet, edgeSet, faceSet);
    return false;
}

void Foam::bodyTranslate (
    const boost::shared_ptr<Body>& body,const G3D::Vector3int16& translate,
    VertexSet* vertexSet, EdgeSet* edgeSet, FaceSet* faceSet)
{
    BOOST_FOREACH (boost::shared_ptr<OrientedFace> of, body->GetOrientedFaces ())
    {
	const Face& original = *of->GetFace ();
	boost::shared_ptr<Face> duplicate = original.GetDuplicate (
	    GetOriginalDomain (), translate, vertexSet, edgeSet, faceSet);
	of->SetFace (duplicate);
    }
    body->CalculateCenter (Is2D (), IsQuadratic ());
}

void Foam::GetVertexSet (VertexSet* vertexSet) const
{
    for_each (GetBodies ().begin (), GetBodies ().end (),
	      boost::bind (&Body::GetVertexSet, _1, vertexSet));
    for_each (GetStandaloneFaces ().begin (), GetStandaloneFaces ().end (),
	      boost::bind (&Face::GetVertexSet, _1, vertexSet));
    for_each (GetStandaloneEdges ().begin (), GetStandaloneEdges ().end (),
	      boost::bind (&Edge::GetVertexSet, _1, vertexSet));
}

void Foam::GetEdgeSet (EdgeSet* edgeSet) const
{
    for_each (GetBodies ().begin (), GetBodies ().end (),
	      boost::bind (&Body::GetEdgeSet, _1, edgeSet));
    BOOST_FOREACH (boost::shared_ptr<Edge> edge, GetStandaloneEdges ())
	edgeSet->insert (edge);
    for_each (GetStandaloneFaces ().begin (), GetStandaloneFaces ().end (),
	      boost::bind (&Face::GetEdgeSet, _1, edgeSet));
}

void Foam::GetFaceSet (FaceSet* faceSet) const
{
    const Bodies& bodies = GetBodies ();
    for_each (bodies.begin (), bodies.end (),
	      boost::bind (&Body::GetFaceSet, _1, faceSet));
    BOOST_FOREACH (boost::shared_ptr<Face> face, GetStandaloneFaces ())
	faceSet->insert (face);
}

const AttributesInfo& Foam::GetAttributesInfo (
    DefineAttribute::Enum attributeType) const
{
    return m_attributesInfo[attributeType];
}

boost::shared_ptr<Edge> Foam::GetStandardEdge () const
{
    const Face* f;
    if (m_bodies.size () == 0)
	f = m_standaloneFaces[0].get ();
    else
	f = &GetBody (0).GetFace (0);
    return f->GetOrientedEdge (0).GetEdge ();
}

void Foam::SetViewMatrix (
    double r1c1, double r1c2, double r1c3, double r1c4,
    double r2c1, double r2c2, double r2c3, double r2c4,
    double r3c1, double r3c2, double r3c3, double r3c4,
    double r4c1, double r4c2, double r4c3, double r4c4)
{
    m_viewMatrix.reset (new G3D::Matrix4 (
	r1c1, r1c2, r1c3, r1c4,
	r2c1, r2c2, r2c3, r2c4,
	r3c1, r3c2, r3c3, r3c4,
	r4c1, r4c2, r4c3, r4c4));
}

void Foam::SetPeriods (const G3D::Vector3& x, const G3D::Vector3& y)
{
    using G3D::Vector3;
    Vector3 third = x.cross (y).unit ();
    double thirdLength = min (x.length (), y.length ());
    SetPeriods (x, y, thirdLength * third);
}

void Foam::CalculateMinMaxStatistics ()
{
    for (size_t i = BodyProperty::PROPERTY_BEGIN;
	 i < BodyProperty::PROPERTY_END; ++i)
    {
	// statistics for all time-steps
	BodyProperty::Enum property = BodyProperty::FromSizeT (i);
	calculateMinMaxStatistics (property);
    }
}

void Foam::calculateMinMaxStatistics (BodyProperty::Enum property)
{
    MinMaxStatistics minMax;
    BOOST_FOREACH (const boost::shared_ptr<Body>& body, m_bodies)
    {
	if (body->ExistsPropertyValue (property))
	    minMax (body->GetPropertyValue (property));
    }
    m_min[property] = acc::min (minMax);
    m_max[property] = acc::max (minMax);
}

void Foam::CalculateHistogramStatistics (BodyProperty::Enum property,
					 double min, double max)
{
    m_histogram[property](min);
    m_histogram[property](max);
    BOOST_FOREACH (const boost::shared_ptr<Body>& body, m_bodies)
    {
	if (body->ExistsPropertyValue (property))
	    m_histogram[property] (body->GetPropertyValue (property));
    }
}

bool Foam::ExistsBodyWithValueIn (
    BodyProperty::Enum property, const QwtDoubleInterval& interval) const
{
    Foam::Bodies::const_iterator it = find_if
	(m_bodies.begin (), m_bodies.end (),
	 boost::bind (&QwtDoubleInterval::contains, interval, 
		      boost::bind (&Body::GetPropertyValue, _1, property)));
    return it != m_bodies.end ();
}

void Foam::SetConstraintRotation (const ConstraintRotationNames& names)
{
    m_constraintRotation.m_center.x =  
	GetParsingData ().GetVariableValue (names.m_xName);
    m_constraintRotation.m_center.y =  
	GetParsingData ().GetVariableValue (names.m_yName);
    m_constraintRotation.m_angle =  
	GetParsingData ().GetVariableValue (names.m_angleName);
}


void Foam::SetForces (const vector<ForceNames>& forcesNames)
{
    m_forces.resize (forcesNames.size ());
    for (size_t i = 0; i < forcesNames.size (); ++i)
	setForce (forcesNames[i], &m_forces[i]);
}

void Foam::setForce (const ForceNames& names, Force* forces)
{
    forces->m_body = *FindBody (names.m_bodyId);
    forces->m_networkForce[0] = GetParsingData ().GetVariableValue (
	names.m_networkForceName[0]);
    forces->m_networkForce[1] = GetParsingData ().GetVariableValue (
	names.m_networkForceName[1]);
    forces->m_pressureForce[0] = GetParsingData ().GetVariableValue (
	names.m_pressureForceName[0]);
    forces->m_pressureForce[1] = GetParsingData ().GetVariableValue (
	names.m_pressureForceName[1]);
}

void Foam::StoreAttribute (
    Body* body, BodyProperty::Enum bp, double value)
{
    body->StoreAttribute (Body::GetAttributeKeywordString(bp), value,
			  m_attributesInfo[DefineAttribute::BODY]);
}

void Foam::SortConstraintEdges (size_t constraintIndex)
{
    Edges& edges = *m_constraintEdges[constraintIndex];
    for (size_t i = 0; i < edges.size () - 1; ++i)
    {
	for (size_t j = i + 1; j < edges.size (); ++j)
	{
	    if (edges[i]->GetEnd ().GetId () != edges[j]->GetBegin ().GetId ())
		continue;
	    else
		swap (edges[i+1], edges[j]);
	}
	RuntimeAssert (edges[i]->GetEnd ().GetId () == 
		       edges[i + 1]->GetBegin ().GetId (), 
		       "Next vector not found");
    }
}

void Foam::CreateConstraintBody (size_t constraint)
{
    if (constraint == INVALID_INDEX)
	return;
    SortConstraintEdges (constraint);
    boost::shared_ptr<Face> face (new Face (GetConstraintEdges (constraint)));
    VertexSet vertexSet = GetVertexSet ();
    EdgeSet edgeSet = GetEdgeSet ();
    unwrap (face, &vertexSet, &edgeSet);
    boost::shared_ptr<Body> body (
	new Body (face, (*(m_bodies.end () - 1))->GetId () + 1));
    body->UpdateAdjacentBody (body);
    body->CalculateCenter (Is2D (), IsQuadratic ());
    m_bodies.push_back (body);
}

// Static and Friends Methods
// ======================================================================
ostream& operator<< (ostream& ostr, const Foam& d)
{
    ostr << "Foam:\n";
    ostr << "AABox:\n";
    ostr << d.m_boundingBox << endl;
    {
	ostr << "view matrix:\n";
	ostr << d.m_viewMatrix;
    }
    if (d.IsTorus ())
    {
	ostr << "torus periods:\n";
	ostr << d.m_originalDomain;
    }

    ostr << "bodies:\n";
    ostream_iterator<boost::shared_ptr<Body> > bOutput (ostr, "\n");
    copy (d.m_bodies.begin (), d.m_bodies.end (), bOutput);
    return ostr;
}

// Template instantiations
// ======================================================================
/// @cond
template void Foam::Accumulate<HistogramStatistics> (
    HistogramStatistics* acc, BodyProperty::Enum property) const;
template void Foam::Accumulate<MinMaxStatistics> (
    MinMaxStatistics* acc, BodyProperty::Enum property) const;
/// @endcond
