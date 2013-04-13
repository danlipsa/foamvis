/**
 * @file   Options.cpp
 * @author Dan R. Lipsa
 * @date 3 Oct 2011
 *
 * Implementation of the CommandLineOptions class
 */

#include "Debug.h"
#include "Options.h"
#include "BrowseSimulations.h"

// Private Classes/Functions
// ======================================================================

template<typename Tokenizer, typename TokenizerIterator>
class ReadToken
{
public:
    ReadToken (Tokenizer& tok, const string& errorMessage) :
	m_tok (tok), m_errorMessage (errorMessage)
    {
    }
    
    template<typename T>
    void operator() (T* dest, TokenizerIterator* it)
    {
        if (*it == m_tok.end ())
            throw invalid_argument (m_errorMessage);
        istringstream istr;
        istr.str (*(*it));
        cdbg << istr.str () << endl;
        istr >> (*dest);
        ++(*it);
    }
private:
    Tokenizer& m_tok;
    const string& m_errorMessage;
};


void filterAndExpandWildcards (vector<string>* fileNames, string filter)
{
    QFileInfo fileInfo (QString::fromStdString ((*fileNames)[0]));
    QString path = fileInfo.path ();
    QString fileName = fileInfo.fileName ();
    int questionMarkIndex = fileName.indexOf ('?');
    if (questionMarkIndex == -1)
	ThrowException ("No ? in simulation parameters: ", 
                        fileName.toStdString ());
    QRegExp rx("\\?+");
    fileName.replace (rx, QString (filter.c_str ()));
    QDir dir (path, fileName);
    QStringList fns = dir.entryList ();
    fileNames->resize (fns.size ());
    vector<string>::iterator itDest = fileNames->begin ();
    QStringList::const_iterator itSrc = fns.constBegin ();
    while (itSrc != fns.constEnd ())
    {
	QString full = path + "/" + *itSrc;
	*itDest = full.toStdString ();
	++itSrc;
	++itDest;
    }
}

po::options_description getIniOptions (
    vector<string>* names = 0,
    vector<Labels>* labels = 0, 
    vector<string>* parameters = 0)
{
    po::options_description iniOptions (
	"INI_OPTIONS: (see simulations.ini for an example)");
    iniOptions.add_options()
	(Option::m_name[Option::NAME], 
	 po::value< vector<string> >(names), 
	 "name of a simulation.\n"
	 "arg=<simulationName> A JPG file <simulationName>.jpg is "
	 "read from the same folder as the ini file.")
	(Option::m_name[Option::LABELS], 
	 po::value< vector<Labels> >(labels), 
	 "labels for the simulation.\n"
	 "arg=<labels> where labels is a comma separated list of labels. "
	 "You can use labels to group related simulations.")
	(Option::m_name[Option::PARAMETERS], 
	 po::value< vector<string> >(parameters), 
	 "foamvis parameters.\n"
	 "arg=<parameters> where <parameters> is a "
	 "string \"[COMMON_OPTIONS] <files> ...\"")
	;
    return iniOptions;
}

void getIniOptions (const string& iniFileName, 
		    vector<string>* names, 
		    vector<Labels>* labels, 
		    vector<string>* parametersArray)
{
    po::variables_map vm;
    po::options_description iniOptions = getIniOptions (
	names, labels, parametersArray);
    
    ifstream iniStream (iniFileName.c_str ());
    if (iniStream.fail ())
    {
	cerr << "Cannot open \"" << iniFileName << "\" for reading." << endl;
	exit (13);
    }
    string parameters;
    po::store (po::parse_config_file (iniStream, iniOptions), vm);
    po::notify(vm);
}


po::options_description getHiddenOptions (vector<string>* fileNames)
{
    po::options_description hiddenOptions("Hidden options");
    hiddenOptions.add_options()
	(Option::m_name[Option::DMP_FILES], 
	 po::value< vector<string> >(fileNames), "dmp file");
    return hiddenOptions;
}

po::options_description getCommonAndHiddenOptions (
    vector<string>* fileNames,
    const po::options_description& commonOptions)
{
    po::options_description options;
    po::options_description hiddenOptions = getHiddenOptions (fileNames);
    options.add(commonOptions).add(hiddenOptions);
    return options;
}

