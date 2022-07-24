#include <funcData.hpp>

namespace HFData {

constexpr int CallerDegradeFactor = 8;

void cluster::merge_to_caller(cluster *caller, cluster *callee) {

        const double newDensity = ((double)caller->m_freq + (double)callee->m_freq) / 
                                  ((double)caller->m_size + (double)callee->m_size);
        const double predDensity = ((double)caller->m_freq) / ((double)caller->m_size);
        if (predDensity > newDensity * CallerDegradeFactor)
                return;

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

bool cluster::try_best_reorder()
{
    // cant check all permutation
    if (m_functions.size() > 8 || m_functions.size() <= 1) return false;


    std::vector<size_t> dists(m_functions.size());
    auto dist_for_func = [this, &dists](node *f) -> int{
        for(int i = 0; i < m_functions.size(); i++) {
            if(m_functions[i] == f) return dists[i];
        }
        return m_size;
    };
    auto get_dist_for_call = [&](node *caller, node *callee) -> int {
        return std::abs(dist_for_func(caller) + (int)caller->size_ / 2 - dist_for_func(callee));
    };
    auto get_metic = [&]() -> double {
        dists[0] = 0;
        for(int i = 1; i < m_functions.size(); i++) {
            dists[i] = dists[i - 1] + m_functions[i]->size_;
        }

        double cur_metric = 0;
        for (auto node : m_functions) {
            for (auto e : node->callers) {
                auto caller = e->caller;
                auto callee = e->callee;
                if (callee->aux_ != this || caller->aux_ != this) continue;
                cur_metric += e->freq / get_dist_for_call(caller, callee);
            }
        }
        return cur_metric;
    };

    // todo: need move functions with 0 size to the end of array

    double metric = get_metic();
    std::cerr << "For cluster with n_funcs = " << m_functions.size() << "\n";
    std::cerr << " :::cur metric::: metric = " << metric << "\n";

    std::sort(m_functions.begin(), m_functions.end());
    int idx = 0;
    int answ_prem = 0;
    do {
        auto cur_metric = get_metic();
        if (cur_metric < metric && idx != 0) {
            metric = cur_metric;
            answ_prem = idx;
        }
        idx++;
    }while (std::next_permutation(m_functions.begin(), m_functions.end()));

    std::cerr << " :::new metric::: metric = " << metric << "\n";

    std::sort(m_functions.begin(), m_functions.end());
    while(answ_prem--) {
        std::next_permutation(m_functions.begin(), m_functions.end());
    }
    std::cerr << " :::second try metric::: metric = " << get_metic() << "\n";

    return true;
}

}
