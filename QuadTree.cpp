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
void QuadTreeSim::Quadtree::destroyChildren(){
    if (nw_ != nullptr) delete nw_;
    if (ne_ != nullptr) delete ne_;
    if (sw_ != nullptr) delete sw_;
    if (se_ != nullptr) delete se_;
    nw_ = nullptr;
    ne_ = nullptr;
    sw_ = nullptr;
    se_ = nullptr;
    subdivided_ = false;
}

// clear the node's ids and remove children
void QuadTreeSim::Quadtree::clear(){
    ids_.clear(); 
    destroyChildren();
}

// split curr node into 4 quadrant children
void QuadTreeSim::Quadtree::subdivide() {
    float hx2 = boundary_.hx * 0.5f; // new children half-width
    float hy2 = boundary_.hy * 0.5f; // new children half-height
    float cx0 = boundary_.cx; // parent center x
    float cy0 = boundary_.cy; // parent center y
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