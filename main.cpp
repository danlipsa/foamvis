/**
 * @file main.cpp
 * @author Dan R. Lipsa
 *
 * Parses an Evolver DMP file and displays the data from the file.
 */
#include "Application.h"
#include "BrowseSimulations.h"
#include "Debug.h"
#include "Foam.h"
#include "FoamAlongTime.h"
#include "Force.h"
#include "ParsingData.h"
#include "MainWindow.h"
#include "DebugStream.h"

struct Option
{
    enum Enum
    {
	CONSTRAINT,
	CONSTRAINT_ROTATION,
	DEBUG_PARSING,
	DEBUG_SCANNING,
	DMP_FILES,
	FILTER,
	FORCES,
	HELP,
	INI_FILE,
	NAME,
	LABELS,
	ORIGINAL_PRESSURE,
	OUTPUT_TEXT,
	PARAMETERS,
	SIMULATION,
	T1S,
	T1S_LOWER,
	USE_ORIGINAL,
	VERSION
    };

    static const char* m_name[];
};

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
    "simulation",
    "t1s",
    "t1s-lower",
    "use-original",
    "version"
};


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

void validate(boost::any& v, const std::vector<std::string>& values,
              ConstraintRotationNames* ignore1, int ignore2)
{
    (void) ignore1;(void)ignore2;
    ConstraintRotationNames crn;
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
    boost::tokenizer<> tok (values[0]);
    for (boost::tokenizer<>::iterator it = tok.begin ();
	 it != tok.end (); ++it)
	labels.m_values.push_back (*it);
    v = boost::any(labels);
}

void printVersion ()
{
    /**
     * DO NOT CHANGE
     * the version number and date it is generated by release.pl
     */
    const char* version = "0.6.2118";
    const char* date = "2011-08-26 16:47:22";
    cout << "foam version " << version << ", date " << date;
#ifdef QT_NO_DEBUG
    cout << ", RELEASE";
#else
    cout << ", DEBUG";
#endif
    cout << endl;
}

po::options_description getCommonOptions (
    string* t1sFile,
    ConstraintRotationNames* constraintRotationNames,
    vector<ForceNames>* forcesNames)
{
    po::options_description commonOptions (
	"\"foam [COMMAND_LINE_OPTIONS] [COMMON_OPTIONS] <files> ...\"\n"
	"\"foam [COMMAND_LINE_OPTIONS]\" where --ini-file has to be specified\n"
	"<files> - one or more DMP files\n"
	"COMMON_OPTIONS");
    commonOptions.add_options()
	(Option::m_name[Option::CONSTRAINT],
	 po::value<size_t>(&constraintRotationNames->m_constraintIndex), 
	 "a constraint that specifies an object.\n"
         "arg=<constraint> where <constraint> is the constraint number.")
	(Option::m_name[Option::CONSTRAINT_ROTATION], 
	 po::value<ConstraintRotationNames>(constraintRotationNames), 
	 "a constraint that specifies an object which rotates.\n"
	 "arg=\"<constraint> <xName> <yName> <angleName>\" where " 
	 "<constraint> specifies the constraint number, <xName>, <yName> "
	 "specify names for parameters that store the center of rotation and "
	 "<angleName> specifies the name of the parameter that stores "
	 "the rotation angle.")
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
	(Option::m_name[Option::T1S],
	 po::value<string>(t1sFile), 
	 "reads T1 positions.\n"
	 "arg=<file> where <file> specifies a text file with "
	 "T1 times and positions. Reading T1s won't work if you "
	 "skip time steps")
	(Option::m_name[Option::T1S_LOWER],
	 "Shift T1s one time step lower.")
	(Option::m_name[Option::USE_ORIGINAL], "uses the ORIGINAL atribute "
	 "to figure out the body id.")
	;
    return commonOptions;
}

po::options_description getCommandLineOptions (
    string* iniFileName, string* simulationName, string* iniFilter)
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
	 po::value<string>(simulationName),
	 "parse the simulation files labeld by <name> in the ini file.\n"
	 "arg=<name>." )
	(Option::m_name[Option::VERSION], "prints version information")
	;
    return commandLineOptions;
}

po::options_description getIniOptions (
    vector<string>* names, vector<Labels>* labels, vector<string>* parameters)
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

po::options_description getOptions (
    vector<string>* fileNames,
    po::options_description& commonOptions,
    po::options_description& commandLineOptions)
{
    po::options_description options;
    po::options_description hiddenOptions = getHiddenOptions (fileNames);
    options.add(commonOptions).add (commandLineOptions).add(hiddenOptions);
    return options;
}

