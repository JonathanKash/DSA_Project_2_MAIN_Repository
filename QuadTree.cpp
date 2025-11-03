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

QuadTreeSim::Quadtree::Quadtree(AABB &region, int capacity, vector<float> *xs, vector<float> *ys)
    : boundary_(region),
      capacity_(capacity),
      xs_(xs),
      ys_(ys),
      ids_(),
      subdivided_(false),
      nw_(nullptr), ne_(nullptr), sw_(nullptr), se_(nullptr) {}

QuadTreeSim::Quadtree::~Quadtree()
{
    destroyChildren();
}