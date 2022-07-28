#include "Algos.hpp"

namespace {

double DecreaseTemperature (const double initT, const int iteration)
{
    constexpr double coef = 1.0;
    return (initT * coef) / iteration;
}

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

void Reorder (std::vector<HFData::node *> &functions_, const std::vector<int> &state)
{
    std::vector<HFData::node *> copy_functions = functions_;
    for (auto ind : state)
        functions_[ind] = copy_functions[ind];
}

}  // namespace

bool Annealing::runOncluster (HFData::cluster &cluster)
{
    auto &functions = cluster.functions_;

    std::ofstream fp ("annealing.dump", std::ios_base::app);

    auto fSize = functions.size ();
    std::vector<int> state (fSize);
    RandPerm (state, fSize);
    fp << "Before cost = " << cluster.evaluate_energy (state) << ", " << fSize << std::endl;

    double currentEnergy = cluster.evaluate_energy (state);  // start energy
    constexpr double initT = 10.;                            // initial temperature
    double curT = initT;                                     // start temperature
    constexpr double minT = 0.0001;                          // minimum temperature

    std::vector<int> best_state (fSize);
    double best_score = 1e+15;

    for (int i = 0; i < 100000; ++i) {  // just in case, we limit the number of iterations

        std::vector<int> stateCandidate = state;
        GenerateStateCandidate (stateCandidate);
        double candidateEnergy = cluster.evaluate_energy (stateCandidate);

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
    Reorder (functions, state);

    fp << "*****************" << std::endl;
    for (auto f : functions) {
        fp << f->name_ << " -> ";
    }
    fp << std::endl;
    fp << "Result cost = " << cluster.evaluate_energy (state) << std::endl;
    fp << "*****************" << std::endl << std::endl;

    fp.close ();

    return true;
}