void getSelectedIndexes (
    const string& iniFileName, 
    const vector<string>& allNames,
    const vector<Labels>& labels, const vector<size_t>& questionMarkCount,
    vector<size_t>* selectedIndexes, vector<string>* filter)
{
    QFileInfo iniFileInfo (iniFileName.c_str ());
    // browse simulations and choose a name.
    BrowseSimulations browseSimulations (
	(iniFileInfo.absolutePath () + "/" +
	 iniFileInfo.baseName ()).toAscii (), 
        allNames, questionMarkCount, labels);
    if (browseSimulations.exec () == QDialog::Accepted)
    {
	*selectedIndexes = browseSimulations.GetSelectedIndexes ();
	*filter = browseSimulations.GetFilter ();
    }
}

void getSelectedIndexes (const vector<string>& selectedNames,
			 const vector<string>& allNames,
			 vector<size_t>* selectedIndexes)
{
    selectedIndexes->resize (selectedNames.size ());
    for (size_t i = 0; i < selectedNames.size (); ++i)
    {
	string name = selectedNames[i];
	vector<string>::const_iterator it = find (
	    allNames.begin (), allNames.end (), name);
	if (it == allNames.end ())
	{
	    cerr << "Cannot find " << name 
		 << " in the ini file." << endl;
	    exit (13);
	}
	size_t index = it - allNames.begin ();
	(*selectedIndexes)[i] = index;
    }
}


void printVersion ()
{
    /**
     * DO NOT CHANGE
     * the version number and date it is generated by release.pl
     */
    const char* version = "0.8.2740";
    const char* date = "2013-03-01 09:15:45";
    cout << "foam version " << version << ", date " << date;
#ifdef QT_NO_DEBUG
    cout << ", RELEASE";
#else
    cout << ", DEBUG";
#endif
    cout << endl;
}


void questionMarkCount (const vector<string>& parameters, vector<size_t>* c)
{
    c->resize (parameters.size (), 0);
    for (size_t i = 0; i < parameters.size (); ++i)
    {
        size_t qIndex = parameters[i].find ('?');
        if (qIndex == string::npos)
            ThrowException ("No ? in simulation parameters: ", parameters[i]);
        do
        {
            ++(*c)[i];
            ++qIndex;
        }
        while (qIndex < parameters[i].size () && parameters[i][qIndex] == '?');
    }
}

// validate functions for boost::program_options
// ======================================================================

void validate(boost::any& v, const std::vector<std::string>& values,
              DmpObjectInfo* ignore1, int ignore2)
{
    (void)ignore1;(void)ignore2;
    DmpObjectInfo crn;
    boost::tokenizer<> tok (values[0]);
    boost::tokenizer<>::iterator it = tok.begin ();
    ReadToken<boost::tokenizer<>, boost::tokenizer<>::iterator> 
        readToken (tok, "--constraint-rotation needs four parameters.");
    readToken (&crn.m_constraintIndex, &it);
    readToken (&crn.m_xName, &it);
    readToken (&crn.m_yName, &it);
    readToken (&crn.m_angleName, &it);
    v = boost::any(crn);
}

namespace G3D
{
void validate(boost::any& v, const std::vector<std::string>& values,
              G3D::AABox* ignore1, int ignore2)
{
    (void)ignore1;(void)ignore2;
    typedef boost::tokenizer<boost::escaped_list_separator<char> > Tokenizer;
    Tokenizer tok (values[0]);
    Tokenizer::iterator it = tok.begin ();
    ReadToken<Tokenizer, Tokenizer::iterator> 
        readToken (tok, "--simulation-box needs six parameters.");
    float x0 = 0, y0 = 0, z0 = 0, x1 = 0, y1 = 0, z1 = 0;
    readToken (&x0, &it);
    readToken (&y0, &it);
    readToken (&z0, &it);
    readToken (&x1, &it);
    readToken (&y1, &it);
    readToken (&z1, &it);
    v = boost::any (
        G3D::AABox (G3D::Vector3 (x0, y0, z0), G3D::Vector3 (x1, y1, z1)));
}
};

