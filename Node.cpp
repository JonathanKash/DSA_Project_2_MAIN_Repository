#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>
#include <queue>

class Node {
private:
    int id = 0;
    int state = 0; // state of node 0 - safe, 1 - exposed, 2 - infected, 3 - recovered

};