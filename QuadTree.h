#pragma once
#include <vector>
#include <random>
#include "Node.h"
using namespace std;

class QuadTreeSim
{
public:
    class Counts
    { // stores counts for SEIR at a time step
    public:
        int S, E, I, R;
        Counts(); // default constr
    };
    QuadTreeSim(int N,                                                      // total node num
                float domainW,                                              // width of 2D domain for node placement
                float domainH,                                              // height of 2D domain for node placement
                float interactionR,                                         // infection interaction radius
                float infectProbPerContact,                                 // beta, infection probability
                float incubationProbPerStep,                                // alpha, prob E->I
                float recoveryProbPerStep,                                  // gamma, prob I->R
                int rngSeed = random_device{}());                           // seed for RNG
    void seedInfectedPercent(float percent, bool ensureAtLeastOnce = true); // infect % of nodes, unique random ids
    void seedInfectedCount(int k);                                          // infect k unique random ids
    Counts step(float t);                                                   // advance simul one step at a time t
    vector<Counts> runToExtinction();                                       // run steps until no infectious left

    void snapshot(vector<float> &outX,     // output x positions
                  vector<float> &outY,     // output y positions
                  vector<float> &outState, // output state S,E,I, or R per node
                  vector<float> &outID);   // output node ids
    // accessors:
    vector<Node> &nodes() { return nodes_; } // reference to node list
    int size() { return N_; }                // num nodes
    float width() { return W_; }             // domain width
    float height() { return H_; }            // domain height
    float radius() { return R_; }            // interation radius
private:
    int N_;              // total node count
    float W_;            // domain width
    float H_;            // domain height
    float R_;            // infec interaction radius
    vector<Node> nodes_; // vector of node objs holding SEIR state/time
    vector<float> xs_;   // vector of x coord per node index
    vector<float> ys_;   // vector of y coord per node index
    float beta_, alpha_, gamma_;

    mt19937 rng_;                        // rnd num generator
    uniform_real_distribution<float> U_; // uniform distribution in [0,1) for prob draws

    class AABB
    {
    public:
        float cx, cy; // center for x and y coord
        float hx, hy; // half width/height in x/y
        AABB();
        AABB(float c_x, float c_y, float h_x, float h_y);
        bool contains(float x, float y);                    // true if pt (x,y) is within the box
        bool intersectsCircle(float qx, float qy, float r); // true if box intersects circle centered (qx, qy) radius
    };
    class Quadtree
    { // manages spatial partitioning and circle queries
    public:
        Quadtree(AABB &region,       // bounding region represented by this quadtree node
                 int capacity,       // max ids stored before subdivision
                 vector<float> *xs,  // ptr to x array
                 vector<float> *ys); // ptr to y array
        ~Quadtree();
        bool insert(int id);
        void queryCircle(float qx, float qy, float r, vector<int> &out); // append ids within circle to out
        void clear();

    private:
        AABB boundary_; // region covered by this node
        int capacity_;  // max items before subdividing
        vector<float> *xs_;
        vector<float> *ys_;

        vector<int> ids_; // stored ids when size<= capacity_
        bool subdivided_; // true if subdivided into 4 children
        Quadtree *nw_;    // ptr to northwest child
        Quadtree *ne_;
        Quadtree *sw_;
        Quadtree *se_;

        void subdivide();                                                         // create child nodes that each cover a quadrant
        void destroyChildren();                                                   // helper to delete children and reset ptr
        void queryCircleRecursive(float qx, float qy, float r, vector<int> &out); // recursive query helper
    };
    Quadtree *tree_;
    void rebuildQuadtree(); // rebuild quadtree from curr pos
    void layoutPositionsNearGrid();
    float dist2(int a, int b);
};
