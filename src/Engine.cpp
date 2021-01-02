#include <trianglelite/Engine.h>
#ifdef WITH_MSHIO
#include <MshIO/mshio.h>
#endif

#include <array>
#include <exception>
#include <iostream>
#include <numeric>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#ifdef TRIANGLELITE_SINGLE
#define REAL float
#else
#define REAL double
#endif
#define VOID void
#define ANSI_DECLARATORS
static_assert(
    std::is_same<REAL, trianglelite::Scalar>::value, "Inconsistent Scalar types detected.");

extern "C" {
#include <triangle.h>
}

using namespace trianglelite;

namespace {

void clear_triangulateio(triangulateio& io)
{
    // Points.
    if (io.pointlist != nullptr) {
        delete[] io.pointlist;
        io.pointlist = nullptr;
    }
    if (io.pointmarkerlist != nullptr) {
        delete[] io.pointmarkerlist;
        io.pointmarkerlist = nullptr;
    }
    if (io.pointattributelist != nullptr) {
        delete[] io.pointattributelist;
        io.pointattributelist = nullptr;
    }
    io.numberofpoints = 0;
    io.numberofpointattributes = 0;

    // Triangles.
    if (io.trianglelist != nullptr) {
        delete[] io.trianglelist;
        io.trianglelist = nullptr;
    }
    if (io.trianglearealist != nullptr) {
        delete[] io.trianglearealist;
        io.trianglearealist = nullptr;
    }
    if (io.triangleattributelist != nullptr) {
        delete[] io.triangleattributelist;
        io.triangleattributelist = nullptr;
    }
    if (io.neighborlist != nullptr) {
        delete[] io.neighborlist;
        io.neighborlist = nullptr;
    }
    io.numberoftriangles = 0;
    io.numberoftriangleattributes = 0;
    io.numberofcorners = 0;

    // Segments.
    if (io.segmentlist != nullptr) {
        delete[] io.segmentlist;
        io.segmentlist = nullptr;
    }
    if (io.segmentmarkerlist != nullptr) {
        delete[] io.segmentmarkerlist;
        io.segmentmarkerlist = nullptr;
    }
    io.numberofsegments = 0;

    // Edges.
    if (io.edgelist != nullptr) {
        delete[] io.edgelist;
        io.edgelist = nullptr;
    }
    if (io.edgemarkerlist != nullptr) {
        delete[] io.edgemarkerlist;
        io.edgemarkerlist = nullptr;
    }
    io.numberofedges = 0;

    // Misc.
    // Note that holes and regions in the output are mirrored from the input.
    // No memory to de-allocate.
    // if (io.holelist != nullptr) {
    //    delete[] io.holelist;
    //    io.holelist = nullptr;
    //}
    // io.numberofholes = 0;
    // if (io.regionlist != nullptr) {
    //    delete[] io.regionlist;
    //    io.regionlist = nullptr;
    //}
    // io.numberofregions = 0;
    if (io.normlist != nullptr) {
        delete[] io.normlist;
        io.normlist = nullptr;
    }
}

void initialize_triangulateio(triangulateio& io)
{
    io.numberofpoints = 0;
    io.numberofpointattributes = 0;
    io.numberoftriangles = 0;
    io.numberoftriangleattributes = 0;
    io.numberofcorners = 0;
    io.numberofsegments = 0;
    io.numberofholes = 0;
    io.numberofregions = 0;
    io.numberofedges = 0;
    io.pointlist = nullptr;
    io.pointattributelist = nullptr;
    io.pointmarkerlist = nullptr;
    io.trianglelist = nullptr;
    io.triangleattributelist = nullptr;
    io.trianglearealist = nullptr;
    io.neighborlist = nullptr;
    io.segmentlist = nullptr;
    io.segmentmarkerlist = nullptr;
    io.edgelist = nullptr;
    io.edgemarkerlist = nullptr;
    io.holelist = nullptr;
    io.regionlist = nullptr;
    io.normlist = nullptr;
}

std::string generate_command_line_options(const triangulateio& io, const Config& config)
{
    // Basic flag:
    //   z: index starts from zero.
    //   n: output triangle neighbor info.
    //   e: output edge list.
    std::string opt = "zne";

    if (io.numberofpoints == 0) {
        throw std::runtime_error("Empty input detected for triangulation");
    } else if (io.numberofsegments > 0) {
        opt += "p"; // Triangulate PSLG.
    } else if (io.numberoftriangles > 0) {
        opt += "r"; // Refinement.
    } else {
        opt += "v"; // Also compute Voronoi diagram.
    }

    if (config.min_angle > 0) {
        if (config.min_angle > 34) {
            std::cerr << "Warning: min angle > 34 degrees may cause algorithm to not terminate"
                      << std::endl;
        } else if (config.min_angle > 20.7f) {
            std::cerr << "Warning: Theoretical guarentee of termination is lost "
                      << "for min angle > 20.7 degrees.  "
                      << "But in practice, it often succeed for min angle >= 33 degrees."
                      << std::endl;
        }
        opt += "q" + std::to_string(config.min_angle);
    } else if (config.min_angle < 0.0) {
        std::cerr << "Warning: min angle < 0 degrees.  Ignored." << std::endl;
    }

    if (config.max_area > 0) {
        opt += "a" + std::to_string(config.max_area);
    } else if (io.trianglearealist != nullptr) {
        opt += "a";
    }
    if (config.convex_hull) {
        opt += "c";
    }
    if (config.conforming) {
        opt += "D";
    }
    if (!config.exact) {
        opt += "X";
    }
    if (!config.split_boundary) {
        opt += "Y";
    }
    if (config.max_num_steiner >= 0) {
        opt += "S" + std::to_string(config.max_num_steiner);
    }
    switch (config.verbose_level) {
    case 0: opt += "Q"; break;
    case 1: break;
    case 2: opt += "V"; break;
    case 3: opt += "VV"; break;
    case 4: opt += "VVVV"; break;
    default:
        throw std::runtime_error("Unknown verbose level: " + std::to_string(config.verbose_level));
    }
    switch (config.algorithm) {
    case Algorithm::DIVIDE_AND_CONQUER: break;
    case Algorithm::SWEEPLINE: opt += "F"; break;
    case Algorithm::INCREMENTAL: opt += "i"; break;
    default: throw std::runtime_error("Unknown triangulation algorithm");
    }
    return opt;
}

#ifdef WITH_MSHIO
void debug_save(const std::string& filename,
    const trianglelite::Engine& engine,
    const std::vector<int>& region_ids)
{
    auto vertices2D = engine.get_out_points();
    auto points = engine.get_out_points();
    auto triangles = engine.get_out_triangles();

    const int num_points = static_cast<int>(points.rows());
    const int num_triangles = static_cast<int>(triangles.rows());

    mshio::MshSpec msh_spec;
    msh_spec.nodes.num_entity_blocks = 1;
    msh_spec.nodes.num_nodes = num_points;
    msh_spec.nodes.min_node_tag = 1;
    msh_spec.nodes.max_node_tag = num_points;
    msh_spec.nodes.entity_blocks.emplace_back();

    auto& node_block = msh_spec.nodes.entity_blocks.back();
    node_block.entity_dim = 2;
    node_block.entity_tag = 1;
    node_block.parametric = 0;
    node_block.num_nodes_in_block = num_points;
    node_block.tags.resize(num_points);
    std::iota(node_block.tags.begin(), node_block.tags.end(), 1);
    node_block.data.resize(num_points * 3);
    for (int i = 0; i < num_points; i++) {
        node_block.data[i * 3] = points(i, 0);
        node_block.data[i * 3 + 1] = points(i, 1);
        node_block.data[i * 3 + 2] = 0;
    }

    msh_spec.elements.num_entity_blocks = 1;
    msh_spec.elements.num_elements = num_triangles;
    msh_spec.elements.min_element_tag = 1;
    msh_spec.elements.max_element_tag = num_triangles;
    msh_spec.elements.entity_blocks.emplace_back();

    auto& element_block = msh_spec.elements.entity_blocks.back();
    element_block.entity_dim = 2;
    element_block.entity_tag = 1;
    element_block.element_type = 2;
    element_block.num_elements_in_block = num_triangles;
    element_block.data.resize(num_triangles * 4);
    for (int i = 0; i < num_triangles; i++) {
        element_block.data[i * 4] = i + 1;
        element_block.data[i * 4 + 1] = triangles(i, 0) + 1;
        element_block.data[i * 4 + 2] = triangles(i, 1) + 1;
        element_block.data[i * 4 + 3] = triangles(i, 2) + 1;
    }

    msh_spec.element_data.emplace_back();
    auto& element_data = msh_spec.element_data.back();
    element_data.header.string_tags.push_back("region_id");
    element_data.header.real_tags.push_back(0);
    element_data.header.int_tags = {0, 1, num_triangles, 0};
    element_data.entries.resize(num_triangles);
    for (int i=0; i<num_triangles; i++) {
        element_data.entries[i].tag = i+1;
        element_data.entries[i].data.push_back(region_ids[i]);
    }

    mshio::save_msh(filename, msh_spec);
};
#endif

} // namespace

