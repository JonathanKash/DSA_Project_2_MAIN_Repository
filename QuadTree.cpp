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
    return (dx * dx + dy * dy); // intersects if sq distance <= r^2
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