#include <trianglelite/Engine.h>

#include <exception>
#include <iostream>
#include <numeric>
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
    if (io.pointlist != nullptr) delete[] io.pointlist;
    if (io.pointmarkerlist != nullptr) delete[] io.pointmarkerlist;
    if (io.pointattributelist != nullptr) delete[] io.pointattributelist;

    // Triangles.
    if (io.trianglelist != nullptr) delete[] io.trianglelist;
    if (io.trianglearealist != nullptr) delete[] io.trianglearealist;
    if (io.triangleattributelist != nullptr) delete[] io.triangleattributelist;
    if (io.neighborlist != nullptr) delete[] io.neighborlist;

    // Segments.
    if (io.segmentlist != nullptr) delete[] io.segmentlist;
    if (io.segmentmarkerlist != nullptr) delete[] io.segmentmarkerlist;

    // Edges.
    if (io.edgelist != nullptr) delete[] io.edgelist;
    if (io.edgemarkerlist != nullptr) delete[] io.edgemarkerlist;

    // Misc.
    if (io.holelist != nullptr) delete[] io.holelist;
    if (io.regionlist != nullptr) delete[] io.regionlist;
    if (io.normlist != nullptr) delete[] io.normlist;
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

void Engine::set_in_segments(const Index* segments, Index num_segments)
{
    m_in->numberofsegments = num_segments;
    m_in->segmentlist = const_cast<Index*>(segments); // TODO: check cosnt_cast.
}

Matrix2IrMap Engine::get_in_segments()
{
    return Matrix2IrMap(m_in->segmentlist, m_in->numberofsegments, 2);
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

void Engine::set_in_holes(const Scalar* holes, Index num_holes)
{
    m_in->numberofholes = num_holes;
    m_in->holelist = const_cast<Scalar*>(holes);
}

Matrix2FrMap Engine::get_in_holes()
{
    return Matrix2FrMap(m_in->holelist, m_in->numberofholes, 2);
}

void Engine::set_in_areas(const Scalar* areas, Index num_areas)
{
    if (m_in->numberoftriangles != 0) {
        assert(m_in->numberoftriangles == num_areas);
    }
    m_in->trianglearealist = const_cast<Scalar*>(areas);
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

const Matrix1IMap Engine::get_out_parent_segments() const
{
    return Matrix1IMap(m_out->edgemarkerlist, m_out->numberofedges, 1);
}

void Engine::run(const Config& config)
{
    const auto opt = generate_command_line_options(*m_in, config);

    // Set segment marker;
    std::vector<int> segment_markers(m_in->numberofsegments);
    std::iota(segment_markers.begin(), segment_markers.end(), 0);
    m_in->segmentmarkerlist = segment_markers.data();

    triangulate(const_cast<char*>(opt.c_str()), m_in.get(), m_out.get(), m_vorout.get());

    m_in->segmentmarkerlist = nullptr;
}

