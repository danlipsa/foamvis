/**
 * @file   ParsingDriver.h
 * @author Dan R. Lipsa
 *
 * Drives parsing and scanning
 */
#ifndef __PARSING_DRIVER_H__
#define __PARSING_DRIVER_H__

#include "EvolverData_yacc.h"

class Data;

/**
 * Drives parsing and scanning
 */
class ParsingDriver
{
public:
    /**
     * Constructor
     */
    ParsingDriver ();
    /**
     * Destructor
     */
    virtual ~ParsingDriver ();
    /**
     * Initializes the scanner
     */
    void ScanBegin ();
    /**
     * Cleans-up the scanner
     */
    void ScanEnd ();
    /**
     * Turns on or off debugging for scanning
     * @param debugScanning true if you want debugging, false otherwise
     */
    void SetDebugScanning (bool debugScanning)
    {
        m_debugScanning = debugScanning;
    }

    /**
     * Parses a data file and stores the parsed data in an Data object
     * @param f the file to be parsed
     * @param data object where data is to be stored
     * @return 0 for success, <> than 0 otherwise
     */
    int Parse (const std::string& f, Data& data);
    /**
     * Turns on or off debugging for parsing
     * @param debugParsing true if you want debugging, false otherwise
     */
    void SetDebugParsing (bool debugParsing)
    {
        m_debugParsing = debugParsing;
    }
    /**
     * Sets the file to be parsed
     * @param file the file to be parsed
     */
    void SetFile (const std::string& file)
    {
        m_file = file;
    }
    /**
     * Gets the file to be parsed
     * @return file to be parsed
     */
    std::string& GetFile ()
    {
        return m_file;
    }
    /**
     * Called by the parser in case of errors
     * @param l line and column where error occurend in the parsed file
     * @param m message describing the error
     */
    void PrintError (const EvolverData::location& l, const std::string& m);
    /**
     * Called by the parser in case of errors
     * @param m message describing the error
     */
    void PrintError (const std::string& m);
    /**
     * Converts a  string in a given  base to an integer.  Prints an error
     * and exits if the conversion fails.
     * @param str string to be converted to an integer
     * @param base the base that the string uses to represent the integer
     * @return the converted integer
     */
    long ReadInteger (char* str, int base);
    /**
     * Gets the ID associated with a keyword
     * @param keyword keyword name
     * @return keyword ID
     */
    static int GetKeywordId (const char* keyword);
    /**
     * Gets the name associated with a keyword ID
     * @param id keyword ID
     * @return keyword name
     */
    static const char* GetKeywordString (int id);
private:
    /**
     * Do we want debugging information from the scanner?
     */
    bool m_debugScanning;
    /**
     * Opaque object describing the scanner
     */
    void* m_scanner;
    /**
     * Keywords used in the data files
     */
    static const char* m_keywordTable[];
    /**
     * The ID of the first token
     */
    static const int FIRST_TOKEN;
    /**
     * Do we want debugging information from the parser?
     */
    bool m_debugParsing;
    /**
     * Parsed file
     */
    std::string m_file;
};

#endif // __PARSING_DRIVER_H__

// Local Variables:
// mode: c++
// End:
