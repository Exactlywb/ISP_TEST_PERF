#include <funcData.hpp>

std::vector<long long> graps(std::vector<std::vector<long long>> weigth);


namespace HFData {

void cluster::merge_to_caller (cluster *caller, cluster *callee)
{
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

double cluster::evaluate_energy (const std::vector<int> &state)
{  //! TODO check it
    std::vector<size_t> dists (m_functions.size ());
    auto dist_for_func = [this, &dists, state] (node *f) -> int {
        for (std::size_t i = 0; i < m_functions.size (); i++) {
            if (m_functions[state[i]] == f)
                return dists[state[i]];
        }
        return m_size;
    };

    auto get_dist_for_call = [&] (node *caller, node *callee) -> int {
        return std::abs (dist_for_func (caller) + (int)caller->size_ / 2 -
                         dist_for_func (callee));
    };

    dists[state[0]] = 0;
    for (std::size_t i = 1; i < m_functions.size (); i++) {
        dists[state[i]] = dists[state[i - 1]] + m_functions[state[i]]->size_;
    }

    double cur_metric = 0;
    for (auto node : m_functions) {
        for (auto e : node->callers) {
            auto caller = e->caller;
            auto callee = e->callee;
            if (callee->aux_ != this || caller->aux_ != this)
                continue;
            cur_metric += e->freq * (get_dist_for_call (caller, callee));
        }
    }
    return cur_metric;
}

}  // namespace HFData
