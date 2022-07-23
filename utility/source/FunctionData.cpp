#include <funcData.hpp>

namespace HFData {

void cluster::merge_to_caller(cluster *caller, cluster *callee) {
    caller->m_size += callee->m_size;
    caller->m_freq += callee->m_freq;
    caller->m_miss += callee->m_miss;

    /* Append all cgraph_nodes from callee to caller.  */
    for (unsigned i = 0; i < callee->m_functions.size (); i++) {
        caller->m_functions.push_back (callee->m_functions[i]);
    }

    callee->m_functions.clear ();
    callee->m_size = 0;
    callee->m_freq = 0;
    callee->m_miss = 0;

    /* Iterate all cluster_edges of callee and add them to the caller. */
    for (auto &it : callee->m_callers) {
        it.second->m_callee = caller;
        auto ce = caller->get (it.first);

        if (ce != nullptr) {
            ce->m_count += it.second->m_count;
            ce->m_miss += it.second->m_miss;
        }
        else
            caller->put (it.first, it.second);
    }

    callee->m_callers.clear ();
}


}
