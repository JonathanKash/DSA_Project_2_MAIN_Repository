#include <vector>
using namespace std;
#pragma once

class Node {
private:
    int node_id; // unique id for each node
    int degree_of_separation; // how many nodes it took from the original infected node to infect it
    char curr_state; // state of node: S, E, I, or R
    float time_exposed; // time of exposure
    float time_infect; // time of becoming infectious
    float time_recovery; // time to recover
    float infection_rate; // rate at which nodes get infected, S to E
    float incubation_rate; // rate of E to I
    float recovery_rate; // rate of I to R
    vector<int> neighbors; // adjacency list of neighbor node ids

public:
    // constructor
    Node(int id, char state = 'S', float infectionRate = 0.0, float incubationRate = 0.0, float recoveryRate = 0.0);

    // getters
    int getId();
    int getDegree(); // num of immediate neighbors
    int getDegreeOfSeparation(); // distance from a source
    char getState();
    float getTimeExposed();
    float getTimeInfected();
    float getTimeRecovered();
    float getInfectionRate();
    float getIncubationRate();
    float getRecoveryRate();
    vector<int> &getNeighbors(); // adjacency list

    // setters
    void setDegreeOfSeparation(int degree);
    void setInfectionRate(float rate);
    void setIncubationRate(float rate);
    void setRecoveryRate(float rate);

    // SEIR helpers
    void markExposed(float time);
    void markInfectious(float time);
    void markRecovered(float time);

    // markers
    bool isSuceptible();
    bool isExposed();
    bool isInfectious();
    bool isRecovered();
};