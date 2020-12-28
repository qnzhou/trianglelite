#include <catch2/catch.hpp>

#include <trianglelite/trianglelite.h>

#include <Eigen/Core>

TEST_CASE("Simple", "[trianglelite]") {
    using namespace trianglelite;

    Config config;
    Engine engine;

    config.convex_hull = true;
    config.verbose_level = 0;

    Eigen::Matrix<Scalar, 3, 2, Eigen::RowMajor> points;
    points << 0.0, 0.0,
              1.0, 0.0,
              0.0, 1.0;
    engine.set_in_points(points.data(), points.rows());
    engine.run(config);

    auto out_points = engine.get_out_points();
    REQUIRE(out_points.rows() >= 3);
}
