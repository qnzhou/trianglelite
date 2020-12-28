#pragma once

#include <trianglelite/common.h>

namespace trianglelite {

enum class Algorithm {
    DIVIDE_AND_CONQUER, // Default algorithm.
    SWEEPLINE, // Steven Fortune's sweepline algorithm (-F option)
    INCREMENTAL // Incremental algorithm (-i options).
};

struct Config
{
    Scalar min_angle = 20.0f; // degrees.
    Scalar max_area = -1.0f; // Not set.
    Index max_num_steiner = -1; // Unlimited.
    Index verbose_level = 1; // Normal
    Algorithm algorithm = Algorithm::DIVIDE_AND_CONQUER;
    bool convex_hull = false; // Not keeping convex hull.
    bool conforming = false; // Not require conforming.
    bool exact = true; // Use exact arithmetic.
    bool split_boundary = true; // Allow splitting of boundary.
    bool auto_hole_detection = false; // Auto hole detection using winding number.
};

} // namespace triangle
