/**
 * @file   Options.h
 * @author Dan R. Lipsa
 * @date 3 Oct 2011
 *
 * Declaration of the Options class
 */

#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include "Labels.h"
#include "ObjectPosition.h"
#include "Force.h"

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
	TICKS_FOR_TIMESTEP,
	USE_ORIGINAL,
	VERSION
    };

    static const char* m_name[];
};

class CommandLineOptions
{
public:
    CommandLineOptions ();
    void read (int argc, char *argv[]);
    static po::options_description getDescription (
	string* iniFileName = 0,
	vector<string>* simulationName = 0, string* iniFilter = 0);

public:    
    string m_iniFileName;
    vector<string> m_names;
    vector<Labels> m_labels;
    vector<string> m_parametersArray;
    vector<string> m_simulationNames;
    vector<size_t> m_simulationIndexes;
    string m_filter;
    po::variables_map m_vm;

private:
    po::options_description m_commandLineOptions;
};


class CommonOptions
{
public:
    CommonOptions ();
    void read (int argc, char *argv[]);
    void read (string parameters, string filter);
    static po::options_description getDescription (
	string* t1sFile = 0,
	DmpObjectInfo* dmpObjectInfo = 0,
	vector<ForceNames>* forcesNames = 0,
	size_t* ticksForTimeStep = 0);

public:
    string m_t1sFile;
    vector<string> m_fileNames;
    DmpObjectInfo m_dmpObjectInfo;
    vector<ForceNames> m_forcesNames;
    size_t m_ticksForTimeStep;
    po::variables_map m_vm;

private:
    po::options_description m_commonOptions;
    po::positional_options_description m_positionalOptions;
};


#endif //__OPTIONS_H__

// Local Variables:
// mode: c++
// End:
