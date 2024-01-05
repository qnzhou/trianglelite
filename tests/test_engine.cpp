#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <trianglelite/trianglelite.h>

#include <Eigen/Core>
#include <iostream>

TEST_CASE("Point cloud", "[trianglelite]")
{
    using namespace trianglelite;

    Config config;
    Engine engine;

    SECTION("Convex hull")
    {
        config.convex_hull = true;
        config.verbose_level = 0;
    }
    SECTION("With max area")
    {
        config.max_area = 0.1;
        config.verbose_level = 0;
    }
    SECTION("With max area too small")
    {
        config.max_area = 1e-7;
        config.verbose_level = 0;
    }

    Eigen::Matrix<Scalar, 3, 2, Eigen::RowMajor> points;
    points << 0.0, 0.0, 1.0, 0.0, 0.0, 1.0;
    engine.set_in_points(points.data(), static_cast<int>(points.rows()));
    engine.run(config);

    auto out_points = engine.get_out_points();
    REQUIRE(out_points.rows() >= 3);
    auto out_triangles = engine.get_out_triangles();
    REQUIRE(out_triangles.rows() >= 1);
}

TEST_CASE("Quad", "[trianglelite]")
{
    using namespace trianglelite;

    Config config;
    Engine engine;

    config.max_area = 0.1;
    config.verbose_level = 0;

    Eigen::Matrix<Scalar, 4, 2, Eigen::RowMajor> points;
    points << 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0;
    engine.set_in_points(points.data(), static_cast<int>(points.rows()));

    Eigen::Matrix<int, 4, 2, Eigen::RowMajor> segments;
    segments << 0, 1, 1, 2, 2, 3, 3, 0;
    engine.set_in_segments(segments.data(), static_cast<int>(segments.rows()));

    engine.run(config);

    auto out_points = engine.get_out_points();
    REQUIRE(out_points.rows() >= 3);
    auto out_triangles = engine.get_out_triangles();
    REQUIRE(out_triangles.rows() >= 1);
}

TEST_CASE("QuadWithHole", "[trianglelite][hole]")
{
    using namespace trianglelite;

    Config config;
    Engine engine;

    config.max_area = 0.05;
    config.verbose_level = 0;
    config.auto_hole_detection = true;

    Eigen::Matrix<Scalar, 8, 2, Eigen::RowMajor> points;
    points << 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 0.2, 0.2, 0.2, 0.8, 0.8, 0.8, 0.8, 0.2;
    engine.set_in_points(points.data(), static_cast<int>(points.rows()));

    Eigen::Matrix<int, 8, 2, Eigen::RowMajor> segments;
    segments << 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4;
    engine.set_in_segments(segments.data(), static_cast<int>(segments.rows()));

    engine.run(config);

    auto out_points = engine.get_out_points();
    auto out_edges = engine.get_out_edges();
    auto out_triangles = engine.get_out_triangles();

    const int euler = static_cast<int>(out_points.rows()) - static_cast<int>(out_edges.rows()) +
                      static_cast<int>(out_triangles.rows());
    REQUIRE(euler == 0);
}

