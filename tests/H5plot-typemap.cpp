/*
 * Copyright (c) 2018 vargaconsulting, Toronto,ON Canada
 * Author: Varga, Steven <steven@vargaconsulting.ca>
 */
#include "utils/abstract.hpp"
#include "plot/all"
#include <cstdlib>
#include <random>
#include <fmt/core.h>
#include <fmt/format.h>
#include "../h5cpp/H5meta.hpp"

namespace ns = h5::test;

template<class T>
using type_class = std::tuple< // when changing content, update the `y_axis`
	std::is_trivial<T>, std::is_standard_layout<T>, std::is_pod<T>, std::is_empty<T>, std::is_aggregate<T>, ns::separator_t<T>,
	h5::impl::is_scalar<T>, h5::impl::is_vector<T>, h5::impl::is_matrix<T>, h5::impl::is_cube<T>, ns::separator_t<T>,
	h5::impl::is_stl<T>, h5::impl::is_array<T>, h5::impl::is_contiguous<T>
    >;
std::vector<std::string> y_axis = { // must match `type_class<T>`
	"std::is_compound","std::is_trivial","std::is_standard_layout", "std::is_pod", "std::is_empty", "std::is_aggregate", "",
	"is_scalar","is_vector","is_matrix","is_cube", "", "is_stl", "is_array", "is_contiguous"
};

template <typename Type> struct IntegralTest : public ::testing::Test {};
typedef ::testing::Types<float,std::string, ns::pod_t> PrimitiveTypes;
TYPED_TEST_SUITE(IntegralTest, PrimitiveTypes, ns::element_names_t);
TYPED_TEST(IntegralTest,create_current_dims) {
    using all_containers =  ns::all_t<TypeParam>;
    std::vector<std::string> x_axis = h5::test::names<all_containers>();
    plot::color::fg palette{0x6b8e23, 0xff4500, 0xd8d8d8};
    std::vector<plot::data::point_t<unsigned>> data;

    h5::meta::static_for<all_containers>( [&]( auto y ){
        using Ty = typename std::tuple_element<y, all_containers>::type;
        h5::meta::static_for<type_class<Ty>>( [=, &data]( auto x ){
            using Tx = typename std::tuple_element<x, type_class<Ty>>::type;
            if( ns::is_separator<Tx>::value || ns::is_separator<Ty>::value )
                data.push_back({x,y, 2});
            else
                data.push_back({x,y, !Tx::value});
        });
    });

    //M := outcome,  x_axis := pretty names of containers, y_axis := elementary_types 
    using namespace plot;
	std::string type_name(h5::name<TypeParam>::value);
    heatmap("pix/type_templates-test-" + type_name + ".svg", data, palette,
		axis::x{y_axis, rotate::ccw{45}, position{10, 80} }, axis::y{x_axis, palette[1]},
		title{type_name, position{align::left, 15}},
		legend{
			text{"yes", "no", "n/a"},
			position{align::left, align::bottom}, palette, marker::rect
		});
}

using element_t = std::tuple<
	unsigned char, unsigned short, unsigned int, unsigned long long int,
	char, short, int, long long int, float, double, std::string>;

template <class T> using linalg_t = ns::all_t<T>;

TEST(Eigen3, element_type) {

    plot::color::fg palette{0x6b8e23, 0xff4500, 0xd8d8d8};
    std::vector<plot::data::point_t<unsigned>> data;
    std::vector<std::string> x_axis = h5::names<element_t>();
    std::vector<std::string> y_axis = ns::names<linalg_t<int>>();
    h5::meta::static_for<element_t>( [&]( auto y ){
        using Ty = typename std::tuple_element<y, element_t>::type;
		h5::meta::static_for<linalg_t<Ty>>( [=, &data]( auto x ){
            using Tx = typename std::tuple_element<x, linalg_t<Ty>>::type;
        });
    });

    using namespace plot;
    heatmap("pix/linalg.svg", data, palette,
		axis::x{y_axis, rotate::ccw{45}, position{10, 80} }, axis::y{x_axis, palette[1]},
		title{"title", position{align::left, 15}},
		legend{
			text{"yes", "???", "n/a"},
			position{align::left, align::bottom}, palette, marker::rect
		});
}

