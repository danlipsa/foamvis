#ifndef PARSER_DRIVER_H
#define PARSER_DRIVER_H
#include <string>
#include "EvolverData_yacc.h"

// Conducting the whole scanning and parsing of Calc++.
class ParserDriver
{
public:
    ParserDriver ();
    virtual ~ParserDriver ();

    // Handling the scanner.
    void ScanBegin ();
    void ScanEnd ();
    bool m_debugScanning;

    // Handling the parser.
    bool m_debugParsing;
    std::string m_file;

    // Error handling.
    void PrintError (const EvolverData::location& l, const std::string& m);
    void PrintError (const std::string& m);
    
    void* GetScanner () {return m_scanner;}
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
    void* m_scanner;
    static const char* m_keywordTable[];
    static const int FIRST_TOKEN;
};
#endif // ! PARSER_DRIVER_H


// Local Variables:
// mode: c++
// End:
