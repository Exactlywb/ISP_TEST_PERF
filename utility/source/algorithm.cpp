#include "algorithm.hpp"

#include <vector>

namespace FunctionReordering {

    void C3Reorder::build_cg ()
    {
        /* Declare node for each function, readed from final binary */
        for (const auto &func : nmFunctions_) {
            nodes_.push_back ({func.name_, func.size_, nullptr});
        }

        /* Make map for fast searching node by the function name */
        std::unordered_map<std::string, HFData::node *> f2n;  // func to node
        for (auto &node : nodes_) {
            f2n[node.name_] = &node;
        }

        /* Iterate over all lbr samples and increment edge counter */
        using pointer_pair = std::pair<HFData::node *, HFData::node *>;
        std::map<pointer_pair, HFData::edge *> f2e;  // pair of funcs to edge
        for (auto &e : tlbMissesSamples_) {
            if (e.type_ != perfParser::LbrTraceType::TLB_MISS)
                continue;
            const auto &callee = e.calleeName_;
            const auto &caller = e.callerName_;

            if(f2n.find(caller) == f2n.end() || f2n.find(callee) == f2n.end()) {
                continue;
            }

            auto serach_it = f2e.find ({f2n[caller], f2n[callee]});
            if (serach_it != f2e.end ()) {
                serach_it->second->count++;
            }
            else {
                auto edge = new HFData::edge;
                edge->caller = f2n[caller];
                edge->callee = f2n[callee];
                edge->count = 1;
                edges_.push_back (edge);
                f2e[{f2n[caller], f2n[callee]}] = edge;
            }
        }

        /* After each edges was created, attach it no nodes */
        for (auto e : edges_) {
            e->callee->callers.push_back (e);
        }
    }

    void C3Reorder::run ()
    {
        perfParser::parse_lbr_perf_data (
            tlbMissesSamples_,
            cyclesSample_,
            perfPath_);  //! TODO check perf file for event + tlbMisses
        nmParser::parse_nm_data (nmFunctions_, nmPath_);

        /* Build cg from perf data */
        build_cg ();

        /* Create a cluster for each function.  */
        std::vector<HFData::cluster *> clusters;
        for (auto &node : nodes_) {
            auto c = new HFData::cluster (&node, node.size_, 0u, 0u);
            node.aux_ = c;
            clusters.push_back (c);
        }

        /* Insert edges_ between clusters that have a profile.  */
        std::vector<HFData::cluster_edge *> edges;
        for (int i = 0; i < clusters.size (); i++) {
            auto node = clusters[i]->m_functions[0];
            for (auto &cs : node->callers) {
                auto caller = (HFData::cluster *)cs->caller->aux_;
                auto callee = (HFData::cluster *)cs->callee->aux_;
                auto count = cs->count;

                auto cedge = callee->get (caller);
                if (cedge != NULL)
                    cedge->m_count += count;
                else {
                    auto cedge =
                        new HFData::cluster_edge (caller, callee, count);
                    edges.push_back (cedge);
                    callee->put (caller, cedge);
                }
            }
        }

        /* Now insert all created edges into a heap.  */
        // in original code we extract min from heap, and use inverted count
        std::vector<HFData::cluster_edge *> heap(edges);

        auto edge_cmp = [](const HFData::cluster_edge *l, const HFData::cluster_edge *r) {
            return l->m_count < r->m_count;
        };

        /* Main loop */
        while (!heap.empty()) {
            std::sort(heap.begin(), heap.end(), edge_cmp);
            auto cedge = heap.back(); // extarct edge with max weigth
            heap.pop_back();

            auto caller = cedge->m_caller;
            auto callee = cedge->m_callee;

            if (caller == callee)
    	        continue;
            if (caller->m_size + callee->m_size <= HFData::C3_CLUSTER_THRESHOLD)
    	    {
                caller->m_size += callee->m_size;
                //caller->m_time += callee->m_time;

                /* Append all cgraph_nodes from callee to caller.  */
                for (unsigned i = 0; i < callee->m_functions.size (); i++)
                    caller->m_functions.push_back (callee->m_functions[i]);

                callee->m_functions.clear();

                /* Iterate all cluster_edges of callee and add them to the caller. */
                for (auto &it : callee->m_callers) {
                    it.second->m_callee = caller;
                    auto ce = caller->get(it.first);

                    if (ce != nullptr)
                        ce->m_count += it.second->m_count;
                    else
                        caller->put(it.first, it.second);
                }
            }
        }

        for (auto it = clusters.begin (); it != clusters.end (); it++) {
            if(*it == nullptr)
            std::cout << (size_t)*it << "\n";
        }

        /* Sort the candidate clusters.  */
        std::sort (clusters.begin (),
                   clusters.end (),
                   [&] (HFData::cluster *a, HFData::cluster *b) -> bool {
                       auto fncounta = a->m_functions.size ();
                       auto fncountb = b->m_functions.size ();
                       if (fncounta <= 1 || fncountb <= 1)
                           return fncountb < fncounta;

                       // what is m_time? ....
                       // sreal r = b->m_time * a->m_size - a->m_time *
                       return a->m_freq * a->m_size < b->m_freq * b->m_size;
                   });

        /* Dump function order */
        for (auto &c : clusters) {
            for (auto &func : c->m_functions) {
                std::cerr << func->name_ << '\n';
            }
        }

        /* Release memory */
        for (auto &it : clusters)
            delete it;
        for (auto &it : edges_)
            delete it;
    }

}  // namespace FunctionReordering
