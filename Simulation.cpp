/**
 * @file Simulation.cpp
 * @author Dan R. Lipsa
 *
 * Method implementation for a list of Foam objects
 */

#include "Body.h"
#include "Debug.h"
#include "DebugStream.h"
#include "Foam.h"
#include "Simulation.h"
#include "ParsingData.h"
#include "Utils.h"

// Private Functions and classes
// ======================================================================
class FoamParamMethodList
{
public:
    FoamParamMethodList (Simulation::FoamParamMethod* foamMethods, size_t n):
	m_foamMethods (foamMethods), m_n (n)
    {
    }

    void operator () (boost::shared_ptr<Foam> foam)
    {
	try
	{
	    for (size_t i = 0; i < m_n; ++i)
		(m_foamMethods[i]) (foam.get ());
	}
	catch (const exception& e)
	{
	    cdbg << "Exception: " << e.what () << endl;
	}
    }

private:
    Simulation::FoamParamMethod* m_foamMethods;
    size_t m_n;
};


/**
 * Functor used to parse a DMP file
 */
class ParseDMP : public unary_function< QString, boost::shared_ptr<Foam> >
{
public:
    /**
     * Constructor
     * @param data Where to store the data parsed from the DMP files
     * @param dir directory where all DMP files are
     */
    ParseDMP (
	QString dir, 
	const DmpObjectInfo& dmpObjectInfo, 
	const vector<ForceNames>& forcesNames,
	bool useOriginal, FoamProperties* foamParameters, 
	Foam::ParametersOperation parametersOperation,
	bool debugParsing = false, bool debugScanning = false) : 

        m_dir (qPrintable(dir)), 
	m_dmpObjectInfo (dmpObjectInfo), 
	m_useOriginal (useOriginal),
	m_foamParameters (foamParameters), 
	m_parametersOperation (parametersOperation),
	m_debugParsing (debugParsing),
	m_debugScanning (debugScanning)
    {
	m_forcesNames.resize (forcesNames.size ());
	copy (forcesNames.begin (), forcesNames.end (), m_forcesNames.begin ());
    }
    
    /**
     * Parses one file
     * @param dmpFile name of the DMP file to be parsed.
     */
    boost::shared_ptr<Foam> operator () (QString dmpFile)
    {
	boost::shared_ptr<Foam> foam;
	string file;
	int result;
	file = qPrintable (dmpFile);
	ostringstream ostr;
	ostr << "Parsing " << file << " ..." << endl;
	cdbg << ostr.str ();
	foam.reset (
	    new Foam (m_useOriginal, m_dmpObjectInfo,
		      m_forcesNames, *m_foamParameters, 
		      m_parametersOperation));
	foam->GetParsingData ().SetDebugParsing (m_debugParsing);
	foam->GetParsingData ().SetDebugScanning (m_debugScanning);	    
	string fullPath = m_dir + '/' + file;
	result = foam->GetParsingData ().Parse (fullPath, foam.get ());
	if (result != 0)
	    ThrowException ("Error parsing ", fullPath);
	return foam;
    }
private:
    /**
     * Directory that stores the DMP files.
     */
    const string m_dir;
    const DmpObjectInfo& m_dmpObjectInfo;
    vector<ForceNames> m_forcesNames;
    const bool m_useOriginal;
    FoamProperties* m_foamParameters;
    Foam::ParametersOperation m_parametersOperation;
    const bool m_debugParsing;
    const bool m_debugScanning;
};



// Static Members
// ======================================================================
const vector<G3D::Vector3> Simulation::NO_T1S;



// Members
// ======================================================================
Simulation::Simulation () :
    m_histogram (
        BodyProperty::COUNT, HistogramStatistics (HISTOGRAM_INTERVALS)),
    m_t1sTimeStepShift (0),
    m_useOriginal (false),
    m_rotation2D (0),
    m_reflectAxis (numeric_limits<size_t>::max ())
{
}

