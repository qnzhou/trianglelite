#pragma once

#include <Eigen/Core>

#include <memory>

#include <trianglelite/Config.h>
#include <trianglelite/common.h>

struct triangulateio; // Data structure defined by triangle.

namespace trianglelite {

class Engine
{
public:
    Engine();
    ~Engine();

public:
    //================== Input Geometry ========================
    /**
     * Add a point cloud to be triangulated/Voronoi diagrammed.
     *
     * The array is row major, i.e. [x0, y0, x1, y1, ...]
     */
    void set_in_points(const Scalar* points, Index num_points);
    Matrix2FrMap get_in_points();
    void unset_in_points();

    /**
     * Add segment constraints.  It is equivalent to passing a PSLG as
     * input.
     *
     * The array is row major, i.e. [s00, s01, s10, s11, ...]
     */
    void set_in_segments(const Index* segments, Index num_segments);
    Matrix2IrMap get_in_segments();
    void unset_in_segments();

    /**
     * Add existing triangulation of the point cloud.  Used for refining an
     * existing triangulation.  This is equivalent to passing a .node
     * (specified with `set_points()`) and a .ele file as input (specified
     * with this functions).
     *
     * The array is row major, i.e. [t00, t01, t02, t10, t11, t12, ...]
     */
    void set_in_triangles(const Index* triangles, Index num_triangles);
    Matrix3IrMap get_in_triangles();
    void unset_in_triangles();

    /**
     * Set hole points.  Used by triangle to flood and remove faces
     * representing holes.
     *
     * The array is row major, i.e. [x0, y0, x1, y1, ...]
     */
    void set_in_holes(const Scalar* holes, Index num_holes);
    Matrix2FrMap get_in_holes();
    void unset_in_holes();

    /**
     * Set triangle area constraints.  One area per triangle.
     */
    void set_in_areas(const Scalar* areas, Index num_areas);
    Matrix1FMap get_in_areas();
    void unset_in_areas();

    /**
     * Set point markers.  Only positive values are supported.
     */
    void set_in_point_markers(const int* markers, Index num_markers);
    Matrix1IMap get_in_point_markers();
    void unset_in_point_markers();

    /**
     * Set segment markers.  Only positive values are supported.
     */
    void set_in_segment_markers(const int* markers, Index num_markers);
    Matrix1IMap get_in_segment_markers();
    void unset_in_segment_markers();

public:
    //================== Output Geometry ========================
    const Matrix2FrMap get_out_points() const;

    const Matrix3IrMap get_out_triangles() const;

    const Matrix2IrMap get_out_segments() const;

    const Matrix2IrMap get_out_edges() const;

    const Matrix3IrMap get_out_triangle_neighbors() const;

    const Matrix1IMap get_out_point_markers() const;

    const Matrix1IMap get_out_segment_markers() const;

    const Matrix1IMap get_out_edge_markers() const;

public:
    void run(const Config& config);

private:
    /**
     * Automatically generated a list of hole points based on winding number.
     * This method assumes `m_in` is setup correctly.
     *
     * Warning: This method only works if the input segments forms closed and
     * correctly oriented loops.
     */
    std::vector<Scalar> run_auto_hole_detection();

private:
    std::unique_ptr<triangulateio> m_in;
    std::unique_ptr<triangulateio> m_out;
    std::unique_ptr<triangulateio> m_vorout;
};

} // namespace trianglelite
