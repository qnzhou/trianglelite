#include <catch2/catch.hpp>

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
    engine.set_in_points(points.data(), points.rows());
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
    engine.set_in_points(points.data(), points.rows());

    Eigen::Matrix<int, 4, 2, Eigen::RowMajor> segments;
    segments << 0, 1, 1, 2, 2, 3, 3, 0;
    engine.set_in_segments(segments.data(), segments.rows());

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
    engine.set_in_points(points.data(), points.rows());

    Eigen::Matrix<int, 8, 2, Eigen::RowMajor> segments;
    segments << 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4;
    engine.set_in_segments(segments.data(), segments.rows());

    engine.run(config);

    auto out_points = engine.get_out_points();
    auto out_edges = engine.get_out_edges();
    auto out_triangles = engine.get_out_triangles();

    const int euler = static_cast<int>(out_points.rows()) - static_cast<int>(out_edges.rows()) +
                      static_cast<int>(out_triangles.rows());
    REQUIRE(euler == 0);
}