void Simulation::CalculateBoundingBox ()
{
    G3D::Vector3 low, high;
    CalculateAggregate<Foams, Foams::iterator, 
        BBObjectLessThanAlongLow<Foam> > () (min_element, m_foams, &low);
    CalculateAggregate<Foams, Foams::iterator, 
        BBObjectLessThanAlongHigh<Foam> > () (max_element, m_foams, &high);
    m_boundingBox.set (low, high);

    CalculateAggregate<Foams, Foams::iterator, 
        BBObjectLessThanAlongLowTorus<Foam> > () (min_element, m_foams, &low);
    CalculateAggregate<Foams, Foams::iterator, 
        BBObjectLessThanAlongHighTorus<Foam> > () (max_element, m_foams, &high);
    m_boundingBoxTorus.set (low, high);
}

void Simulation::calculateBodyWraps ()
{
    if (m_foams.size () <= 1)
	return;
    BodiesAlongTime::BodyMap bodyMap = GetBodiesAlongTime ().GetBodyMap ();
    for_each (
	bodyMap.begin (), bodyMap.end (),
	boost::bind (
	    &BodyAlongTime::CalculateBodyWraps,
	    boost::bind (&BodiesAlongTime::BodyMap::value_type::second, 
			 _1), *this));
}


void Simulation::Preprocess ()
{
    cdbg << "Preprocess temporal foam data ..." << endl;
    fixConstraintPoints ();
    boost::array<FoamParamMethod, 7> methods = {{
	    boost::bind (&Foam::CreateConstraintBody, _1, 
			 GetDmpObjectInfo ().m_constraintIndex),
	    boost::bind (&Foam::SetForcesAllObjects, _1),
	    boost::bind (&Foam::ReleaseParsingData, _1),
	    boost::bind (&Foam::CalculateBoundingBox, _1),
	    boost::bind (&Foam::CalculatePerimeterOverArea, _1),
	    boost::bind (&Foam::CalculateBodyNeighbors, _1),
	    boost::bind (&Foam::CalculateBodyDeformationTensor, _1)
    }};
    MapPerFoam (&methods[0], methods.size ());
    CalculateBoundingBox ();
    CacheBodiesAlongTime ();
    calculateBodyWraps ();
    calculateVelocity ();
    FoamParamMethod f = boost::bind (&Foam::CalculateMinMaxStatistics, _1);
    MapPerFoam (&f, 1);
    if (IsPressureAdjusted ())
        adjustPressureAlignMedians ();
    else
    {
	cdbg << "Show ORIGINAL pressure values." << endl;
    }
    calculateStatistics ();
}


void Simulation::fixConstraintPoints ()
{
    Foams& foams = GetFoams ();
    Foam* prevFoam = 0;
    for (size_t i = 0; i < foams.size (); ++i)
    {
	Foam* foam = foams[i].get ();
	foam->FixConstraintPoints (prevFoam);
	prevFoam = foam;
    }
}


void Simulation::MapPerFoam (FoamParamMethod* foamMethods, size_t n)
{
    FoamParamMethodList fl (foamMethods, n);
    QtConcurrent::blockingMap (m_foams.begin (), m_foams.end (), fl);
}

size_t foamsIndex (
    Simulation::Foams::iterator current, Simulation::Foams::iterator begin)
{
    return (current - begin) / sizeof *begin;
}


double GetPressureBody0 (const boost::shared_ptr<Foam>& foam)
{
    return foam->GetBody (0).GetPropertyValue (BodyProperty::PRESSURE);
}

void Simulation::adjustPressureSubtractReference ()
{
    QtConcurrent::blockingMap (
	m_foams.begin (), m_foams.end (),
	boost::bind (&Foam::AdjustPressure, _1, 
		     boost::bind (GetPressureBody0, _1)));
}

void Simulation::adjustPressureAlignMedians ()
{
    // adjust pressure in every time step,
    // by substracting the minimum pressure of a bubble in that time step.
    // this makes every pressure greater than 0.
    QtConcurrent::blockingMap (
	m_foams.begin (), m_foams.end (),
	boost::bind (&Foam::AdjustPressure, _1, 
		     boost::bind (&Foam::GetMin, _1, BodyProperty::PRESSURE)));

    // adjust the pressure by aligning the medians in every time step,
    // with the max median for all time steps
    vector<double> medians = 
	QtConcurrent::blockingMapped< vector<double> > (
	    m_foams.begin (), m_foams.end (),
	    boost::bind (&Foam::CalculateMedian, _1, BodyProperty::PRESSURE));
    double maxMedian = *max_element (medians.begin (), medians.end ());
    for (size_t i = 0; i < m_foams.size (); ++i)
	m_foams[i]->AdjustPressure (medians[i] - maxMedian);
}

