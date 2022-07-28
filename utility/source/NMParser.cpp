#include "NMParser.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace NMParser {

namespace {

class size_t_from_hex {  // To use with boost::lexical_cast for hex
                         // numbers
    std::size_t value;

public:
    operator std::size_t () const { return static_cast<std::size_t> (value); }

    friend std::istream &operator>> (std::istream &in, size_t_from_hex &outValue)
    {
        in >> std::hex >> outValue.value;
        return in;
    }
};

void parseCurNMStr (std::vector<FuncInfo> &nmFunctionInfo, const std::string &str)
{
    std::vector<std::string> noWS;
    boost::split (noWS, str, boost::is_any_of (" "), boost::token_compress_on);

    auto &name_str = noWS[7];
    auto &size_str = noWS[2];
    auto &addr_str = noWS[1];

    uint64_t size = 0;
    if (size_str.starts_with ("0x")) {
        size = boost::lexical_cast<size_t_from_hex> (size_str);
    }
    else {
        size = boost::lexical_cast<std::size_t> (size_str);
    }

    uint64_t addr = boost::lexical_cast<size_t_from_hex> (addr_str);

    nmFunctionInfo.emplace_back (std::move (name_str), size, addr, 0);
}

}  // namespace

void parse_nm_data (std::vector<FuncInfo> &nmFunctionInfo, const char *nmPath)
{
    TraceStream traceReader (nmPath);

    while (!traceReader.isAtEOF ()) {
        while (traceReader.getCurrentLine ().empty ())
            traceReader.advance ();

        std::string curStr = traceReader.getCurrentLine ();
        boost::trim (curStr);

        parseCurNMStr (nmFunctionInfo, curStr);

        traceReader.advance ();
    }

    std::cout << "Before removing dublicates size = " << nmFunctionInfo.size () << "\n";
    // remove copies
    auto &FI = nmFunctionInfo;
    std::sort (FI.begin (), FI.end (), [] (const FuncInfo &a, const FuncInfo &b) {
        return a.interal_addr_ < b.interal_addr_;
    });
    auto last =
        std::unique (FI.begin (), FI.end (), [] (const FuncInfo &a, const FuncInfo &b) {
            return a.interal_addr_ == b.interal_addr_;
        });
    FI.erase (last, FI.end ());
    std::cout << "After removing dublicates size = " << nmFunctionInfo.size () << "\n";
}

}  // namespace NMParser
