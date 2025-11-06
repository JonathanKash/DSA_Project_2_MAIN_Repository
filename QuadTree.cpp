#include "QuadTree.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>

QuadTreeSim::Counts::Counts() : S(0), E(0), I(0), R(0) {}
QuadTreeSim::AABB::AABB() : cx(0.0f), cy(0.0f), hx(0.0f), hy(0.0f) {}
QuadTreeSim::AABB::AABB(float c_x, float c_y, float h_x, float h_y) : cx(c_x), cy(c_y), hx(h_x), hy(h_y) {}

// see if point is inside this box
bool QuadTreeSim::AABB::contains(float x, float y)
{
    return (x >= cx - hx && x < cx + hx && y >= cy - hy && y < cy + hy);
};

// see if AABB intersects the circle
bool QuadTreeSim::AABB::intersectsCircle(float qx, float qy, float r)
{
    float px = qx; // circle center x
    if (px < cx - hx)
        px = cx - hx; // move left if outside
    if (px > cx + hx)
        px = cx + hx; // move right
    float py = qy;    // circle center y
    if (py < cy - hy)
        py = cy - hy; // move to top edge
    if (py > cy + hy)
        py = cy + hy;           // move to bottom edge
    float dx = qx - px;         // horiz dist from clamped pt to circle center
    float dy = qy - py;         // vert dist
    return (dx * dx + dy * dy)<= (r*r); // intersects if sq distance <= r^2
}

// constructor
QuadTreeSim::Quadtree::Quadtree(AABB &region, int capacity, vector<float> *xs, vector<float> *ys)
    : boundary_(region),
      capacity_(capacity),
      xs_(xs),
      ys_(ys),
      ids_(),
      subdivided_(false),
      nw_(nullptr), ne_(nullptr), sw_(nullptr), se_(nullptr) {}

// destructor
QuadTreeSim::Quadtree::~Quadtree()
{
    destroyChildren();
}

// helper to delete child nodes
void QuadTreeSim::Quadtree::destroyChildren()
{
    if (nw_ != nullptr)
        delete nw_;
    if (ne_ != nullptr)
        delete ne_;
    if (sw_ != nullptr)
        delete sw_;
    if (se_ != nullptr)
        delete se_;
    nw_ = nullptr;
    ne_ = nullptr;
    sw_ = nullptr;
    se_ = nullptr;
    subdivided_ = false;
}

// clear the node's ids and remove children
void QuadTreeSim::Quadtree::clear()
{
    ids_.clear();
    destroyChildren();
}

// split curr node into 4 quadrant children
void QuadTreeSim::Quadtree::subdivide()
{
    float hx2 = boundary_.hx * 0.5f; // new children half-width
    float hy2 = boundary_.hy * 0.5f; // new children half-height
    float cx0 = boundary_.cx;        // parent center x
    float cy0 = boundary_.cy;        // parent center y
    // nw, ne, sw, se regions
    AABB qnw(cx0 - hx2, cy0 - hy2, hx2, hy2);
    AABB qne(cx0 + hx2, cy0 - hy2, hx2, hy2);
    AABB qsw(cx0 - hx2, cy0 + hy2, hx2, hy2);
    AABB qse(cx0 + hx2, cy0 + hy2, hx2, hy2);
    // allocate nw, nw, sw, se children
    nw_ = new Quadtree(qnw, capacity_, xs_, ys_);
    ne_ = new Quadtree(qne, capacity_, xs_, ys_);
    sw_ = new Quadtree(qsw, capacity_, xs_, ys_);
    se_ = new Quadtree(qse, capacity_, xs_, ys_);
    subdivided_ = true;
}

// insert id into the tree
bool QuadTreeSim::Quadtree::insert(int id)
{
    float x = (*xs_)[id]; // get x pos
    float y = (*ys_)[id]; // get y pos
    if (!boundary_.contains(x, y))
        return false; // false if outside node's region
    if (ids_.size() < capacity_)
    { // if there's space store id here
        ids_.push_back(id);
        return true;
    }
    if (!subdivided_)
        subdivide(); // if full and not subdivided, create children
    // try inserting into nw, ne, sw, se
    if (nw_->insert(id))
        return true;
    if (ne_->insert(id))
        return true;
    if (sw_->insert(id))
        return true;
    if (se_->insert(id))
        return true;
    return false;
}