void Simulation::calculateStatistics ()
{
    for (size_t i = BodyProperty::PROPERTY_BEGIN; 
	 i < BodyProperty::COUNT; ++i)
    {
	MinMaxStatistics minMaxStat;
	// statistics for all time-steps
	BodyProperty::Enum property = BodyProperty::FromSizeT (i);
	forAllBodiesAccumulateProperty (&minMaxStat, property);
	m_histogram[property] (acc::min (minMaxStat));
	m_histogram[property] (acc::max (minMaxStat));
	forAllBodiesAccumulateProperty (&m_histogram[property], property);

	// statistics per time-step
	double min = acc::min(m_histogram[property]);
	double max = acc::max(m_histogram[property]);
	QtConcurrent::blockingMap (
	    m_foams.begin (), m_foams.end (),
	    boost::bind (&Foam::CalculateHistogramStatistics, _1,
			 BodyProperty::FromSizeT (i), min, max));
    }
    {
	MinMaxStatistics minMaxStat;
	forAllBodiesAccumulate (&minMaxStat, 
				getBodyDeformationEigenValue<0> ());
	m_maxDeformationEigenValue = acc::max (minMaxStat);
    }
}

template <typename Accumulator, typename GetBodyProperty>
void Simulation::forAllBodiesAccumulate (
    Accumulator* acc, GetBodyProperty getBodyProperty)
{
    QtConcurrent::blockingMap (
	m_foams.begin (), m_foams.end (),
	boost::bind (&Foam::Accumulate<Accumulator, GetBodyProperty>, 
		     _1, acc, getBodyProperty));
}


template <typename Accumulator>
void Simulation::forAllBodiesAccumulateProperty (
    Accumulator* acc, BodyProperty::Enum property)
{
    QtConcurrent::blockingMap (
	m_foams.begin (), m_foams.end (),
	boost::bind (&Foam::AccumulateProperty<Accumulator>, 
		     _1, acc, property));
}


void Simulation::calculateVelocityBody (
    pair< size_t, boost::shared_ptr<BodyAlongTime> > p)
{
    const BodyAlongTime& bat = *p.second;
    StripIterator stripIt = bat.GetStripIterator (*this);
    stripIt.ForEachSegment (boost::bind (&Simulation::storeVelocity,
					 this, _1, _2, _3, _4));    
}

void Simulation::calculateVelocity ()
{
    BodiesAlongTime::BodyMap& map = GetBodiesAlongTime ().GetBodyMap ();
    QtConcurrent::blockingMap (
	map.begin (), map.end (), 
	boost::bind (&Simulation::calculateVelocityBody, this, _1));
}

void Simulation::storeVelocity (
    const StripIteratorPoint& beforeBegin,
    const StripIteratorPoint& begin,
    const StripIteratorPoint& end,
    const StripIteratorPoint& afterEnd)
{
    (void)beforeBegin;
    (void)afterEnd;
    G3D::Vector3 velocity = end.m_point - begin.m_point;
    begin.m_body->SetVelocity (velocity);
    if (end.m_location == StripPointLocation::END)
	end.m_body->SetVelocity (velocity);
}

void Simulation::CacheBodiesAlongTime ()
{
    Foam::Bodies& bodies = m_foams[0]->GetBodies ();
    for_each (bodies.begin (), bodies.end (), 
	      boost::bind (&BodiesAlongTime::Allocate,
			   &m_bodiesAlongTime, _1, m_foams.size ()));
    for (size_t timeStep = 0; timeStep < m_foams.size (); timeStep++)
    {
	Foam::Bodies& bodies = m_foams[timeStep]->GetBodies ();
	BOOST_FOREACH (boost::shared_ptr<Body>  body, bodies)
	    m_bodiesAlongTime.Cache (body, timeStep);
    }
    for_each (bodies.begin (), bodies.end (),
	      boost::bind (&BodiesAlongTime::Resize, &m_bodiesAlongTime, _1));
}

bool Simulation::Is2D () const
{
    return GetFoam (0).Is2D ();
}