void validate(boost::any& v, const std::vector<std::string>& values,
              ForceNamesOneObject* ignore1, int ignore2)
{
    (void)ignore1;(void)ignore2;
    typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
    boost::char_separator<char> sep(",", "", boost::keep_empty_tokens);
    Tokenizer tok (values[0], sep);
    ForceNamesOneObject fn;
    ReadToken<Tokenizer, Tokenizer::iterator> 
        readToken (tok, "--force needs 7 parameters.");
    Tokenizer::iterator it = tok.begin ();
    readToken (&fn.m_bodyId, &it);
    --fn.m_bodyId;
    readToken (&fn.m_networkForceName[0], &it);
    readToken (&fn.m_networkForceName[1], &it);
    readToken (&fn.m_networkForceName[2], &it);
    readToken (&fn.m_pressureForceName[0], &it);
    readToken (&fn.m_pressureForceName[1], &it);
    readToken (&fn.m_pressureForceName[2], &it);
    try
    {
	// these can be omitted
	readToken (&fn.m_networkTorqueName, &it);
	readToken (&fn.m_pressureTorqueName, &it);
    }
    catch (invalid_argument& e)
    {
    }
    v = boost::any(fn);    
}

void validate(boost::any& v, const std::vector<std::string>& values,
              Labels* ignore1, int ignore2)
{
    (void)ignore1;(void)ignore2;
    Labels labels;
    typedef boost::tokenizer< boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep(", ");
    tokenizer tok (values[0], sep);
    for (tokenizer::iterator it = tok.begin ();
	 it != tok.end (); ++it)
	labels.m_values.push_back (*it);
    v = boost::any(labels);
}


// Option
// ======================================================================

const char* Option::m_name[] = {
    "constraint",
    "constraint-rotation",
    "debug-parsing",
    "debug-scanning",
    "dmp-files",
    "filter",
    "force",
    "help",
    "ini-file",
    "name",
    "labels",
    "original-pressure",
    "output-text",
    "parameters",
    "reflection-axis",
    "resolution",
    "rotation-2d",
    "simulation",
    "simulation-box",
    "t1s",
    "t1s-lower",
    "ticks-for-timestep",
    "use-original",
    "version"
};


// CommandLineOptions
// ======================================================================
CommandLineOptions::CommandLineOptions () :
    m_commandLineOptions (
	getDescription (&m_iniFileName, &m_simulationNames, &m_filter))
{
}

void CommandLineOptions::read (int argc, char *argv[])
{
    po::store(po::command_line_parser (argc, argv).
	      options (m_commandLineOptions).run (), m_vm);
    po::notify(m_vm);
    if (m_vm.count (Option::m_name[Option::VERSION]))
    {
	printVersion ();
	exit (0);
    }
    if (m_vm.count (Option::m_name[Option::HELP])) 
    {	
	cout << CommonOptions::getDescription () << "\n";
	cout << CommandLineOptions::getDescription () << endl;
	cout << getIniOptions () << endl;
	exit (0);
    }
    if (m_vm.count (Option::m_name[Option::INI_FILE]))
    {
	getIniOptions (m_iniFileName, &m_names, 
		       &m_labels, &m_parametersArray);	
	if (m_simulationNames.empty ())
        {
            vector<size_t> qmc;
            questionMarkCount (m_parametersArray, &qmc);
	    getSelectedIndexes (
		m_iniFileName, m_names, m_labels, qmc,
		&m_simulationIndexes, &m_filter);
        }
	else
	    getSelectedIndexes (m_simulationNames, m_names, 
				&m_simulationIndexes);
    }
}


po::options_description CommandLineOptions::getDescription (
    string* iniFileName,
    vector<string>* simulationName, vector<string>* iniFilter)
{
    po::options_description commandLineOptions (
	"COMMAND_LINE_OPTIONS");
    commandLineOptions.add_options()
	(Option::m_name[Option::DEBUG_PARSING], 
	 "produces output that help debugging the parser")
	(Option::m_name[Option::DEBUG_SCANNING], 
	 "produces output that helps debugging the scanner")
	(Option::m_name[Option::FILTER], 
	 po::value< vector<string> >(iniFilter), 
	 "Filter simulation DMPs. Default value is '0001'.\n"
	 "arg=<filter> where <filter> characters replace the question marks "
	 "in the patern specified in the ini file staring with the least "
	 "significant. For example, filter '1', results in the pattern "
	 "'???1' which selects DMP files numbered 0001, 0011, 0021, ..., 0091, "
	 "0101, ...., filter '0001' results in patern '0001' which selects "
	 "only the DMP numbered 0001.")
	(Option::m_name[Option::HELP], "produce help message")
	(Option::m_name[Option::INI_FILE], 
	 po::value<string>(iniFileName), 
	 "choose simulation and read visualization parameters " 
	 "from the ini file.\n"
	 "arg=<iniFileName>. See simulations.ini for an example.")
	(Option::m_name[Option::OUTPUT_TEXT],
	 "outputs a text representation of the data")
	(Option::m_name[Option::SIMULATION],
	 po::value< vector<string> >(simulationName),
	 "arg=<simulationNames>, parse the simulations with names "
	 "<simulationNames> in the ini file.")
	(Option::m_name[Option::VERSION], "prints version information")
	;
    return commandLineOptions;
}


