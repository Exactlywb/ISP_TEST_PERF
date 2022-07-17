#include "algorithm.hpp"

namespace FunctionReordering {

void C3Reorder::run () {

    perfParser::parse_lbr_perf_data (tlbMissesSamples_, cyclesSample_, perfPath_); //!TODO check perf file for event + tlbMisses
    // fillTbl (perfFuncTbl_, lbrSamples_.begin (), lbrSamples_.end ());

    //!TODO


}

}
