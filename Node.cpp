#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>
#include <queue>
#include <algorithm>
#include "Node.h"

using namespace std;

Node::Node(int id, char state, float infectionRate, float incubationRate, float recoveryRate){
    node_id = id;
    curr_state = state;
    infection_rate = infectionRate;
    incubation_rate = incubationRate;
    recovery_rate = recovery_rate;
}

int Node::getId(){
    return node_id;
    }
int Node::getDegree(){
    return neighbors.size();
}
int Node::getDegreeOfSeparation(){
    return degree_of_separation;
}
char Node::getState(){
    return curr_state;
}
char Node::getState(){
    return curr_state;
}
float Node::getTimeExposed(){
    return time_exposed;
}
float Node::getTimeInfected(){
    return time_infect;
}
float Node::getTimeRecovered(){
    return time_recovery;
}
float Node::getInfectionRate(){
    return infection_rate;
}
float Node::getIncubationRate(){
    return incubation_rate;
}
float Node::getRecoveryRate(){
    return recovery_rate;
}
vector<int>& Node::getNeighbors(){
    return neighbors;
}

// setters
void Node::setDegreeOfSeparation(int degree){
    degree_of_separation = degree;
}
void Node::setInfectionRate(float rate){
    infection_rate = rate;
}
void Node::setIncubationRate(float rate){
    incubation_rate = rate;
}
void Node::setRecoveryRate(float rate)
{
    recovery_rate = rate;
}
