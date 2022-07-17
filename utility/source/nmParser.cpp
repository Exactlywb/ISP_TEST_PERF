#include "nmParser.hpp"

namespace nmParser {

namespace {

class size_t_from_hex {  // To use with boost::lexical_cast for hex numbers
    std::size_t value;

public:
    operator std::size_t () const
    {
        return static_cast<std::size_t> (value);
    }

    friend std::istream &operator>> (
        std::istream &in, size_t_from_hex &outValue)
    {
        in >> std::hex >> outValue.value;
        return in;
    }
};

void parseCurNMStr (std::vector<nmFuncInfo>& nmFunctionInfo, const std::string& str) {

    std::vector<std::string> noWS;
    boost::split (noWS, str, boost::is_any_of (" "), boost::token_compress_on);

    // std::cerr << "here = " << boost::lexical_cast<std::size_t> (noWS [2]) << std::endl;

    if (boost::starts_with (noWS [2], "0x"))
        nmFunctionInfo.push_back ({noWS [7], boost::lexical_cast<size_t_from_hex> (noWS [2])});
    else
        nmFunctionInfo.push_back ({noWS [7], boost::lexical_cast<std::size_t> (noWS [2])});

}

}

void parse_nm_data (std::vector<nmFuncInfo>& nmFunctionInfo, const char* nmPath) {

    TraceStream traceReader (nmPath);

    while (!traceReader.isAtEOF ()) {
        while (traceReader.getCurrentLine ().empty ())
            traceReader.advance ();

        std::string curStr = traceReader.getCurrentLine ();
        boost::trim (curStr);

        parseCurNMStr (nmFunctionInfo, curStr);

        traceReader.advance ();
    }

}

}