bool Simulation::IsTorus () const
{
    return GetFoam (0).IsTorus ();
}

const Body& Simulation::GetBody (size_t bodyId, size_t timeStep) const
{
    const BodyAlongTime& bat = GetBodiesAlongTime ().GetBodyAlongTime (bodyId);
    return *bat.GetBody (timeStep);
}

string Simulation::ToString () const
{
    ostringstream ostr;
    ostr << "Simulation: " << endl;
    ostr << m_boundingBox << endl;
    ostream_iterator< boost::shared_ptr<const Foam> > output (ostr, "\n");
    copy (m_foams.begin (), m_foams.end (), output);
    ostr << m_bodiesAlongTime;
    return ostr.str ();
}

string Simulation::ToHtml () const
{
    const Foam& firstFoam = GetFoam (0);
    size_t timeSteps = GetTimeSteps ();
    const Foam& lastFoam = GetFoam (timeSteps - 1);
    size_t bodies[2] = 
	{
	    firstFoam.GetBodies ().size (), lastFoam.GetBodies ().size ()
	};
    size_t faces[2] = 
	{
	    firstFoam.GetFaceSet ().size (), lastFoam.GetFaceSet ().size ()
	};
    size_t edges[2] =
	{
	    firstFoam.GetEdgeSet ().size (), lastFoam.GetEdgeSet ().size ()
	};
    size_t vertices[2] = 
	{
	    firstFoam.GetVertexSet ().size (), lastFoam.GetVertexSet ().size ()
	};

    ostringstream ostr;
    ostr << 
	"<table border>"
	"<tr><th>Bodies</th><td>" 
	 << bodies[0] << "</td><td>" << bodies[1] << "</td></tr>"
	"<tr><th>Faces</th><td>" 
	 << faces[0] << "</td><td>" << faces[1] << "</td></tr>"
	"<tr><th>Edges</th><td>" 
	 << edges[0] << "</td><td>" << edges[1] << "</td></tr>"
	"<tr><th>Vertices</th><td>" 
	 << vertices[0] << "</td><td>" << vertices[1] << "</td></tr>"
	"<tr><th>Time step</th><td>" 
	 << 0 << "</td><td>" << (timeSteps - 1) << "</td></tr>"
	"</table>" << endl;
    return ostr.str ();
}


void Simulation::SetTimeSteps (size_t timeSteps)
{
    m_foams.resize (timeSteps);
}


void Simulation::GetTimeStepSelection (
    BodyProperty::Enum property,
    const vector<QwtDoubleInterval>& valueIntervals,
    vector<bool>* timeStepSelection) const
{
    timeStepSelection->resize (GetTimeSteps ());
    fill (timeStepSelection->begin (), timeStepSelection->end (), false);
    BOOST_FOREACH (QwtDoubleInterval valueInterval, valueIntervals)
	GetTimeStepSelection (property, valueInterval, timeStepSelection);
}

void Simulation::GetTimeStepSelection (
    BodyProperty::Enum property,
    const QwtDoubleInterval& valueInterval,
    vector<bool>* timeStepSelection) const
{
    for (size_t timeStep = 0; timeStep < GetTimeSteps (); ++timeStep)
    {
	const Foam& foam = GetFoam (timeStep);
	if (valueInterval.intersects (foam.GetRange (property))
	    && foam.ExistsBodyWithValueIn (property, valueInterval))
	    (*timeStepSelection)[timeStep] = true;
    }
}

bool Simulation::IsQuadratic () const
{
    return m_foams[0]->IsQuadratic ();
}

size_t Simulation::GetMaxCountPerBinIndividual (
    BodyProperty::Enum property) const
{
    size_t size = GetTimeSteps ();
    size_t max = 0;
    for (size_t i = 0; i < size; ++i)
	max = std::max (
	    max, GetFoam (i).GetHistogram (property).GetMaxCountPerBin ());
    return max;
}

bool Simulation::T1sAvailable () const
{
    BOOST_FOREACH (const vector<G3D::Vector3> timeStepT1s, m_t1s)
	if (timeStepT1s.size () != 0)
	    return true;
    return false;
}

size_t Simulation::GetT1sTimeSteps () const
{
    return m_t1s.size ();
}

