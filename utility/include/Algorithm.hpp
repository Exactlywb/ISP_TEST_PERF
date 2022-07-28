#ifndef _ALGORITHM_C3_REORDER__
#define _ALGORITHM_C3_REORDER__

#include <fstream>

#include "Algos.hpp"
#include "FuncData.hpp"
#include "NMParser.hpp"
#include "PerfParser.hpp"

namespace FunctionReordering {

class C3Reorder final {
    const std::string command_;
    const char *nmPath_;
    const char *resPath_;
    int runs_;
    int delta_;

public:
    C3Reorder (const std::string &command,
               const char *nmPath,
               const char *resPath = "out.txt",
               const int runs = 0,
               const int delta = 0)
        : command_ (command),
          nmPath_ (nmPath),
          resPath_ (resPath),
          runs_ (runs),
          delta_ (delta)
    {
        local_reordering_.push_back (new Percise ());
        local_reordering_.push_back (new Annealing ());
        local_reordering_.push_back (new QAP ());
    }

    void run ();

private:
    std::vector<ReorderAlorithm *> local_reordering_;

    std::vector<perfParser::LbrSample> tlbMissesSamples_;
    std::vector<perfParser::LbrSample> cyclesSample_;

    std::vector<NMParser::FuncInfo> nmFunctions_;

    std::vector<HFData::edge *> edges_;
    std::vector<HFData::node> nodes_;

    void build_cg ();

    using pointer_pair = std::pair<HFData::node *, HFData::node *>;
    void build_edges_cg (std::unordered_map<std::string, HFData::node *> &f2n);
};

}  // namespace FunctionReordering

#endif
