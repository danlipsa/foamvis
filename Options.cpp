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

class ReadStringToken
{
public:
    ReadStringToken (boost::tokenizer<>& tok, const string& errorMessage) :
	m_tok (tok), m_errorMessage (errorMessage)
    {
    }

    void operator() (string* dest, boost::tokenizer<>::iterator* it)
    {
	if (++(*it) == m_tok.end ())
	    throw invalid_argument (m_errorMessage);
	*dest = *(*it);
    }
private:
    boost::tokenizer<>& m_tok;
    const string& m_errorMessage;
};


void filterAndExpandWildcards (vector<string>* fileNames, string filter)
{
    QFileInfo fileInfo (QString::fromStdString ((*fileNames)[0]));
    QString path = fileInfo.path ();
    QString fileName = fileInfo.fileName ();
    int questionMarkIndex = fileName.lastIndexOf ('?');
    if (questionMarkIndex == -1)
	ThrowException ("No ? in simulation parameters: ", 
			fileName.toStdString ());
    if (filter.empty ())
	filter = "0001";
    int filterLength = filter.length ();
    for (int i = 0; i < filterLength; ++i)
    {
	if (fileName[questionMarkIndex - i] == '?')
	    fileName[questionMarkIndex - i] = filter[filterLength - 1 - i];
	else
	    break;
    }
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

// ======================================================================

void validate(boost::any& v, const std::vector<std::string>& values,
              DmpObjectInfo* ignore1, int ignore2)
{
    (void) ignore1;(void)ignore2;
    DmpObjectInfo crn;
    boost::tokenizer<> tok (values[0]);
    istringstream istr;
    string errorMessage ("--constraint-rotation needs four parameters.");
    boost::tokenizer<>::iterator it = tok.begin ();
    if (it == tok.end ())
	throw invalid_argument (errorMessage);
    istr.str (*it);
    istr >> crn.m_constraintIndex;
    ReadStringToken readStringToken (tok, errorMessage);
    readStringToken (&crn.m_xName, &it);
    readStringToken (&crn.m_yName, &it);
    readStringToken (&crn.m_angleName, &it);
    v = boost::any(crn);
}


void validate(boost::any& v, const std::vector<std::string>& values,
              ForceNames* ignore1, int ignore2)
{
    (void)ignore1;(void)ignore2;
    ForceNames fn;
    boost::tokenizer<> tok (values[0]);
    istringstream istr;
    string errorMessage ("--forces needs 5 parameters.");
    boost::tokenizer<>::iterator it = tok.begin ();
    if (it == tok.end ())
	throw invalid_argument (errorMessage);
    istr.str (*it);
    istr >> fn.m_bodyId;
    --fn.m_bodyId;
    ReadStringToken readStringToken (tok, errorMessage);
    readStringToken (&fn.m_networkForceName[0], &it);
    readStringToken (&fn.m_networkForceName[1], &it);
    readStringToken (&fn.m_pressureForceName[0], &it);
    readStringToken (&fn.m_pressureForceName[1], &it);
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

// ======================================================================

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

void getSelectedIndexesFromDialog (
    const string& iniFileName, const vector<string>& allNames,
    const vector<Labels>& labels,
    vector<size_t>* selectedIndexes, string* filter)
{
    QFileInfo iniFileInfo (iniFileName.c_str ());
    // browse simulations and choose a name.
    BrowseSimulations browseSimulations (
	(iniFileInfo.absolutePath () + "/" +
	 iniFileInfo.baseName ()).toAscii (), allNames, labels);
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
    const char* version = "0.7.2269";
    const char* date = "2011-11-30 09:13:56";
    cout << "foam version " << version << ", date " << date;
#ifdef QT_NO_DEBUG
    cout << ", RELEASE";
#else
    cout << ", DEBUG";
#endif
    cout << endl;
}

// ======================================================================

const char* Option::m_name[] = {
    "constraint",
    "constraint-rotation",
    "debug-parsing",
    "debug-scanning",
    "dmp-files",
    "filter",
    "forces",
    "help",
    "ini-file",
    "name",
    "labels",
    "original-pressure",
    "output-text",
    "parameters",
    "rotation-2d",
    "simulation",
    "t1s",
    "t1s-lower",
    "ticks-for-timestep",
    "use-original",
    "version"
};

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
	if (m_simulationNames.size () == 0)
	    getSelectedIndexesFromDialog (
		m_iniFileName, m_names, m_labels,
		&m_simulationIndexes, &m_filter);
	else
	    getSelectedIndexes (m_simulationNames, m_names, 
				&m_simulationIndexes);
    }
}


po::options_description CommandLineOptions::getDescription (
    string* iniFileName,
    vector<string>* simulationName, string* iniFilter)
{
    po::options_description commandLineOptions (
	"COMMAND_LINE_OPTIONS");
    commandLineOptions.add_options()
	(Option::m_name[Option::DEBUG_PARSING], 
	 "produces output that help debugging the parser")
	(Option::m_name[Option::DEBUG_SCANNING], 
	 "produces output that helps debugging the scanner")
	(Option::m_name[Option::FILTER], 
	 po::value<string>(iniFilter), 
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


// ======================================================================

CommonOptions::CommonOptions () :
    m_ticksForTimeStep (1),
    m_rotation2D (0),
    m_commonOptions (
	getCommonAndHiddenOptions (
	    &m_fileNames, 
	    getDescription (&m_t1sFile, &m_dmpObjectInfo, &m_forcesNames, 
			    &m_ticksForTimeStep, &m_rotation2D)))
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
    vector<ForceNames>* forcesNames,
    size_t* ticksForTimeStep,
    int *rotation2D)
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
	 "the user is clockwise.")
	(Option::m_name[Option::FORCES], 
	 po::value< vector<ForceNames> >(forcesNames),
	 "reads the forces acting on a body.\n"
	 "arg=\"<bodyId> <networkXName> <networkYName> "
	 "<pressureXName> <pressureYName>\" where <bodyId> is the ID of the "
	 "body the force acts on, (<networkXName>, <networkYName>) are the "
	 "names of the X and Y components of the network force and "
	 "(<pressureXName>, <pressureYName>) are the X and Y components of "
	 "the pressure force.")
	(Option::m_name[Option::ORIGINAL_PRESSURE],
	 "shows original pressure values")
	(Option::m_name[Option::ROTATION_2D],
	 po::value<int> (rotation2D),
	 "rotate around Z axes.\n"
	 "arg=<angle>, where <angle> can be 90 or -90, positive rotation is "
	 "counterclockwise.")
	(Option::m_name[Option::T1S],
	 po::value<string>(t1sFile), 
	 "reads T1 positions.\n"
	 "arg=<file> where <file> specifies a text file with "
	 "T1 times and positions. T1s won't be displayed correctly if you "
	 "skip time steps")
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

