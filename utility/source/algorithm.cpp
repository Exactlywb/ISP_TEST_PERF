#include "algorithm.hpp"

namespace FunctionReordering {

    void C3Reorder::run ()
    {
        perfParser::parse_lbr_perf_data (
            tlbMissesSamples_,
            cyclesSample_,
            perfPath_);  //! TODO check perf file for event + tlbMisses
        nmParser::parse_nm_data (nmFunctions_, nmPath_);


        
    }

}  // namespace FunctionReordering
