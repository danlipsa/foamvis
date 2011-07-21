/**
 * @file FoamAlongTime.cpp
 * @author Dan R. Lipsa
 *
 * Method implementation for a list of Foam objects
 */

#include "Body.h"
#include "Debug.h"
#include "DebugStream.h"
#include "Foam.h"
#include "FoamAlongTime.h"
#include "ParsingData.h"
#include "Utils.h"

// Private Functions and classes
// ======================================================================
class FoamParamMethodList
{
public:
    FoamParamMethodList (FoamAlongTime::FoamParamMethod* foamMethods, size_t n):
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
    FoamAlongTime::FoamParamMethod* m_foamMethods;
    size_t m_n;
};


QString lastName (const QString& path)
{
    int slashPos = path.lastIndexOf ('/');
    QString ret = path;
    return ret.remove (0, slashPos + 1);
}

/**
 * Functor used to parse a DMP file
 */
class parseFile : public unary_function< QString, boost::shared_ptr<Foam> >
{
public:
    /**
     * Constructor
     * @param data Where to store the data parsed from the DMP files
     * @param dir directory where all DMP files are
     */
    parseFile (
	QString dir, 
	const ConstraintRotationNames& constraintRotationNames, 
	const vector<ForceNames>& forcesNames,
	bool useOriginal, 
	bool debugParsing = false, bool debugScanning = false) : 

        m_dir (qPrintable(dir)), 
	m_constraintRotationNames (constraintRotationNames), 
	m_useOriginal (useOriginal),
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
	try
	{
	    int result;
	    file = qPrintable (dmpFile);
	    ostringstream ostr;
	    ostr << "Parsing " << file << " ..." << endl;
	    cdbg << ostr.str ();
	    foam.reset (
		new Foam (m_useOriginal, 
			  m_constraintRotationNames,
			  m_forcesNames));
	    foam->GetParsingData ().SetDebugParsing (m_debugParsing);
	    foam->GetParsingData ().SetDebugScanning (m_debugScanning);
	    string fullPath = m_dir + '/' + file;
	    result = foam->GetParsingData ().Parse (fullPath, foam.get ());
	    if (result != 0)
		ThrowException ("Error parsing ", fullPath);
	}
	catch (const exception& e)
	{
	    cdbg << "Exception for " << file << ": "
		 << e.what () << endl;
	    foam.reset ();
	}
	return foam;
    }
private:
    /**
     * Directory that stores the DMP files.
     */
    const string m_dir;
    const ConstraintRotationNames& m_constraintRotationNames;
    vector<ForceNames> m_forcesNames;
    const bool m_useOriginal;
    const bool m_debugParsing;
    const bool m_debugScanning;
};



// Static Members
// ======================================================================
const vector<G3D::Vector3> FoamAlongTime::NO_T1S;



// Members
// ======================================================================
FoamAlongTime::FoamAlongTime () :
    m_histogram (
        BodyProperty::PROPERTY_END, HistogramStatistics (HISTOGRAM_INTERVALS)),
    m_t1sTimestepShift (0),
    m_useOriginal (false)
{
}

void FoamAlongTime::CalculateBoundingBox ()
{
    G3D::Vector3 low, high;
    CalculateAggregate<Foams, Foams::iterator, 
        BBObjectLessThanAlongLow<Foam> > () (min_element, m_foams, &low);
    CalculateAggregate<Foams, Foams::iterator, 
        BBObjectLessThanAlongHigh<Foam> > () (max_element, m_foams, &high);
    m_boundingBox.set (low, high);
}

void FoamAlongTime::calculateBodyWraps ()
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

