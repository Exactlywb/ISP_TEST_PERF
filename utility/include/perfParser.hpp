#ifndef PERF_PARSER_HPP__
#define PERF_PARSER_HPP__

#include <boost/algorithm/string.hpp>
#include <boost/convert.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
#include <tuple>
#include <unordered_map>

#include "traceReader.hpp"

namespace perfParser {

enum class PerfContent {

    Unknown,
    LBR,
    LBRStack

};

enum class LbrTraceType {

    Unknown,
    TLB_MISS,
    CYCLE,
    EmptyEvent

};

struct LbrSample {
    std::string callerName_;
    std::string calleeName_;

    LbrTraceType type_;

    LbrSample (const std::string &sample, const LbrTraceType &type);
};

void parse_lbr_perf_data (std::vector<perfParser::LbrSample> &tlbMissesSamples,
                          std::vector<perfParser::LbrSample> &cyclesSamples,
                          const char *perf_script_path);

std::tuple<std::string, LbrTraceType> getTraceType (const std::string &str);

}  // namespace perfParser

#endif