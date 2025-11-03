#pragma once
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
connectNeighbors4;
}

void Grid::buildGrid(int N){
    //make it as square as possible
    W_ = max(1, (int)floor(sqrt((double)N)));
    H_ = (int)ceil((double)N / (double)W_);

    //memory optimization
    nodes_.reserve(N_);

    //initialize Nodes
    for (int i=0; i < N_; i++){
        nodes_.emplace_back(i, 'S', beta_, alpha_, gamma_)
    }
}

void Grid::connectNeighbors4(){
    auto coords = [&](int id) {return pair<int, int>(id % W, id /W_); };

    for (int id =0, id < N_; id++){
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
    for (int i=0, i <k; i++){
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
