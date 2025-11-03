#pragma once
#include <vector>
#include <random>
#include "Node.h"

class Grid{
    private:
    int N_, W_, H_; //counts for building grid
    std::vector<Node> nodes_; 
    std::mt19937 rng_; 
    std::uniform_real_distribution<float> U_{0.0f, 1.0f};  //random number generator

    float beta_, alpha_, gamma_;

    void buildGrid(int N);
    void connectNeighbors4();
    inline int index(int x, int y) {return y* W_ + x;}
    inline bool validID(int id) {return id>= 0 && id < N_;}

    public:
    struct Counts{int S, E, I, R;}; //count of nodes in each state

    Grid(int N,
        float infectionProbability,  //beta
        float incubationProbability //alpha
        float recoveryProbability, //gamma
        unsigned int rngSeed= random_device{}());

    //Seeding infected nodes
    void seedInfectedPercent(float percent, bool atLeastOne = True);
    void seedInfectedCount(int k);

    //simulating
    Counts step(float t); //step through once
    std::vector<Counts> runTillEnd(); //return history of counts

    //getters
    std::vector<Nodes>& nodes() const {return nodes_};
    int width const {return W_};
    int height const {return H_};

}