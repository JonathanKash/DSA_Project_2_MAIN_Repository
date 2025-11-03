#include "QuadTree.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>

QuadTreeSim::Counts::Counts() : S(0), E(0), I(0), R(0) {}
QuadTreeSim::AABB::AABB() : cx(0.0f), cy(0.0f), hx(0.0f), hy(0.0f) {}
QuadTreeSim::AABB::AABB(float c_x, float c_y, float h_x, float h_y) : cx(c_x), cy(c_y), hx(h_x), hy(h_y) {}

bool QuadTreeSim::AABB::contains(float x, float y) {
    return (x >= cx - hx && x < cx + hx && y >= cy - hy && y < cy + hy);
};