Engine::Engine()
{
    m_in = std::make_unique<triangulateio>();
    m_out = std::make_unique<triangulateio>();
    m_vorout = std::make_unique<triangulateio>();
    initialize_triangulateio(*m_in);
    initialize_triangulateio(*m_out);
    initialize_triangulateio(*m_vorout);
}

Engine::~Engine()
{
    // m_in's memory is mamanged by user.
    // However, m_out and m_vorout need to be cleaned.
    clear_triangulateio(*m_out);
    clear_triangulateio(*m_vorout);
}

void Engine::set_in_points(const Scalar* points, Index num_points)
{
    m_in->numberofpoints = num_points;
    m_in->pointlist =
        const_cast<Scalar*>(points); // TODO: ensure const_cast does not cause trouble.
}

Matrix2FrMap Engine::get_in_points()
{
    return Matrix2FrMap(m_in->pointlist, m_in->numberofpoints, 2);
}

void Engine::unset_in_points()
{
    m_in->numberofpoints = 0;
    m_in->pointlist = nullptr;
}

void Engine::set_in_segments(const Index* segments, Index num_segments)
{
    m_in->numberofsegments = num_segments;
    m_in->segmentlist = const_cast<Index*>(segments); // TODO: check cosnt_cast.
}

Matrix2IrMap Engine::get_in_segments()
{
    return Matrix2IrMap(m_in->segmentlist, m_in->numberofsegments, 2);
}

