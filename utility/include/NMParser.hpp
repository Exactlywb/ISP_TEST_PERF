#ifndef _NM_PARSER_HPP__
#define _NM_PARSER_HPP__

#include <iostream>
#include <vector>

#include "TraceReader.hpp"

namespace NMParser {

struct FuncInfo {
    std::string name_;
    uint64_t size_;
    uint64_t interal_addr_;
    uint64_t ID;
};

void parse_nm_data (std::vector<FuncInfo> &nmFunctionInfo, const char *nmPath);

}  // namespace NMParser

#endif
