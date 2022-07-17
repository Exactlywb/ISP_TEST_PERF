#include "perfParser.hpp"

namespace perfParser {

namespace {

        class uint64_t_from_hex {  // To use with boost::lexical_cast
            std::uint64_t value;

        public:
            operator std::uint64_t () const
            {
                return static_cast<std::uint64_t> (value);
            }

            friend std::istream &operator>> (
                std::istream &in, uint64_t_from_hex &outValue)
            {
                in >> std::hex >> outValue.value;
                return in;
            }
        };

}

    std::tuple<std::string::iterator, LbrTraceType> getTraceType (const std::string& str) {

        

    }

    void parse_lbr_perf_data (
        std::vector<perfParser::LbrSample> &tlbMissesSamples,
        std::vector<perfParser::LbrSample> &cyclesSamples,
        const char *perf_script_path)
    {
        perfParser::TraceStream traceReader (perf_script_path);
        std::vector<std::pair<std::string, std::string>>
            lbrSamplesPreRecord;

        while (!traceReader.isAtEOF ()) {
            while (traceReader.getCurrentLine ().empty ())
                traceReader.advance ();

            std::string curStr =
                boost::trim_copy (traceReader.getCurrentLine ());
            auto [traceNoEvent, traceType] = getTraceType (curStr);

            traceReader.advance ();
        }

    }

}  // namespace perfParser