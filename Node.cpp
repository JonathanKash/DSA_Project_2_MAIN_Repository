#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>
#include <queue>
#include <algorithm>
#include "Node.h"

using namespace std;

Node::Node(int id, char state, float infectionRate): 
    node_id(id), 
    degree_of_separation(0), 
    curr_state(state), 
    time_exposed(-1.0), 
    time_infect(-1.0), 
    recovery_time(-1.0), 
    infection_rate(infectionRate) {}