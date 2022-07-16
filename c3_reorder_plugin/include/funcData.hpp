#ifndef FUNC_DATA_HPP__
#define FUNC_DATA_HPP__

#include <iostream>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "perf_parser.hpp"

namespace HFData {

    class FuncInfo final {
        using calls =
            std::unordered_map<std::uint64_t,
                               std::size_t>;  // offset + occurance
        std::unordered_map<FuncInfo *, calls>
            callees_;  // our + 0xoffset -> another
                       // (std::size_t is occurance)

        std::string funcName_;

        int funcSize_ = -1;  //! TODO

    public:
        FuncInfo (const std::string &funcName) : funcName_ (funcName)
        {
        }

        std::unordered_map<FuncInfo *, calls> getCalleesSlow () const {
            return callees_;
        }

        void addCall (FuncInfo *callee, const std::uint64_t offset);

        std::string getFuncName () const { return funcName_; }

        void setFuncSize (const int funcSize)
        {
            funcSize_ = funcSize;
        }
        int getFuncSize () const { return funcSize_; }
    };

    class FuncInfoTbl final {
        std::unordered_map<std::string, FuncInfo *> tbl_;

    public:
        FuncInfoTbl (
            const std::vector<perfParser::LbrSample> &samples);

        using tblIt = std::unordered_map<std::string,
                                         FuncInfo *>::const_iterator;

        tblIt lookup (const std::string &str) const
        {
            return tbl_.find (str);
        }

        tblIt begin () const { return tbl_.begin (); }
        tblIt end () const { return tbl_.end (); }

        std::size_t size () const { return tbl_.size (); }

        static void FillTable (
            std::unordered_map<std::string, FuncInfo *> &tbl,
            const std::vector<perfParser::LbrSample> &samples)
        {
            std::set<std::string>
                funcNames;  //! TODO static functions in different
                            //! files
            for (const auto& sample : samples) {
                funcNames.insert (sample.calleeName_);
                funcNames.insert (sample.callerName_);
            }

            for (const auto& name : funcNames)
                tbl.insert ({name, new FuncInfo (name)});
        }

        void textDump () const;
    };

}  // namespace HFData

#endif
