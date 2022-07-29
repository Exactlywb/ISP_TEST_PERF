
/* This file from another repository */


/*

Iterative Local Search: Quadratic Assignment Problem
====================================================

Implementation of iterative local search for the quadratic assignment problem. The QAP is a
strongly NP-hard problem, and solving instances with more than 20 variables is considered
intractable. The implementation uses a population of individuals and performs local improvement
on them. Local improvement accepts worse solutions with a probability of 0.0. The parameters of
the algorithm can be adjusted by changing the global variables. The algorithm can be used to
find approximate solutions for large QAP instances for which exact methods are not suitable.

Author: Shalin Shah
Email: shah.shalin@gmail.com

*/

#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <ctime>
#include <iostream>
#include <vector>
using namespace std;

/* Globals */
int **DISTANCE;      // populated in processData()
int **FLOW;          // populated in processData()
int OPTIMUM;         // can be populated if known
int NUMBER_OBJECTS;  // populated in processData()

/* Probability of accepting a worse solution in localImprovement */
const double WORSE_ACCEPTANCE_PROB = 0.0;
/* Number of Generations */
int GENERATIONS = 10000;
/* Number of local Improvements to perform */
const int LOCAL_IMPROVEMENT = 1000;
/* Size of the population */
const int POPULATION = 5;
/* If the global best does not change for 10 generations, the population is shuffled */
const int SHUFFLE_TOLERANCE = 10;
/* Number of exchanges to perform in a shuffle */
const int NUMBER_SHUFFLES = 15;

/* Generate a random number in [min, max] */
int generateRandomNumber (int min, int max)
{
    int r;
    double randd = ((double)rand () / ((double)(RAND_MAX) + (double)(1)));
    r = (int)(randd * (double)(max - min + 1)) + min;
    return r;
}

/* Generate a random number in [0, 1) */
double generateDoubleRandomNumber ()
{
    return ((double)rand () / ((double)(RAND_MAX) + (double)(1)));
}

/* A QAP solution */
class QNode {
private:
    /* Array of assignments - The index represents the facility and the value of that index
      represents the location at which the facility is assigned */
    int *assignments;

    /* The objective function value */
    int value;

public:
    /* Constructor */
    QNode () { assignments = NULL; }

    /* Destructor */
    ~QNode ()
    {
        if (assignments != NULL)
            delete[] assignments;

        assignments = NULL;
    }

    /* Constructor */
    QNode (int *as)
    {
        assignments = new int[NUMBER_OBJECTS];
        for (int i = 0; i < NUMBER_OBJECTS; i++) {
            assignments[i] = as[i];
            if (assignments[i] >= NUMBER_OBJECTS) {
                printf ("\n\nInvalid Data in Copy Constructor");
                exit (1);
            }
        }
        calculateValue ();
    }

    /* Assignment Operator */
    void operator= (const QNode &copy)
    {
        if (assignments != NULL)
            delete[] assignments;

        assignments = new int[NUMBER_OBJECTS];
        int *as = copy.getAssignments ();
        for (int i = 0; i < NUMBER_OBJECTS; i++) {
            assignments[i] = as[i];
        }

        calculateValue ();
    }

    /* Operator '<' Used by std::sort */
    bool operator< (const QNode &right) const
    {
        int av = this->getValue ();
        int bv = right.getValue ();
        return av < bv;
    }

    /* Operator '>' */
    bool operator> (const QNode &right) const
    {
        int av = this->getValue ();
        int bv = right.getValue ();
        return av > bv;
    }

    /* Exchange the assignments at indices i and j */
    void exchange (int i, int j)
    {
        calculateValueSwap (i, j);
        int temp = assignments[i];
        assignments[i] = assignments[j];
        assignments[j] = temp;
    }

    /* Calculate the objective function value for this solution */
    void calculateValue ()
    {
        value = 0;
        for (int i = 0; i < NUMBER_OBJECTS; i++) {
            int as = assignments[i];
            for (int j = 0; j < NUMBER_OBJECTS; j++) {
                int aas = assignments[j];
                int distance = DISTANCE[as][aas];
                int flow = FLOW[i][j];
                int vv = distance * flow;
                value += vv;
            }
        }
    }

