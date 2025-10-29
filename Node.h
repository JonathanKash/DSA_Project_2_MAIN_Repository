#include <vector>
using namespace std;

class Node {
public:
    Node(int id, char state = 'S', float infectionRate = 0.0);

private : int node_id;
    int degree_of_separation;
    char curr_state;
    float time_exposed;
    float time_infect;
    float recovery_time;
    float infection_rate;
    vector<int> neighbors;
};