// recursive helper
void QuadTreeSim::Quadtree::queryCircleRecursive(float qx, float qy, float r, vector<int> &out)
{
    if (!boundary_.intersectsCircle(qx, qy, r))
        return; // skip subtree if region doesn't intersect circle
    for (int i = 0; i < ids_.size(); i++)
    {
        int id = ids_[i];
        float dx = (*xs_)[id] - qx; // delta x from center
        float dy = (*ys_)[id] - qy;
        if (dx * dx + dy * dy <= r * r) // if within r^2 append id to output vector
            out.push_back(id);
    }
    // if there are children, query them recursively
    if (subdivided_)
    {
        nw_->queryCircleRecursive(qx, qy, r, out);
        ne_->queryCircleRecursive(qx, qy, r, out);
        sw_->queryCircleRecursive(qx, qy, r, out);
        se_->queryCircleRecursive(qx, qy, r, out);
    }
}
// perform a circle-range search using the recursive helper
void QuadTreeSim::Quadtree::queryCircle(float qx, float qy, float r, vector<int> &out)
{
    out.clear();
    queryCircleRecursive(qx, qy, r, out);
}

// constructor
QuadTreeSim::QuadTreeSim(int N, float domainW, float domainH, float interactionR, float infectProbPerContact, float incubationProbPerStep, float recoveryProbPerStep, int rngSeed)
    : N_(N), W_(domainW), H_(domainH), R_(interactionR), nodes_(), xs_(), ys_(), rng_(rngSeed), U_(0.0f, 1.0f),
      beta_(infectProbPerContact), alpha_(incubationProbPerStep), gamma_(recoveryProbPerStep), tree_(nullptr)
{
    nodes_.reserve(N_); // allocate space for N nodes
    xs_.resize(N_);     // size x-array to N
    ys_.resize(N_);     // size y-array to N
    for (int i = 0; i < N_; i++)
    {
        nodes_.emplace_back(i, 'S', beta_, alpha_, gamma_); // construct Node
    }
    layoutPositionsNearGrid(); // place nodes across the domain
    rebuildQuadtree();         // build initial quadtree over the static pos
}

void QuadTreeSim::layoutPositionsNearGrid()
{
    int wc = 1; // initialize columns
    // estimate near-square grid
    if (N_ > 1)
    {
        wc = (int)floor(sqrt((double)N_));
        if (wc < 1)
        {
            wc = 1;
        }
    }
    int hc = (int)ceil((double)N_ / (double)wc); // init rows

    // declare spacing
    float dx = 0.0f;
    float dy = 0.0f;

    // even out spacing
    if (wc > 1)
    {
        dx = W_ / (float)(wc - 1);
    }
    if (hc > 1)
    {
        dy = H_ / (float)(hc - 1);
    }

    // set each nodes positions
    for (int i = 0; i < N_; i++)
    {
        int xg = i % wc;
        int yg = i / wc;

        if (wc == 1)
        {
            xs_[i] = (W_ * 0.5f);
        }
        else
        {
            xs_[i] = (xg * dx);
        }

        if (hc == 1)
        {
            ys_[i] = (H_ * 0.5f);
        }
        else
        {
            ys_[i] = (yg * dy);
        }
    }
}

void QuadTreeSim::rebuildQuadtree()
{
    // delete existing quadtree
    if (tree_ != NULL)
    {
        tree_->clear();
        delete tree_;
        tree_ = NULL;
    }

    // define bounding box
    AABB bounds(W_ * 0.5f, H_ * 0.5f, W_ * 0.5f, H_ * 0.5f);

    // allocate new quadtree
    tree_ = new Quadtree(bounds, 8, &xs_, &ys_);

    // rebuild tree
    for (int i = 0; i < N_; i++)
    {
        (void)tree_->insert(i);
    }
}

void QuadTreeSim::seedInfectedCount(int k)
{
    // rule out nonpositives
    if (k <= 0)
    {
        return;
    }
    if (k > N_)
    {
        k = N_;
    }

    // create and mix ids in a new vector
    vector<int> ids(N_);
    iota(ids.begin(), ids.end(), 0);
    shuffle(ids.begin(), ids.end(), rng_);

    // infect the first k unique ids in the shuffled vector
    int i = 0;
    while (i < k)
    {
        int id = ids[i];
        if (nodes_[id].isSuceptible())
        {
            nodes_[id].markExposed(0.0f);
            nodes_[id].markInfectious(0.0f);
        }
        i++;
    }
}

void QuadTreeSim::seedInfectedPercent(float percent, bool atLeastOne)
{
    if (percent <= 0.0f)
    {
        if (atLeastOne)
            seedInfectedCount(1); // if true, 0% will still run the simulation
        return;
    }
    if (percent >= 100.0f)
    {
        seedInfectedCount(N_); // infect everyone and return
        return;
    }

    // get Percentage
    int k = static_cast<int>(round((percent / 100.0f) * N_));
    if (atLeastOne)
        k = max(1, k);
    seedInfectedCount(k);
}