size_t Simulation::GetT1sSize () const
{
    size_t size = 0;
    BOOST_FOREACH (const vector<G3D::Vector3>& t1sTimeStep, m_t1s)
    {
	size += t1sTimeStep.size ();
    }
    return size;
}


void Simulation::ParseT1s (
    const string& fileName, size_t ticksForTimeStep, bool shiftT1sLower)
{
    cdbg << "Parsing T1s file...";
    const size_t LINE_LENGTH = 256;
    ifstream in;
    in.open (fileName.c_str ());
    if (! in)
	ThrowException ("Cannot open \"" + fileName + "\"");
    in.exceptions (ios::failbit | ios::badbit);
    size_t timeStep;
    float x, y;
    while (! in.eof ())
    {
	if (in.peek () == '#')
	{
	    in.ignore (LINE_LENGTH, '\n');
	    continue;
	}
	in >> timeStep >> x >> y >> ws;
	timeStep /= ticksForTimeStep;
	// in the file: first time step is 1 and T1s occur before timeStep
	// in memory: first time step is 0 and T1s occur after timeStep
	timeStep -= 1;
	if (timeStep >= m_t1s.size ())
	    m_t1s.resize (timeStep + 1);
	m_t1s[timeStep].push_back (G3D::Vector3 (x, y, Foam::Z_COORDINATE_2D));
    }
    SetT1sTimeStepShift (shiftT1sLower ? 1 : 0);
    cdbg << "last T1s'  timestep: " << timeStep << endl;
}

const vector<G3D::Vector3>& Simulation::GetT1s (size_t timeStep) const
{
    int t = int(timeStep) + m_t1sTimeStepShift;
    if (t < 0 || size_t (t) >= m_t1s.size ())
	return NO_T1S;
    else
	return m_t1s[t];
}

void Simulation::ParseDMPs (
    const vector<string>& fileNames,
    bool useOriginal,
    const DmpObjectInfo& dmpObjectInfo,
    const vector<ForceNames>& forcesNames,
    bool debugParsing, bool debugScanning)
{
    QDir dir;
    QStringList files;
    m_useOriginal = useOriginal;
    m_dmpObjectInfo = dmpObjectInfo;
    m_forcesNames.resize (forcesNames.size ());
    copy (forcesNames.begin (), forcesNames.end (), m_forcesNames.begin ());
    QFileInfo fileInfo (fileNames[0].c_str ());
    dir = fileInfo.absoluteDir ();
    if (! dir.exists ())
	ThrowException ("Directory does not exist: \"" + 
			dir.path ().toStdString () + "\"");
    BOOST_FOREACH (const string& fn, fileNames)
	files << QFileInfo(fn.c_str ()).fileName ();
    if (files.size () == 0)
	ThrowException (
	    "No files match: \"" + 
	    fileInfo.filePath ().toStdString () + "\"");

    SetTimeSteps (files.size ());
    // FoamProperties are shared between all Foams
    GetFoams ()[0] = ParseDMP (
	dir.absolutePath (), GetDmpObjectInfo (),
	GetForcesNames (), OriginalUsed (), GetFoamProperties (),
	Foam::SET_FOAM_PROPERTIES,
	debugParsing, debugScanning) (*files.begin ());
    QList< boost::shared_ptr<Foam> > foams = QtConcurrent::blockingMapped 
	< QList < boost::shared_ptr<Foam> > > (
	    files.begin () + 1, files.end (),
	    ParseDMP (	
		dir.absolutePath (), GetDmpObjectInfo (),
		GetForcesNames (), OriginalUsed (), GetFoamProperties (),
		Foam::TEST_FOAM_PROPERTIES, debugParsing, debugScanning));
    if (count_if (foams.constBegin (), foams.constEnd (),
		  bl::_1 != boost::shared_ptr<Foam>()) != foams.size ())
	ThrowException ("Could not process all files\n");
    copy (foams.constBegin (), foams.constEnd (), GetFoams ().begin () + 1);
}


string SimulationGroup::ToString () const
{
    ostringstream ostr;
    BOOST_FOREACH (const Simulation& fat, m_simulation)
    {
	ostr << fat << endl;
    }
    return ostr.str ();
}

