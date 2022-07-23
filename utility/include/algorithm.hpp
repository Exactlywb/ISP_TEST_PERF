#ifndef _ALGORITHM_C3_REORDER__
#define _ALGORITHM_C3_REORDER__

#include <fstream>

#include "funcData.hpp"
#include "nmParser.hpp"
#include "perfParser.hpp"

namespace FunctionReordering {

    class C3Reorder final {
        const std::string command_;
        const char *nmPath_;
        const char *resPath_;
        int runs_;

    public:
        C3Reorder (const std::string &command,
                   const char *nmPath,
                   const char *resPath = "out.txt",
                   const int runs = 0)
            : command_ (command),
              nmPath_ (nmPath),
              resPath_ (resPath),
              runs_ (runs)
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

        using pointer_pair = std::pair<HFData::node *, HFData::node *>;
        void build_edges_cg (
            std::unordered_map<std::string, HFData::node *> &f2n,
            const std::vector<perfParser::LbrSample> &samples,
            const perfParser::LbrTraceType type);
    };

}  // namespace FunctionReordering

#endif
