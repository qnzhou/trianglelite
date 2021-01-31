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

There are basically 4 steps in using TriangleLite: import input, configure, run
and extract output.

### Input

There are a few ways of using the [triangle library], and they each involve
slightly different input.  We will illustrate each use case one by one below.

In order to minimize the amount of copying involved, TriangleLite assumes the
user is responsible for managing the input memory.  So its API only takes raw
pointers and a size parameter for all input data.

#### Import points

Input points must be stored in contiguous memory of `Scalar`s in the form of
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

Input segments are stored in contiguous memory of point indices.  For example:

```c++
std::vector<Index> segments {
    0, 1,  // segment 1 connects vertex 0 and 1.
    1, 2,  // segment 2
    2, 0   // segemnt 3
};
engine.set_in_segments(segments.data(), 3);
```

Note that while triangle works with unoriented segments, it is actually
beneficial to use oriented segments (i.e. segments should be oriented
in a counterclockwise fashion).  Specifically, with oriented segments,
TriangleLite can deduce holes using winding number with its auto hole detection
feature.

#### Import hole list

Without using the auto hole detection feature, users must provide a set of hole
points to indicate hole locations.  To set hole points:

```c++
std::vector<Scalar> holes {
    0.0, 0.0,  // hole point 0
    1.0, 0.0   // hole point 1
};
engine.set_in_holes(holes.data(), 2);
```

#### Import triangles

Sometimes, we have a triangulation to start with and want to have it refined.
The triangulation can be imported with the following snippet:

```c++
std::vector<Index> triangles {
    0, 1, 2,  // triangle 0
    2, 1, 3   // triangle 1
};
engine.set_in_triangles(triangles.data(), 2)
```

#### Import area constraint

For triangle refinement, one can import an area field to control triangle
density.  To do that:

```c++
std::vector<Scalar> areas {
   0.1, 0.2, 0.5, 0.1, ...
};
engine.set_in_areas(areas.data(), areas.size());
```

#### Set point and segment markers

One of the important features of triable is to track the input points and
segments in the output triangulation.  This is done by setting the input point
and segment markers:

```c++
std::vector<int> point_markers {
    ...
};
engine.set_in_point_markers(point_markers.data(), point_markers.size());

std::vector<int> segment_markers {
    ...
};
engine.set_in_segemnt_markers(segment_markers.data(), segment_markers.size());
```

### Configure

The [triangle library] uses a set of command line switches to configure
triangulation parameters.  I find it a bit difficult to memorize, instead,
TriangleLite uses a `Config` struct to make the code more readable.  Here is a
list of fields one can configure:

|            Field name |  Type  | Description |
|----------------------:|--------|-------------|
|           `min_angle` | Scalar | Controls the triangulation quality.  Default is 20 degree. |
|            `max_area` | Scalar | Controls the triangulation density.  Default is -1 (i.e. unconstrained). |
|     `max_num_steiner` | Index  | Number of inserted [Steiner points].  Default is -1 (i.e. unlimited). |
|       `verbose_level` | Index  | Verbosity level ranges from 0 to 4.  0: quiet, 4: debug only.  Default is 1. |
|           `algorithm` | Enum   | `DIVIDE_AND_CONQUER` (default), `SWEEP_LINE` or `INCREMENTAL`. |
|         `convex_hull` | Bool   | Whether to triangulate the entire convex hull.  Default is false. |
|          `conforming` | Bool   | Enforce all triangle to be [Delaunay][Delaunay triangulation], not just [constrained Delaunay][Constrained Delaunay triangulation]. Default is false. |
|               `exact` | Bool   | Use exact arithmetic.  Default is true. |
|      `split_boundary` | Bool   | Allow mesh boundary to be split.  Default is true. |
| `auto_hole_detection` | Bool   | Using winding number to automatically detect holes. Default is false. |


### Run

Once all inputs are imported, the next step is to triangulate the domain with
the specified configuration:

```c++
engine.run(config);
```

### Output

To extract the output triangulation:

```c++
Eigen::Matrix<Scalar, -1, 2> points = engine.get_out_points();
Eigen::Matrix<Index, -1, 3> triangles = engine.get_out_triangles();
```

Note that the above code involves copying the triangulation data.  The return
types of `get_out_*` methods are `Eigen::Map`, which wraps around a internal
memory managed by TriangleLite.  For users who are familiar with Eigen, it is
possible to avoid this copy by working directly with `Eigen::Map` objects while
keeping the `engine` object alive.

In addition to the basic triangulation, it is also possible to extract output
edges, segments, triangle connectivity, point/segment markers:

```c++
Eigen::Matrix<Index, -1, 2> edges = engine.get_out_edges();
Eigen::Matrix<Index, -1, 2> segments = engine.get_out_segments();
Eigen::Matrix<Index, -1, 3> tri_neighbors = engine.get_out_triangle_neighbors();
Eigen::Matrix<Index, -1, 1> point_marker = engine.get_out_point_markers();
Eigen::Matrix<Index, -1, 1> segment_marker = engine.get_out_segment_markers();
Eigen::Matrix<Index, -1, 1> edge_marker = engine.get_out_edge_markers();
```

* Edges are the edges of the triangulation.
* Segments are a set of edges that maps back to input segments.
* Triangle neighbors provide information of triangle-triangle connectivity.
* Point/segment markers are markers that got mapped from the input point/segments.
* Edge markers are markers that got mapped from the input segments to output
  edges.


[triangle library]: https://www.cs.cmu.edu/~quake/triangle.html
[Steiner points]: https://en.wikipedia.org/wiki/Steiner_point_(computational_geometry)
[Delaunay triangulation]: https://mathworld.wolfram.com/DelaunayTriangulation.html
[Constrained Delaunay triangulation]: https://en.wikipedia.org/wiki/Constrained_Delaunay_triangulation