void storeIniOptions (
    const string& iniFileName, const string& simulationName,
    const vector<string>& names, const vector<Labels>& labels, 
    const vector<string>& parametersArray, 
    string* iniFilter,
    po::options_description& options,
    po::positional_options_description& positionalOptions,
    po::options_description& iniOptions,
    po::variables_map* vm)
{
    ifstream iniStream (iniFileName.c_str ());
    if (iniStream.fail ())
    {
	cerr << "Cannot open \"" << iniFileName << "\" for reading." << endl;
	exit (13);
    }
    size_t i;
    string parameters;
    po::store (po::parse_config_file (iniStream, iniOptions), *vm);
    po::notify(*vm);
    if (vm->count (Option::m_name[Option::SIMULATION]))
    {
	vector<string>::const_iterator it = find (
	    names.begin (), names.end (), simulationName);
	if (it == names.end ())
	{
	    cerr << "Cannot find " << simulationName 
		 << " in the ini file." << endl;
	    exit (13);
	}
	i = it - names.begin ();
    }
    else
    {
	// browse simulations and choose a name.
	BrowseSimulations browseSimulations (names, labels);
	if (browseSimulations.exec () == QDialog::Accepted)
	{
	    i = browseSimulations.GetIndex ();
	    *iniFilter = browseSimulations.GetFilter ();
	}
	else
	    exit (0);
    }
    cdbg << "Simulation name " << names[i] << "." << endl;
    parameters = parametersArray[i];
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
	//cout << *(tvs.end () - 1) << endl;
    }
    tvs.push_back (0);
    po::store (po::command_line_parser (tvs.size () - 1, &tvs[0]).
	       options (options).positional (positionalOptions).run (), *vm);
    po::notify (*vm);
}


void filterAndExpandWildcards (vector<string>* fileNames, string filter)
{
    QFileInfo fileInfo (QString::fromStdString ((*fileNames)[0]));
    QString path = fileInfo.path ();
    QString fileName = fileInfo.fileName ();
    int questionMarkIndex = fileName.lastIndexOf ('?');
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


void parseOptions (
    int argc, char *argv[], string* t1sFile,
    vector<string>* fileNames, ConstraintRotationNames* constraintRotationNames,
    vector<ForceNames>* forcesNames, po::variables_map* vm)
{
    vector<string> names, parametersArray;
    vector<Labels> labels;
    string iniFileName, simulationName, iniFilter;
    po::options_description commandLineOptions = getCommandLineOptions (
	&iniFileName, &simulationName, &iniFilter);
    po::options_description commonOptions = getCommonOptions (
	t1sFile, constraintRotationNames, forcesNames);
    po::options_description iniOptions = getIniOptions (
	&names, &labels, &parametersArray);
    po::options_description options = getOptions (
	fileNames, commonOptions, commandLineOptions);
    po::positional_options_description positionalOptions;
    positionalOptions.add (Option::m_name[Option::DMP_FILES], -1);

    po::store(po::command_line_parser (argc, argv).
	      options (options).positional (positionalOptions).run (), *vm);
    po::notify(*vm);
    if (vm->count (Option::m_name[Option::INI_FILE]))
    {
	storeIniOptions (
	    iniFileName, simulationName, names, labels, parametersArray, 
	    &iniFilter,
	    options, positionalOptions, iniOptions, vm);
	filterAndExpandWildcards (fileNames, iniFilter);
    }

    if (constraintRotationNames->m_constraintIndex != INVALID_INDEX)
	--constraintRotationNames->m_constraintIndex;
    if (vm->count (Option::m_name[Option::HELP])) 
    {
	cout << commonOptions << "\n";
	cout << commandLineOptions << endl;
	cout << iniOptions << endl;
	exit (0);
    }
    if (vm->count (Option::m_name[Option::VERSION]))
    {
	printVersion ();
	exit (0);
    }
    if (argc == 1 || ! vm->count (Option::m_name[Option::DMP_FILES]))
    {
	printVersion ();
	cerr << "No DMP file specified" << endl;
	cerr << commonOptions << endl;
	cerr << commandLineOptions << endl;
	cerr << iniOptions << endl;
	exit (13);
    }
}


/**
 * Parses the data file, reads in vertices, edges, etc and displays them.
 * @return 0 for success, different than 0 otherwise
 */
int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName ("Swansea University");
    QCoreApplication::setOrganizationDomain ("www.swansea.ac.uk");
    QCoreApplication::setApplicationName ("FoamVis");
    boost::shared_ptr<Application> app = Application::Get (
	argc, argv);
    try
    {
	FoamAlongTime foamAlongTime;
	string t1sFile;
	vector<string> fileNames;
	ConstraintRotationNames constraintRotationNames;
	vector<ForceNames> forcesNames;
	po::variables_map vm;

	parseOptions (argc, argv, 
		      &t1sFile, &fileNames, &constraintRotationNames,
		      &forcesNames,
		      &vm);	
	foamAlongTime.ParseFiles (
	    fileNames, vm.count (Option::m_name[Option::USE_ORIGINAL]),
	    constraintRotationNames, forcesNames,
	    vm.count (Option::m_name[Option::DEBUG_PARSING]), 
	    vm.count (Option::m_name[Option::DEBUG_SCANNING]));
	size_t timeSteps = foamAlongTime.GetTimeSteps ();
	if (vm.count (Option::m_name[Option::T1S]))
	    foamAlongTime.ReadT1s (
		t1sFile, timeSteps, 
		vm.count (Option::m_name[Option::T1S_LOWER]));
        if (timeSteps == 0)
	{
	    cdbg << "Error: The patern provided does not match any file" 
		 << endl;
	    exit (13);
	}

	foamAlongTime.SetAdjustPressure (
	    ! vm.count (Option::m_name[Option::ORIGINAL_PRESSURE]));
	foamAlongTime.Preprocess ();
	if (vm.count (Option::m_name[Option::OUTPUT_TEXT]))
	    cdbg << foamAlongTime;
	else
	{
	    int result;
	    MainWindow window (foamAlongTime);
	    window.show();
	    result = app->exec();
	    app->release ();
	    return result;
	}
	return 0;
    }
    catch (const exception& e)
    {
	cdbg << "Exception: " << e.what () << endl;
	exit (13);
    }
}
