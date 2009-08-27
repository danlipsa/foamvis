#ifndef __PARSING_DRIVER_H__
#define __PARSING_DRIVER_H__
#include <string>
#include "EvolverData_yacc.h"

class Data;
// Conducting the whole scanning and parsing of Calc++.
class ParsingDriver
{
public:
    ParsingDriver ();
    virtual ~ParsingDriver ();

    // Handling the scanner.
    void ScanBegin ();
    void ScanEnd ();
    void SetDebugScanning (bool debugScanning)
    {
        m_debugScanning = debugScanning;
    }

    // Handling the parser.
    int Parse (const std::string& f, Data& data);
    void SetDebugParsing (bool debugParsing)
    {
        m_debugParsing = debugParsing;
    }
	void SetFile (const std::string& file)
    {
        m_file = file;
    }
	std::string& GetFile ()
    {
        return m_file;
    }

    // Error handling.
    void PrintError (const EvolverData::location& l, const std::string& m);
    void PrintError (const std::string& m);
    
    /**
     * Converts a  string in a given  base to an integer.  Prints an error
     * and exits if the conversion fails.
     * @param str string to be converted to an integer
     * @param base the base that the string uses to represent the integer
     * @return the converted integer
     */
    long ReadInteger (char* str, int base);

    static int GetKeywordId (const char* keyword);
    static const char* GetKeywordString (int id);
private:
    bool m_debugScanning;
    void* m_scanner;
    static const char* m_keywordTable[];
    static const int FIRST_TOKEN;

    bool m_debugParsing;
    std::string m_file;
};

#endif // __PARSING_DRIVER_H__

// Local Variables:
// mode: c++
// End:
