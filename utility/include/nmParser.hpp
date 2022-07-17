#ifndef _NM_PARSER_HPP__
#define _NM_PARSER_HPP__

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <vector>

#include "traceReader.hpp"

namespace nmParser {

    struct nmFuncInfo {
        std::string name_;
        std::size_t size_;
    };

    void parse_nm_data (std::vector<nmFuncInfo> &nmFunctionInfo,
                        const char *nmPath);

}  // namespace nmParser

#endif