void Engine::unset_in_segments()
{
    m_in->numberofsegments = 0;
    m_in->segmentlist = nullptr;
}

void Engine::set_in_triangles(const Index* triangles, Index num_triangles)
{
    m_in->numberoftriangles = num_triangles;
    m_in->trianglelist = const_cast<Index*>(triangles);
    m_in->numberofcorners = 3;
}

Matrix3IrMap Engine::get_in_triangles()
{
    return Matrix3IrMap(m_in->trianglelist, m_in->numberoftriangles, 3);
}

void Engine::unset_in_triangles()
{
    m_in->numberoftriangles = 0;
    m_in->trianglelist = nullptr;
    m_in->numberofcorners = 0;
}

void Engine::set_in_holes(const Scalar* holes, Index num_holes)
{
    m_in->numberofholes = num_holes;
    m_in->holelist = const_cast<Scalar*>(holes);
}

Matrix2FrMap Engine::get_in_holes()
{
    return Matrix2FrMap(m_in->holelist, m_in->numberofholes, 2);
}

void Engine::unset_in_holes()
{
    m_in->numberofholes = 0;
    m_in->holelist = nullptr;
}

void Engine::set_in_areas(const Scalar* areas, Index num_areas)
{
    if (m_in->numberoftriangles != 0) {
        assert(m_in->numberoftriangles == num_areas);
    }
    m_in->trianglearealist = const_cast<Scalar*>(areas);
}

Matrix1FMap Engine::get_in_areas()
{
    return Matrix1FMap(m_in->trianglearealist, m_in->numberoftriangles);
}

void Engine::unset_in_areas()
{
    m_in->trianglearealist = nullptr;
}

void Engine::set_in_point_markers(const int* markers, Index num_markers)
{
    if (m_in->numberofpoints != 0) {
        assert(num_markers == m_in->numberofpoints);
    }
    m_in->pointmarkerlist = const_cast<int*>(markers);
}