TEST(string, element_type) {

    plot::color::fg palette{ 0xd8d8d8, 0x6b8e23,   0xd8d8d8};
    std::vector<std::string> x_axis;
    std::vector<std::string> y_axis = {
        "compact layout", "contiguous layout", "chunked layout", "",
        "extendable data space", "",
        "fixed length type", "variable length type", "",
        "scalar", "array", "hyper cube","",
        "h5::current_dims{n,..}", "h5::max_dims{m,..}","h5::offset{..}","h5::chunk{..}", "h5::gzip{9}"
    };
    arma::Mat<unsigned> M(0,18);
    using row_t = std::pair<arma::Row<unsigned int>, std::string>;
    std::vector<row_t> rows = {
        {{1,1,0,0, 0,0,  0,1,0,  1,0,0,0,  0,0,0,0,0}, " 1 std::string"},
        {{1,1,0,0, 0,0,  0,1,0,  0,1,0,0,  0,0,0,0,0}, " 3 std::array<std::string,N>"},
        {{1,1,0,0, 0,0,  0,1,0,  0,1,0,0,  1,1,0,0,0}, " 3 std::array<std::string,N>"},
        {{1,0,1,0, 1,0,  0,1,0,  0,0,1,0,  0,1,0,0,0}, " 2 std::vector<std::string>"},
        {{1,1,0,0, 0,0,  0,1,0,  0,1,0,0,  0,0,0,0,0}, " 4 std::initializer_list<std::string>"},
        {{1,1,0,0, 1,0,  0,0,1,  0,1,0,0,  1,1,1,0,0}, " 5 std::array<std::string,N> var"},
        {{1,1,0,0, 1,0,  0,0,1,  0,1,0,0,  1,1,1,0,0}, "std::vector<std::string> "},

        {{0,0,0,0, 0,0,  0,0,0,  0,0,0,0,  0,0,0,0,0}, ""},
        {{1,1,0,0, 0,0,  1,0,0,  1,0,0,0,  0,0,0,0,0}, "char[N]"},
        {{1,1,0,0, 0,0,  1,0,0,  1,0,0,0,  0,0,0,0,0}, "char[][M][N]"},
        {{1,1,0,0, 0,0,  1,0,0,  1,0,0,0,  0,0,0,0,0}, "char[][N]"},
        {{1,1,0,0, 0,0,  1,0,0,  1,0,0,0,  0,0,0,0,0}, "character literal"},
        {{1,0,1,0, 1,0,  1,0,0,  1,0,0,0,  1,1,0,1,0}, "char[N]"},

        {{0,0,0,0, 0,0,  0,0,0,  0,0,0,0,  0,0,0,0,0}, ""},
        {{1,0,1,0, 1,0,  1,0,0,  0,0,1,0,  0,1,0,0,0}, "std::vector<short> var"},
        {{1,0,1,0, 0,0,  1,0,0,  0,0,1,0,  0,0,0,1,0}, "std::vector<pod_t> var"},
        {{1,0,1,0, 0,0,  1,0,0,  0,0,1,0,  1,0,0,1,1}, "std::vector<pod_t> var"},
        {{1,0,1,0, 0,0,  1,0,0,  0,0,1,0,  1,1,0,1,1}, "std::vector<pod_t> var"},
        {{1,1,0,0, 0,0,  1,0,0,  0,1,0,0,  0,0,0,0,0}, "std::array<pod_t,M> var[N]"},
        {{1,1,0,0, 0,0,  1,0,0,  0,1,0,0,  0,0,0,0,0}, "std::array<pod_t[M],N> var"},
        {{1,1,0,0, 0,0,  1,0,0,  0,1,0,0,  0,0,0,0,0}, "std::array<std::array<h5::test::pod_t,M>,N> var"},
        {{1,1,0,0, 0,0,  1,0,0,  0,1,0,0,  0,0,0,0,0}, "std::array<int,N> data"},
        {{1,1,0,0, 0,0,  1,0,0,  0,1,0,0,  0,0,0,0,0}, "std::initializer_list<T>"},
        {{1,0,1,0, 1,0,  1,0,0,  0,0,1,0,  1,1,0,1,0}, "std::vector<T>"}
    };
    for( auto& r:rows )  M.insert_rows(M.n_rows, r.first), x_axis.push_back(r.second);



  //  arma::Mat<unsigned> M = m.t();
    using namespace plot;
    heatmap("pix/string.svg", M, palette,
		axis::x{y_axis, rotate::ccw{45}, position{10, 80} }, axis::y{x_axis},
		title{"C++ Type to HDF5 storage", position{align::left, 15}},
		legend {
			text{"n/a","yes"},
			position{align::left, align::bottom}, palette, marker::rect
		});
}



/*----------- BEGIN TEST RUNNER ---------------*/
H5CPP_BASIC_RUNNER( int argc, char**  argv );
/*----------------- END -----------------------*/

