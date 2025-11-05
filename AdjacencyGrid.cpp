#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>
#include <queue>
#include <algorithm>
#include <cmath>
#include <numeric>
#include "AdjacencyGrid.h"

using namespace std;

Grid::Grid (int N, float infectionProbability, float incubationProbability, float recoveryProbability, unsigned int rngSeed)
: N_(N), rng_(rngSeed), beta_(infectionProbability), alpha_(incubationProbability), gamma_(recoveryProbability){
buildGrid(N_);
connectNeighbors4();
}

void Grid::buildGrid(int N){
    //make it as square as possible
    W_ = max(1, (int)floor(sqrt((double)N)));
    H_ = (int)ceil((double)N / (double)W_);

    //memory optimization
    nodes_.reserve(N_);

    //initialize Nodes
    for (int i=0; i < N_; i++){
        nodes_.emplace_back(i, 'S', beta_, alpha_, gamma_);
    }
}

void Grid::connectNeighbors4(){
    auto coords = [&](int id)
    { return pair<int, int>(id % W_, id / W_); };

    for (int id = 0; id < N_; id++)
    {
        auto [x,y] = coords(id);
        auto &adj = nodes_[id].getNeighbors();
        adj.clear();

        //upwards
        if (y>0){
            int neighborID= index(x, y-1);
            if (validID(neighborID)){
                adj.push_back(neighborID);
            }
        }
        
        //downwards
        if (y+1<H_){
            int neighborID= index(x, y+1);
            if (validID(neighborID)){
                adj.push_back(neighborID);
            }
        }

        //right
        if (x+1<W_){
            int neighborID= index(x+1, y);
            if (validID(neighborID)){
                adj.push_back(neighborID);
            }
        }

        //left
        if (x>0){
            int neighborID= index(x-1, y);
            if (validID(neighborID)){
                adj.push_back(neighborID);
            }
        }
    }
}

//Seeding 
void Grid::seedInfectedCount(int k){
    if (k <=0) return;
    if (k > N_) k=N_;

    //pick ids to be infected
    vector<int> ids(N_);
    iota(ids.begin(), ids.end(),0);
    shuffle(ids.begin(), ids.end(), rng_);

    //infect ids
    for (int i = 0; i < k; i++)
    {
        int id=ids[i];
        if (nodes_[id].isSuceptible()){
            nodes_[id].markExposed(0.0f);
            nodes_[id].markInfectious(0.0f);
        }
    }
}

void Grid::seedInfectedPercent(float percent, bool atLeastOne){
    if (percent<=0.0f){
        if (atLeastOne) seedInfectedCount(1); //if true, 0% will still run the simulation
        return;
    }
    if (percent >= 100.0f){
        seedInfectedCount(N_);
        return;
    }
    int k= static_cast<int>(round((percent/100.0f) *N_));
    if (atLeastOne) k=max(1,k);
    seedInfectedCount(k);
}


//Simulation
Grid::Counts Grid::step(float t){
    vector<char> nextState(N_);
    for (int i=0; i < N_; i++) {
        nextState[i] = nodes_[i].getState();
    }

    //Exposing Nodes
    for (int i=0; i<N_; i++){
        if (nodes_[i].isSuceptible()){
            int infectNeighbors = 0;
            for (int neighbor : nodes_[i].getNeighbors()){
                if (nodes_[neighbor].isInfectious()) {
                    infectNeighbors++;
                }
            }
            if (infectNeighbors>0){
                float safeProb= 1.0f;
                for (int j=0; j < infectNeighbors; j++){
                    safeProb *= (1.0f - beta_);
                }
                float infectProb = 1.0f - safeProb;
                if (U_(rng_) <infectProb){
                    nextState[i]='E';
                }
            }
        }
    }

    //Incubating Nodes
    for (int i=0; i< N_; i++){
        if (nodes_[i].isExposed()){
            if (U_(rng_) <alpha_){
                nextState[i]= 'I';
            }
        }
    }

    //Recovering Nodes
    for (int i=0; i< N_; i++){
        if (nodes_[i].isInfectious()){
            if (U_(rng_) <gamma_){
                nextState[i]= 'R';
            }
        }
    }

    for (int i=0; i < N_; i++){
        char current = nodes_[i].getState();
        char next = nextState[i];
        if (current == 'S' && next=='E') nodes_[i].markExposed(t);
        else if (current == 'E' && next=='I') nodes_[i].markInfectious(t);
        else if ((current == 'E' || current =='R') && next== 'R') nodes_[i].markRecovered(t);
        }

    Counts c{0,0,0,0};
    for (int i =0; i< N_; i++){
        switch(nodes_[i].getState()){
            case 'S' : ++c.S; break;
            case 'E' : ++c.E; break;
            case 'I' : ++c.I; break;
            case 'R' : ++c.R; break;
        }
    }

    return c;
}


vector<Grid::Counts> Grid::runTillEnd(){
    vector<Counts> history;
    float t= 0.0f;
    const float dt = 1.0f;

    //initial
    Counts c0{0,0,0,0};
    for (auto &n :nodes_){
        switch(n.getState()){
            case 'S' : ++c0.S; break;
            case 'E' : ++c0.E; break;
            case 'I' : ++c0.I; break;
            case 'R' : ++c0.R; break;
        }
    } 
    history.push_back(c0);

    while (history.back().I > 0){
        t+= dt;
        history.push_back(step(t));
    }
    return history;
}