// CommonOptions
// ======================================================================
CommonOptions::CommonOptions () :
    m_ticksForTimeStep (1),
    m_reflectionAxis (numeric_limits<size_t>::max ()),
    m_resolution (0),
    m_rotation2D (0),
    m_commonOptions (
	getCommonAndHiddenOptions (
            &m_fileNames, 
            getDescription (
                &m_t1sFile, &m_dmpObjectInfo, &m_forceNames, 
                &m_ticksForTimeStep, &m_simulationBoundingBoxAllTimeSteps, 
                &m_resolution, &m_rotation2D, &m_reflectionAxis)))
{
    m_positionalOptions.add (Option::m_name[Option::DMP_FILES], -1);    
}

void CommonOptions::read (int argc, char *argv[])
{
    po::store(po::command_line_parser (argc, argv).
	      options (m_commonOptions).positional (m_positionalOptions).
	      allow_unregistered ().run (), 
	      m_vm);
    po::notify(m_vm);
    if (m_rotation2D != 0)
	RuntimeAssert (abs (m_rotation2D) == 90, 
		       "Invalid rotation: ", m_rotation2D);
    if (m_reflectionAxis != numeric_limits<size_t>::max ())
	RuntimeAssert (m_reflectionAxis <= 2,
		       "Invalid axis: ", m_reflectionAxis);
    if (m_dmpObjectInfo.m_constraintIndex != INVALID_INDEX)
	--m_dmpObjectInfo.m_constraintIndex;
    if (argc == 1 || ! m_vm.count (Option::m_name[Option::DMP_FILES]))
    {
	printVersion ();
	cerr << "No DMP file specified" << endl;
	cerr << CommonOptions::getDescription () << endl;
	cerr << CommandLineOptions::getDescription () << endl;
	cerr << getIniOptions () << endl;
	exit (13);
    }
}


void CommonOptions::read (string parameters, string filter)
{
    typedef boost::tokenizer< boost::escaped_list_separator<char> > 
	Tokenizer;
    boost::escaped_list_separator<char> els ('\\', ' ', '\"');
    Tokenizer tok (parameters, els);
    vector<string> tokenizedParameters;
    vector<char*> tvs;
    tvs.push_back (0);
    for (Tokenizer::iterator it = tok.begin(); it != tok.end(); ++it)
    {
	tokenizedParameters.push_back (*it);
	tvs.push_back (const_cast<char*> (
			   (tokenizedParameters.end () - 1)->c_str ()));
    }
    tvs.push_back (0);
    read (tvs.size () - 1, &tvs[0]);
    filterAndExpandWildcards (&m_fileNames, filter);
}


