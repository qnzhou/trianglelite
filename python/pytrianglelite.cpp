#include <trianglelite/trianglelite.h>

#include <fmt/core.h>
#include <nanobind/eigen/dense.h>
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include <exception>
#include <string>


namespace nb = nanobind;

std::string algorithm_to_string(trianglelite::Algorithm algorithm)
{
    switch (algorithm) {
    case trianglelite::Algorithm::DIVIDE_AND_CONQUER: return "divide_and_conquer";
    case trianglelite::Algorithm::SWEEPLINE: return "sweepline";
    case trianglelite::Algorithm::INCREMENTAL: return "incremental";
    default: throw std::runtime_error("Unknown algorithm");
    }
}

trianglelite::Algorithm string_to_algorithm(const std::string& value)
{
    if (value == "divide_and_conquer") {
        return trianglelite::Algorithm::DIVIDE_AND_CONQUER;
    } else if (value == "sweepline") {
        return trianglelite::Algorithm::SWEEPLINE;
    } else if (value == "incremental") {
        return trianglelite::Algorithm::INCREMENTAL;
    } else {
        throw std::runtime_error("Unknown algorithm");
    }
}

NB_MODULE(pytrianglelite, m)
{
    nb::class_<trianglelite::Config>(m, "Config", "Triangulation configuration.")
        .def(nb::init<>())
        .def("__repr__",
            [](trianglelite::Config& self) {
                return fmt::format("Config(\n  min_angle={},\n  max_area={},\n  "
                                   "max_num_steiner={},\n  verbose_level={},\n  "
                                   "algorithm={},\n  convex_hull={},\n  conforming={},\n  "
                                   "exact={},\n  split_boundary={},\n  "
                                   "auto_hole_detection={}\n)",
                    self.min_angle,
                    self.max_area,
                    self.max_num_steiner,
                    self.verbose_level,
                    algorithm_to_string(self.algorithm),
                    self.convex_hull,
                    self.conforming,
                    self.exact,
                    self.split_boundary,
                    self.auto_hole_detection);
            })
        .def_rw("min_angle",
            &trianglelite::Config::min_angle,
            R"(Minimum angle constraint in degrees.)")
        .def_rw("max_area",
            &trianglelite::Config::max_area,
            R"(Maximum area constraint. Negative value means not set.)")
        .def_rw("max_num_steiner",
            &trianglelite::Config::max_num_steiner,
            R"(Maximum number of Steiner points. Negative value means unlimited.)")
        .def_rw("verbose_level",
            &trianglelite::Config::verbose_level,
            R"(Verbose level (0-4, 0 == quiet).)")
        .def_prop_rw(
            "algorithm",
            [](trianglelite::Config& self) { return algorithm_to_string(self.algorithm); },
            [](trianglelite::Config& self, std::string value) {
                return algorithm_to_string(self.algorithm);
            },
            R"(Algorithm: "divide_and_conquer", "sweepline", "incremental")")
        .def_rw("convex_hull", &trianglelite::Config::convex_hull, R"(Whether to keep convex hul)")
        .def_rw("conforming",
            &trianglelite::Config::conforming,
            R"(Whether to require conforming triangulation.)")
        .def_rw("exact",
            &trianglelite::Config::exact,
            R"(Whether to use exact arithmetic (strongly recommended).)")
        .def_rw("split_boundary",
            &trianglelite::Config::split_boundary,
            R"(Whether to allow splitting the boundary.)")
        .def_rw("auto_hole_detection",
            &trianglelite::Config::auto_hole_detection,
            R"(Whether to detect holes automatically based on winding number.)");

    nb::class_<trianglelite::Engine>(m, "Engine", "Triangulation engine.")
        .def(nb::init<>())
        .def_prop_rw(
            "in_points",
            [](trianglelite::Engine& self) { return self.get_in_points(); },
            [](trianglelite::Engine& self, trianglelite::Matrix2FrMap value) {
                self.set_in_points(value.data(), static_cast<trianglelite::Index>(value.rows()));
            },
            R"(Input 2D point cloud to be triangulated or Voronoi diagrammed.)")
        .def_prop_rw(
            "in_segments",
            [](trianglelite::Engine& self) { return self.get_in_segments(); },
            [](trianglelite::Engine& self, trianglelite::Matrix2IrMap value) {
                self.set_in_segments(value.data(), static_cast<trianglelite::Index>(value.rows()));
            },
            R"(Input segment constraints.)")
        .def_prop_rw(
            "in_triangles",
            [](trianglelite::Engine& self) { return self.get_in_triangles(); },
            [](trianglelite::Engine& self, trianglelite::Matrix3IrMap value) {
                self.set_in_triangles(value.data(), static_cast<trianglelite::Index>(value.rows()));
            },
            R"(Input existing triangulation of the point cloud. Used for refining an existing triangulation.)")
        .def_prop_rw(
            "in_holes",
            [](trianglelite::Engine& self) { return self.get_in_holes(); },
            [](trianglelite::Engine& self, trianglelite::Matrix2FrMap value) {
                self.set_in_holes(value.data(), static_cast<trianglelite::Index>(value.rows()));
            },
            R"(Input hole points. Used by triangle to flood and remove faces representing holes.)")
        .def_prop_rw(
            "in_areas",
            [](trianglelite::Engine& self) { return self.get_in_areas(); },
            [](trianglelite::Engine& self, trianglelite::Matrix1FMap value) {
                self.set_in_areas(value.data(), static_cast<trianglelite::Index>(value.rows()));
            },
            R"(Input triangle area constraints. One area per triangle.)")
        .def_prop_rw(
            "in_point_markers",
            [](trianglelite::Engine& self) { return self.get_in_point_markers(); },
            [](trianglelite::Engine& self, trianglelite::Matrix1IMap value) {
                self.set_in_point_markers(
                    value.data(), static_cast<trianglelite::Index>(value.rows()));
            },
            R"(Input point markers. One positive integer marker per point.)")
        .def_prop_rw(
            "in_segment_markers",
            [](trianglelite::Engine& self) { return self.get_in_segment_markers(); },
            [](trianglelite::Engine& self, trianglelite::Matrix1IMap value) {
                self.set_in_segment_markers(
                    value.data(), static_cast<trianglelite::Index>(value.rows()));
            },
            R"(Input segment markers. One positive integer marker per segment.)")
        .def_prop_ro(
            "out_points",
            [](trianglelite::Engine& self) { return self.get_out_points(); },
            R"(Output 2D point cloud.)")
        .def_prop_ro(
            "out_segments",
            [](trianglelite::Engine& self) { return self.get_out_segments(); },
            R"(Output segment constraints.)")
        .def_prop_ro(
            "out_triangles",
            [](trianglelite::Engine& self) { return self.get_out_triangles(); },
            R"(Output triangulation.)")
        .def_prop_ro(
            "out_edges",
            [](trianglelite::Engine& self) { return self.get_out_edges(); },
            R"(Output edges.)")
        .def_prop_ro(
            "out_triangle_neighbors",
            [](trianglelite::Engine& self) { return self.get_out_triangle_neighbors(); },
            R"(Output triangle neighbors.)")
        .def_prop_ro(
            "out_point_markers",
            [](trianglelite::Engine& self) { return self.get_out_point_markers(); },
            R"(Output point markers.)")
        .def_prop_ro(
            "out_segment_markers",
            [](trianglelite::Engine& self) { return self.get_out_segment_markers(); },
            R"(Output segment markers.)")
        .def_prop_ro(
            "out_edge_markers",
            [](trianglelite::Engine& self) { return self.get_out_edge_markers(); },
            R"(Output edge markers.)")
        .def("run", &trianglelite::Engine::run, R"(Run triangulation.)");
}
