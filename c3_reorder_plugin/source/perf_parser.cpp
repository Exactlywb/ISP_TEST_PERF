#include "../include/perf_parser.hpp"

namespace perfParser {

    // A LBR sample is like:
    // 40062f 0x5c6313f/0x5c63170/P/-/-/0  0x5c630e7/0x5c63130/P/-/-/0
    // ... A heuristic for fast detection by checking whether a
    // leading "0x" and the '/' exist.
    bool isLBRSample (const std::string &str)
    {
        return boost::contains (str, "+0x") && boost::contains (str, "/");
    }

    PerfContent checkPerfScriptType (const char *perf_script_path)
    {
        perfParser::TraceStream traceReader (perf_script_path);

        while (!traceReader.isAtEOF ()) {
            while (!traceReader.getCurrentLine ().size ())
                traceReader.advance ();

            // Check for call stack
            int count = 0;
            int addr = 0;
            while (!traceReader.isAtEOF () &&
                   boost::conversion::try_lexical_convert<int> (
                       boost::trim_left_copy (
                           traceReader.getCurrentLine ()),
                       addr)) {
                count++;
                traceReader.advance ();
            }

            if (!traceReader.isAtEOF ()) {
                if (isLBRSample (traceReader.getCurrentLine ())) {
                    if (count > 0)
                        return PerfContent::LBRStack;
                    else
                        return PerfContent::LBR;
                }

                traceReader.advance ();
            }
        }

        return PerfContent::Unknown;
    }

namespace {

        std::string parseFuncNameTillSlash (const std::string &str,
                                            std::size_t *i)
        {
            const std::size_t beg = *i;
            std::size_t end = beg;

            const std::size_t strSize = str.size ();
            while (str[end] != '/' && end < strSize)
                ++end;

            std::string res (str, beg, end - beg);
            *i = end + 1;

            return res;
        }

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

    LbrSample::LbrSample (const std::string &src,
                          const std::string &dst)
    {
        std::vector<std::string> splitted;
        boost::split (splitted,
                      dst,
                      boost::is_any_of ("+"),
                      boost::token_compress_on);
        calleeName_ = cpp_filt (splitted[0].c_str ());

        boost::split (splitted,
                      src,
                      boost::is_any_of ("+"),
                      boost::token_compress_on);

        callerName_ = cpp_filt (splitted[0].c_str ());
        callerOffset_ =
            boost::lexical_cast<uint64_t_from_hex> (splitted[1]);
    }

    void lbrSampleReParse (
        std::vector<LbrSample> &res,
        const std::vector<std::pair<std::string, std::string>> &pre)
    {
        auto i = 0;
        auto preSize = pre.size ();
        while (i < preSize) {
            if (pre[i].first == "[unknown]" ||
                pre[i].second == "[unknown]") {
                ++i;
                continue;
            }

            LbrSample newLbr (pre[i].first, pre[i].second);
            res.push_back (newLbr);
            ++i;
        }
    }

    void lbrPreParse (
        std::vector<std::pair<std::string, std::string>> &preRecord,
        const std::string &str)
    {
        std::size_t strSize = str.size ();
        std::size_t i = 0;
        while (i < strSize) {
            std::string caller = parseFuncNameTillSlash (str, &i);
            std::string callee = parseFuncNameTillSlash (str, &i);

            while (str[i] != ' ' && i < strSize)
                ++i;

            preRecord.push_back ({caller, callee});

            ++i;
        }
    }

}  // namespace perfParser