po::options_description CommonOptions::getDescription (
    string* t1sFile,
    DmpObjectInfo* dmpObjectInfo,
    vector<ForceNamesOneObject>* forceNames,
    size_t* ticksForTimeStep, G3D::AABox* simulationBox, size_t* resolution,
    int *rotation2D,
    size_t *reflectionAxis)
{
    po::options_description commonOptions (
	"\"foam [COMMAND_LINE_OPTIONS] [COMMON_OPTIONS] <files> ...\"\n"
	"\"foam [COMMAND_LINE_OPTIONS]\" where --ini-file has to be specified\n"
	"<files> - one or more DMP files\n"
	"Examples...\n"
	"using the ini file and the simulations browser:\n"
	"\t./foam --ini-file simulations.ini\n"
	"using the ini file:\n"
	"\t./foam --ini-file simulations.ini --simulation ctrctndumps_704v_0.1520_0.2400_8.0000_v1 --filter '???1'\n"
	"using command line parameters:\n"
	"\t./foam --t1s /home/dlipsa/Documents/swansea-phd/foam/ctrctndumps_704v_0.1520_0.2400_8.0000_v1/energy_704v_0.1520_0.2400_8.0000_v1.dat /home/dlipsa/Documents/swansea-phd/foam/ctrctndumps_704v_0.1520_0.2400_8.0000_v1/dump_0.1520_0.2400_8.0000_0???.dmp\n"
	"COMMON_OPTIONS");
    commonOptions.add_options()
	(Option::m_name[Option::CONSTRAINT],
	 po::value<size_t>(&dmpObjectInfo->m_constraintIndex), 
	 "a constraint that specifies an object.\n"
         "arg=<constraint> where <constraint> is the constraint number.")
	(Option::m_name[Option::CONSTRAINT_ROTATION], 
	 po::value<DmpObjectInfo>(dmpObjectInfo), 
	 "a constraint that specifies an object that moves (translates" 
	 " and rotates) through foam.\n"
	 "arg=\"<constraint> <xName> <yName> <angleName>\" where " 
	 "<constraint> specifies the constraint number, <xName>, <yName> "
	 "specify names for parameters that store the center of rotation and "
	 "<angleName> specifies the name of the parameter that stores "
	 "the rotation angle. The rotation (in radians) follows the "
	 "left-hand rule: a rotation around z axis pointing toward "
	 "the user is clockwise. Zero angle is the positive Y axis.")
	(Option::m_name[Option::FORCES], 
	 po::value< vector<ForceNamesOneObject> >(forceNames),
	 "reads the forces acting on a body.\n"
	 "arg=\"<bodyId>,"
         "<networkForceXName>,<networkForceYName>,<networkForceZName>,"
	 "<pressureForceXName>,<pressureForceYName>,<pressureForceZName>"
	 "[,<networkTorque>,<pressureTorque>]\" where "
	 "<bodyId> is the ID of the body the force acts on, "
	 "<networkForce.Name> are the "
	 "names of the X, Y and Z components of the network force, "
	 "<pressureForce.Name> are the names of the "
	 "X, Y and Z components of the pressure force. "
         "In 2D leave the name for the Z component empty."
	 "<...Torque> are the names of the network and "
	 "pressure torque on the object, the positive direction is clockwise.")
	(Option::m_name[Option::ORIGINAL_PRESSURE],
	 "shows original pressure values")
	(Option::m_name[Option::REFLECTION_AXIS],
	 po::value<size_t> (reflectionAxis),
	 "reflect about specified axis (after rotation).\n"
	 "arg=<axis>, where <axis> can be 0, 1 or 2.")
	(Option::m_name[Option::RESOLUTION],
	 po::value<size_t>(resolution), 
	 "resolution of the regular grid saved from a simulation time "
	 "step in 3D.\n"
         "arg=<r> where r=0,64,128 or 256. The resolution is r^3, "
	 "r=0 means no regular grid is saved, "
	 "so the average computation is disabled.")
	(Option::m_name[Option::ROTATION_2D],
	 po::value<int> (rotation2D),
	 "rotate around Z axes.\n"
	 "arg=<angle>, where <angle> can be 90 or -90, positive rotation is "
	 "counterclockwise.")
	(Option::m_name[Option::SIMULATION_BOX],
	 po::value<G3D::AABox>(simulationBox), 
	 "specifies the simulation bounding box.\n"
	 "where arg=\"<x0>,<y0>,<z0>,<x1>,<y1>,<z1>\""
         "the lower left and upper right corners of the bounding box. "
         "Used for 3D when caching a regular grid on disk.")
	(Option::m_name[Option::T1S],
	 po::value<string>(t1sFile), 
	 "reads T1 positions from file.\n"
	 "arg=<file> where <file> specifies a text file with "
	 "T1 times and positions. T1s won't be displayed correctly if you "
	 "skip time steps when you load simulation files. "
         "By default, t1s are read from an array called t1positions in the "
         "simulation file.")
	(Option::m_name[Option::T1S_LOWER],
	 "Shift T1s one time step lower.")
	(Option::m_name[Option::TICKS_FOR_TIMESTEP],
	 po::value<size_t>(ticksForTimeStep), 
	 "arg=<numTicks> specifies how many ticks you have in 1 timestep. "
	 "A DMP file is saved every 1 time step or every specified number of "
	 "ticks. If this parameters is not provided, the default number "
	 "of ticks for a timestep is one.")
	(Option::m_name[Option::USE_ORIGINAL], "uses the ORIGINAL atribute "
	 "to figure out the body id.")
	;
    return commonOptions;
}

