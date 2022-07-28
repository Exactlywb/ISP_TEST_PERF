#include "FuncData.hpp"

struct ReorderAlorithm {
    // returns true if algo runs
    virtual bool runOncluster (HFData::cluster &cluster) = 0;
};

struct Percise final : public ReorderAlorithm {
    bool runOncluster (HFData::cluster &cluster) override;
};

struct Annealing final : public ReorderAlorithm {
    bool runOncluster (HFData::cluster &cluster) override;
};

struct QAP final : public ReorderAlorithm {
    bool runOncluster (HFData::cluster &cluster) override;
};