#include "../include/funcData.hpp"

namespace HFData {

    void FuncInfo::addCall (FuncInfo *callee,
                            const std::uint64_t offset)
    {
        auto it = callees_.find (callee);
        if (it != callees_.end ()) {
            auto offsetIt = (*it).second.find (offset);
            if (offsetIt != (*it).second.end ()) {
                (*offsetIt).second++;
                return;
            }

            (*it).second.insert ({offset, 1});

            return;
        }

        callees_.insert ({callee, {{offset, 1}}});
    }

    FuncInfoTbl::FuncInfoTbl (
        const std::vector<perfParser::LbrSample> &samples)
    {
        FillTable (tbl_, samples);

        for (const auto& sample : samples) {
            
            auto it = tbl_.find (sample.callerName_);

            if (it != tbl_.end ()) {

                auto key = tbl_.at (sample.calleeName_);
                (*it).second->addCall (key, sample.callerOffset_);

            } else
                throw std::runtime_error ("Bad table fill in FillTable () function");

        }
    }

    void FuncInfoTbl::textDump () const
    {

        for (const auto& el: tbl_) {

            std::cerr << "(" << el.first << ", " << el.second << ") --> ";
            for (const auto& call: el.second->getCalleesSlow ())
            {
                
                std::cerr << call.first->getFuncName () << "(" << call.first << ") : ";
                for (const auto& callInfo: call.second)
                    std::cerr << "0x" << std::hex << callInfo.first << ", " << std::dec << callInfo.second << " | ";

            }

            std::cerr << "$" << std::endl;

        }

    }

}  // namespace HFData
