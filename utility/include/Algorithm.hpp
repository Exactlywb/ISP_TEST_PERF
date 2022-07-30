#ifndef _ALGORITHM_C3_REORDER__
#define _ALGORITHM_C3_REORDER__

#include <fstream>

#include "Algos.hpp"
#include "FuncData.hpp"
#include "Metrics.hpp"
#include "NMParser.hpp"
#include "PerfParser.hpp"

namespace FunctionReordering {

class C3Reorder final {
    const std::string command_;
    const char *nmPath_;
    const char *resPath_;
    int runs_;
    int delta_;
    //! TODO write destructor
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
    //   cluster_reordering_ (new GlobalAlgorithms::Annealing<RegularClusterMetric> ())
    {
        local_reordering_.push_back (new LocalAlgorithms::Percise ());
        local_reordering_.push_back (new LocalAlgorithms::Annealing<RegularFunctionMetric> ());
        local_reordering_.push_back (new LocalAlgorithms::QAP ());
    }

    void run ();

private:
    std::vector<LocalAlgorithms::ReorderAlgorithm *> local_reordering_;
    GlobalAlgorithms::ReorderAlgorithm
        *cluster_reordering_; /* Let's try another cluster sort.  */

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
