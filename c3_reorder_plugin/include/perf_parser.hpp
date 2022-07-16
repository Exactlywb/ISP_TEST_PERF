#ifndef PERF_PARSER_HPP__
#define PERF_PARSER_HPP__

#include <boost/algorithm/string.hpp>
#include <boost/convert.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
#include <unordered_map>

#include "sysWrapper.hpp"

namespace perfParser {

    class TraceStream final {
        std::string currentLine_;
        std::ifstream perfFile_;

        bool isAtEOF_ = false;
        uint64_t lineNumber_ = 0;

    public:
        TraceStream (const std::string &fileName)
            : perfFile_ (fileName)
        {
            if (!perfFile_.is_open ())
                throw std::runtime_error (
                    "Can't open perf script file");

            advance ();
        }

        bool isAtEOF () const { return isAtEOF_; }
        std::string getCurrentLine () const
        {
            if (isAtEOF_)
                throw std::runtime_error (
                    "Line iterator reaches the End-of-File!");
            return currentLine_;
        }

        uint64_t getLineNumber () const { return lineNumber_; }

        void advance ()
        {
            std::string curLine;
            if (!std::getline (perfFile_, curLine)) {
                isAtEOF_ = true;
                return;
            }

            currentLine_ = curLine;
            lineNumber_++;
        }

        ~TraceStream () { perfFile_.close (); }
    };

    enum PerfContent {

        Unknown,
        LBR,
        LBRStack

    };

    bool isLBRSample (const std::string &str);
    PerfContent checkPerfScriptType (const char *perf_script_path);

    struct LbrSample {
        std::uint64_t callerOffset_;

        std::string callerName_;
        std::string calleeName_;

        LbrSample (const std::string &src, const std::string &dst);
    };

    void lbrSampleReParse (
        std::vector<LbrSample> &res,
        const std::vector<std::pair<std::string, std::string>> &pre);

    void lbrPreParse (
        std::vector<std::pair<std::string, std::string>> &preRecord,
        const std::string &str);

}  // namespace perfParser

#endif
