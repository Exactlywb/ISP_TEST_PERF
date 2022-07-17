#include "algorithm.hpp"

namespace FunctionReordering {

    void C3Reorder::run ()
    {
        perfParser::parse_lbr_perf_data (
            tlbMissesSamples_,
            cyclesSample_,
            perfPath_);  //! TODO check perf file for event + tlbMisses
        // fillTbl (perfFuncTbl_, lbrSamples_.begin (), lbrSamples_.end ());
        nmParser::parse_nm_data (nmFunctions_, nmPath_);

        std::cerr << tlbMissesSamples_.size () << std::endl;
        std::cerr << cyclesSample_.size () << std::endl;

        for (auto v : nmFunctions_)
            std::cerr << v.name_ << ", size = " << v.size_ << std::endl;
        //! TODO
    }

}  // namespace FunctionReordering
