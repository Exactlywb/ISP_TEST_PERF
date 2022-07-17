#ifndef _ALGORITHM_C3_REORDER__
#define _ALGORITHM_C3_REORDER__

#include <fstream>

#include "funcData.hpp"
#include "nmParser.hpp"
#include "perfParser.hpp"

namespace FunctionReordering {

    class C3Reorder final {
        const char *nmPath_;
        const char *perfPath_;

        const char *resPath_;

    public:
        C3Reorder (const char *nmPath,
                   const char *perfPath,
                   const char *resPath = "out.txt")
            : nmPath_ (nmPath), perfPath_ (perfPath), resPath_ (resPath)
        {
        }

        void run ();

    private:
        std::vector<perfParser::LbrSample> tlbMissesSamples_;
        std::vector<perfParser::LbrSample> cyclesSample_;

        std::vector<nmParser::nmFuncInfo> nmFunctions_;

        std::vector<HFData::edge *> edges_;
        std::vector<HFData::node> nodes_;

        void build_cg ();
    };

}  // namespace FunctionReordering

#endif