    /* Calculate the changed value when facilities r and s are swapped (O(n)) */
    void calculateValueSwap (int r, int s)
    {
        int as = assignments[r];
        int aas = assignments[s];

        int delta = 0;

        delta = FLOW[r][r] * (DISTANCE[aas][aas] - DISTANCE[as][as]) +
                FLOW[r][s] * (DISTANCE[aas][as] - DISTANCE[as][aas]) +
                FLOW[s][r] * (DISTANCE[as][aas] - DISTANCE[aas][as]) +
                FLOW[s][s] * (DISTANCE[as][as] - DISTANCE[aas][aas]);

        for (int k = 0; k < NUMBER_OBJECTS; k++) {
            int ak = assignments[k];
            if (k != r && k != s) {
                delta = delta + (FLOW[k][r] * (DISTANCE[ak][aas] - DISTANCE[ak][as]) +
                                 FLOW[k][s] * (DISTANCE[ak][as] - DISTANCE[ak][aas]) +
                                 FLOW[r][k] * (DISTANCE[aas][ak] - DISTANCE[as][ak]) +
                                 FLOW[s][k] * (DISTANCE[as][ak] - DISTANCE[aas][ak]));
            }
        }

        value += delta;
    }

    /* Get the assignments of this solution */
    int *getAssignments () const { return assignments; }

    /* Get the objective function value of this solution */
    int getValue () const { return value; }

    /* Set the objective function value of this solution */
    void setValue (int v) { value = v; }

    /* Return a deep copied clone of this solution */
    QNode *clone ()
    {
        int *asn = new int[NUMBER_OBJECTS];
        for (int i = 0; i < NUMBER_OBJECTS; i++) {
            asn[i] = assignments[i];
        }

        QNode *clone = new QNode (asn);
        delete[] asn;
        return clone;
    }
};

/* Process the data from the file */
void processData (char *filename)
{
    FILE *file;
    file = fopen (filename, "r");
    if (file == NULL) {
        printf ("File Not Found in Current Directory.");
        exit (1);
    }

    char *line = new char[1000];
    fgets (line, 1000, file);
    while (strlen (line) <= 1) {
        fgets (line, 1000, file);
    }

    NUMBER_OBJECTS = atoi (line);
    fgets (line, 1000, file);

    int i = 0;
    char *tok = NULL;

    /* FLOW MATRIX */
    FLOW = new int *[NUMBER_OBJECTS];
    for (int n = 0; n < NUMBER_OBJECTS; n++) {
        i = 0;
        fgets (line, 1000, file);
        FLOW[n] = new int[NUMBER_OBJECTS];
        tok = strtok (line, " \t");
        int vv = atoi (tok);
        FLOW[n][i] = vv;
        i++;
        while (true) {
            tok = strtok (NULL, " \t");
            while (tok != NULL && i < NUMBER_OBJECTS) {
                vv = atoi (tok);
                FLOW[n][i] = vv;
                tok = strtok (NULL, " \t");
                i++;
                if (tok == NULL) {
                    break;
                }
                if (strcmp (tok, " ") == 0 || strcmp (tok, "\n") == 0) {
                    break;
                }
            }

            if (i < NUMBER_OBJECTS) {
                fgets (line, 1000, file);
                tok = strtok (line, " \t");
                int vv = atoi (tok);
                FLOW[n][i] = vv;
                i++;
            }
            else {
                break;
            }
        }
    }

    fgets (line, 1000, file);

    /* DISTANCE MATRIX */
    DISTANCE = new int *[NUMBER_OBJECTS];
    for (int n = 0; n < NUMBER_OBJECTS; n++) {
        i = 0;
        fgets (line, 1000, file);
        DISTANCE[n] = new int[NUMBER_OBJECTS];
        tok = strtok (line, " \t");
        int vv = atoi (tok);
        DISTANCE[n][i] = vv;
        i++;
        while (true) {
            tok = strtok (NULL, " \t");
            while (tok != NULL && i < NUMBER_OBJECTS) {
                vv = atoi (tok);
                DISTANCE[n][i] = vv;
                tok = strtok (NULL, " \t");
                i++;
                if (tok == NULL) {
                    break;
                }
                if (strcmp (tok, " ") == 0 || strcmp (tok, "\n") == 0) {
                    break;
                }
            }

            if (i < NUMBER_OBJECTS) {
                fgets (line, 1000, file);
                tok = strtok (line, " \t");
                int vv = atoi (tok);
                DISTANCE[n][i] = vv;
                i++;
            }
            else {
                break;
            }
        }
    }

    delete[] line;
}

