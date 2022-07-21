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

            friend std::istream &operator>> (std::istream &in,
                                             uint64_t_from_hex &outValue)
            {
                in >> std::hex >> outValue.value;
                return in;
            }
        };

    }  // namespace

    std::tuple<std::string, LbrTraceType> getTraceType (const std::string &str)
    {
        auto pos = str.find_first_of (' ');
        if (pos != std::string::npos) {
            std::string traceNoEvent = str.substr (pos + 1);
            boost::trim (traceNoEvent);
            if (boost::starts_with (str, "cycles"))
                return {traceNoEvent, LbrTraceType::CYCLE};
            else if (boost::starts_with (str, "iTLB-load-misses"))
                return {traceNoEvent, LbrTraceType::TLB_MISS};
            else
                return {traceNoEvent, LbrTraceType::Unknown};
        }
        else
            return {str, LbrTraceType::EmptyEvent};
    }

    namespace {

        void ParseCleanTrace (const std::string &trace,
                              const LbrTraceType &type,
                              std::vector<perfParser::LbrSample> &samples)
        {
            std::vector<std::string> splittedTrace;
            boost::split (splittedTrace,
                          trace,
                          boost::is_any_of (" "),
                          boost::token_compress_on);
            for (auto &sample : splittedTrace)
                 if (!boost::contains (sample, "[unknown]"))
                    samples.push_back ({boost::trim_copy (sample), type});
        }

    }  // namespace

    LbrSample::LbrSample (const std::string &sample, const LbrTraceType &type)
        : type_ (type)
    {
        std::vector<std::string> res;
        boost::split (
            res, sample, boost::is_any_of ("/"), boost::token_compress_on);

        auto findNul = res[0].find ("+0x");
        res[0].erase (findNul, res[0].length () - findNul);

        auto findFirst = res[1].find ("+0x");
        res[1].erase (findFirst, res[1].length () - findFirst);

        callerName_ = res[0];
        calleeName_ = res[1];
    }

    void parse_lbr_perf_data (
        std::vector<perfParser::LbrSample> &tlbMissesSamples,
        std::vector<perfParser::LbrSample> &cyclesSamples,
        const char *perf_script_path)
    {
        TraceStream traceReader (perf_script_path);

        while (!traceReader.isAtEOF ()) {
            while (traceReader.getCurrentLine ().empty ())
                traceReader.advance ();

            std::string curStr =
                boost::trim_copy (traceReader.getCurrentLine ());
            auto [traceNoEvent, traceType] = getTraceType (curStr);

            switch (traceType) {
                case LbrTraceType::CYCLE:
                    ParseCleanTrace (traceNoEvent, traceType, cyclesSamples);
                    break;
                case LbrTraceType::TLB_MISS:
                    ParseCleanTrace (traceNoEvent, traceType, tlbMissesSamples);
                    break;
                case LbrTraceType::Unknown:
                    throw std::runtime_error ("Uknown perf file format");
                case LbrTraceType::EmptyEvent: break;
            }

            traceReader.advance ();
        }
    }

}  // namespace perfParser