Matrix1IMap Engine::get_in_point_markers()
{
    return Matrix1IMap(m_in->pointmarkerlist, m_in->numberofpoints);
}

void Engine::unset_in_point_markers()
{
    m_in->pointmarkerlist = nullptr;
}

void Engine::set_in_segment_markers(const int* markers, Index num_markers)
{
    if (m_in->numberofsegments != 0) {
        assert(num_markers == m_in->numberofsegments);
    }
    m_in->segmentmarkerlist = const_cast<int*>(markers);
}

Matrix1IMap Engine::get_in_segment_markers()
{
    return Matrix1IMap(m_in->segmentmarkerlist, m_in->numberofsegments);
}

void Engine::unset_in_segment_markers()
{
    m_in->segmentmarkerlist = nullptr;
}

const Matrix2FrMap Engine::get_out_points() const
{
    return Matrix2FrMap(m_out->pointlist, m_out->numberofpoints, 2);
}

const Matrix3IrMap Engine::get_out_triangles() const
{
    return Matrix3IrMap(m_out->trianglelist, m_out->numberoftriangles, 3);
}

const Matrix2IrMap Engine::get_out_segments() const
{
    return Matrix2IrMap(m_out->segmentlist, m_out->numberofsegments, 2);
}

const Matrix2IrMap Engine::get_out_edges() const
{
    return Matrix2IrMap(m_out->edgelist, m_out->numberofedges, 2);
}

const Matrix3IrMap Engine::get_out_triangle_neighbors() const
{
    return Matrix3IrMap(m_out->neighborlist, m_out->numberoftriangles, 3);
}

const Matrix1IMap Engine::get_out_point_markers() const
{
    return Matrix1IMap(m_out->pointmarkerlist, m_out->numberofpoints);
}

const Matrix1IMap Engine::get_out_segment_markers() const
{
    return Matrix1IMap(m_out->segmentmarkerlist, m_out->numberofsegments);
}

const Matrix1IMap Engine::get_out_edge_markers() const
{
    return Matrix1IMap(m_out->edgemarkerlist, m_out->numberofedges);
}

void Engine::run(const Config& config)
{
    std::vector<Scalar> holes;
    if (config.auto_hole_detection) {
        holes = run_auto_hole_detection();
        set_in_holes(holes.data(), holes.size() / 2);
    }

    // Cleanup to ensure repeated call does not leak memory.
    clear_triangulateio(*m_out);
    clear_triangulateio(*m_vorout);

    const auto opt = generate_command_line_options(*m_in, config);
    triangulate(const_cast<char*>(opt.c_str()), m_in.get(), m_out.get(), m_vorout.get());

    if (config.auto_hole_detection) {
        unset_in_holes();
    }
}