/* Does the passed in array contain the passed in value? Used only while generating the
initial population */
bool contains (int *arr, int value)
{
    for (int i = 0; i < NUMBER_OBJECTS; i++) {
        if (arr[i] == value) {
            return true;
        }
    }

    return false;
}

/* Generate random initial population for the algorithm */
vector<QNode *> *generateRandomPopulation (int pop)
{
    vector<QNode *> *population = new vector<QNode *> ();
    population->reserve (pop);
    for (int i = 0; i < pop; i++) {
        int *assign = new int[NUMBER_OBJECTS];
        for (int n = 0; n < NUMBER_OBJECTS; n++) {
            assign[n] = -1;
        }

        for (int n = 0; n < NUMBER_OBJECTS; n++) {
            int rand = generateRandomNumber (0, NUMBER_OBJECTS - 1);
            while (contains (assign, rand)) {
                rand = generateRandomNumber (0, NUMBER_OBJECTS - 1);
            }

            assign[n] = rand;
        }

        QNode *node = new QNode (assign);
        delete[] assign;
        population->push_back (node);
    }

    return population;
}

/* Randomly shuffle the population */
void shufflePopulation (vector<QNode *> *population)
{
    for (int i = 0; i < POPULATION; i++) {
        QNode *node = population->at (i);
        for (int j = 0; j < NUMBER_SHUFFLES; j++) {
            int rand1 = generateRandomNumber (0, NUMBER_OBJECTS - 1);
            int rand2 = generateRandomNumber (0, NUMBER_OBJECTS - 1);
            while (rand1 == rand2) {
                rand1 = generateRandomNumber (0, NUMBER_OBJECTS - 1);
                rand2 = generateRandomNumber (0, NUMBER_OBJECTS - 1);
            }

            node->exchange (rand1, rand2);
        }

        node->calculateValue ();
    }
}

/* Functor used by std::sort */
struct DescendingSort {
    bool operator() (QNode *&start, QNode *&end)
    {
        return start->getValue () < end->getValue ();
    }
};

/* Quadratic 2-OPT improvement */
void twoOptImprovement (QNode *node)
{
    for (int i = 0; i < NUMBER_OBJECTS; i++) {
        for (int j = i + 1; j < NUMBER_OBJECTS; j++) {
            int value = node->getValue ();
            node->exchange (i, j);
            int newValue = node->getValue ();
            if (newValue > value) {
                double rand = generateDoubleRandomNumber ();
                if (rand > WORSE_ACCEPTANCE_PROB) {
                    node->exchange (i, j);
                }
            }
        }
    }
}

/* Randomized 3-OPT improvement */
void threeOptImprovement (QNode *node)
{
    for (int i = 0; i < LOCAL_IMPROVEMENT; i++) {
        int value = node->getValue ();
        int rand1 = generateRandomNumber (0, NUMBER_OBJECTS - 1);
        int rand2 = generateRandomNumber (0, NUMBER_OBJECTS - 1);
        int rand3 = generateRandomNumber (0, NUMBER_OBJECTS - 1);
        while (rand1 == rand2 || rand2 == rand3 || rand1 == rand3) {
            rand1 = generateRandomNumber (0, NUMBER_OBJECTS - 1);
            rand2 = generateRandomNumber (0, NUMBER_OBJECTS - 1);
            rand3 = generateRandomNumber (0, NUMBER_OBJECTS - 1);
        }

        node->exchange (rand1, rand2);
        node->exchange (rand2, rand3);

        // node->calculateValue();
        int newValue = node->getValue ();
        if (newValue > value) {
            double rand = generateDoubleRandomNumber ();
            if (rand > WORSE_ACCEPTANCE_PROB) {
                node->exchange (rand2, rand3);
                node->exchange (rand1, rand2);
                // node->setValue(value);
            }
        }
    }
}

