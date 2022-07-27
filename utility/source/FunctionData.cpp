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

namespace {

void RandPerm (std::vector<int> &state, const std::size_t fSize)
{
    for (std::size_t i = 0; i < fSize; ++i)
        state[i] = i;

    std::random_shuffle (state.begin (), state.end ());
}

void GenerateStateCandidate (std::vector<int> &stateCandidate)
{
    auto size = stateCandidate.size ();

    std::random_device random_device;
    std::mt19937 generator (random_device ());

    std::uniform_int_distribution<> distribution (0, size - 1);

    int range_first_ind = distribution (generator);
    int range_second_ind = distribution (generator);
    // std::cerr << ":::::" << range_first_ind << std::endl;
    // std::cerr << ":::::" << range_second_ind << std::endl;

    auto range_first_it = stateCandidate.begin ();
    std::advance (range_first_it, range_first_ind);
    //     std::cerr << "range_first_ind: " << range_first_ind << std::endl;
    auto range_second_it = stateCandidate.begin ();
    std::advance (range_second_it, range_second_ind);
    //     std::cerr << "range_second_it: " << range_second_ind << std::endl;

    //     std::cerr << "stateCandidate (before): ";
    //     for (auto s: stateCandidate)
    //         std::cerr << s << ", ";

    if (range_first_ind > range_second_ind)
        std::reverse (range_second_it, range_first_it);
    else
        std::reverse (range_first_it, range_second_it);

    //     std::cerr << "stateCandidate (after): ";
    //     for (auto s: stateCandidate)
    //         std::cerr << s << ", ";
}

double GetTransitionProbability (const double energy, const double temperature)
{
    return std::exp (-energy / temperature);  // Boltzmann's function
}

bool MakeTransit (const double probability)
{
    std::random_device rd;
    std::mt19937 gen (rd ());
    auto pivot = std::generate_canonical<double, 10> (gen);  //! TODO is this correct?

    return pivot <= probability;
    /*

    Here we're trying to check transit
    |----------------------|-----------|
    0                      P           1
    <--- make transit ---> * <- none ->

    Where P is a random pivot

    */
}

double DecreaseTemperature (const double initT, const int iteration)
{
    constexpr double coef = 1.0;
    return (initT * coef) / iteration;
}

void Reorder (std::vector<HFData::node *> &m_functions, const std::vector<int> &state)
{
    std::vector<HFData::node *> copy_functions = m_functions;
    for (auto ind : state)
        m_functions[ind] = copy_functions[ind];
}

}  // namespace

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

bool cluster::try_simulated_annealing ()
{
    std::ofstream fp ("annealing.dump", std::ios_base::app);

    auto fSize = m_functions.size ();
    std::vector<int> state (fSize);
    RandPerm (state, fSize);
    fp << "Before cost = " << evaluate_energy (state) << ", " << fSize << std::endl;

    double currentEnergy = evaluate_energy (state);  // start energy
    constexpr double initT = 10.;                    // initial temperature
    double curT = initT;                             // start temperature
    constexpr double minT = 0.0001;                    // minimum temperature

    std::vector<int> best_state(fSize);
    double best_score = 1e+15;

    for (int i = 0; i < 100000; ++i) {  // just in case, we limit the number of iterations

        std::vector<int> stateCandidate = state;
        GenerateStateCandidate (stateCandidate);
        double candidateEnergy = evaluate_energy (stateCandidate);

        if (candidateEnergy < currentEnergy) {
            currentEnergy = candidateEnergy;
            state = stateCandidate;

            best_score = candidateEnergy;
            best_state = stateCandidate;
        }
        else {
            auto probability =
                GetTransitionProbability (candidateEnergy - currentEnergy,
                                          curT);  // else we calculate transition probability
            if (MakeTransit (probability)) {      // check for transition
                currentEnergy = candidateEnergy;
                state = stateCandidate;
            }
        }

        curT = DecreaseTemperature (initT, i);
        if (curT < minT)
            break;
    }

    if (best_score < currentEnergy) {
        state = best_state;
    }

    // Now we have the finish state and we have to make function reordering in cluster
    Reorder (m_functions, state);

    fp << "*****************" << std::endl;
    for (auto f : m_functions) {
        fp << f->name_ << " -> ";
    }
    fp << std::endl;
    fp << "Result cost = " << evaluate_energy (state) << std::endl;
    fp << "*****************" << std::endl << std::endl;

    fp.close ();

    return true;
}

bool cluster::try_best_reorder ()
{
    std::vector<size_t> dists (m_functions.size ());
    auto dist_for_func = [this, &dists] (node *f) -> int {
        for (std::size_t i = 0; i < m_functions.size (); i++) {
            if (m_functions[i] == f)
                return dists[i];
        }
        return m_size;
    };
    auto get_dist_for_call = [&] (node *caller, node *callee) -> int {
        return std::abs (dist_for_func (caller) + (int)caller->size_ / 2 -
                         dist_for_func (callee));
    };
    auto get_metric = [&] () -> double {
        dists[0] = 0;
        for (std::size_t i = 1; i < m_functions.size (); i++) {
            dists[i] = dists[i - 1] + m_functions[i]->size_;
        }

        double cur_metric = 0;
        for (auto node : m_functions) {
            for (auto e : node->callers) {
                auto caller = e->caller;
                auto callee = e->callee;
                if (callee->aux_ != this || caller->aux_ != this)
                    continue;
                cur_metric += e->freq * get_dist_for_call (caller, callee);
            }
        }
        return cur_metric;
    };

    if (m_functions.size () > 8 || m_functions.size () <= 1) {
        if (m_functions.size () <= 1)
            return false;
        else
            return try_simulated_annealing ();
    }

    // todo: need move functions with 0 size to the end of array

    double metric = get_metric ();
    std::cerr << "For cluster with n_funcs = " << m_functions.size () << "\n";
    std::cerr << " :::cur metric::: metric = " << metric << "\n";

    std::sort (m_functions.begin (), m_functions.end ());
    int idx = 0;
    int answ_prem = 0;
    do {
        auto cur_metric = get_metric ();
        if (cur_metric < metric && idx != 0) {
            metric = cur_metric;
            answ_prem = idx;
        }
        idx++;
    } while (std::next_permutation (m_functions.begin (), m_functions.end ()));

    std::cerr << " :::new metric::: metric = " << metric << "\n";

    std::sort (m_functions.begin (), m_functions.end ());
    while (answ_prem--) {
        std::next_permutation (m_functions.begin (), m_functions.end ());
    }
    std::cerr << " :::second try metric::: metric = " << get_metric () << "\n";

    return true;
}

}  // namespace HFData