void FoamAlongTime::Preprocess ()
{
    cdbg << "Preprocess temporal foam data ..." << endl;
    fixConstraintPoints ();
    boost::array<FoamParamMethod, 6> methods = {{
	    boost::bind (&Foam::CreateConstraintBody, _1, 
			 GetConstraintRotationNames ().m_constraintIndex),
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


void FoamAlongTime::fixConstraintPoints ()
{
    Foams foams = GetFoams ();
    Foam* prevFoam = 0;
    for (size_t i = 0; i < foams.size (); ++i)
    {
	Foam* foam = foams[i].get ();
	foam->FixConstraintPoints (prevFoam);
	prevFoam = foam;
    }
}


void FoamAlongTime::MapPerFoam (FoamParamMethod* foamMethods, size_t n)
{
    FoamParamMethodList fl (foamMethods, n);
    QtConcurrent::blockingMap (m_foams.begin (), m_foams.end (), fl);
}

size_t foamsIndex (
    FoamAlongTime::Foams::iterator current, FoamAlongTime::Foams::iterator begin)
{
    return (current - begin) / sizeof *begin;
}


double GetPressureBody0 (const boost::shared_ptr<Foam>& foam)
{
    return foam->GetBody (0).GetPropertyValue (BodyProperty::PRESSURE);
}

void FoamAlongTime::adjustPressureSubtractReference ()
{
    QtConcurrent::blockingMap (
	m_foams.begin (), m_foams.end (),
	boost::bind (&Foam::AdjustPressure, _1, 
		     boost::bind (GetPressureBody0, _1)));
}

void FoamAlongTime::adjustPressureAlignMedians ()
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

void FoamAlongTime::calculateStatistics ()
{
    vector<MinMaxStatistics> minMaxStat(BodyProperty::PROPERTY_END);
    for (size_t i = BodyProperty::PROPERTY_BEGIN; 
	 i < BodyProperty::PROPERTY_END; ++i)
    {
	// statistics for all time-steps
	BodyProperty::Enum property = BodyProperty::FromSizeT (i);
	forAllBodiesAccumulate (&minMaxStat[property], property);
	m_histogram[property] (acc::min (minMaxStat[property]));
	m_histogram[property] (acc::max (minMaxStat[property]));
	forAllBodiesAccumulate (&m_histogram[property], property);


	// statistics per time-step
	double min = acc::min(m_histogram[property]);
	double max = acc::max(m_histogram[property]);
	QtConcurrent::blockingMap (
	    m_foams.begin (), m_foams.end (),
	    boost::bind (&Foam::CalculateHistogramStatistics, _1,
			 BodyProperty::FromSizeT (i), min, max));
    }
}

template <typename Accumulator>
void FoamAlongTime::forAllBodiesAccumulate (
    Accumulator* acc, BodyProperty::Enum property)
{
    QtConcurrent::blockingMap (
	m_foams.begin (), m_foams.end (),
	boost::bind (&Foam::Accumulate<Accumulator>, _1, acc, property));
}


void FoamAlongTime::calculateVelocityBody (
    pair< size_t, boost::shared_ptr<BodyAlongTime> > p)
{
    const BodyAlongTime& bat = *p.second;
    StripIterator stripIt = bat.GetStripIterator (*this);
    stripIt.ForEachSegment (boost::bind (&FoamAlongTime::storeVelocity,
					 this, _1, _2, _3, _4));    
}

void FoamAlongTime::calculateVelocity ()
{
    BodiesAlongTime::BodyMap& map = GetBodiesAlongTime ().GetBodyMap ();
    QtConcurrent::blockingMap (
	map.begin (), map.end (), 
	boost::bind (&FoamAlongTime::calculateVelocityBody, this, _1));
}

void FoamAlongTime::storeVelocity (
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

void FoamAlongTime::CacheBodiesAlongTime ()
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

bool FoamAlongTime::Is2D () const
{
    return GetFoam (0).Is2D ();
}

const Body& FoamAlongTime::GetBody (size_t bodyId, size_t timeStep) const
{
    const BodyAlongTime& bat = GetBodiesAlongTime ().GetBodyAlongTime (bodyId);
    return *bat.GetBody (timeStep);
}

string FoamAlongTime::ToString () const
{
    ostringstream ostr;
    ostr << "FoamAlongTime: " << endl;
    ostr << m_boundingBox << endl;
    ostream_iterator< boost::shared_ptr<const Foam> > output (ostr, "\n");
    copy (m_foams.begin (), m_foams.end (), output);
    ostr << m_bodiesAlongTime;
    return ostr.str ();
}

string FoamAlongTime::ToHtml () const
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


void FoamAlongTime::SetTimeSteps (size_t timeSteps)
{
    m_foams.resize (timeSteps);
    m_t1s.resize (timeSteps);
}


void FoamAlongTime::GetTimeStepSelection (
    BodyProperty::Enum property,
    const vector<QwtDoubleInterval>& valueIntervals,
    vector<bool>* timeStepSelection) const
{
    timeStepSelection->resize (GetTimeSteps ());
    fill (timeStepSelection->begin (), timeStepSelection->end (), false);
    BOOST_FOREACH (QwtDoubleInterval valueInterval, valueIntervals)
	GetTimeStepSelection (property, valueInterval, timeStepSelection);
}

void FoamAlongTime::GetTimeStepSelection (
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

bool FoamAlongTime::IsQuadratic () const
{
    return m_foams[0]->IsQuadratic ();
}

size_t FoamAlongTime::GetMaxCountPerBinIndividual (
    BodyProperty::Enum property) const
{
    size_t size = GetTimeSteps ();
    size_t max = 0;
    for (size_t i = 0; i < size; ++i)
	max = std::max (
	    max, GetFoam (i).GetHistogram (property).GetMaxCountPerBin ());
    return max;
}

bool FoamAlongTime::T1sAvailable () const
{
    BOOST_FOREACH (const vector<G3D::Vector3> timeStepT1s, m_t1s)
	if (timeStepT1s.size () != 0)
	    return true;
    return false;
}

void FoamAlongTime::ReadT1s (const string& fileName, size_t timeSteps)
{
    cdbg << "Parsing T1s file..." << endl;
    const size_t LINE_LENGTH = 256;
    ifstream in;
    in.exceptions (ios::failbit | ios::badbit);
    in.open (fileName.c_str ());
    size_t timeStep;
    float x, y;
    if (m_t1s.size () < timeSteps)
	m_t1s.resize (timeSteps);
    while (! in.eof ())
    {
	if (in.peek () == '#')
	{
	    in.ignore (LINE_LENGTH, '\n');
	    continue;
	}
	in >> timeStep >> x >> y >> ws;
	// in the file: first time step is 1 and T1s occur before timeStep
	// in memory: first time step is 0 and T1s occur after timeStep
	timeStep -= 1;
	if (timeStep >= timeSteps)
	    break;
	m_t1s[timeStep].push_back (G3D::Vector3 (x, y, Foam::Z_COORDINATE_2D));
    }
}

const vector<G3D::Vector3>& FoamAlongTime::GetT1s (size_t timeStep) const
{
    int t = int(timeStep) + m_t1sTimestepShift;
    if (t < 0 || size_t (t) >= m_t1s.size ())
	return NO_T1S;
    else
	return m_t1s[t];
}

void FoamAlongTime::ParseFiles (
    const vector<string>& fileNames,
    bool useOriginal,
    const ConstraintRotationNames& constraintRotationNames,
    const vector<ForceNames>& forcesNames,
    bool debugParsing, bool debugScanning)
{
    QDir dir;
    QStringList files;
    string filePattern;
    m_useOriginal = useOriginal;
    m_constraintRotationNames = constraintRotationNames;
    m_forcesNames.resize (forcesNames.size ());
    copy (forcesNames.begin (), forcesNames.end (), m_forcesNames.begin ());
    QFileInfo fileInfo (fileNames[0].c_str ());
    dir = fileInfo.absoluteDir ();
    BOOST_FOREACH (const string& fn, fileNames)
	files << QFileInfo(fn.c_str ()).fileName ();
    filePattern = string (
	(lastName (dir.absolutePath ()) + 
	 '/' + fileInfo.fileName ()).toAscii ());

    SetTimeSteps (files.size ());
    SetFilePattern (filePattern);

    QList< boost::shared_ptr<Foam> > foams = QtConcurrent::blockingMapped (
	files,
	parseFile (
	    dir.absolutePath (), 
	    GetConstraintRotationNames (),
	    GetForcesNames (),
	    OriginalUsed (),
	    debugParsing, debugScanning));
    if (count_if (foams.constBegin (), foams.constEnd (),
		  bl::_1 != boost::shared_ptr<Foam>()) != foams.size ())
	ThrowException ("Could not process all files\n");
    copy (foams.constBegin (), foams.constEnd (), GetFoams ().begin ());
}