/* Randomized 4-OPT improvement */
void fourOptImprovement (QNode *node)
{
    for (int i = 0; i < LOCAL_IMPROVEMENT; i++) {
        int value = node->getValue ();
        int rand1 = generateRandomNumber (0, NUMBER_OBJECTS - 1);
        int rand2 = generateRandomNumber (0, NUMBER_OBJECTS - 1);
        int rand3 = generateRandomNumber (0, NUMBER_OBJECTS - 1);
        int rand4 = generateRandomNumber (0, NUMBER_OBJECTS - 1);

        while (rand1 == rand2 || rand1 == rand3 || rand1 == rand4 || rand2 == rand3 ||
               rand2 == rand4 || rand3 == rand4) {
            rand1 = generateRandomNumber (0, NUMBER_OBJECTS - 1);
            rand2 = generateRandomNumber (0, NUMBER_OBJECTS - 1);
            rand3 = generateRandomNumber (0, NUMBER_OBJECTS - 1);
            rand4 = generateRandomNumber (0, NUMBER_OBJECTS - 1);
        }

        node->exchange (rand1, rand2);
        node->exchange (rand2, rand3);
        node->exchange (rand3, rand4);

        // node->calculateValue();
        int newValue = node->getValue ();
        if (newValue > value) {
            double rand = generateDoubleRandomNumber ();
            if (rand > WORSE_ACCEPTANCE_PROB) {
                node->exchange (rand3, rand4);
                node->exchange (rand2, rand3);
                node->exchange (rand1, rand2);
                // node->setValue(value);
            }
        }
    }
}

/* Generic function for local improvement.
    Calls 2,3 and 4 OPT improvement functions
*/
void localImprovement (QNode *node)
{
    twoOptImprovement (node);
    // threeOptImprovement(node);
    // fourOptImprovement(node);
}

/* Main */
int main (int argc, char *argv[])
{
    if (argc < 3) {
        printf ("\nUsage: ./a.out filename iterations\n\n");
        exit (0);
    }

    /* Initialize the random number generator */
    srand (time (NULL));

    /* Read the number of locations, flow and distance matrices from a file */
    processData (argv[1]);
    GENERATIONS = atoi (argv[2]);

    /* Generate random initial population */
    vector<QNode *> *population = generateRandomPopulation (POPULATION);

    /* Sort the population and find the global best individual */
    sort (population->begin (), population->end (), DescendingSort ());
    QNode *gBest = population->at (0)->clone ();
    int value = gBest->getValue ();

    int count = 0;
    for (int i = 0; i < GENERATIONS; i++) {
        std::cerr << i << "\n";
        sort (population->begin (), population->end (), DescendingSort ());
        QNode *newBest = population->at (0);
        int newValue = newBest->getValue ();

        if (newBest->getValue () < gBest->getValue ()) {
            delete gBest;
            gBest = newBest->clone ();
            count = 0;
        }
        else {
            count++;
            if (count > SHUFFLE_TOLERANCE) {
                shufflePopulation (population);
                count = 0;
            }
        }

        // printf("\n%d : %d", i, gBest->getValue());
        for (int j = 0; j < POPULATION; j++) {
            QNode *node = population->at (j);
            localImprovement (node);
        }
    }

    /* Print the final assignments */
    int *assignments = gBest->getAssignments ();
    for (int i = 0; i < NUMBER_OBJECTS; i++) {
        std::cout << assignments[i] << " ";
    }

    for (int i = 0; i < POPULATION; i++) {
        QNode *node = population->at (i);
        delete node;
    }

    delete population;

    for (int i = 0; i < NUMBER_OBJECTS; i++) {
        delete[] DISTANCE[i];
        delete[] FLOW[i];
    }

    delete[] DISTANCE;
    delete[] FLOW;

    return 0;
}
