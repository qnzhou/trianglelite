# TriangleLite

TriangleLite is a lite wrapper of the [triangle library] in C++.  It is created
by Qingnan Zhou as a coding exercise to make using the triangle library less
tedious.

## Quick start

```c++
#include <trianglelite/trianglelite.h>

using Scalar = trianglelite::Scalar;
using Index = trianglelite::Index;

// Initialize a single unit triangle as input.
std::vector<Scalar> points{ x0, y0, x1, y1, ... };
std::vector<int> segments { s00, s01, s10, s11, ... };

// Set input.
trianglelite::Engine engine;
engine.set_in_points(points.data(), points.size()/2);
engine.set_in_segments(segments.data(), segments.size()/2);

// Set triangle configuration.
trianglelite::Config config;
config.max_area = 0.1;

// Run!
engine.run(config);

// Extract output.
Eigen::Matrix<Scalar, -1, 2> out_points = engine.get_out_points();
Eigen::Matrix<Index, -1, 2> out_triangles = engine.get_out_triangles();
```

## Build

```sh
mkdir build
cd build
cmake ..
make
```

## Detailed usage

There are basically 3 steps in using trianglelite: import input, configure, run
and extract output.

### Import input

There are a few ways of using the [triangle library], and they each involve
slightly different input.  We will illustrate each use case one by one below.

In order to minimize the amount of copying involved, trianglelite assumes the
user is responsible for managing the input memory.  So its API only takes raw
pointers and a size parameter for all input data.

#### Import points

Input points must be stored in continuous memory in the form of
`[x0, y0, x1, y1, ...]`.  For example:

```c++
std::vector<Scalar> points{
    0.0, 0.0,  // point 0
    1.0, 0.0,  // point 1
    0.0, 1.0   // point 2
};
engine.set_in_points(points.data(), 3);
```

#### Import segments



### Configure triangle parameters

### Extract output



[triangle library]: https://shared-assets.adobe.com/link/fb3b4ede-dc2d-40f6-62de-087aa3943d82