// calc squared distance between two node indices
float QuadTreeSim::dist2(int a, int b)
{
    float dx = xs_[a] - xs_[b];
    float dy = ys_[a] - ys_[b];
    return dx * dx + dy * dy;
}

// advances the simulation by one time step
QuadTreeSim::Counts QuadTreeSim::step(float t)
{
    vector<char> nextState(N_);
    for (int i = 0; i < N_; i++)
    {
        nextState[i] = nodes_[i].getState();
    }
    vector<int> candidates;
    candidates.reserve(32);

    // Exposing Nodes S->E
    for (int i = 0; i < N_; i++) {
        if (!nodes_[i].isSuceptible())
        { // only susceptible nodes exposed
            continue;
        }
        tree_->queryCircle(xs_[i], ys_[i], R_, candidates); // query neightbors in R
        int infNeighbors = 0;                               // infec neighbor counter
        for (int k = 0; k < candidates.size(); k++)
        {
            int nId = candidates[k];
            if (nId == i)
                continue;
            if (nodes_[nId].isInfectious())
                infNeighbors++; // increm if neighbor infectious
        }
        if (infNeighbors > 0)
        {
            float probUninfected = 1.0f; // prob to stay uninfected after all contacts
            int c = 0;
            while (c < infNeighbors)
            {
                probUninfected *= (1.0f - beta_);
                c++;
            }
            float probInfect = 1.0f - probUninfected; // total infection prob
            if (U_(rng_) < probInfect)
            {
                nextState[i] = 'E';
            }
        }
    }
    // Incubation E->I
    for (int i = 0; i < N_; i++){
        if (nodes_[i].isExposed())
        {                          // consider only exposed nodes
            if (U_(rng_) < alpha_) // with prob alpha
                nextState[i] = 'I';
        }
    }
    // Recovery I->R
    for (int i = 0; i < N_; i++)
    {
        if (nodes_[i].isInfectious())
        {                          // consider only infectious nodes
            if (U_(rng_) < gamma_) // with prob gamma
                nextState[i] = 'R';
        }
    }
    // apply all transitions and timestamps
    for (int i = 0; i < N_; i++)
    {
        char curr = nodes_[i].getState(); // curr state
        char next = nextState[i];         // next state comp above
        if (curr == 'S' && next == 'E')
            nodes_[i].markExposed(t);
        else if (curr == 'E' && next == 'I')
            nodes_[i].markInfectious(t);
        else if ((curr == 'E' || curr == 'I') && next == 'R')
            nodes_[i].markRecovered(t);
    }
    // count S/E/I/R
    Counts c; // counts result
    for (int i = 0; i < N_; i++)
    {
        char s = nodes_[i].getState(); // state after transitions
        // increment the counts for each state
        if (s == 'S')
            c.S++;
        else if (s == 'E')
            c.E++;
        else if (s == 'I')
            c.I++;
        else if (s == 'R')
            c.R++;
    }
    return c;
}

// run until I == 0
vector<QuadTreeSim::Counts> QuadTreeSim::runToExtinction()
{
    vector<Counts> history; // store cts at each step
    float t = 0.0f;         // simulation time
    float dt = 1.0f;        // time increment per step

    Counts c0; // initial cts
    for (int i = 0; i < nodes_.size(); i++)
    {
        char s = nodes_[i].getState(); // read state
        if (s == 'S')
            c0.S++;
        else if (s == 'E')
            c0.E++;
        else if (s == 'I')
            c0.I++;
        else if (s == 'R')
            c0.R++;
    }
    history.push_back(c0); // record initial cts at t = 0

    while (history.back().I > 0)
    {                               // continue while there are infectious nodes
        t += dt;                    // advance time
        history.push_back(step(t)); // perform a step, record new cts
    }
    return history;
}

// copies curr state into caller provided arrays
void QuadTreeSim::snapshot(vector<float> &outX, vector<float> &outY, vector<char> &outState, vector<int> &outId)
{
    // copy x and y positions
    outX = xs_;
    outY = ys_;
    outState.clear();
    outState.reserve(nodes_.size());
    outId.clear();
    outId.reserve(nodes_.size());
    for (int i = 0; i < nodes_.size(); i++)
    {
        outState.push_back(nodes_[i].getState()); // append state char for node i
        outId.push_back(i);                       // append node id
    }
}