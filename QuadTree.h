#pragma once 
#include <vector>
#include <random>
#include "Node.h"
using namespace std;

class QuadTreeSim{
public:
    class Counts { // stores counts for SEIR at a time step
    public:
        int S, E, I, R;
        Counts(); // default constr
    };
    QuadTreeSim(int N, // total node num
                float domainW, // width of 2D domain for node placement
                float domainH, // height of 2D domain for node placement
                float interactionR, // infection interaction radius 
                float infectProbPerContact, // beta, infection probability 
                float incubationProbPerStep, // alpha, prob E->I
                float recoveryProbPerStep, // gamma, prob I->R
                int rngSeed = random_device{}()); // seed for RNG
    void seedInfectedPercent(float percent, bool ensureAtLeastOnce = true); // infect % of nodes, unique random ids
    void seedInfectedCount(int k); // infect k unique random ids
    void seedInfected(vector<int> &ids); // infect explicit ids
    Counts step(float t); // advance simul one step at a time t
    vector<Counts> runToExtinction(); // run steps until no infectious left
};