std::vector<Scalar> Engine::run_auto_hole_detection()
{
    using Point = Eigen::Matrix<Scalar, 2, 1>;

    // Constrained Delaunay and preserving segments.
    Config config;
    config.split_boundary = false;
    config.convex_hull = true;
    config.auto_hole_detection = false; // To avoid recursion.
    config.verbose_level = 0;

    std::vector<int> seg_markers(m_in->numberofsegments, 1);
    set_in_segment_markers(seg_markers.data(), m_in->numberofsegments);
    run(config);
    unset_in_segment_markers();

    // Extract result.
    auto points = get_out_points();
    auto triangles = get_out_triangles();
    auto edges = get_out_edges();
    auto edge_markers = get_out_edge_markers();
    auto neighbors = get_out_triangle_neighbors();

    // Initialize states.
    const int num_triangles = static_cast<int>(triangles.rows());
    std::vector<std::vector<int>> regions;
    regions.reserve(num_triangles / 2);
    std::vector<bool> visited(num_triangles, false);

    // Initialize seg_edge_set to check if an edge comes from an input segment.
    const int num_edges = static_cast<int>(edges.rows());
    std::set<std::array<int, 2>> seg_edge_set;
    assert(edge_markers.size() == num_edges);
    for (int i = 0; i < num_edges; i++) {
        if (edge_markers[i] != 0) {
            // This edge maps to an input boundary.
            if (edges(i, 0) < edges(i, 1)) {
                seg_edge_set.insert({edges(i, 0), edges(i, 1)});
            } else {
                seg_edge_set.insert({edges(i, 1), edges(i, 0)});
            }
        }
    }

    // Extract the shared edge between 2 triangles.
    auto shared_edge = [&](int ti, int tj) -> std::array<int, 2> {
        for (int i = 0; i < 3; i++) {
            bool found = true;
            for (int j = 0; j < 3; j++) {
                if (triangles(ti, i) == triangles(tj, j)) {
                    found = false;
                    break;
                }
            }
            if (found) {
                int v0 = triangles(ti, (i + 1) % 3);
                int v1 = triangles(ti, (i + 2) % 3);
                if (v0 < v1) {
                    return {v0, v1};
                } else {
                    return {v1, v0};
                }
            }
        }
        throw std::runtime_error("Triangles are not adjcent!");
    };

    // Flood fill from a seed triangle.
    std::function<void(int, std::vector<int>&)> flood_region;
    flood_region = [&](int seed_tri, std::vector<int>& region) {
        region.push_back(seed_tri);
        visited[seed_tri] = true;

        for (int i = 0; i < 3; i++) {
            const auto t = neighbors(seed_tri, i);
            if (t < 0) continue; // No neighbor, boundary triangle.
            if (visited[t]) continue;

            const auto e = shared_edge(seed_tri, t);
            if (seg_edge_set.find(e) == seg_edge_set.end()) {
                flood_region(t, region);
            }
        }
    };

    // Clsoed form of 2D signed triangle area (x2) from vertex coordinates.
    auto compute_area = [](const Point& v0, const Point& v1, const Point& v2) {
        return v2[1] * v1[0] + v1[1] * v0[0] + v0[1] * v2[0] - v2[0] * v1[1] - v1[0] * v0[1] -
               v0[0] * v2[1];
    };

    // Compute signed angle formed by triangle (v0, v1, v2) at v0.
    auto compute_angle = [&](const Point& v0, const Point& v1, const Point& v2) {
        return std::atan2(compute_area(v0, v1, v2), (v1 - v0).dot(v2 - v0));
    };

    // Compute winding number of a point with respect to the input segments.
    auto compute_winding_number = [&](const Point& p) {
        const auto in_points = get_in_points();
        const auto in_segments = get_in_segments();
        const int num_in_segments = static_cast<int>(in_segments.rows());
        Scalar theta = 0;
        for (int i = 0; i < num_in_segments; i++) {
            const Point v0 = in_points.row(in_segments(i, 0));
            const Point v1 = in_points.row(in_segments(i, 1));
            theta += compute_angle(p, v0, v1);
        }

        return theta / (2 * M_PI);
    };

    // Compute regions by flood fill.
    for (int i = 0; i < num_triangles; i++) {
        if (visited[i]) continue;

        regions.emplace_back();
        regions.back().reserve(num_triangles);
        flood_region(i, regions.back());
    }

#ifdef WITH_MSHIO
    {
        std::vector<int> region_ids(num_triangles, 0);
        int region_count = 0;
        for (auto& region : regions) {
            for (auto fid : region) {
                region_ids[fid] = region_count + 1;
            }
            region_count++;
        }
        debug_save("auto_hole_detection_debug.msh", *this, region_ids);
    }
#endif

    // Extract hole points from regions.
    const int num_regions = static_cast<int>(regions.size());
    std::vector<Scalar> holes;
    holes.reserve(num_regions * 2);
    Point v0, v1, v2, center;
    for (int i = 0; i < num_regions; i++) {
        Scalar max_area = 0;
        for (auto fid : regions[i]) {
            v0 = points.row(triangles(fid, 0));
            v1 = points.row(triangles(fid, 1));
            v2 = points.row(triangles(fid, 2));
            const auto area = compute_area(v0, v1, v2);

            if (area > max_area) {
                max_area = area;
                center = (v0 + v1 + v2) / 3;
            }
        }

        const auto winding_number = compute_winding_number(center);
        if (winding_number < 0.5f) {
            holes.push_back(center[0]);
            holes.push_back(center[1]);
        }
    }

    return holes;
}