TEST_CASE("Marker", "[trianglelite][marker]")
{
    using namespace trianglelite;

    Config config;
    Engine engine;

    std::vector<Scalar> points{0, 0, 1, 0, 0, 1};
    std::vector<Index> segments{0, 1, 1, 2, 2, 0};
    std::vector<Index> point_markers{4, 5, 6};
    std::vector<Index> segment_markers{1, 2, 3};
    engine.set_in_points(points.data(), static_cast<int>(points.size() / 2));
    engine.set_in_segments(segments.data(), static_cast<int>(segments.size() / 2));
    engine.set_in_point_markers(point_markers.data(), static_cast<int>(point_markers.size()));
    engine.set_in_segment_markers(segment_markers.data(), static_cast<int>(segment_markers.size()));

    config.max_area = 0.1;
    config.verbose_level = 0;

    engine.run(config);

    auto out_points = engine.get_out_points();
    auto out_edges = engine.get_out_edges();
    auto out_segments = engine.get_out_segments();
    auto out_triangles = engine.get_out_triangles();

    REQUIRE(out_points.rows() > 3);
    REQUIRE(out_edges.rows() > 3);
    REQUIRE(out_segments.rows() > 3);
    REQUIRE(out_triangles.rows() > 1);

    SECTION("Point markers")
    {
        auto out_point_markers = engine.get_out_point_markers();
        const size_t num_out_point_markers = out_point_markers.size();
        REQUIRE(num_out_point_markers == out_points.rows());
        for (size_t i = 0; i < num_out_point_markers; i++) {
            if (out_point_markers[i] == 4) {
                REQUIRE_THAT(out_points(i, 0), Catch::Matchers::WithinAbs(0, 1e-6));
                REQUIRE_THAT(out_points(i, 1), Catch::Matchers::WithinAbs(0, 1e-6));
            } else if (out_point_markers[i] == 5) {
                REQUIRE_THAT(out_points(i, 0), Catch::Matchers::WithinAbs(1, 1e-6));
                REQUIRE_THAT(out_points(i, 1), Catch::Matchers::WithinAbs(0, 1e-6));
            } else if (out_point_markers[i] == 6) {
                REQUIRE_THAT(out_points(i, 0), Catch::Matchers::WithinAbs(0, 1e-6));
                REQUIRE_THAT(out_points(i, 1), Catch::Matchers::WithinAbs(1, 1e-6));
            }
        }
    }

    SECTION("Edge markers")
    {
        auto edge_markers = engine.get_out_edge_markers();
        REQUIRE(edge_markers.size() == out_edges.rows());

        const size_t num_edges = out_edges.rows();
        for (size_t i = 0; i < num_edges; i++) {
            const Index v0 = out_edges(i, 0);
            const Index v1 = out_edges(i, 1);
            if (edge_markers[i] == 1) {
                REQUIRE_THAT(out_points(v0, 1), Catch::Matchers::WithinAbs(0, 1e-6));
                REQUIRE_THAT(out_points(v1, 1), Catch::Matchers::WithinAbs(0, 1e-6));
            } else if (edge_markers[i] == 2) {
                REQUIRE_THAT(out_points.row(v0).sum(), Catch::Matchers::WithinAbs(1.0, 1e-6));
                REQUIRE_THAT(out_points.row(v1).sum(), Catch::Matchers::WithinAbs(1.0, 1e-6));
            } else if (edge_markers[i] == 3) {
                REQUIRE_THAT(out_points(v0, 0), Catch::Matchers::WithinAbs(0, 1e-6));
                REQUIRE_THAT(out_points(v1, 0), Catch::Matchers::WithinAbs(0, 1e-6));
            } else {
                REQUIRE(edge_markers[i] == 0);
            }
        }
    }

    SECTION("Segment markers")
    {
        auto segment_markers = engine.get_out_segment_markers();
        REQUIRE(out_segments.rows() == segment_markers.size());
    }
}

TEST_CASE("Debug Issue 1", "[trianglelite][issue][!mayfail]")
{
    using namespace trianglelite;

    std::vector<Scalar> points{6.899643741648033,
        10.556739733611963,
        6.8743893086546723,
        10.577559204153792,
        6.8990280198173055,
        10.557055643048765,
        6.8994694525740767,
        10.556817827203695,
        6.8996252478389311,
        10.556759464230709,
        6.899537086138448,
        10.5566972172105};
    std::vector<int> segments{0, 1, 2, 3, 3, 4, 4, 3};

    Engine engine;
    Config config;
    config.convex_hull = true;
    config.split_boundary = false;
    config.max_num_steiner = 0;
    engine.set_in_points(points.data(), 6);
    engine.set_in_segments(segments.data(), 4);
    REQUIRE_THROWS(engine.run(config));
}


TEST_CASE("Debug Issue 2", "[trianglelite][issue]")
{
    std::vector<trianglelite::Scalar> points = {1, 0, 0, 1, 0, 0, 0.6, 0.6};
    std::vector<int> segments = {0, 1, 1, 2, 2, 0};

    trianglelite::Engine engine;
    engine.set_in_points(points.data(), points.size() / 2);

    SECTION("Points only")
    {
        trianglelite::Config config;
        config.exact = false;
        config.verbose_level = 0;

        engine.run(config);

        const auto out_points = engine.get_out_points();
        const auto out_triangles = engine.get_out_triangles();

        REQUIRE(out_points.rows() == 4);
        REQUIRE(out_triangles.rows() == 2);
    }

    SECTION("Points and segments")
    {
        engine.set_in_segments(segments.data(), segments.size() / 2);
        trianglelite::Config config;
        config.exact = false;
        config.verbose_level = 0;
        config.convex_hull = true;
        config.split_boundary = false;
        config.max_num_steiner = 0;

        engine.run(config);

        const auto out_points = engine.get_out_points();
        const auto out_triangles = engine.get_out_triangles();

        REQUIRE(out_points.rows() == 4);
        REQUIRE(out_triangles.rows() == 2);
    }
}
